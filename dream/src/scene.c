#define MAX_ENTITIES 1024
#include "scene.h"

bool DoDrawGrid = false; // TODO replace

inline Environment environment_default() {
	Environment env = { 0 };
	env.skyColor = SKYBLUE;
	env.fogColor = GRAY;
	env.fogDistance = 0.2f;

	const char* def = "sky/sky.png";
	strcpy(env.skyboxName, def);

	return env;
}

Model scene_gen_skybox_model(const char* skybox) {
	// TODO put skybox in struct, and dispose properly
	Mesh mesh = GenMeshSphere(-150, 10, 10);
	Model model = LoadModelFromMesh(mesh);
	Texture texture = RequestTexture(skybox);

	// Happy little accident, looks really cool, might use later
//    Image img = LoadImageFromTexture(texture);
//    ImageFlipVertical(&img);
//    ImageRotateCW(&img);

	Image img = LoadImageFromTexture(texture);
	ImageRotateCCW(&img);

	Texture textureFlipped = LoadTextureFromImage(img);
	model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = textureFlipped;

	return model;
}

Scene* scene_init()
{
	Scene* scene = MemAlloc(sizeof(Scene));
	scene->env = environment_default();
	scene->group = CreateEntityGroup();

	Camera cam = { 0 };
	cam.position = Vector3Zero();
	cam.target = Vector3One();
	cam.fovy = 80;
	cam.projection = CAMERA_PERSPECTIVE;
	cam.up = (Vector3){ 0.0f, 1.0f, 0.f };
	SetCameraMode(cam, CAMERA_FREE);
	scene->camera = cam;

	scene->editor = editor_init(scene);
	scene->editorVisible = true;

	// generate skybox 
	Model sky = scene_gen_skybox_model(scene->env.skyboxName);
	EntityID id = AddEntity(scene->group);

	Base base = CreateDefaultBase();
	ModelRenderer renderer = CreateModelRenderer(sky);

	AddEntityComponent(scene->group->bases, Base, &base, id);
	AddEntityComponent(scene->group->modelRenderers, ModelRenderer, &renderer, id);


	return scene;
}

void scene_update_and_render(Scene* scene, float delta)
{
	assert(scene);

	BeginMagmaDrawing();

	UpdateCamera(&scene->camera);

	BeginMode3D(scene->camera);

	ClearBackground(scene->env.skyColor);

	// move skybox around
	Base *skyBase = GetArrayItem(scene->group->bases,0,Base);
	skyBase->pos = scene->camera.position;

	UpdateGroup(scene->group, delta);
	DrawGroup(scene->group);

	if (scene->editorVisible) {
		editor_update_and_draw(scene->editor, delta);
		if (DoDrawGrid) {
			DrawGrid(1000, 1);
		}
	}
	EndMode3D();

	// ui drawing
			//DrawCircleV(GetScaledMousePosition(), 4.f, RED);                              // Draw a color-filled circle

	DrawFPS(10, 10);

	if (((int)GetTime()) % 2 == 0) {
		DrawText("DEMO DISC", WIDTH - MeasureText("DEMO DISC ", 20), HEIGHT - 20, 20, WHITE);
	}

	EndMagmaDrawing();

	scene_update_and_render_gui(scene, delta);

	EndDrawing();
}

void scene_update_and_render_gui(Scene* scene, float delta)
{
	if (scene->editorVisible) {
		scene->editorVisible = editor_update_and_draw_gui(scene->editor);
	}
	if (IsKeyPressed(KEY_F3)) {
		scene->editorVisible = !scene->editorVisible;
	}
}

void scene_dispose(Scene* scene)
{
	MemFree(scene);
}

