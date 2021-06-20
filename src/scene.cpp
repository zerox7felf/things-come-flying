// scene.cpp

#include <stdio.h>
#include <string.h>
#include <string>
#include <stdexcept>
#include <unordered_map>

#include "scene.hpp"
#include "engine.hpp"
#include "entity.hpp"
#include "memory.hpp"
#include "common.hpp"
#include "renderer.hpp"

#define SCENE_BUFFER_SIZE 255

u32 col = 0;
u32 row = 0;

Sun_light* sun_lights;
i32 num_sun_lights;

Point_light* point_lights;
i32 num_point_lights;

std::unordered_map<std::string, i32> mesh_names = {
    {"MESH_SPHERE",         MESH_SPHERE},   
	{"MESH_CUBE",           MESH_CUBE},
	{"MESH_GUY",            MESH_GUY}, 
    {"MESH_MONKE",          MESH_MONKE}, 
    {"MESH_MONKE_FLAT",     MESH_MONKE_FLAT}, 
    {"MESH_PLANE",          MESH_PLANE}, 
    {"MESH_BENT_PLANE",     MESH_BENT_PLANE}, 
    {"MESH_HOUSE",          MESH_HOUSE}, 
    {"MESH_QUAD",           MESH_QUAD}, 
    {"MESH_DESTROYER",      MESH_DESTROYER}, 
    {"MESH_SATURN_RINGS",   MESH_SATURN_RINGS}, 
    {"MESH_GROUND01",       MESH_GROUND01}, 
    {"MESH_GROUND01_WATER", MESH_GROUND01_WATER}, 
};

static u8 scene_get_mesh_id(char* mesh_name, u32 mesh_name_size, i32* mesh_id){
    try {
        *mesh_id = mesh_names.at(std::string(mesh_name, mesh_name_size));
        return 1;
    } catch (std::exception* e) {
        fprintf(stderr, "Invalid mesh id.\n");
        return 0;
    }
}

std::unordered_map<std::string, u32> texture_names = {
    {"TEXTURE_HOUSE",			TEXTURE_HOUSE},
    {"TEXTURE_HOUSE_SPECULAR",	TEXTURE_HOUSE_SPECULAR},
    {"TEXTURE_HOUSE_NORMAL",	TEXTURE_HOUSE_NORMAL},
    {"TEXTURE_HOUSE_NORMAL",	TEXTURE_HOUSE_NORMAL},
    {"TEXTURE_GROUND01",	    TEXTURE_GROUND01},
};

static u8 scene_get_texture_id(char* texture_name, u32 texture_name_size, u32* texture_id){
    try {
        *texture_id = texture_names.at(std::string(texture_name, texture_name_size));
        return 1;
    } catch (std::exception* e) {
        fprintf(stderr, "Invalid texture id.\n");
        return 0;
    }
}

std::unordered_map<std::string, u32> shader_names = {
    {"DIFFUSE_SHADER",              DIFFUSE_SHADER},
	{"SKYBOX_SHADER",               SKYBOX_SHADER},
	{"TEXTURE_SHADER",	            TEXTURE_SHADER},
	{"COMBINE_SHADER",	            COMBINE_SHADER},
	{"BLUR_SHADER",	                BLUR_SHADER},
    {"FLARE_SHADER",                FLARE_SHADER},
	{"BRIGHTNESS_EXTRACT_SHADER",	BRIGHTNESS_EXTRACT_SHADER},
	{"GROUND_SHADER",	            GROUND_SHADER},
};

static u8 scene_get_shader_id(char* shader_name, u32 shader_name_size, u32* shader_id){
    try {
        *shader_id = shader_names.at(std::string(shader_name, shader_name_size));
        return 1;
    } catch (std::exception* e) {
        fprintf(stderr, "Invalid shader id.\n");
        return 0;
    }
}

static void scene_print_pos() {
    fprintf(stderr, "At col %d row %d\n", col, row);
}

static u8 scene_parse_float(FILE* fp, float* result){
    char buffer[SCENE_BUFFER_SIZE] = "";
    u32 buffer_size = 0;

    char current = fgetc(fp);
	while (current != EOF) {
		switch (current) {
            case '\n':
                row++;
                col = 0;
                break;
            case ' ' :
            case '\t':
            case '\r':
                col++;
                break;
			case ';':
                try {
                    *result = std::stof(std::string(buffer, buffer_size));
					return 1;
                } catch (std::exception* e) {
                    fprintf(stderr, "Couldn't parse float.\n");
                    return 0;
                }
            default:
                buffer[buffer_size] = current;
                buffer_size++;
                break;
        }
        current = fgetc(fp);
	}
	fprintf(stderr, "Unexpected EOF when parsing float.\n");
	return 0;
}

