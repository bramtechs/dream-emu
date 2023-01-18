#include "entity.h"

Base::Base(EntityID id) {
	Base::Base(id, Vector3Zero(), WHITE);
}

Base::Base(EntityID id, Vector3 pos, Color tint) {
	BoundingBox box = {
		pos,
		Vector3Add(pos,Vector3One())
	};

	this->id = id;
	this->bounds = box;
	this->tint = tint;
}

void Base::Translate(Vector3 offset) {
	bounds.min = Vector3Add(bounds.min, offset);
	bounds.max = Vector3Add(bounds.max, offset);
}
inline void Base::TranslateX(float x) {
	Translate({ x,0,0 });
}
inline void Base::TranslateY(float y) {
	Translate({ 0,y,0 });
}
inline void Base::TranslateZ(float z) {
	Translate({ 0,0,z });
}
inline void Base::TranslateXYZ(float x, float y, float z) {
	Translate({ x,y,z });
}

void Base::SetBaseCenter(Vector3 pos) {
	bounds.min = Vector3Subtract(pos, halfSize());
	bounds.max = Vector3Add(pos, halfSize());
}
inline void Base::ResetTranslation() {
	SetBaseCenter(Vector3Zero());
}

inline Vector3 Base::center() {
	return Vector3Add(bounds.min, halfSize());
}

inline Vector3 Base::size() {
	return Vector3Subtract(bounds.max, bounds.min);
}

inline Vector3 Base::halfSize() {
	return Vector3Scale(size(), 0.5f);
}

RayCollision Base::GetMouseRayCollision(Camera camera) {
	Ray ray = GetWindowMouseRay(camera);
	return GetRayCollisionBox(ray, bounds);
}

ModelRenderer::ModelRenderer(EntityID id, const char* modelPath, Base* base) {
	Model model = RequestModel(modelPath);

	// make the base big enough to hold the model
	BoundingBox modelBox = GetModelBoundingBox(model);

	Vector3 size = Vector3Subtract(modelBox.max, modelBox.min);
	base->bounds.max = Vector3Add(base->bounds.min, size);

	Vector3 modelCenter = Vector3Add(modelBox.min, Vector3Scale(size, 0.5f));
	Vector3 offset = Vector3Subtract(base->center(), modelCenter);

	this->id = id;
	this->model = modelPath;
	this->accurate = false;
	this->offset = offset;
}

RayCollision EntityGroup::GetRayCollision(Ray ray) {
	float closestDistance = 10000000;
	RayCollision hit = { 0 };

	for (const auto& comp : comps) {
		if (comp.first == COMP_MODEL_RENDERER) {
			auto render = (ModelRenderer*)comp.second.get();
			auto base = (Base*)GetEntityComponent(render->id, COMP_BASE);
			Model model = RequestModel(render->model);

			if (render->accurate) { // do per triangle collisions

				Vector3 offset = Vector3Add(base->center(), render->offset);
				for (int j = 0; j < model.meshCount; j++) {
					RayCollision col = GetRayCollisionMesh(ray, model.meshes[j],
						MatrixTranslate(offset.x, offset.y, offset.z));

					if (col.hit && col.distance < closestDistance) {
						closestDistance = col.distance;
						hit = col;
					}
				}
			}
			else { // do bounds collision
				RayCollision col = GetRayCollisionBox(ray, base->bounds);
				if (col.hit && col.distance < closestDistance) {
					closestDistance = col.distance;
					hit = col;
				}
			}
		}
	}
	return hit;
}


bool EntityGroup::GetMousePickedBase(Camera camera, Base** result) {
	RayCollision col = { 0 };
	return GetMousePickedBaseEx(camera, result, &col);
}

bool EntityGroup::GetMousePickedBaseEx(Camera camera, Base** result, RayCollision* col) {
	for (const auto& entry : comps) {
		auto base = (Base*)entry.second.get();
		RayCollision rayCol = base->GetMouseRayCollision(camera);

		if (rayCol.hit) {
			*result = base;
			*col = rayCol;
			return true;
		}
	}
	*result = NULL;
	return false;
}

void EntityGroup::LoadGroup(const char* fileName) {
	comps.clear();
	INFO("Loaded entitygroup from %s", fileName);
}

void EntityGroup::SaveGroup(const char* fileName) {
	// TODO
	INFO("TODO");
	INFO("Exported entity component to %s", fileName);
}

EntityID EntityGroup::AddEntity() {
	EntityID id = entityCount;
	entityCount++;
	return id;
}

template <typename T>
void EntityGroup::AddEntityComponent(ItemType type, EntityID id, T data) {
	auto ptr = std::shared_ptr(data);
	group->comps.insert({ type, ptr });
}

void* EntityGroup::GetEntityComponent(EntityID id, ItemType filter) {
	for (const auto& comp : comps) {
		// TODO dirty hack 
		EntityID otherId = *((EntityID*)comp.second.get());
		if (otherId == id) {
			return comp.second.get();
		}
	}
	return NULL;
}


size_t EntityGroup::UpdateGroup(float delta) {
	for (const auto& comp : comps) {
		switch (comp.first) {
		case COMP_BASE:
		{
			auto base = (Base*)comp.second.get();
		} break;
		default:
			break;
		}
	}
	return entityCount;
}

size_t EntityGroup::DrawGroup(Camera camera, bool drawOutlines) {
	for (const auto& comp : comps) {
		switch (comp.first) {
		case COMP_MODEL_RENDERER:
		{
			// draw modelrenderers
			auto renderer = (ModelRenderer*)comp.second.get();
			auto base = (Base*)GetEntityComponent(renderer->id, COMP_BASE);

			if (base == NULL) {
				assert(false); // model renderer has no base! TODO shouldn't crash
			}

			Model model = RequestModel(renderer->model);
			DrawModelEx(model, Vector3Add(base->center(), renderer->offset),
				Vector3Zero(), 0, Vector3One(), base->tint);

		} break;
		case COMP_BASE:
		{
			if (drawOutlines) {
				auto base = (Base*)comp.second.get();
				RayCollision col = base->GetMouseRayCollision(camera);
				Color tint = col.hit ? WHITE : GRAY;
				DrawBoundingBox(base->bounds, tint);
				DrawPoint3D(base->center(), col.hit ? WHITE : GRAY);
			}
		} break;
		default:
			break;
		}
	}
	return entityCount;
}

