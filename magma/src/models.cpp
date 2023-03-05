#include "magma.h"

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include <external/tinyobj_loader_c.h>

// TODO intergrate without needing callbacks
Model LoadOBJFromMemory(const char* fileName)
{
    Model model = { 0 };

    tinyobj_attrib_t attrib = { 0 };
    tinyobj_shape_t* meshes = NULL;
    unsigned int meshCount = 0;

    tinyobj_material_t* materials = NULL;
    unsigned int materialCount = 0;

    char* fileText = RequestCustom(fileName, NULL, ".obj");

    if (fileText != NULL)
    {
        unsigned int dataSize = (unsigned int)strlen(fileText);
        char currentDir[1024] = { 0 };
        strcpy(currentDir, GetWorkingDirectory());
        const char* workingDir = GetDirectoryPath(fileName);
        if (!ChangeDirectory(workingDir))
        {
            WARN("MODEL: [%s] Failed to change working directory", workingDir);
        }

        unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;
        int ret = tinyobj_parse_obj(&attrib, &meshes, &meshCount, &materials, &materialCount, fileText, dataSize, flags);

        if (ret != TINYOBJ_SUCCESS) WARN("MODEL: [%s] Failed to load OBJ data", fileName);
        else WARN("MODEL: [%s] OBJ data loaded successfully: %i meshes/%i materials", fileName, meshCount, materialCount);

        model.meshCount = materialCount;

        // Init model materials array
        if (materialCount > 0)
        {
            model.materialCount = materialCount;
            model.materials = (Material*)RL_CALLOC(model.materialCount, sizeof(Material));
            TraceLog(LOG_INFO, "MODEL: model has %i material meshes", materialCount);
        }
        else
        {
            model.meshCount = 1;
            TraceLog(LOG_INFO, "MODEL: No materials, putting all meshes in a default material");
        }

        model.meshes = (Mesh*)RL_CALLOC(model.meshCount, sizeof(Mesh));
        model.meshMaterial = (int*)RL_CALLOC(model.meshCount, sizeof(int));

        // Count the faces for each material
        int* matFaces = (int*) RL_CALLOC(model.meshCount, sizeof(int));

        // iff no materials are present use all faces on one mesh
        if (materialCount > 0)
        {
            for (unsigned int fi = 0; fi < attrib.num_faces; fi++)
            {
                //tinyobj_vertex_index_t face = attrib.faces[fi];
                int idx = attrib.material_ids[fi];
                matFaces[idx]++;
            }

        }
        else
        {
            matFaces[0] = attrib.num_faces;
        }

        //--------------------------------------
        // Create the material meshes

        // Running counts/indexes for each material mesh as we are
        // building them at the same time
        int* vCount = (int*) RL_CALLOC(model.meshCount, sizeof(int));
        int* vtCount = (int*) RL_CALLOC(model.meshCount, sizeof(int));
        int* vnCount = (int*) RL_CALLOC(model.meshCount, sizeof(int));
        int* faceCount = (int*) RL_CALLOC(model.meshCount, sizeof(int));

        // Allocate space for each of the material meshes
        for (int mi = 0; mi < model.meshCount; mi++)
        {
            model.meshes[mi].vertexCount = matFaces[mi] * 3;
            model.meshes[mi].triangleCount = matFaces[mi];
            model.meshes[mi].vertices = (float*)RL_CALLOC(model.meshes[mi].vertexCount * 3, sizeof(float));
            model.meshes[mi].texcoords = (float*)RL_CALLOC(model.meshes[mi].vertexCount * 2, sizeof(float));
            model.meshes[mi].normals = (float*)RL_CALLOC(model.meshes[mi].vertexCount * 3, sizeof(float));
            model.meshMaterial[mi] = mi;
        }

        // Scan through the combined sub meshes and pick out each material mesh
        for (unsigned int af = 0; af < attrib.num_faces; af++)
        {
            int mm = attrib.material_ids[af];   // mesh material for this face
            if (mm == -1) { mm = 0; }           // no material object..

            // Get indices for the face
            tinyobj_vertex_index_t idx0 = attrib.faces[3 * af + 0];
            tinyobj_vertex_index_t idx1 = attrib.faces[3 * af + 1];
            tinyobj_vertex_index_t idx2 = attrib.faces[3 * af + 2];

            // Fill vertices buffer (float) using vertex index of the face
            for (int v = 0; v < 3; v++) { model.meshes[mm].vertices[vCount[mm] + v] = attrib.vertices[idx0.v_idx * 3 + v]; } vCount[mm] += 3;
            for (int v = 0; v < 3; v++) { model.meshes[mm].vertices[vCount[mm] + v] = attrib.vertices[idx1.v_idx * 3 + v]; } vCount[mm] += 3;
            for (int v = 0; v < 3; v++) { model.meshes[mm].vertices[vCount[mm] + v] = attrib.vertices[idx2.v_idx * 3 + v]; } vCount[mm] += 3;

            if (attrib.num_texcoords > 0)
            {
                // Fill texcoords buffer (float) using vertex index of the face
                // NOTE: Y-coordinate must be flipped upside-down to account for
                // raylib's upside down textures...
                model.meshes[mm].texcoords[vtCount[mm] + 0] = attrib.texcoords[idx0.vt_idx * 2 + 0];
                model.meshes[mm].texcoords[vtCount[mm] + 1] = 1.0f - attrib.texcoords[idx0.vt_idx * 2 + 1]; vtCount[mm] += 2;
                model.meshes[mm].texcoords[vtCount[mm] + 0] = attrib.texcoords[idx1.vt_idx * 2 + 0];
                model.meshes[mm].texcoords[vtCount[mm] + 1] = 1.0f - attrib.texcoords[idx1.vt_idx * 2 + 1]; vtCount[mm] += 2;
                model.meshes[mm].texcoords[vtCount[mm] + 0] = attrib.texcoords[idx2.vt_idx * 2 + 0];
                model.meshes[mm].texcoords[vtCount[mm] + 1] = 1.0f - attrib.texcoords[idx2.vt_idx * 2 + 1]; vtCount[mm] += 2;
            }

            if (attrib.num_normals > 0)
            {
                // Fill normals buffer (float) using vertex index of the face
                for (int v = 0; v < 3; v++) { model.meshes[mm].normals[vnCount[mm] + v] = attrib.normals[idx0.vn_idx * 3 + v]; } vnCount[mm] += 3;
                for (int v = 0; v < 3; v++) { model.meshes[mm].normals[vnCount[mm] + v] = attrib.normals[idx1.vn_idx * 3 + v]; } vnCount[mm] += 3;
                for (int v = 0; v < 3; v++) { model.meshes[mm].normals[vnCount[mm] + v] = attrib.normals[idx2.vn_idx * 3 + v]; } vnCount[mm] += 3;
            }
        }

        // Init model materials
        for (unsigned int m = 0; m < materialCount; m++)
        {
            // Init material to default
            // NOTE: Uses default shader, which only supports MATERIAL_MAP_DIFFUSE
            model.materials[m] = LoadMaterialDefault();

            // Get default texture, in case no texture is defined
            // NOTE: rlgl default texture is a 1x1 pixel UNCOMPRESSED_R8G8B8A8
            model.materials[m].maps[MATERIAL_MAP_DIFFUSE].texture = { 0, 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };

            const char* diffuse = GetFileNameWithoutExt(materials[m].diffuse_texname);
            if (materials[m].diffuse_texname != NULL) model.materials[m].maps[MATERIAL_MAP_DIFFUSE].texture = RequestTexture(diffuse);  //char *diffuse_texname; // map_Kd

            model.materials[m].maps[MATERIAL_MAP_DIFFUSE].color = { (unsigned char)(materials[m].diffuse[0] * 255.0f), (unsigned char)(materials[m].diffuse[1] * 255.0f), (unsigned char)(materials[m].diffuse[2] * 255.0f), 255 }; //float diffuse[3];
            model.materials[m].maps[MATERIAL_MAP_DIFFUSE].value = 0.0f;

            const char* specular = GetFileNameWithoutExt(materials[m].specular_texname);
            if (materials[m].specular_texname != NULL) model.materials[m].maps[MATERIAL_MAP_SPECULAR].texture = RequestTexture(specular);  //char *specular_texname; // map_Ks
            model.materials[m].maps[MATERIAL_MAP_SPECULAR].color = { (unsigned char)(materials[m].specular[0] * 255.0f), (unsigned char)(materials[m].specular[1] * 255.0f), (unsigned char)(materials[m].specular[2] * 255.0f), 255 }; //float specular[3];
            model.materials[m].maps[MATERIAL_MAP_SPECULAR].value = 0.0f;

            const char* bump = GetFileNameWithoutExt(materials[m].bump_texname);
            if (materials[m].bump_texname != NULL) model.materials[m].maps[MATERIAL_MAP_NORMAL].texture = RequestTexture(bump);  //char *bump_texname; // map_bump, bump
            model.materials[m].maps[MATERIAL_MAP_NORMAL].color = WHITE;
            model.materials[m].maps[MATERIAL_MAP_NORMAL].value = materials[m].shininess;

            model.materials[m].maps[MATERIAL_MAP_EMISSION].color = { (unsigned char)(materials[m].emission[0] * 255.0f), (unsigned char)(materials[m].emission[1] * 255.0f), (unsigned char)(materials[m].emission[2] * 255.0f), 255 }; //float emission[3];

            const char* displace = GetFileNameWithoutExt(materials[m].displacement_texname);
            if (materials[m].displacement_texname != NULL) model.materials[m].maps[MATERIAL_MAP_HEIGHT].texture = RequestTexture(displace);  //char *displacement_texname; // disp
        }

        tinyobj_attrib_free(&attrib);
        tinyobj_shapes_free(meshes, meshCount);
        tinyobj_materials_free(materials, materialCount);

        RL_FREE(matFaces);
        RL_FREE(vCount);
        RL_FREE(vtCount);
        RL_FREE(vnCount);
        RL_FREE(faceCount);

        if (!ChangeDirectory(currentDir))
        {
            WARN("MODEL: [%s] Failed to change working directory", currentDir);
        }
        
        // UPLOAD TO GPU

        // Make sure model transform is set to identity matrix!
        model.transform = MatrixIdentity();

        if (model.meshCount == 0)
        {
            model.meshCount = 1;
            model.meshes = (Mesh*)RL_CALLOC(model.meshCount, sizeof(Mesh));
            WARN("MESH: [%s] Failed to load mesh data, default to cube mesh", fileName);
            model.meshes[0] = GenMeshCube(1.0f, 1.0f, 1.0f);
            WARN("MESH: [%s] Failed to load mesh data", fileName);
        }
        else
        {
            // Upload vertex data to GPU (static mesh)
            for (int i = 0; i < model.meshCount; i++) UploadMesh(&model.meshes[i], false);
        }

        if (model.materialCount == 0)
        {
            WARN("MATERIAL: [%s] Failed to load material data, default to white material", fileName);

            model.materialCount = 1;
            model.materials = (Material*)RL_CALLOC(model.materialCount, sizeof(Material));
            model.materials[0] = LoadMaterialDefault();

            if (model.meshMaterial == NULL) model.meshMaterial = (int*)RL_CALLOC(model.meshCount, sizeof(int));
        }
    }

    return model;
}