static u8 scene_parse_v3(FILE* fp, Engine* engine, v3* result){
    char buffer[SCENE_BUFFER_SIZE] = "";
    u32 buffer_size = 0;
    u8 filled = 0;

    char current = fgetc(fp);
    while (filled != 3) {
        //printf("current:%c filled:%d\n", current, filled);
        switch (current) {
            case EOF:
                fprintf(stderr, "Unexpected EOF while parsing v3.\n");
                return 0;
            case '\n':
                row++;
                col = 0;
                break;
            case ' ' :
            case '\t':
            case '\r':
                col++;
                break;
            case ';':
                if (filled != 2) {
                    fprintf(stderr, "Unexpected ; in v3.\n");
                    return 0;
                }
            case ',': {
                float* component = filled == 0 ? &result->x : filled == 1 ? &result->y : &result->z;
                try {
                    *component = std::stof(std::string(buffer, buffer_size));
                } catch (std::exception* e) {
                    fprintf(stderr, "Couldn't parse float in v3.\n");
                    return 0;
                }
                filled++;
                buffer_size = 0;
                break;
            }
            default:
                buffer[buffer_size] = current;
                buffer_size++;
                break;
        }
        current = fgetc(fp);
    }
    return 1;
}

static u8 scene_get_name(FILE* fp, char* name, u32* name_size) {
    char buffer[SCENE_BUFFER_SIZE] = "";
    u32 buffer_size = 0;
    char current = fgetc(fp);
    while (current != EOF) {
        switch (current) {
            case '\n':
                row++;
                col = 0;
                break;
            case ' ' :
            case '\t':
            case '\r':
                col++;
                break;
            case ';':
                if (buffer_size == 0) {
                    fprintf(stderr, "Expected field name.\n");
                    return 0;
                }
                strncpy(name, buffer, buffer_size);
                *name_size = buffer_size;
                return 1;
            default:
                buffer[buffer_size] = current;
                buffer_size++;
                break;
        }
        current = fgetc(fp);
    }
    fprintf(stderr, "Unexpected EOF in field.\n");
    return 0;
}

