#!/usr/bin/python2.5
#
# Copyright 2013 Emilie Gillet.
# 
# Author: Emilie Gillet (emilie.o.gillet@gmail.com)
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# 
# See http://creativecommons.org/licenses/MIT/ for more information.
#
# -----------------------------------------------------------------------------
#
# Fsk encoder for converting firmware .bin files into .

import logging
import numpy
import optparse
import zlib

from stm_audio_bootloader import audio_stream_writer


class FskEncoder(object):
  
  def __init__(
      self,
      sample_rate=48000,
      pause_period=32,
      one_period=8,
      zero_period=4,
      packet_size=256):
    self._sr = sample_rate
    self._pause_period = pause_period
    self._one_period = one_period
    self._zero_period = zero_period
    self._packet_size = packet_size
    self._state = 1
    
  def _encode(self, symbol_stream):
    symbol_stream = numpy.array(symbol_stream)
    counts = [numpy.sum(symbol_stream == symbol) for symbol in range(3)]
    durations = [self._zero_period, self._one_period, self._pause_period]
    total_length = numpy.dot(durations, counts)
    signal = numpy.zeros((total_length, 1))
    state = self._state
    index = 0
    for symbol in symbol_stream:
      d = durations[symbol]
      signal[index:index + d] = state
      state = -state
      index += d
    assert index == signal.shape[0]
    self._state = state
    return signal
    
  def _code_blank(self, duration):
    num_symbols = int(duration * self._sr / self._pause_period) + 1
    return self._encode([2] * num_symbols)
    
  def _code_packet(self, data):
    assert len(data) <= self._packet_size
    if len(data) != self._packet_size:
      data = data + '\x00' * (self._packet_size - len(data))

    crc = zlib.crc32(data) & 0xffffffff

    data = map(ord, data)
    crc_bytes = [crc >> 24, (crc >> 16) & 0xff, (crc >> 8) & 0xff, crc & 0xff]
    bytes = [0x55] * 4 + data + crc_bytes
    
    symbol_stream = []
    for byte in bytes:
      mask = 0x80
      for _ in range(0, 8):
        symbol_stream.append(1 if (byte & mask) else 0)
        mask >>= 1
    
    return self._encode(symbol_stream)

  def code_intro(self):
    yield numpy.zeros((1 * self._sr, 1)).ravel()
    yield self._code_blank(1.0)
  
  def code_outro(self, duration=1.0):
    yield self._code_blank(duration)
    
  def code(self, data, page_size=1024, blank_duration=0.06):
    if len(data) % page_size != 0:
      tail = page_size - (len(data) % page_size)
      data += '\xff' * tail
    
    offset = 0
    remaining_bytes = len(data)
    num_packets_written = 0
    while remaining_bytes:
      size = min(remaining_bytes, self._packet_size)
      yield self._code_packet(data[offset:offset+size])
      num_packets_written += 1
      if num_packets_written == page_size / self._packet_size:
        yield self._code_blank(blank_duration)
        num_packets_written = 0
      remaining_bytes -= size
      offset += size


STM32F4_SECTOR_BASE_ADDRESS = [
  0x08000000,
  0x08004000,
  0x08008000,
  0x0800C000,
  0x08010000,
  0x08020000,
  0x08040000,
  0x08060000,
  0x08080000,
  0x080A0000,
  0x080C0000,
  0x080E0000
]

PAGE_SIZE = { 'stm32f1': 1024, 'stm32f3': 2048 }
PAUSE = { 'stm32f1': 0.06, 'stm32f3': 0.15 }
STM32F4_BLOCK_SIZE = 16384
STM32F4_APPLICATION_START = 0x08008000

def main():
  parser = optparse.OptionParser()
  parser.add_option(
      '-s',
      '--sample_rate',
      dest='sample_rate',
      type='int',
      default=48000,
      help='Sample rate in Hz')
  parser.add_option(
      '-b',
      '--pause_period',
      dest='pause_period',
      type='int',
      default=64,
      help='Period (in samples) of a blank symbol')
  parser.add_option(
      '-n',
      '--one_period',
      dest='one_period',
      type='int',
      default=16,
      help='Period (in samples) of a one symbol')
  parser.add_option(
      '-z',
      '--zero_period',
      dest='zero_period',
      type='int',
      default=8,
      help='Period (in samples) of a zero symbol')
  parser.add_option(
      '-p',
      '--packet_size',
      dest='packet_size',
      type='int',
      default=256,
      help='Packet size in bytes')
  parser.add_option(
      '-g',
      '--page_size',
      dest='page_size',
      type='int',
      default=1024,
      help='Flash page size')
  parser.add_option(
      '-o',
      '--output_file',
      dest='output_file',
      default=None,
      help='Write output file to FILE',
      metavar='FILE')
  parser.add_option(
      '-t',
      '--target',
      dest='target',
      default='stm32f1',
      help='Set page size and erase time for TARGET',
      metavar='TARGET')

  options, args = parser.parse_args()
  data = file(args[0], 'rb').read()
  if len(args) != 1:
    logging.fatal('Specify one, and only one firmware .bin file!')
    sys.exit(1)

  if options.target not in ['stm32f1', 'stm32f3', 'stm32f4']:
    logging.fatal('Unknown target: %s' % options.target)
    sys.exit(2)

  output_file = options.output_file
  if not output_file:
    if '.bin' in args[0]:
      output_file = args[0].replace('.bin', '.wav')
    else:
      output_file = args[0] + '.wav'

  encoder = FskEncoder(
      options.sample_rate,
      options.pause_period,
      options.one_period,
      options.zero_period,
      options.packet_size)
  writer = audio_stream_writer.AudioStreamWriter(
      output_file,
      options.sample_rate,
      16,
      1)

  # INTRO
  for block in encoder.code_intro():
    writer.append(block)
  
  blank_duration = 1.0
  if options.target in PAGE_SIZE.keys():
    for block in encoder.code(data, PAGE_SIZE[options.target], PAUSE[options.target]):
      if len(block):
        writer.append(block)
  elif options.target == 'stm32f4':
    for x in xrange(0, len(data), STM32F4_BLOCK_SIZE):
      address = STM32F4_APPLICATION_START + x
      block = data[x:x+STM32F4_BLOCK_SIZE]
      pause = 2.5 if address in STM32F4_SECTOR_BASE_ADDRESS else 0.2
      for block in encoder.code(block, STM32F4_BLOCK_SIZE, pause):
        if len(block):
          writer.append(block)
    blank_duration = 5.0
  for block in encoder.code_outro(blank_duration):
    writer.append(block)

if __name__ == '__main__':
  main()
