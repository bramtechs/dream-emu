#pragma once

#include "raylib.h"
#include "raymath.h"

#include "window.h"
#include "memory_custom.h"
#include "assets.h"

#include <map>
#include <memory>

#define COMP_ALL                0
#define COMP_BASE               1
#define COMP_MODEL_RENDERER     2

typedef unsigned int uint;
typedef uint ItemType;
typedef uint EntityID;

struct Base {
    EntityID id;
    BoundingBox bounds;
    Color tint;

    Base(EntityID id);
    Base(EntityID id, Vector3 pos, Color tint);

    void Translate(Vector3 offset);
    inline void TranslateX(float x);
    inline void TranslateY(float y);
    inline void TranslateZ(float z);
    inline void TranslateXYZ(float x, float y, float z);

    void SetBaseCenter(Vector3 pos);
    inline void ResetTranslation();

	RayCollision GetMouseRayCollision(Camera camera);

    inline Vector3 center();
    inline Vector3 size();
    inline Vector3 halfSize();
};

struct ModelRenderer{
    EntityID id;
    const char* model;
    bool accurate;
    Vector3 offset; //from base center

    ModelRenderer(EntityID id, const char* modelPath, Base* base);
};

struct EntityGroup {
    uint entityCount;
    std::multimap<ItemType, std::shared_ptr<void*>> comps;

	RayCollision GetRayCollision(Ray ray);

	bool GetMousePickedBase(Camera camera, Base** result);
	bool GetMousePickedBaseEx(Camera camera, Base** result, RayCollision* col);

	void LoadGroup(const char* fileName);
	void SaveGroup(const char* fileName);

	EntityID AddEntity();

	template <typename T>
	void AddEntityComponent(ItemType type, EntityID id, T data);

	void* GetEntityComponent(EntityID id, ItemType filter);

	size_t UpdateGroup(float delta);
	size_t DrawGroup(Camera camera, bool drawOutlines);
};