std::unordered_map<std::string, Material*> scene_materials = {};
static u8 scene_parse_material(FILE* fp, Engine* engine) {
    char current = fgetc(fp);
    char buffer[SCENE_BUFFER_SIZE] = "";
    u32 buffer_size = 0;

	char material_name[SCENE_BUFFER_SIZE] = "";
	u32 material_name_size = 0;
    Material* material = (Material*)m_malloc(sizeof(Material));
    *material = (Material){
        .ambient    = {.value = {.constant = 0.5f}, .type = VALUE_MAP_CONST},
        .diffuse    = {.value = {.constant = 1.0f}, .type = VALUE_MAP_CONST},
        .specular   = {.value = {.constant = 1.0f}, .type = VALUE_MAP_CONST},
        .normal     = {.value = {.constant = 1.0f}, .type = VALUE_MAP_CONST},
        .shininess  = 10.0f,
        .color_map  = {.id = TEXTURE_MISSING},
        .texture1   = {}, .texture_mix = 0,
        .shader_index = DIFFUSE_SHADER
    };

    while (current != EOF) {
        switch (current) {
            case '\n':
                row++;
                col = 0;
                break;
            case ' ' :
            case '\t':
            case '\r':
                col++;
                break;
            case ':':
                if (strncmp("id", buffer, buffer_size) == 0) {
                    if (!scene_get_name(fp, material_name, &material_name_size)) return 0;
                } else if (strncmp("ambient_amp", buffer, buffer_size) == 0) {
					float ambient_amp = 0;
                    if (!scene_parse_float(fp, &ambient_amp)) return 0;
					material->ambient = {.value = {.constant = ambient_amp}, .type = VALUE_MAP_CONST};
                } else if (strncmp("ambient_map", buffer, buffer_size) == 0) {
                    char map_name[SCENE_BUFFER_SIZE] = "";
                    u32 map_name_size = 0;
                    u32 map_id = 0;
                    if (!scene_get_name(fp, map_name, &map_name_size)) return 0;
                    if (!scene_get_texture_id(map_name, map_name_size, &map_id)) return 0;
                    material->ambient = {.value = {.map = {.id = map_id}}, .type = VALUE_MAP_MAP};
                } else if (strncmp("diffuse_amp", buffer, buffer_size) == 0) {
					float diffuse_amp = 0;
                    if (!scene_parse_float(fp, &diffuse_amp)) return 0;
					material->diffuse = {.value = {.constant = diffuse_amp}, .type = VALUE_MAP_CONST};
                } else if (strncmp("diffuse_map", buffer, buffer_size) == 0) {
                    char map_name[SCENE_BUFFER_SIZE] = "";
                    u32 map_name_size = 0;
                    u32 map_id = 0;
                    if (!scene_get_name(fp, map_name, &map_name_size)) return 0;
                    if (!scene_get_texture_id(map_name, map_name_size, &map_id)) return 0;
                    material->diffuse = {.value = {.map = {.id = map_id}}, .type = VALUE_MAP_MAP};
                } else if (strncmp("specular_amp", buffer, buffer_size) == 0) {
					float specular_amp = 0;
                    if (!scene_parse_float(fp, &specular_amp)) return 0;
					material->specular = {.value = {.constant = specular_amp}, .type = VALUE_MAP_CONST};
                } else if (strncmp("specular_map", buffer, buffer_size) == 0) {
                    char map_name[SCENE_BUFFER_SIZE] = "";
                    u32 map_name_size = 0;
                    u32 map_id = 0;
                    if (!scene_get_name(fp, map_name, &map_name_size)) return 0;
                    if (!scene_get_texture_id(map_name, map_name_size, &map_id)) return 0;
                    material->specular = {.value = {.map = {.id = map_id}}, .type = VALUE_MAP_MAP};
                } else if (strncmp("normal_map", buffer, buffer_size) == 0) {
                    char map_name[SCENE_BUFFER_SIZE] = "";
                    u32 map_name_size = 0;
                    u32 map_id = 0;
                    if (!scene_get_name(fp, map_name, &map_name_size)) return 0;
                    if (!scene_get_texture_id(map_name, map_name_size, &map_id)) return 0;
                    material->normal = {.value = {.map = {.id = map_id}}, .type = VALUE_MAP_MAP};
                } else if (strncmp("shininess", buffer, buffer_size) == 0) {
					float shininess = 0;
                    if (!scene_parse_float(fp, &shininess)) return 0;
					material->shininess = shininess;
                } else if (strncmp("color_map", buffer, buffer_size) == 0) {
                    char map_name[SCENE_BUFFER_SIZE] = "";
                    u32 map_name_size = 0;
                    u32 map_id = 0;
                    if (!scene_get_name(fp, map_name, &map_name_size)) return 0;
                    if (!scene_get_texture_id(map_name, map_name_size, &map_id)) return 0;
                    material->color_map  = {.id = map_id};
                } else if (strncmp("shader", buffer, buffer_size) == 0) {
                    char shader_name[SCENE_BUFFER_SIZE] = "";
                    u32 shader_name_size = 0;
                    u32 shader_id = 0;
                    if (!scene_get_name(fp, shader_name, &shader_name_size)) return 0;
                    if (!scene_get_shader_id(shader_name, shader_name_size, &shader_id)) return 0;
                    material->shader_index = shader_id;
                }
                buffer_size = 0;
                break;
            case '}':
                if (buffer_size != 0) {
                    fprintf(stderr, "Unexpected }.\n");
                    return 0;
                }
                if (material_name_size == 0) {
                    fprintf(stderr, "Material missing id field.\n");
                    return 0;
                }
                scene_materials[std::string(material_name, material_name_size)] = material;
                return 1;
            default:
                buffer[buffer_size] = current;
                buffer_size++;
                break;
        }
        current = fgetc(fp);
    }
    fprintf(stderr, "Unexpected EOF while parsing entity.\n");
    return 0;
}

