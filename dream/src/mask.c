#include "mask.h"

void entity_mask_create(EntityGroup* group, Vector3 pos){
    Mask mask = { 0 };
    mask.base = CreateBase(pos,WHITE);

    Components comps = COMP_BASE | COMP_PICKABLE | COMP_MASK;
    entity_mask_regenerate(&mask);
    
    AddGroupEntity(group,&mask,sizeof(Mask),comps,NULL,entity_mask_draw);
}

void entity_mask_regenerate(Mask* mask){
    mask->vertexCount = 4;
    if (mask->vertexCount < 4){
        return;
    }

    Mesh mesh = { 0 };
    mesh.triangleCount = mask->vertexCount - 2;
    mesh.vertexCount = mesh.triangleCount * 3;
    mesh.vertices = (float *)MemAlloc(mesh.vertexCount*3*sizeof(float));    // 3 vertices, 3 coordinates each (x, y, z)
    mesh.texcoords = (float *)MemAlloc(mesh.vertexCount*2*sizeof(float));   // 3 vertices, 2 coordinates each (x, y)
    mesh.normals = (float *)MemAlloc(mesh.vertexCount*3*sizeof(float));     // 3 vertices, 3 coordinates each (x, y, z)
                                                                            
    // Vertex at (0, 0, 0)
    mesh.vertices[0] = 0;
    mesh.vertices[1] = 0;
    mesh.vertices[2] = 0;
    mesh.normals[0] = 0;
    mesh.normals[1] = 1;
    mesh.normals[2] = 0;
    mesh.texcoords[0] = 0;
    mesh.texcoords[1] = 0;

    // Vertex at (1, 0, 2)
    mesh.vertices[3] = 1;
    mesh.vertices[4] = 0;
    mesh.vertices[5] = 2;
    mesh.normals[3] = 0;
    mesh.normals[4] = 1;
    mesh.normals[5] = 0;
    mesh.texcoords[2] = 0.5f;
    mesh.texcoords[3] = 1.0f;

    // Vertex at (2, 0, 0)
    mesh.vertices[6] = 2;
    mesh.vertices[7] = 0;
    mesh.vertices[8] = 0;
    mesh.normals[6] = 0;
    mesh.normals[7] = 1;
    mesh.normals[8] = 0;
    mesh.texcoords[4] = 1;
    mesh.texcoords[5] = 0;

    // Upload mesh data from CPU (RAM) to GPU (VRAM) memory
    UploadMesh(&mesh, false);

    mask->model = LoadModelFromMesh(mesh);
    Texture img = LoadTextureFromImage(GenImageChecked(32, 32, 2, 2, WHITE, BLACK));
    mask->model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = img;
}

void entity_mask_draw(void* ptr){
    Mask* mask = (Mask*) ptr;
    Base* base = &mask->base;

    DrawModel(mask->model,base->pos,1.f,base->tint);
}
