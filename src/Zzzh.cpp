#include "ParalisModular.hpp"

struct Zzzh : Module {
  enum ParamIds {
    SWITCH1_PARAM,
    SWITCH2_PARAM,
    SWITCH3_PARAM,
    SWITCH4_PARAM,
    SWATCH1_PARAM,
    SWATCH2_PARAM,
    SWATCH3_PARAM,
    SWATCH4_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    SH1_INPUT,
    SH2_INPUT,
    SH3_INPUT,
    SH4_INPUT,
// Dummy Inputs    
    DD1_INPUT,
    DD2_INPUT,
    DD3_INPUT,
    DD4_INPUT,
    TRIG1_INPUT,
    TRIG2_INPUT,
    TRIG3_INPUT,
    TRIG4_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    RECT1_OUTPUT,
    RECT2_OUTPUT,
    RECT3_OUTPUT,
    RECT4_OUTPUT,
    NOISE1_OUTPUT,
    NOISE2_OUTPUT,
    NOISE3_OUTPUT,
    NOISE4_OUTPUT,
    SH1_OUTPUT,
    SH2_OUTPUT,
    SH3_OUTPUT,
    SH4_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    SH_POS1_LIGHT,
    SH_NEG1_LIGHT,
    SH_POS2_LIGHT,
    SH_NEG2_LIGHT,
    SH_POS3_LIGHT,
    SH_NEG3_LIGHT,
    SH_POS4_LIGHT,
    SH_NEG4_LIGHT,
    NUM_LIGHTS
  };

  dsp::SchmittTrigger trigger[4];
  float sample[4] = {0.0, 0., 0., 0.};
  float unirando[4] = {0.0, 0., 0., 0.};

  Zzzh() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(Zzzh::SWITCH1_PARAM, 0.0, 1.0, 0.0, "Random/Uni");
    configParam(Zzzh::SWITCH2_PARAM, 0.0, 1.0, 0.0, "Random/Uni");
    configParam(Zzzh::SWITCH3_PARAM, 0.0, 1.0, 0.0, "Random/Uni");
    configParam(Zzzh::SWITCH4_PARAM, 0.0, 1.0, 0.0, "Random/Uni");
    configParam(Zzzh::SWATCH1_PARAM, 0.0, 1.0, 0.0, "Noise/Inv");
    configParam(Zzzh::SWATCH2_PARAM, 0.0, 1.0, 0.0, "Noise/Inv");
    configParam(Zzzh::SWATCH3_PARAM, 0.0, 1.0, 0.0, "Noise/Inv");
    configParam(Zzzh::SWATCH4_PARAM, 0.0, 1.0, 0.0, "Noise/Inv");
    //trigger.setThresholds(0.0, 0.7);
  }
  void reset() {

    for (unsigned int i = 0; i < 4; i++)
      sample[i] = 0.;     
  }

  void process(const ProcessArgs &args) override;
};

void Zzzh::process(const ProcessArgs &args) {

  float in[4], trig[4];
  

  for (unsigned int i = 0; i < 4; i++) { 

    // Gaussian noise generator
    // TO DO: check correlation between calls
    float noise = 5.0 * random::normal();

    if (i == 0) {
      trig[0] = inputs[TRIG1_INPUT].getNormalVoltage(0);
    } else {
      trig[i] = inputs[TRIG1_INPUT + i].getNormalVoltage(trig[i - 1]);
    }

    in[i] = inputs[SH1_INPUT + i].getNormalVoltage(noise);

    if (trigger[i].process(trig[i])) {
      sample[i] = inputs[SH1_INPUT + i].getNormalVoltage(noise);
      unirando[i] = inputs[DD1_INPUT + i].getNormalVoltage(noise); 
    }

 

    // lights
    lights[SH_POS1_LIGHT + 2 * i].setBrightness(fmaxf(0.0, sample[i] / 5.0));
    lights[SH_NEG1_LIGHT + 2 * i].setBrightness(fmaxf(0.0, -sample[i] / 5.0));

    // outputs
    outputs[SH1_OUTPUT + i].setVoltage(sample[i]);

    double Switch = params[SWITCH1_PARAM + i].getValue();
		if(Switch > 0.0) {
    outputs[RECT1_OUTPUT + i].setVoltage(fabsf(sample[i]));
		}
		else {
    outputs[RECT1_OUTPUT + i].setVoltage(fabsf(unirando[i]));
		}

    double Swatch = params[SWATCH1_PARAM + i].getValue();
		if(Swatch > 0.0) {
    outputs[NOISE1_OUTPUT + i].setVoltage(-(sample[i]));
		}
		else {
    outputs[NOISE1_OUTPUT + i].setVoltage(noise);
		}

  }
}

struct ZzzhWidget : ModuleWidget {

  ZzzhWidget(Zzzh *module) {
    setModule(module);

    box.size = Vec(15 * 4, 380);

    {
      SvgPanel *panel = new SvgPanel();
      panel->box.size = box.size;
      panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Zzzh.svg")));
      addChild(panel);
    }

    float y1 = 50;
    float yh = 80;

    for (unsigned int i = 0; i < 4; i++) {
      addParam(createParam<_Hsw>(Vec(25, y1 + i * yh + 44), module, Zzzh::SWITCH1_PARAM + i));
      addParam(createParam<_Hsw>(Vec(40, y1 + i * yh + 67), module, Zzzh::SWATCH1_PARAM + i));      
      addInput(createInput<Port16>(Vec(8, y1 + i * yh + (1.3 + 10)), module, Zzzh::TRIG1_INPUT + i));
      addInput(createInput<Port18>(Vec(36, y1 + i * yh + 10), module, Zzzh::SH1_INPUT + i));
      addOutput(createOutput<Port18>(Vec(7, 35 + y1 + i * yh + 10), module, Zzzh::RECT1_OUTPUT + i));
      addOutput(createOutput<Port18>(Vec(36, 35 + y1 + i * yh + 10), module, Zzzh::SH1_OUTPUT + i));
      addOutput(createOutput<Port14>(Vec(22, y1 + i * yh + 57 + 10), module, Zzzh::NOISE1_OUTPUT + i));

      addChild(createLight<SmallLight<GreenRedLight>>(Vec(26, y1 + i * yh + (15 + 10)), module, Zzzh::SH_POS1_LIGHT + 2 * i));
    }
  }
};

Model *modelZzzh = createModel<Zzzh, ZzzhWidget>("Zzzh");