std::unordered_map<std::string, Entity_update> entity_updates = {
    {"HOUSE_UPDATE", [](Entity* entity, struct Engine* engine){
		entity->rotation = V3(
			entity->rotation.x,
			fmodf(engine->total_time * entity->angular_speed, 360),
			entity->rotation.x
		);
    }}
};
std::unordered_map<std::string, Entity*> entity_ids = {};
static u8 scene_parse_entity(FILE* fp, Engine* engine) {
    char current = fgetc(fp);
    char buffer[SCENE_BUFFER_SIZE] = "";
    u32 buffer_size = 0;

    Entity* entity = engine_push_empty_entity(engine);
    entity_initialize(entity, V3(0,0,0), V3(1,1,1), V3(0,0,0), V3(0,0,0), NULL, MESH_HOUSE, NULL, NULL);
    entity->material = (Material){
        .ambient    = {.value = {.constant = 0.5f}, .type = VALUE_MAP_CONST},
        .diffuse    = {.value = {.constant = 1.0f}, .type = VALUE_MAP_CONST},
        .specular   = {.value = {.map = {.id = TEXTURE_HOUSE_SPECULAR}}, .type = VALUE_MAP_MAP},
        .normal     = {.value = {.map = {.id = TEXTURE_HOUSE_NORMAL}}, .type = VALUE_MAP_MAP},
        .shininess  = 10.0f,
        .color_map  = {.id = TEXTURE_HOUSE},
        .texture1   = {}, .texture_mix = 0,
        .shader_index = DIFFUSE_SHADER
    };

    while (current != EOF) {
        switch (current) {
            case '\n':
                row++;
                col = 0;
                break;
            case ' ' :
            case '\t':
            case '\r':
                col++;
                break;
            case ':':
                if (strncmp("id", buffer, buffer_size) == 0) {
                    char entity_name[SCENE_BUFFER_SIZE] = "";
                    u32 entity_name_size = 0;
                    if (!scene_get_name(fp, entity_name, &entity_name_size)) return 0;
                    entity_ids[std::string(entity_name, entity_name_size)] = entity; // TODO: disallow overwrites (or do we give a shit? atm, no)
                } else if (strncmp("pos", buffer, buffer_size) == 0) {
                    if (!scene_parse_v3(fp, engine, &entity->position)) return 0;
                } else if (strncmp("scale", buffer, buffer_size) == 0) {
                    if (!scene_parse_v3(fp, engine, &entity->size)) return 0;
                } else if (strncmp("rot", buffer, buffer_size) == 0) {
                    if (!scene_parse_v3(fp, engine, &entity->rotation)) return 0;
                } else if (strncmp("pivot", buffer, buffer_size) == 0) {
                    if (!scene_parse_v3(fp, engine, &entity->rotation_pivot)) return 0;
                } else if (strncmp("update", buffer, buffer_size) == 0) {
                    char update_name[SCENE_BUFFER_SIZE] = "";
                    u32 update_name_size = 0;
                    if (!scene_get_name(fp, update_name, &update_name_size)) return 0;
                    try {
                        entity->update = entity_updates.at(std::string(update_name, update_name_size));
                    } catch (std::exception* e) {
                        fprintf(stderr, "Invalid update function id.\n");
                        return 0;
                    }
                } else if (strncmp("material", buffer, buffer_size) == 0) {
                    char material_name[SCENE_BUFFER_SIZE] = "";
                    u32 material_name_size = 0;
                    if (!scene_get_name(fp, material_name, &material_name_size)) return 0;
                    try {
                        entity->material = *scene_materials.at(std::string(material_name, material_name_size));
                    } catch (std::exception* e) {
                        fprintf(stderr, "Invalid material id.\n");
                        return 0;
                    }
                } else if (strncmp("mesh", buffer, buffer_size) == 0) {
                    char mesh_name[SCENE_BUFFER_SIZE] = "";
                    u32 mesh_name_size = 0;
                    if (!scene_get_name(fp, mesh_name, &mesh_name_size)) return 0;
                    if (!scene_get_mesh_id(mesh_name, mesh_name_size, &entity->mesh_id)) return 0;
                } else if (strncmp("parent", buffer, buffer_size) == 0) {
                    char entity_name[SCENE_BUFFER_SIZE] = "";
                    u32 entity_name_size = 0;
                    if (!scene_get_name(fp, entity_name, &entity_name_size)) return 0;
                    try {
                        entity->parent = entity_ids.at(std::string(entity_name, entity_name_size));
                    } catch (std::exception* e) {
                        fprintf(stderr, "Invalid entity id.\n");
                        return 0;
                    }
                } else if (strncmp("following", buffer, buffer_size) == 0) {
                    char entity_name[SCENE_BUFFER_SIZE] = "";
                    u32 entity_name_size = 0;
                    if (!scene_get_name(fp, entity_name, &entity_name_size)) return 0;
                    try {
                        entity->following = entity_ids.at(std::string(entity_name, entity_name_size));
                    } catch (std::exception* e) {
                        fprintf(stderr, "Invalid entity id.\n");
                        return 0;
                    }
                }
                buffer_size = 0;
                break;
            case '}':
                if (buffer_size != 0) {
                    fprintf(stderr, "Unexpected }.\n");
                    return 0;
                }
                return 1;
            default:
                buffer[buffer_size] = current;
                buffer_size++;
                break;
        }
        current = fgetc(fp);
    }
    fprintf(stderr, "Unexpected EOF while parsing entity.\n");
    return 0;
}

