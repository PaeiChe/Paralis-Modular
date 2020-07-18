#include "ParalisModular.hpp"


struct Stiks : Module {
	enum ParamIds {
		THRESHOLD1_PARAM,
		THRESHOLD2_PARAM,
		MODE1_PARAM,
		MODE2_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN1_INPUT,
		IN2_INPUT,
		P1_INPUT,
		P2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1A_OUTPUT,
		OUT2A_OUTPUT,
		OUT1B_OUTPUT,
		OUT2B_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		MODE1_LIGHT,
		MODE2_LIGHT,
		STATE1_POS_LIGHT, STATE1_NEG_LIGHT,
		STATE2_POS_LIGHT, STATE2_NEG_LIGHT,
		NUM_LIGHTS
	};

	dsp::SchmittTrigger gateTriggers[2];
	dsp::SchmittTrigger modeTriggers[2];
	bool modes[2] = {};
	bool outcomes[2] = {};

	Stiks() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(THRESHOLD1_PARAM, 0.0, 1.0, 0.5, "Probability 1");
		configParam(MODE1_PARAM, 0.0, 1.0, 0.0, "Mode 1");
		configParam(THRESHOLD2_PARAM, 0.0, 1.0, 0.5, "Probability 2");
		configParam(MODE2_PARAM, 0.0, 1.0, 0.0, "Mode 2");
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_t* modesJ = json_array();
		for (int i = 0; i < 2; i++) {
			json_array_insert_new(modesJ, i, json_boolean(modes[i]));
		}
		json_object_set_new(rootJ, "modes", modesJ);
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* modesJ = json_object_get(rootJ, "modes");
		if (modesJ) {
			for (int i = 0; i < 2; i++) {
				json_t* modeJ = json_array_get(modesJ, i);
				if (modeJ)
					modes[i] = json_boolean_value(modeJ);
			}
		}
	}

	void process(const ProcessArgs& args) override {
		float gate = 0.0;
		for (int i = 0; i < 2; i++) {
			// mode button
			if (modeTriggers[i].process(params[MODE1_PARAM + i].getValue()))
				modes[i] = !modes[i];

			if (inputs[IN1_INPUT + i].isConnected())
				gate = inputs[IN1_INPUT + i].getVoltage();

			if (gateTriggers[i].process(gate)) {
				// trigger
				float r = random::uniform();
				float threshold = clamp(params[THRESHOLD1_PARAM + i].getValue() + inputs[P1_INPUT + i].getVoltage() / 10.f, 0.f, 1.f);
				bool toss = (r < threshold);
				if (!modes[i]) {
					// direct modes
					outcomes[i] = toss;
				}
				else {
					// toggle modes
					outcomes[i] = (outcomes[i] != toss);
				}

				if (!outcomes[i])
					lights[STATE1_POS_LIGHT + 2 * i].value = 1.0;
				else
					lights[STATE1_NEG_LIGHT + 2 * i].value = 1.0;
			}

			lights[STATE1_POS_LIGHT + 2 * i].value *= 1.0 - args.sampleTime * 15.0;
			lights[STATE1_NEG_LIGHT + 2 * i].value *= 1.0 - args.sampleTime * 15.0;
			lights[MODE1_LIGHT + i].value = modes[i] ? 1.0 : 0.0;

			outputs[OUT1A_OUTPUT + i].setVoltage(outcomes[i] ? 0.0 : gate);
			outputs[OUT1B_OUTPUT + i].setVoltage(outcomes[i] ? gate : 0.0);
		}
	}

	void onReset() override {
		for (int i = 0; i < 2; i++) {
			modes[i] = false;
			outcomes[i] = false;
		}
	}
};


struct StiksWidget : ModuleWidget {
	StiksWidget(Stiks* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Stiks.svg")));

		//addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		//addChild(createWidget<ScrewSilver>(Vec(15, 365)));

		addParam(createParamCentered<SmallKnobBlack>(mm2px(Vec(5.08, 32.567)), module, Stiks::THRESHOLD1_PARAM));
		addParam(createParamCentered<ParalisButton>(mm2px(Vec(5.08, 14.947)), module, Stiks::MODE1_PARAM));
		addInput(createInputCentered<Port18>(mm2px(Vec(5.08, 19.447)), module, Stiks::IN1_INPUT));
		addInput(createInputCentered<Port16>(mm2px(Vec(5.08, 38.636)), module, Stiks::P1_INPUT));
		addOutput(createOutputCentered<Port14>(mm2px(Vec(5.08, 53.819)), module, Stiks::OUT1A_OUTPUT));
		addOutput(createOutputCentered<Port14>(mm2px(Vec(5.08, 61.105)), module, Stiks::OUT1B_OUTPUT));

		addParam(createParamCentered<SmallKnobBlack>(mm2px(Vec(5.08, 89.188)), module, Stiks::THRESHOLD2_PARAM));
		addParam(createParamCentered<ParalisButton>(mm2px(Vec(5.08, 71.568)), module, Stiks::MODE2_PARAM));
		addInput(createInputCentered<Port18>(mm2px(Vec(5.08, 76.068)), module, Stiks::IN2_INPUT));
		addInput(createInputCentered<Port16>(mm2px(Vec(5.08, 95.256)), module, Stiks::P2_INPUT));
		addOutput(createOutputCentered<Port14>(mm2px(Vec(5.08, 110.44)), module, Stiks::OUT2A_OUTPUT));
		addOutput(createOutputCentered<Port14>(mm2px(Vec(5.08, 117.726)), module, Stiks::OUT2B_OUTPUT));

		addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(5.08, 45.173)), module, Stiks::STATE1_POS_LIGHT));
		addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(5.08, 101.794)), module, Stiks::STATE2_POS_LIGHT));
	}

	void appendContextMenu(Menu* menu) override {
		Stiks* branches = dynamic_cast<Stiks*>(module);
		assert(branches);

		struct StiksModeItem : MenuItem {
			Stiks* branches;
			int channel;
			void onAction(const event::Action& e) override {
				branches->modes[channel] ^= 1;
			}
			void step() override {
				rightText = branches->modes[channel] ? "Toggle" : "Latch";
				MenuItem::step();
			}
		};

		menu->addChild(new MenuSeparator);

		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Channels"));
		menu->addChild(construct<StiksModeItem>(&MenuItem::text, "Channel 1 modes", &StiksModeItem::branches, branches, &StiksModeItem::channel, 0));
		menu->addChild(construct<StiksModeItem>(&MenuItem::text, "Channel 2 modes", &StiksModeItem::branches, branches, &StiksModeItem::channel, 1));
	}
};


Model* modelStiks = createModel<Stiks, StiksWidget>("Stiks");
