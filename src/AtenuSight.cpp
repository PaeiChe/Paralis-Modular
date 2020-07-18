#include "ParalisModular.hpp"


struct AtenuSight : Module {
	enum ParamIds {
		    GAIN1_PARAM,
    		GAIN2_PARAM,
    		GAIN3_PARAM,
    		GAIN4_PARAM,
    		MOD1_PARAM,
    		MOD2_PARAM,
    		MOD3_PARAM,
    		MOD4_PARAM,
			OFFSET1_PARAM,
			OFFSET2_PARAM,
			OFFSET3_PARAM,
			OFFSET4_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
	    	IN1_INPUT,
    		IN2_INPUT,
    		IN3_INPUT,
    		IN4_INPUT,
    		CV1_INPUT,
    		CV2_INPUT,
   	 		CV3_INPUT,
    		CV4_INPUT,		
		NUM_INPUTS
	};
	enum OutputIds {
		    OUT1_OUTPUT,
    		OUT2_OUTPUT,
    		OUT3_OUTPUT,
    		OUT4_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
			CV1_POS_LIGHT,
    		CV1_NEG_LIGHT,
    		CV2_POS_LIGHT,
    		CV2_NEG_LIGHT,
    		CV3_POS_LIGHT,
    		CV3_NEG_LIGHT,
    		CV4_POS_LIGHT,
    		CV4_NEG_LIGHT,
    		OUT1_POS_LIGHT,
    		OUT1_NEG_LIGHT,
    		OUT2_POS_LIGHT,
    		OUT2_NEG_LIGHT,
    		OUT3_POS_LIGHT,
    		OUT3_NEG_LIGHT,
    		OUT4_POS_LIGHT,
    		OUT4_NEG_LIGHT,
		NUM_LIGHTS
	};

	AtenuSight() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
			configParam(AtenuSight::GAIN1_PARAM, -2.0, 2.0, 0.0, "Ch 1 Gain");
    		configParam(AtenuSight::GAIN2_PARAM, -2.0, 2.0, 0.0, "Ch 2 Gain");
    		configParam(AtenuSight::GAIN3_PARAM, -2.0, 2.0, 0.0, "Ch 3 Gain");
    		configParam(AtenuSight::GAIN4_PARAM, -2.0, 2.0, 0.0, "Ch 4 Gain");
    		configParam(AtenuSight::MOD1_PARAM, -2.0, 2.0, 0.0, "Mod 1", " V");
    		configParam(AtenuSight::MOD2_PARAM, -2.0, 2.0, 0.0, "Mod 2", " V");
    		configParam(AtenuSight::MOD3_PARAM, -2.0, 2.0, 0.0, "Mod 3", " V");
    		configParam(AtenuSight::MOD4_PARAM, -2.0, 2.0, 0.0, "Mod 4", " V");
	    	configParam(AtenuSight::OFFSET1_PARAM, -10.0, 10.0, 0.0, "Ch 1 offset", " V");
    		configParam(AtenuSight::OFFSET2_PARAM, -10.0, 10.0, 0.0, "Ch 2 offset", " V");
    		configParam(AtenuSight::OFFSET3_PARAM, -10.0, 10.0, 0.0, "Ch 3 offset", " V");
    		configParam(AtenuSight::OFFSET4_PARAM, -10.0, 10.0, 0.0, "Ch 4 offset", " V");
	}

void process(const ProcessArgs &args) override;
};

void AtenuSight::process(const ProcessArgs &args) {
  float out = 0.0;

  for (int i = 0; i < 4; i++) {
    float g = params[GAIN1_PARAM + i].getValue();
    g += params[MOD1_PARAM + i].getValue() * inputs[CV1_INPUT + i].getVoltage() / 10.0;
    g = clamp(g, -2.0, 2.0);
    lights[CV1_POS_LIGHT + 2 * i].setSmoothBrightness(fmaxf(0.0, g), args.sampleTime);
    lights[CV1_NEG_LIGHT + 2 * i].setSmoothBrightness(fmaxf(0.0, -g), args.sampleTime);
    out += g * inputs[IN1_INPUT + i].getNormalVoltage(1.0) + params[OFFSET1_PARAM +i].getValue();
	out = clamp(out, -10.f, 10.f);
    lights[OUT1_POS_LIGHT + 2 * i].setSmoothBrightness(fmaxf(0.0, out / 10.0), args.sampleTime);
    lights[OUT1_NEG_LIGHT + 2 * i].setSmoothBrightness(fmaxf(0.0, -out / 10.0), args.sampleTime);
    if (outputs[OUT1_OUTPUT + i].isConnected()) {
      outputs[OUT1_OUTPUT + i].setVoltage(out);
      out = 0.0;
    }
  }
};

