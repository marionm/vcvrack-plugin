#include "plugin.hpp"

Plugin* pluginInstance;

void init(Plugin* p) {
	pluginInstance = p;
	p->addModel(entropyPoolModel);
	p->addModel(entropyPuddleModel);
}
