#pragma once

/*

#include "magma.h"
#include "settings.h"

#define MAX_MODELS 128

#define EDITOR_NORMAL      0
#define EDITOR_SPAWN       1
#define EDITOR_MOVE        2
#define EDITOR_ROTATE      3
#define EDITOR_SCALE       4
#define EDITOR_ARCHITECT   5
#define EDITOR_MODE_COUNT  6

// bandaid fix to avoid circular dependencies
struct Scene;

struct EditorMode {
	size_t id;
	const char name[40];
	KeyboardKey key;
	Color color;
};

struct Editor {
	size_t mode;
	EntityID subject;

	float elapsedTime;
	bool prevFreecamMode;

	// model selector
	std::vector<std::string> models;
	size_t selectedModel;

	Editor(Scene* scene);

	void update_and_render(float delta);
	bool update_and_draw_gui();

private:
	void architect_gui(Base* base);
	void architect(Base* base);

	void move(Base* base);
	void move_gui(Base* base);

	void spawner_gui();

	void print_transform(Base* base);
};

#include "scene.h" // TODO vile and disgusting, don't put this here!!!

*/
