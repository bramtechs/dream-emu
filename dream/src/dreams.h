#pragma once

#include "magma.h"
#include "scene.h"

struct HubDream : Scene {
	HubDream();

	void update(float delta);
};

struct GardenDream : Scene {
	GardenDream();

	void update(float delta);
};
