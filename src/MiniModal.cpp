#include "ParalisModular.hpp"
#include "elements/dsp/part.h"


struct MiniModal : Module {
	enum ParamIds {
		CONTOUR_PARAM,
		BOW_PARAM,
		BLOW_PARAM,
		STRIKE_PARAM,
		COARSE_PARAM,
		FINE_PARAM,
		FM_PARAM,

		FLOW_PARAM,
		MALLET_PARAM,
		GEOMETRY_PARAM,
		BRIGHTNESS_PARAM,

		BOW_TIMBRE_PARAM,
		BLOW_TIMBRE_PARAM,
		STRIKE_TIMBRE_PARAM,
		DAMPING_PARAM,
		POSITION_PARAM,
		SPACE_PARAM,

		BOW_TIMBRE_MOD_PARAM,
		FLOW_MOD_PARAM,
		BLOW_TIMBRE_MOD_PARAM,
		MALLET_MOD_PARAM,
		STRIKE_TIMBRE_MOD_PARAM,
		DAMPING_MOD_PARAM,
		GEOMETRY_MOD_PARAM,
		POSITION_MOD_PARAM,
		BRIGHTNESS_MOD_PARAM,
		SPACE_MOD_PARAM,

		PLAY_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		NOTE_INPUT,
		FM_INPUT,
		GATE_INPUT,
		STRENGTH_INPUT,
		BLOW_INPUT,
		STRIKE_INPUT,

		BOW_TIMBRE_MOD_INPUT,
		FLOW_MOD_INPUT,
		BLOW_TIMBRE_MOD_INPUT,
		MALLET_MOD_INPUT,
		STRIKE_TIMBRE_MOD_INPUT,
		DAMPING_MOD_INPUT,
		GEOMETRY_MOD_INPUT,
		POSITION_MOD_INPUT,
		BRIGHTNESS_MOD_INPUT,
		SPACE_MOD_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		AUX_OUTPUT,
		MAIN_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		GATE_LIGHT,
		EXCITER_LIGHT,
		RESONATOR_LIGHT,
		NUM_LIGHTS
	};

	dsp::SampleRateConverter<2> inputSrc;
	dsp::SampleRateConverter<2> outputSrc;
	dsp::DoubleRingBuffer<dsp::Frame<2>, 256> inputBuffer;
	dsp::DoubleRingBuffer<dsp::Frame<2>, 256> outputBuffer;

	uint16_t reverb_buffer[32768] = {};
	elements::Part *part;

	MiniModal() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(CONTOUR_PARAM, 0.0, 1.0, 1.0, "Envelope contour");
		configParam(BOW_PARAM, 0.0, 1.0, 0.0, "Bow exciter");
		configParam(BLOW_PARAM, 0.0, 1.0, 0.0, "Blow exciter");
		configParam(STRIKE_PARAM, 0.0, 1.0, 0.5, "Percussive noise amount");
		configParam(COARSE_PARAM, -30.0, 30.0, 0.0, "Coarse frequency adjustment");
		configParam(FINE_PARAM, -2.0, 2.0, 0.0, "Fine frequency adjustment");
		configParam(FM_PARAM, -1.0, 1.0, 0.0, "FM input attenuverter");
		configParam(FLOW_PARAM, 0.0, 1.0, 0.5, "Air flow noise color");
		configParam(MALLET_PARAM, 0.0, 1.0, 0.5, "Percussive noise type");
		configParam(GEOMETRY_PARAM, 0.0, 1.0, 0.5, "Resonator geometry");
		configParam(BRIGHTNESS_PARAM, 0.0, 1.0, 0.5, "Brightness");
		configParam(BOW_TIMBRE_PARAM, 0.0, 1.0, 0.5, "Bow timbre");
		configParam(BLOW_TIMBRE_PARAM, 0.0, 1.0, 0.5, "Blow timbre");
		configParam(STRIKE_TIMBRE_PARAM, 0.0, 1.0, 0.5, "Strike timbre");
		configParam(DAMPING_PARAM, 0.0, 1.0, 0.5, "Energy dissipation speed");
		configParam(POSITION_PARAM, 0.0, 1.0, 0.5, "Excitation position");
		configParam(SPACE_PARAM, 0.0, 2.0, 0.0, "Reverb space");
		configParam(BOW_TIMBRE_MOD_PARAM, -1.0, 1.0, 0.0, "Bow timbre attenuverter");
		configParam(FLOW_MOD_PARAM, -1.0, 1.0, 0.0, "Air flow noise attenuverter");
		configParam(BLOW_TIMBRE_MOD_PARAM, -1.0, 1.0, 0.0, "Blow timbre attenuverter");
		configParam(MALLET_MOD_PARAM, -1.0, 1.0, 0.0, "Percussive noise attenuverter");
		configParam(STRIKE_TIMBRE_MOD_PARAM, -1.0, 1.0, 0.0, "Strike timbre attenuverter");
		configParam(DAMPING_MOD_PARAM, -1.0, 1.0, 0.0, "Energy dissipation speed attenuverter");
		configParam(GEOMETRY_MOD_PARAM, -1.0, 1.0, 0.0, "Resonator geometry attenuverter");
		configParam(POSITION_MOD_PARAM, -1.0, 1.0, 0.0, "Excitation position attenuverter");
		configParam(BRIGHTNESS_MOD_PARAM, -1.0, 1.0, 0.0, "Brightness attenuverter");
		configParam(SPACE_MOD_PARAM, -2.0, 2.0, 0.0, "Reverb space attenuverter");
		configParam(PLAY_PARAM, 0.0, 1.0, 0.0, "Play");

