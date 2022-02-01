#pragma once

#include <spine/spine.h>
#include <spine/Extension.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <stdio.h>
#include <vector>
#include <map>
#include <string>
#include "SOIL.h"
#include "entity.h"
#include "paintbrush.h"

GLuint Soil_Load_Texture(std::string filename);

class MyTextureLoader : public spine::TextureLoader
{
    // this should be fixed (memory thing)
    std::map<std::string, GLuint>  loaded_texture;

    virtual void load(spine::AtlasPage& page, const spine::String& path) {
        loaded_texture[path.buffer()] = PaintBrush::get_texture(path.buffer());

        void* texture = &loaded_texture[path.buffer()];
        page.setRendererObject(texture); // use the texture later in your rendering code
    }

    virtual void unload(void* texture) {
        //delete texture;
    }
};

class SpineManager
{
public:

    static std::map<std::string, spine::SkeletonData*> skeletonData;
    static spine::TextureLoader* textureLoader;
    static std::map <std::string, spine::Atlas*> atlas;
    static std::map<std::string, spine::AnimationStateData*> stateData;

    SpineManager();
    static t_VBO make_vbo(spine::Skeleton* skeleton);
    static void update_vbo(spine::Skeleton* skeleton, t_VBO* vbo, float y=0);
    static void reset_vbo(spine::Skeleton* skeleton, t_VBO* vbo);
    static void get_num_faces(spine::Skeleton* skeleton, t_VBO* vbo);

    static void LoadData(std::string spine_folder);
    static t_transform getAABB(spine::Skeleton* skeleton);
};