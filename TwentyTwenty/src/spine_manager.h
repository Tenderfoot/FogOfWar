#pragma once

#include <spine/spine.h>
#include <spine/Extension.h>
#include <SDL_opengl.h>
#include <stdio.h>
#include <vector>
#include <string>
#include "SOIL.h"

GLuint Soil_Load_Texture(std::string filename);

struct Vertex {
    // Position in x/y plane
    float x, y;

    // UV coordinates
    float u, v;

    // Color, each channel in the range from 0-1
    // (Should really be a 32-bit RGBA packed color)
    spine::Color color;
};

spine::SpineExtension *spine::getDefaultExtension() {
    return new spine::DefaultSpineExtension();
}

class MyTextureLoader : public spine::TextureLoader
{
    virtual void load(spine::AtlasPage& page, const spine::String& path) {
        GLuint loaded_texture;

        //loaded_texture = Soil_Load_Texture("data/skeleton.png");

        void* texture = &loaded_texture;
        page.setRendererObject(texture); // use the texture later in your rendering code
        //page.width = 1880;
        //page.height = 806;
    }

    virtual void unload(void* texture) {
        //delete texture;
    }
};

class SpineManager
{
public:

    spine::SkeletonData* skeletonData;
    spine::TextureLoader* textureLoader;
    spine::Atlas* atlas;
    spine::Skeleton* skeleton;
    spine::AnimationState* animationState;

    SpineManager()
    {
        skeletonData = nullptr;
        textureLoader = nullptr;
        atlas = nullptr;
        skeleton = nullptr;
    }

    void LoadData()
    {
        textureLoader = new MyTextureLoader();
        atlas = new spine::Atlas("data/spine/skeleton.atlas", textureLoader);

        // Create a SkeletonJson used for loading and set the scale
        // to make the loaded data two times as big as the original data
        spine::SkeletonJson json(atlas);
        json.setScale(0.01);

        // Load the skeleton .json file into a SkeletonData
        skeletonData = json.readSkeletonDataFile("data/spine/skeleton.json");
        skeleton = new spine::Skeleton(skeletonData);

        skeleton->setToSetupPose();
        skeleton->updateWorldTransform();
        skeleton->setSkin("cheer");
        
        spine::AnimationStateData *stateData = new spine::AnimationStateData(skeletonData);
        animationState = new spine::AnimationState(stateData);
        animationState->addAnimation(0, "walk_two", true, 0);

        // If loading failed, print the error and exit the app
        if (!skeletonData) {
            printf("%s\n", json.getError().buffer());
            exit(0);
        }
    }

    void updateSkeleton(float deltatime)
    {
        animationState->update(deltatime);
        animationState->apply(*skeleton);
    }

    mutable spine::Vector<float> worldVertices;

    void drawSkeleton() {

        unsigned short quadIndices[] = { 0, 1, 2, 2, 3, 0 };
        spine::Vector<float> *vertices = &worldVertices;
        spine::Vector<float> *uvs = NULL;
        spine::Vector<unsigned short> *indices = NULL;
        int indicesCount = 0;
        int verticesCount = 0;

        skeleton->updateWorldTransform();

        // For each slot in the draw order array of the skeleton
        for (size_t i = 0, n = skeleton->getSlots().size(); i < n; ++i) {
            spine::Slot* slot = skeleton->getDrawOrder()[i];

            spine::Attachment* attachment = slot->getAttachment();
            if (!attachment) continue;

            GLuint* texture = NULL;

            if (attachment->getRTTI().isExactly(spine::RegionAttachment::rtti)) {
            }
            else if (attachment->getRTTI().isExactly(spine::MeshAttachment::rtti)) {
                spine::MeshAttachment* mesh = (spine::MeshAttachment*)attachment;
                worldVertices.setSize(mesh->getWorldVerticesLength(), 0);
                texture = (GLuint*)((spine::AtlasRegion*)mesh->getRendererObject())->page->getRendererObject();
                mesh->computeWorldVertices(*slot, 0, mesh->getWorldVerticesLength(), worldVertices, 0, 2);
                verticesCount = mesh->getWorldVerticesLength() >> 1;
                uvs = &mesh->getUVs();
                indices = &mesh->getTriangles();
                indicesCount = mesh->getTriangles().size();
            }

            glBegin(GL_TRIANGLES);
            for (size_t j = 0, l = 0; j < worldVertices.size(); j+=2, l += 2) {
                for (int ii = 0; ii < indicesCount; ++ii) {
                    int index = (*indices)[ii] << 1;
                    glTexCoord2f((*uvs)[index], (*uvs)[index + 1]);
                    glVertex3f((*vertices)[index], (*vertices)[index + 1], -10.0f);
                }
            }
            glEnd();

        }
    }

};