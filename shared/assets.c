#include "assets.h"

constructor(Assets){
	INFO("Loading assets...");
	return Assets;
}

destructor(Assets) {
	INFO("Disposing assets!");
	return Assets;
}
