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
    // this should be fixed (memory thing)
    GLuint loaded_texture;

    virtual void load(spine::AtlasPage& page, const spine::String& path) {
        loaded_texture = Soil_Load_Texture(path.buffer());

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
        animationState = nullptr;
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
        skeleton->setSkin("witch");
        
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
        GLuint* texture = nullptr;

        skeleton->updateWorldTransform();

        // For each slot in the draw order array of the skeleton
        for (size_t i = 0, n = skeleton->getSlots().size(); i < n; ++i) {
            spine::Slot* slot = skeleton->getDrawOrder()[i];

            spine::Attachment* attachment = slot->getAttachment();
            if (!attachment) continue;

            if (attachment->getRTTI().isExactly(spine::RegionAttachment::rtti)) {
                spine::RegionAttachment* regionAttachment = (spine::RegionAttachment*)attachment;

                worldVertices.setSize(8, 0);
                regionAttachment->computeWorldVertices(slot->getBone(), worldVertices, 0, 2);
                verticesCount = 4;
                uvs = &regionAttachment->getUVs();
                //indices = &quadIndices;
                indicesCount = 6;
                texture = (GLuint*)((spine::AtlasRegion*)regionAttachment->getRendererObject())->page->getRendererObject();

                glBegin(GL_TRIANGLES);
                for (size_t j = 0, l = 0; j < worldVertices.size(); j += 2, l += 2) {
                    for (int ii = 0; ii < indicesCount; ++ii) {
                        int index = quadIndices[ii] << 1;
                        glTexCoord2f((*uvs)[index], (*uvs)[index + 1]);
                        glVertex3f((*vertices)[index], (*vertices)[index + 1], -20.0f);
                    }
                }
                glEnd();
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

                // This draw section should be removed and these should be batched and drawn as an arraylist
                if (texture != nullptr)
                    glBindTexture(GL_TEXTURE_2D, *texture);

                glBegin(GL_TRIANGLES);
                for (size_t j = 0, l = 0; j < worldVertices.size(); j += 2, l += 2) {
                    for (int ii = 0; ii < indicesCount; ++ii) {
                        int index = (*indices)[ii] << 1;
                        glTexCoord2f((*uvs)[index], (*uvs)[index + 1]);
                        glVertex3f((*vertices)[index], (*vertices)[index + 1], -20.0f);
                    }
                }
                glEnd();
            } 
        }
    }

};