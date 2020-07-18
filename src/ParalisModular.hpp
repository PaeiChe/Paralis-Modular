#include "rack.hpp"


using namespace rack;


extern Plugin *pluginInstance;

extern Model *modelMiniModal;
extern Model* modelStiks;
extern Model *modelAtenuSight;
extern Model *modelZzzh;

// GUI COMPONENTS

struct Port18 : app::SvgPort {
  Port18() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Port18.svg")));
shadow->blurRadius = 2.0;
shadow->opacity = 0.15;
shadow->box.pos = Vec(0.5, box.size.y * 0.1);
  }
};

struct Port16 : app::SvgPort {
  Port16() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Port16.svg")));
shadow->blurRadius = 2.0;
shadow->opacity = 0.15;
shadow->box.pos = Vec(0.5, box.size.y * 0.1);
  }
};

struct Switch : app::SvgSwitch {
	Switch() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Switch_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Switch_1.svg")));
	}
};

struct ParalisButton : app::SvgSwitch {
	ParalisButton() {
		//momentary = true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ParalisButton_0.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ParalisButton_1.svg")));
	}
};

struct _Hsw : SvgSwitch {
	_Hsw() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/HSW_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/HSW_1.svg")));
	}
};


struct Port14 : app::SvgPort {
    Port14() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Port14.svg")));
        shadow->blurRadius = 2.0;
        shadow->opacity = 0.15;
        shadow->box.pos = Vec(0.5, box.size.y * 0.1);
    }
};


struct KnobBlack : app::SvgKnob {
  KnobBlack() {
    minAngle = -0.83 * M_PI;
    maxAngle = 0.83 * M_PI;
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/KnobBlack.svg")));
shadow->blurRadius = 2.0;
shadow->opacity = 0.15;
shadow->box.pos = Vec(0.5, box.size.y * 0.1);
  }
};

struct SmallKnobBlack : app::SvgKnob {
  SmallKnobBlack() {
    minAngle = -0.83 * M_PI;
    maxAngle = 0.83 * M_PI;
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SmallKnobBlack.svg")));
shadow->blurRadius = 2.0;
shadow->opacity = 0.15;
shadow->box.pos = Vec(0.5, box.size.y * 0.1);
  }
};