		part = new elements::Part();
		// In the Mutable Instruments code, Part doesn't initialize itself, so zero it here.
		memset(part, 0, sizeof(*part));
		part->Init(reverb_buffer);
		// Just some random numbers
		uint32_t seed[3] = {1, 2, 3};
		part->Seed(seed, 3);
	}

	~MiniModal() {
		delete part;
	}

	void process(const ProcessArgs &args) override {
		// Get input
		if (!inputBuffer.full()) {
			dsp::Frame<2> inputFrame;
			inputFrame.samples[0] = inputs[BLOW_INPUT].getVoltage() / 5.0;
			inputFrame.samples[1] = inputs[STRIKE_INPUT].getVoltage() / 5.0;
			inputBuffer.push(inputFrame);
		}

		// Render frames
		if (outputBuffer.empty()) {
			float blow[16] = {};
			float strike[16] = {};
			float main[16];
			float aux[16];

			// Convert input buffer
			{
				inputSrc.setRates(args.sampleRate, 32000);
				dsp::Frame<2> inputFrames[16];
				int inLen = inputBuffer.size();
				int outLen = 16;
				inputSrc.process(inputBuffer.startData(), &inLen, inputFrames, &outLen);
				inputBuffer.startIncr(inLen);

				for (int i = 0; i < outLen; i++) {
					blow[i] = inputFrames[i].samples[0];
					strike[i] = inputFrames[i].samples[1];
				}
			}

			// Set patch from parameters
			elements::Patch* p = part->mutable_patch();
			p->exciter_envelope_shape = params[CONTOUR_PARAM].getValue();
			p->exciter_bow_level = params[BOW_PARAM].getValue();
			p->exciter_blow_level = params[BLOW_PARAM].getValue();
			p->exciter_strike_level = params[STRIKE_PARAM].getValue();

#define BIND(_p, _m, _i) clamp(params[_p].getValue() + 3.3f*dsp::quadraticBipolar(params[_m].getValue())*inputs[_i].getVoltage()/5.0f, 0.0f, 0.9995f)

			p->exciter_bow_timbre = BIND(BOW_TIMBRE_PARAM, BOW_TIMBRE_MOD_PARAM, BOW_TIMBRE_MOD_INPUT);
			p->exciter_blow_meta = BIND(FLOW_PARAM, FLOW_MOD_PARAM, FLOW_MOD_INPUT);
			p->exciter_blow_timbre = BIND(BLOW_TIMBRE_PARAM, BLOW_TIMBRE_MOD_PARAM, BLOW_TIMBRE_MOD_INPUT);
			p->exciter_strike_meta = BIND(MALLET_PARAM, MALLET_MOD_PARAM, MALLET_MOD_INPUT);
			p->exciter_strike_timbre = BIND(STRIKE_TIMBRE_PARAM, STRIKE_TIMBRE_MOD_PARAM, STRIKE_TIMBRE_MOD_INPUT);
			p->resonator_geometry = BIND(GEOMETRY_PARAM, GEOMETRY_MOD_PARAM, GEOMETRY_MOD_INPUT);
			p->resonator_brightness = BIND(BRIGHTNESS_PARAM, BRIGHTNESS_MOD_PARAM, BRIGHTNESS_MOD_INPUT);
			p->resonator_damping = BIND(DAMPING_PARAM, DAMPING_MOD_PARAM, DAMPING_MOD_INPUT);
			p->resonator_position = BIND(POSITION_PARAM, POSITION_MOD_PARAM, POSITION_MOD_INPUT);
			p->space = clamp(params[SPACE_PARAM].getValue() + params[SPACE_MOD_PARAM].getValue()*inputs[SPACE_MOD_INPUT].getVoltage()/5.0f, 0.0f, 2.0f);

			// Get performance inputs
			elements::PerformanceState performance;
			performance.note = 12.0*inputs[NOTE_INPUT].getVoltage() + roundf(params[COARSE_PARAM].getValue()) + params[FINE_PARAM].getValue() + 69.0;
			performance.modulation = 3.3*dsp::quarticBipolar(params[FM_PARAM].getValue()) * 49.5 * inputs[FM_INPUT].getVoltage()/5.0;
			performance.gate = params[PLAY_PARAM].getValue() >= 1.0 || inputs[GATE_INPUT].getVoltage() >= 1.0;
			performance.strength = clamp(1.0 - inputs[STRENGTH_INPUT].getVoltage()/5.0f, 0.0f, 1.0f);

			// Generate audio
			part->Process(performance, blow, strike, main, aux, 16);

			// Convert output buffer
			{
				dsp::Frame<2> outputFrames[16];
				for (int i = 0; i < 16; i++) {
					outputFrames[i].samples[0] = main[i];
					outputFrames[i].samples[1] = aux[i];
				}

				outputSrc.setRates(32000, args.sampleRate);
				int inLen = 16;
				int outLen = outputBuffer.capacity();
				outputSrc.process(outputFrames, &inLen, outputBuffer.endData(), &outLen);
				outputBuffer.endIncr(outLen);
			}

			// Set lights
			lights[GATE_LIGHT].setBrightness(performance.gate ? 0.75 : 0.0);
			lights[EXCITER_LIGHT].setBrightness(part->exciter_level());
			lights[RESONATOR_LIGHT].setBrightness(part->resonator_level());
		}

		// Set output
		if (!outputBuffer.empty()) {
			dsp::Frame<2> outputFrame = outputBuffer.shift();
			outputs[AUX_OUTPUT].setVoltage(5.0 * outputFrame.samples[0]);
			outputs[MAIN_OUTPUT].setVoltage(5.0 * outputFrame.samples[1]);
		}
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "model", json_integer(getModel()));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t *modelJ = json_object_get(rootJ, "model");
		if (modelJ) {
			setModel(json_integer_value(modelJ));
		}
	}

	int getModel() {
		return (int)part->resonator_model();
	}

	void setModel(int model) {
		part->set_resonator_model((elements::ResonatorModel)model);
	}
};


