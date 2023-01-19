#include "editor.h"
/*
static Scene* ActiveScene = NULL;

static EditorMode Modes[EDITOR_MODE_COUNT] = {
	{
		EDITOR_NORMAL,
		"Normal Mode",
		KEY_ESCAPE,
		{ 255, 255, 255, 255 },
	},
	{
		EDITOR_SPAWN,
		"Spawn Mode",
		KEY_I,
		{ 0, 228, 48, 255 },
	},
	{
		EDITOR_MOVE,
		"Move Mode",
		KEY_G,
		{ 230, 41, 55, 255 },
	},
	{
		EDITOR_ROTATE,
		"Rotation Mode",
		KEY_R,
		{ 255, 109, 194, 255 },
	},
	{
		EDITOR_SCALE,
		"Scale Mode",
		KEY_S,
		{ 200, 122, 255, 255 },
	},
	{
		EDITOR_ARCHITECT,
		"Architect Mode",
		KEY_A,
		{ 163, 73, 164 },
	},
};

static int LayoutY = 0;

static Rectangle LAYOUT(int x, int w, int h) {
	Rectangle rect = {
		x,LayoutY,w,h
	};
	LayoutY += h + 10;

	return rect;
}

Editor::Editor(Scene* scene) {
	ActiveScene = scene;

	FilePathList paths = Assets::IndexModels();
	for (int i = 0; i < paths.count; i++) {
		const char* path = paths.paths[i];
		models.push_back(path);
	}
}

void editor_print_transform(Editor* e, Base* base) {
	Color color = Modes[e->mode].color;
	Vector3 center = base->center();
	DrawText(TextFormat("center\nx %f\ny %f\nz %f", center.x, center.y, center.z), 150, 550, 16, color);
}

void editor_dispose(Editor* editor) {
	M_MemFree(editor);
}

void Editor::move_gui(Base* base) {
	print_transform(base);
}

void Editor::move(Base* base) {
	Vector2 delta = Vector2Scale(GetMouseDelta(), 0.5f);
	if (IsKeyDown(KEY_F)) {
		base->SetCenter(ActiveScene->player.feet);
	}

	if (IsKeyDown(KEY_P)) {
		Ray ray = { 0 };
		ray.position = Vector3Add(base->center(), { 0.f, -0.01f, 0.f });
		ray.direction = { 0,-1,0 };

		RayCollision col = ActiveScene->group.GetRayCollision(ray);
		if (col.hit) {
			base->SetCenter(col.point);
		}
	}

	float LINE_LEN = 100.f;
	float LINE_THICK = 0.3f;
	if (IsKeyDown(KEY_X)) {
		base->TranslateX(delta.x);
		DrawCube(base->center(), LINE_LEN, LINE_THICK, LINE_THICK, RED);
	}
	if (IsKeyDown(KEY_Y)) {
		base->TranslateY(-delta.y);
		DrawCube(base->center(), LINE_THICK, LINE_LEN, LINE_THICK, GREEN);
	}
	if (IsKeyDown(KEY_Z)) {
		base->TranslateZ(delta.x);
		DrawCube(base->center(), LINE_THICK, LINE_THICK, LINE_LEN, BLUE);
	}
	if (IsKeyDown(KEY_ZERO)) {
		base->ResetTranslation();
	}
	if (IsKeyDown(KEY_TAB)) {
		Vector3 pos = {
			floorf(base->center().x),
			floorf(base->center().y),
			floorf(base->center().z),
		};
		base->SetCenter(pos);
	}

}

void Editor::architect_gui(Base* base) {
}

void Editor::architect(Base* base) {
	if (base == NULL) {
		// spawn in empty and set as subject
		EntityID id = ActiveScene->group.AddEntity();
		Base base = Base(id, Vector3Zero(), RAYWHITE);
		ActiveScene->group.AddEntityComponent(COMP_BASE, id, &base);
		subject = id;
	}

	// draw mouse
	float RADIUS = 10000.f;
	Vector3 vertices[] = {
		{ -RADIUS, 0.f, -RADIUS },
		{ -RADIUS, 0.f, RADIUS },
		{ RADIUS, 0.f, RADIUS },
		{ RADIUS, 0.f, -RADIUS },
	};

	Color color = RED;
	Ray mouseRay = GetWindowMouseRay(ActiveScene->player.camera);
	RayCollision col = GetRayCollisionQuad(mouseRay, vertices[0], vertices[1], vertices[2], vertices[3]);
	if (col.hit) {
		Vector3 pos = { roundf(col.point.x), 0, roundf(col.point.z) };
		DrawSphere(pos, 0.1f, color);
		Vector3 below = Vector3Subtract(pos, { 0.f,1.f,0.f });
		Vector3 above = Vector3Add(pos, { 0.f,3.f,0.f });
		DrawLine3D(below, above, color);
	}
}

void Editor::spawner_gui() {
	Camera cam = { 0 };
	cam.position = { 0, 0, -10 };
	cam.up = { 0, 1, 1 };
	cam.fovy = 45;
	cam.projection = CAMERA_PERSPECTIVE;

	int SIZE = 256;
	if (((int)elapsedTime) % 2 == 1) {
		DrawText(TextFormat("SPAWN MENU (%d)", selectedModel), 650, 250, 36, YELLOW);
		DrawRectangleLines(GetScreenWidth() / 2 - SIZE / 2, GetScreenHeight() / 2 - SIZE / 2, SIZE, SIZE, RED);
	}

	BeginMode3D(cam);

	// draw catalog preview
	Vector3 pos = { -models.size() / 2, 0, 0 };
	pos.x = selectedModel;
	for (int i = 0; i < models.size(); i++) {
		Model model = Assets::RequestModel(models[i].c_str());
		BoundingBox box = GetModelBoundingBox(model);
		float diameter = Vector3Length(Vector3Subtract(box.max, box.min));
		float scale = 1 / diameter;
		DrawModelEx(model, pos, { 1,0.5f,0.3f }, (i * 10.f) + elapsedTime * 40.f, { scale, scale, scale }, WHITE);

		pos.x--;
	}

	if (IsKeyPressed(KEY_LEFT) && selectedModel > 0) {
		selectedModel--;
	}

	if (IsKeyPressed(KEY_RIGHT) && selectedModel < models.size() - 1) {
		selectedModel++;
	}

	if (IsKeyPressed(KEY_ENTER)) {
		std::string modelPath = models[selectedModel];

		// spawn new entity
		EntityID id = ActiveScene->group.AddEntity();
		Base base = Base(id, Vector3Zero(), RAYWHITE);

		// TODO FIX
		ModelRenderer renderer = ModelRenderer(id, modelPath.c_str(), &base);

		ActiveScene->group.AddEntityComponent(COMP_BASE, id, &base);
		ActiveScene->group.AddEntityComponent(COMP_MODEL_RENDERER, id, &renderer);

		mode = EDITOR_MOVE;
		subject = id;
	}

	elapsedTime += GetFrameTime();

	EndMode3D();
}

void Editor::update_and_render(float delta)
{
	if (prevFreecamMode != Prefs.freeCam) {
		prevFreecamMode = Prefs.freeCam;
		SetCameraMode(ActiveScene->player.camera, Prefs.freeCam ? CAMERA_FIRST_PERSON : CAMERA_FREE);
	}

	// highlight selected
	auto subjectBase = (Base*) ActiveScene->group.GetEntityComponent(subject, COMP_BASE);
	if (subjectBase != NULL) {
		DrawBoundingBox(subjectBase->bounds, Modes[mode].color);
	}

	// change selected subject on clicking it
	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
		Base* base = NULL;
		if (ActiveScene->group.GetMousePickedBase(ActiveScene->player.camera, &base)) {
			EntityID id = base->id;

			subject = id;
			DEBUG("Changed editor subject to %d", id);
		}
	}

	// draw grid
	if (Prefs.drawGrid || mode == EDITOR_ARCHITECT) {
		DrawGrid(1000, 1);
	}

	// draw spawnpoint
	DrawSphere(ActiveScene->spawnPoint, 0.4f, ORANGE);

	switch (mode) {
	case EDITOR_MOVE:
		move(subjectBase);
		break;
	case EDITOR_ARCHITECT:
		architect(subjectBase);
		break;
	}
}

bool Editor::update_and_draw_gui()
{
	int WIN_X = 10;
	int WIN_Y = 10;
	int WIN_W = 300;
	int WIN_H = 700;

	LayoutY = WIN_Y + 40;

	EditorMode curMode = Modes[mode];

	Rectangle rect = { WIN_X, WIN_Y, WIN_W, WIN_H };
	bool visible = !GuiWindowBox(rect, curMode.name);

	ActiveScene->env.skyColor.r = GuiSlider(LAYOUT(40, 100, 20), "Red", "255", ActiveScene->env.skyColor.r, 0.f, 255.f);
	ActiveScene->env.skyColor.g = GuiSlider(LAYOUT(40, 100, 20), "Green", "255", ActiveScene->env.skyColor.g, 0.f, 255.f);
	ActiveScene->env.skyColor.b = GuiSlider(LAYOUT(40, 100, 20), "Blue", "255", ActiveScene->env.skyColor.b, 0.f, 255.f);

	const char* fogStr = TextFormat("%f", ActiveScene->env.fogDistance);
	ActiveScene->env.fogDistance = GuiSlider(LAYOUT(40, 100, 20), "Fog", fogStr, ActiveScene->env.fogDistance, 0.f, 1.f);

	GuiLabel(LAYOUT(20, WIN_W - 50, 50), "Hold middle mouse to move around,\nhold alt to look around.\nUse scrollwheel");

	Settings.unlockFrameRate = GuiCheckBox(LAYOUT(20, 30, 30), "Unlock framerate (not recommended)", Settings.unlockFrameRate);
	Prefs.drawOutlines = GuiCheckBox(LAYOUT(20, 30, 30), "Draw outlines", Prefs.drawOutlines);
	Prefs.drawGrid = GuiCheckBox(LAYOUT(20, 30, 30), "Draw grid", Prefs.drawGrid);
	Prefs.freeCam = GuiCheckBox(LAYOUT(20, 30, 30), "Unlock camera", Prefs.freeCam);

	// get current selected base
	auto subjectBase = (Base*) ActiveScene->group.GetEntityComponent(subject, COMP_BASE);

	// vomit out log
	DrawLog(350, 80, 24);

	DrawText(curMode.name, 350, HEIGHT - 50, 36, curMode.color);

	// check keys
	for (int i = 0; i < EDITOR_MODE_COUNT; i++) {
		if (IsKeyPressed(Modes[i].key)) {
			mode = i;
			break;
		}
	}

	// set spawnpoint
	if (IsKeyDown(KEY_LEFT_CONTROL))
	{
		if (IsKeyPressed(KEY_HOME)) {
			Vector3 spawn = ActiveScene->spawnPoint = ActiveScene->player.feet;
			INFO("Changed spawnpoint to %f, %f, %f!", spawn.x, spawn.y, spawn.z);
		}
		if (IsKeyPressed(KEY_L)) {
			ActiveScene->group.LoadGroup("savedmap001.comps");
			INFO("Loaded scene!");
		}
		if (IsKeyPressed(KEY_S)) {
			ActiveScene->group.SaveGroup("savedmap001.comps");
			INFO("Saved scene!");
		}
	}

	switch (mode) {
	case EDITOR_NORMAL:
		break;
	case EDITOR_MOVE:
		Editor::move_gui(subjectBase);
		break;
	case EDITOR_ROTATE:
		break;
	case EDITOR_SCALE:
		break;
	case EDITOR_ARCHITECT:
		Editor::architect_gui(subjectBase);
		break;
	case EDITOR_SPAWN:
		Editor::spawner_gui();
		break;

	default:
		assert(false);
	}

	return visible;
}
*/