struct AtenuSightWidget : ModuleWidget {
	AtenuSightWidget(AtenuSight* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/AtenuSight.svg")));
	


    addParam(createParam<KnobBlack>(mm2px(Vec(1.3, 19.1)), module, AtenuSight::GAIN1_PARAM));
    addParam(createParam<KnobBlack>(mm2px(Vec(1.3, 44.2)), module, AtenuSight::GAIN2_PARAM));
    addParam(createParam<KnobBlack>(mm2px(Vec(1.3, 68.7)), module, AtenuSight::GAIN3_PARAM));
    addParam(createParam<KnobBlack>(mm2px(Vec(1.3, 93.7)), module, AtenuSight::GAIN4_PARAM));

	addParam(createParam<KnobBlack>(mm2px(Vec(16.8, 19.1)), module, AtenuSight::OFFSET1_PARAM));
    addParam(createParam<KnobBlack>(mm2px(Vec(16.8, 44.2)), module, AtenuSight::OFFSET2_PARAM));
    addParam(createParam<KnobBlack>(mm2px(Vec(16.8, 68.7)), module, AtenuSight::OFFSET3_PARAM));
    addParam(createParam<KnobBlack>(mm2px(Vec(16.8, 93.7)), module, AtenuSight::OFFSET4_PARAM));

	addParam(createParam<SmallKnobBlack>(mm2px(Vec(9.6, 20.4)), module, AtenuSight::MOD1_PARAM));
    addParam(createParam<SmallKnobBlack>(mm2px(Vec(9.6, 45.5)), module, AtenuSight::MOD2_PARAM));
    addParam(createParam<SmallKnobBlack>(mm2px(Vec(9.6, 70)), module, AtenuSight::MOD3_PARAM));
    addParam(createParam<SmallKnobBlack>(mm2px(Vec(9.6, 95.1)), module, AtenuSight::MOD4_PARAM));

    addInput(createInput<Port18>(mm2px(Vec(1.9, 26.4)), module, AtenuSight::IN1_INPUT));
    addInput(createInput<Port18>(mm2px(Vec(1.9, 51.5)), module, AtenuSight::IN2_INPUT));
    addInput(createInput<Port18>(mm2px(Vec(1.9, 76.0)), module, AtenuSight::IN3_INPUT));
    addInput(createInput<Port18>(mm2px(Vec(1.9, 101.1)), module, AtenuSight::IN4_INPUT));

    addInput(createInput<Port16>(mm2px(Vec(10.0, 26.6)), module, AtenuSight::CV1_INPUT));
    addInput(createInput<Port16>(mm2px(Vec(10.0, 51.6)), module, AtenuSight::CV2_INPUT));
    addInput(createInput<Port16>(mm2px(Vec(10.0, 76.2)), module, AtenuSight::CV3_INPUT));
    addInput(createInput<Port16>(mm2px(Vec(10.0, 101.2)), module, AtenuSight::CV4_INPUT));

    addOutput(createOutput<Port18>(mm2px(Vec(17.4, 26.3)), module, AtenuSight::OUT1_OUTPUT));
    addOutput(createOutput<Port18>(mm2px(Vec(17.4, 51.4)), module, AtenuSight::OUT2_OUTPUT));
    addOutput(createOutput<Port18>(mm2px(Vec(17.4, 75.9)), module, AtenuSight::OUT3_OUTPUT));
    addOutput(createOutput<Port18>(mm2px(Vec(17.4, 101)), module, AtenuSight::OUT4_OUTPUT));

    // addChild(createLight<SmallLight<GreenRedLight>>(mm2px(Vec(11.6, 34.4)), module, AtenuSight::CV1_POS_LIGHT));
    // addChild(createLight<SmallLight<GreenRedLight>>(mm2px(Vec(11.6, 59.4)), module, AtenuSight::CV2_POS_LIGHT));
    // addChild(createLight<SmallLight<GreenRedLight>>(mm2px(Vec(11.6, 84.4)), module, AtenuSight::CV3_POS_LIGHT));
    // addChild(createLight<SmallLight<GreenRedLight>>(mm2px(Vec(11.6, 109.4)), module, AtenuSight::CV4_POS_LIGHT));

    // addChild(createLight<MediumLight<GreenRedLight>>(mm2px(Vec(18.9, 15.0)), module, AtenuSight::OUT1_POS_LIGHT));
    // addChild(createLight<MediumLight<GreenRedLight>>(mm2px(Vec(18.9, 40.0)), module, AtenuSight::OUT2_POS_LIGHT));
    // addChild(createLight<MediumLight<GreenRedLight>>(mm2px(Vec(18.9, 64.5)), module, AtenuSight::OUT3_POS_LIGHT));
    // addChild(createLight<MediumLight<GreenRedLight>>(mm2px(Vec(18.9, 89.5)), module, AtenuSight::OUT4_POS_LIGHT));
		//addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		//addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		//addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		//addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	}
};


Model* modelAtenuSight = createModel<AtenuSight, AtenuSightWidget>("AtenuSight");