struct MiniModalModalItem : MenuItem {
	MiniModal *elements;
	int model;
	void onAction(const event::Action &e) override {
		elements->setModel(model);
	}
	void step() override {
		rightText = CHECKMARK(elements->getModel() == model);
		MenuItem::step();
	}
};


struct MiniModalWidget : ModuleWidget {
	MiniModalWidget(MiniModal *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/MiniModal.svg")));

		//addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		//addChild(createWidget<ScrewSilver>(Vec(480, 0)));
		//addChild(createWidget<ScrewSilver>(Vec(15, 365)));
		//addChild(createWidget<ScrewSilver>(Vec(480, 365)));

		addParam(createParamCentered<SmallKnobBlack>(mm2px(Vec(71.424, 22.963)), module, MiniModal::FINE_PARAM));
		addParam(createParamCentered<KnobBlack>(mm2px(Vec(19.579, 22.965)), module, MiniModal::BLOW_PARAM));
		addParam(createParamCentered<KnobBlack>(mm2px(Vec(58.456, 22.966)), module, MiniModal::COARSE_PARAM));
		addParam(createParamCentered<KnobBlack>(mm2px(Vec(32.537, 22.967)), module, MiniModal::STRIKE_PARAM));
		addParam(createParamCentered<KnobBlack>(mm2px(Vec(6.613, 22.968)), module, MiniModal::BOW_PARAM));
		addParam(createParamCentered<KnobBlack>(mm2px(Vec(84.443, 22.971)), module, MiniModal::FM_PARAM));
		addParam(createParamCentered<KnobBlack>(mm2px(Vec(58.437, 37.22)), module, MiniModal::DAMPING_PARAM));
		addParam(createParamCentered<KnobBlack>(mm2px(Vec(32.527, 37.239)), module, MiniModal::STRIKE_TIMBRE_PARAM));
		addParam(createParamCentered<KnobBlack>(mm2px(Vec(71.435, 37.245)), module, MiniModal::POSITION_PARAM));
		addParam(createParamCentered<KnobBlack>(mm2px(Vec(6.615, 37.258)), module, MiniModal::BOW_TIMBRE_PARAM));
		addParam(createParamCentered<KnobBlack>(mm2px(Vec(19.57, 37.26)), module, MiniModal::BLOW_TIMBRE_PARAM));
		addParam(createParamCentered<KnobBlack>(mm2px(Vec(84.439, 37.26)), module, MiniModal::SPACE_PARAM));
		addParam(createParamCentered<SmallKnobBlack>(mm2px(Vec(58.486, 51.561)), module, MiniModal::DAMPING_MOD_PARAM));
		addParam(createParamCentered<SmallKnobBlack>(mm2px(Vec(71.441, 51.561)), module, MiniModal::POSITION_MOD_PARAM));
		addParam(createParamCentered<SmallKnobBlack>(mm2px(Vec(6.6, 51.564)), module, MiniModal::BOW_TIMBRE_MOD_PARAM));
		addParam(createParamCentered<SmallKnobBlack>(mm2px(Vec(32.533, 51.564)), module, MiniModal::STRIKE_TIMBRE_MOD_PARAM));
		addParam(createParamCentered<SmallKnobBlack>(mm2px(Vec(45.57, 51.566)), module, MiniModal::MALLET_MOD_PARAM));
		addParam(createParamCentered<SmallKnobBlack>(mm2px(Vec(19.554, 51.574)), module, MiniModal::BLOW_TIMBRE_MOD_PARAM));
		addParam(createParamCentered<SmallKnobBlack>(mm2px(Vec(84.421, 51.574)), module, MiniModal::SPACE_MOD_PARAM));
		addParam(createParamCentered<KnobBlack>(mm2px(Vec(19.571, 65.779)), module, MiniModal::FLOW_PARAM));
		addParam(createParamCentered<KnobBlack>(mm2px(Vec(58.446, 65.792)), module, MiniModal::GEOMETRY_PARAM));
		addParam(createParamCentered<KnobBlack>(mm2px(Vec(32.531, 65.793)), module, MiniModal::MALLET_PARAM));
		addParam(createParamCentered<SmallKnobBlack>(mm2px(Vec(45.439, 65.794)), module, MiniModal::GEOMETRY_MOD_PARAM));
		addParam(createParamCentered<SmallKnobBlack>(mm2px(Vec(6.556, 65.796)), module, MiniModal::FLOW_MOD_PARAM));
		addParam(createParamCentered<SmallKnobBlack>(mm2px(Vec(84.522, 65.796)), module, MiniModal::BRIGHTNESS_MOD_PARAM));
		addParam(createParamCentered<KnobBlack>(mm2px(Vec(71.422, 65.8)), module, MiniModal::BRIGHTNESS_PARAM));
		addParam(createParamCentered<KnobBlack>(mm2px(Vec(45.475, 86.267)), module, MiniModal::CONTOUR_PARAM));
		addParam(createParamCentered<CKD6>(mm2px(Vec(45.508, 111.801)), module, MiniModal::PLAY_PARAM));

		addInput(createInputCentered<Port16>(mm2px(Vec(6.556, 85.811)), module, MiniModal::BOW_TIMBRE_MOD_INPUT));
		addInput(createInputCentered<Port16>(mm2px(Vec(19.571, 85.811)), module, MiniModal::BLOW_TIMBRE_MOD_INPUT));
		addInput(createInputCentered<Port16>(mm2px(Vec(32.531, 85.811)), module, MiniModal::STRIKE_TIMBRE_MOD_INPUT));
		addInput(createInputCentered<Port16>(mm2px(Vec(58.435, 85.811)), module, MiniModal::DAMPING_MOD_INPUT));
		addInput(createInputCentered<Port16>(mm2px(Vec(71.449, 85.811)), module, MiniModal::POSITION_MOD_INPUT));
		addInput(createInputCentered<Port16>(mm2px(Vec(84.409, 85.811)), module, MiniModal::SPACE_MOD_INPUT));
		addInput(createInputCentered<Port16>(mm2px(Vec(6.556, 99.049)), module, MiniModal::STRENGTH_INPUT));
		addInput(createInputCentered<Port16>(mm2px(Vec(19.571, 99.049)), module, MiniModal::FLOW_MOD_INPUT));
		addInput(createInputCentered<Port16>(mm2px(Vec(32.531, 99.049)), module, MiniModal::MALLET_MOD_INPUT));
		addInput(createInputCentered<Port16>(mm2px(Vec(58.435, 99.049)), module, MiniModal::GEOMETRY_MOD_INPUT));
		addInput(createInputCentered<Port16>(mm2px(Vec(71.449, 99.049)), module, MiniModal::BRIGHTNESS_MOD_INPUT));
		addInput(createInputCentered<Port16>(mm2px(Vec(84.409, 99.049)), module, MiniModal::FM_INPUT));
		addInput(createInputCentered<Port16>(mm2px(Vec(6.556, 111.749)), module, MiniModal::GATE_INPUT));
		addInput(createInputCentered<Port16>(mm2px(Vec(19.571, 111.749)), module, MiniModal::NOTE_INPUT));
		addInput(createInputCentered<Port18>(mm2px(Vec(32.531, 111.749)), module, MiniModal::BLOW_INPUT));
		addInput(createInputCentered<Port18>(mm2px(Vec(58.435, 111.749)), module, MiniModal::STRIKE_INPUT));

		addOutput(createOutputCentered<Port18>(mm2px(Vec(71.428, 111.797)), module, MiniModal::AUX_OUTPUT));
		addOutput(createOutputCentered<Port18>(mm2px(Vec(84.445, 111.811)), module, MiniModal::MAIN_OUTPUT));

		struct GateLight : YellowLight {
			GateLight() {
				box.size = Vec(28-6, 28-6);
				bgColor = color::BLACK_TRANSPARENT;
			}
		};

		addChild(createLight<GateLight>(mm2px(Vec(41.80, 108.20)), module, MiniModal::GATE_LIGHT));
		addChild(createLight<MediumLight<GreenLight>>(mm2px(Vec(43.9, 21.5)), module, MiniModal::EXCITER_LIGHT));
		addChild(createLight<MediumLight<RedLight>>(mm2px(Vec(43.9, 35.7)), module, MiniModal::RESONATOR_LIGHT));
	}

	void appendContextMenu(Menu *menu) override {
		MiniModal *elements = dynamic_cast<MiniModal*>(module);
		assert(elements);

		menu->addChild(construct<MenuLabel>());
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Alternative models"));
		menu->addChild(construct<MiniModalModalItem>(&MenuItem::text, "Original", &MiniModalModalItem::elements, elements, &MiniModalModalItem::model, 0));
		menu->addChild(construct<MiniModalModalItem>(&MenuItem::text, "Non-linear string", &MiniModalModalItem::elements, elements, &MiniModalModalItem::model, 1));
		menu->addChild(construct<MiniModalModalItem>(&MenuItem::text, "Chords", &MiniModalModalItem::elements, elements, &MiniModalModalItem::model, 2));
	}
};


Model *modelMiniModal = createModel<MiniModal, MiniModalWidget>("MiniModal");