static u8 scene_parse_light_sun(FILE* fp, Engine* engine) {
    char current = fgetc(fp);
    char buffer[SCENE_BUFFER_SIZE] = "";
    u32 buffer_size = 0;

    Sun_light light = {
        .angle = V3(1, 0, 0),
        .color = V3(1, 1, 1),
        .ambient = 1.0f,
        .falloff_linear = 0,
        .falloff_quadratic = 0
    };

    while (current != EOF) {
        switch (current) {
            case '\n':
                row++;
                col = 0;
                break;
            case ' ' :
            case '\t':
            case '\r':
                col++;
                break;
            case ':':
                if (strncmp("angle", buffer, buffer_size) == 0) {
                    if (!scene_parse_v3(fp, engine, &light.angle)) return 0;
                } else if (strncmp("color", buffer, buffer_size) == 0) {
                    if (!scene_parse_v3(fp, engine, &light.color)) return 0;
                } else if (strncmp("ambient", buffer, buffer_size) == 0) {
                    if (!scene_parse_float(fp, &light.ambient)) return 0;
                }
                buffer_size = 0;
                break;
            case '}':
                if (buffer_size != 0) {
                    fprintf(stderr, "Unexpected }.\n");
                    return 0;
                }
                list_push(sun_lights, num_sun_lights, light);
                return 1;
            default:
                buffer[buffer_size] = current;
                buffer_size++;
                break;
        }
        current = fgetc(fp);
    }
    fprintf(stderr, "Unexpected EOF while parsing sun light.\n");
    return 0;
}

static u8 scene_parse_light_point(FILE* fp, Engine* engine) {
    //TODO
    return 0;
}

//TODO: we leak memory when returning error status
u8 initialize_scene(Engine* engine, char* scene_path) {
    FILE* fp = fopen(scene_path, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error, couldn't load scene file %s\n", scene_path);
        return 0;
    }

    char current = fgetc(fp);
    char buffer[SCENE_BUFFER_SIZE] = "";
    u32 buffer_size = 0;

    sun_lights = NULL;
    num_sun_lights = 0;
    point_lights = NULL;
    num_point_lights = 0;

    while (current != EOF) {
        switch (current) {
            case '\n':
                row++;
                col = 0;
                break;
            case ' ' :
            case '\t':
            case '\r':
                col++;
                break;
            case '{':
                if (strncmp("Material", buffer, buffer_size) == 0) {
                    if (!scene_parse_material(fp, engine)) {
                        scene_print_pos();
                        fclose(fp);
                        return 0;
                    }
                } else if (strncmp("Entity", buffer, buffer_size) == 0) {
                    if (!scene_parse_entity(fp, engine)) {
                        scene_print_pos();
                        fclose(fp);
                        return 0;
                    }
                } else if (strncmp("Light_Point", buffer, buffer_size) == 0) {
                    if (!scene_parse_light_point(fp, engine)) {
                        scene_print_pos();
                        fclose(fp);
                        return 0;
                    }
                } else if (strncmp("Light_Sun", buffer, buffer_size) == 0) {
                    if (!scene_parse_light_sun(fp, engine)) {
                        scene_print_pos();
                        fclose(fp);
                        return 0;
                    }
                } else {
                    fprintf(stderr, "Invalid entry type in scene file\n");
                    scene_print_pos();
                    fclose(fp);
                    return 0;
                }
                buffer_size = 0;
                break;
            default:
                buffer[buffer_size] = current;
                buffer_size++;
                break;
        }
        current = fgetc(fp);
    }
    if (buffer_size != 0) {
        fprintf(stderr, "Unexpected EOF.\n");
        scene_print_pos();
        return 0;
    }

    fclose(fp);

    engine->scene = (Scene) {
        .lights = point_lights,
        .num_lights = num_point_lights,
        .sun_lights = sun_lights,
        .num_sun_lights = num_sun_lights
    };

    // Since we copy material contents over to the entities we can free them now.
    for (const std::pair<std::string, Material*> item : scene_materials) {
        m_free(item.second, sizeof(Material));
    }
    return 1;
}
