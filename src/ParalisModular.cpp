#include "ParalisModular.hpp"


Plugin *pluginInstance;

void init(rack::Plugin *p) {
	pluginInstance = p;

	p->addModel(modelMiniModal);
	p->addModel(modelStiks);
	p->addModel(modelAtenuSight);
	p->addModel(modelZzzh);
}
