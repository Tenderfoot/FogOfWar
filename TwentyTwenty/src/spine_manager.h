#pragma once

#include <spine/spine.h>
#include <spine/Extension.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <stdio.h>
#include <vector>
#include <string>
#include "SOIL.h"
#include "entity.h"
#include "paintbrush.h"

GLuint Soil_Load_Texture(std::string filename);

class MyTextureLoader : public spine::TextureLoader
{
    // this should be fixed (memory thing)
    GLuint loaded_texture;

    virtual void load(spine::AtlasPage& page, const spine::String& path) {
        loaded_texture = PaintBrush::Soil_Load_Texture(path.buffer());

        void* texture = &loaded_texture;
        page.setRendererObject(texture); // use the texture later in your rendering code
    }

    virtual void unload(void* texture) {
        //delete texture;
    }
};

class SpineManager
{
public:

    static spine::SkeletonData* skeletonData;
    static spine::TextureLoader* textureLoader;
    static spine::Atlas* atlas;
    static spine::AnimationStateData* stateData; 
    static t_VBO *vertexbuffer;

    SpineManager();
    static void LoadData();
    static void drawSkeleton(spine::Skeleton* skeleton);
    static t_transform getAABB(spine::Skeleton* skeleton);

};