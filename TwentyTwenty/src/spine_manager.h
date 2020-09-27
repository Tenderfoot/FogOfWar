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

        loaded_texture = Soil_Load_Texture("data/skeleton.png");

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

        // If loading failed, print the error and exit the app
        if (!skeletonData) {
            printf("%s\n", json.getError().buffer());
            exit(0);
        }
    }

    mutable spine::Vector<float> worldVertices;

    void drawSkeleton() {

        printf("In Draw\n");
        unsigned short quadIndices[] = { 0, 1, 2, 2, 3, 0 };
        spine::Vector<float> *vertices = &worldVertices;
        spine::Vector<float>* uvs = NULL;
        spine::Vector<unsigned short> *indices = NULL;
        int indicesCount = 0;
        int verticesCount = 0;

        // For each slot in the draw order array of the skeleton
        for (size_t i = 0, n = skeleton->getSlots().size(); i < n; ++i) {
            spine::Slot* slot = skeleton->getDrawOrder()[i];

            // Fetch the currently active attachment, continue
            // with the next slot in the draw order if no
            // attachment is active on the slot
            spine::Attachment* attachment = slot->getAttachment();
            if (!attachment) continue;

            // Fill the vertices array, indices, and texture depending on the type of attachment
            GLuint* texture = NULL;
            unsigned short* indices = NULL;
            if (attachment->getRTTI().isExactly(spine::RegionAttachment::rtti)) {
                printf("drawing region\n");
            }
            else if (attachment->getRTTI().isExactly(spine::MeshAttachment::rtti)) {

                printf("drawing mesh\n");
   
                spine::MeshAttachment* mesh = (spine::MeshAttachment*)attachment;
                
                worldVertices.setSize(mesh->getWorldVerticesLength(), 0);
                texture = (GLuint*)((spine::AtlasRegion*)mesh->getRendererObject())->page->getRendererObject();
                mesh->computeWorldVertices(*slot, 0, mesh->getWorldVerticesLength(), worldVertices, 0, 2);
                verticesCount = mesh->getWorldVerticesLength() >> 1;
                uvs = &mesh->getUVs();
                //indices = &mesh->getTriangles();
                indicesCount = mesh->getTriangles().size();


            }
            // Draw the mesh we created for the attachment
            //engine_drawMesh(vertices, 0, vertexIndex, texture, engineBlendMode);

            glBegin(GL_TRIANGLES);
            for (size_t j = 0, l = 0; j < worldVertices.size()/2; j+=2, l += 2) {
                printf("vertex %d: %f, %f\n", j, worldVertices[j], worldVertices[j+1]);
                glVertex3f(worldVertices[j], worldVertices[j+1], -10.0f);
            }
            glEnd();

        }
    }

};