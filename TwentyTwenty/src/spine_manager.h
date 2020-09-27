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

    SpineManager()
    {
        skeletonData = nullptr;
        textureLoader = nullptr;
        atlas = nullptr;
    }

    void LoadData()
    {
        textureLoader = new MyTextureLoader();
        atlas = new spine::Atlas("data/spine/skeleton.atlas", textureLoader);

        // Create a SkeletonJson used for loading and set the scale
        // to make the loaded data two times as big as the original data
        spine::SkeletonJson json(atlas);
        json.setScale(1);

        // Load the skeleton .json file into a SkeletonData
        skeletonData = json.readSkeletonDataFile("data/spine/skeleton.json");

        // If loading failed, print the error and exit the app
        if (!skeletonData) {
            printf("%s\n", json.getError().buffer());
            exit(0);
        }
    }

    void drawSkeleton() {

        printf("%d skins\n", skeletonData->getSkins().size());

        spine::Skeleton* skeleton = new spine::Skeleton(skeletonData);
        skeleton->updateWorldTransform();
        skeleton->setSkin("cheer");

        printf("In Draw\n");
        spine::Vector<Vertex> vertices;
        spine::Vector<float> vertbuffer; // <- added by me
        unsigned short quadIndices[] = { 0, 1, 2, 2, 3, 0 };

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
                // Cast to an spRegionAttachment so we can get the rendererObject
                // and compute the world vertices
                spine::RegionAttachment* regionAttachment = (spine::RegionAttachment*)attachment;

                // Our engine specific Texture is stored in the AtlasRegion which was
                // assigned to the attachment on load. It represents the texture atlas
                // page that contains the image the region attachment is mapped to.
                texture = (GLuint*)((spine::AtlasRegion*)regionAttachment->getRendererObject())->page->getRendererObject();

                // Ensure there is enough room for vertices
                vertices.setSize(4, Vertex());
                vertbuffer.setSize(4*8, float());
                // Computed the world vertices positions for the 4 vertices that make up
                // the rectangular region attachment. This assumes the world transform of the
                // bone to which the slot (and hence attachment) is attached has been calculated
                // before rendering via Skeleton::updateWorldTransform(). The vertex positions
                // will be written directoy into the vertices array, with a stride of sizeof(Vertex)
                
                // This is currently the WIP
                //regionAttachment->computeWorldVertices(slot->getBone(), vertbuffer, 0, sizeof(Vertex));

                // copy color and UVs to the vertices
                for (size_t j = 0, l = 0; j < 4; j++, l += 2) {
                    Vertex& vertex = vertices[j];
                    vertex.u = regionAttachment->getUVs()[l];
                    vertex.v = regionAttachment->getUVs()[l + 1];
                }

                // set the indices, 2 triangles forming a quad
                indices = quadIndices;
            }
            else if (attachment->getRTTI().isExactly(spine::MeshAttachment::rtti)) {

                printf("drawing mesh\n");
                // Cast to an MeshAttachment so we can get the rendererObject
                // and compute the world vertices
                spine::MeshAttachment* mesh = (spine::MeshAttachment*)attachment;

                // Ensure there is enough room for vertices
                vertices.setSize(mesh->getWorldVerticesLength() / 2, Vertex());

                // Our engine specific Texture is stored in the AtlasRegion which was
                // assigned to the attachment on load. It represents the texture atlas
                // page that contains the image the region attachment is mapped to.
                texture = (GLuint*)((spine::AtlasRegion*)mesh->getRendererObject())->page->getRendererObject();

                // Computed the world vertices positions for the vertices that make up
                // the mesh attachment. This assumes the world transform of the
                // bone to which the slot (and hence attachment) is attached has been calculated
                // before rendering via Skeleton::updateWorldTransform(). The vertex positions will
                // be written directly into the vertices array, with a stride of sizeof(Vertex)
                size_t numVertices = mesh->getWorldVerticesLength() / 2;
                vertbuffer.setSize((mesh->getWorldVerticesLength() / 2)*8, float());

                // This is currently the WIP
                //mesh->computeWorldVertices(*slot, size_t(0), numVertices, vertbuffer, size_t(0), sizeof(Vertex));

                // Copy color and UVs to the vertices
                for (size_t j = 0, l = 0; j < numVertices; j++, l += 2) {
                    Vertex& vertex = vertices[j];
                    vertex.u = mesh->getUVs()[l];
                    vertex.v = mesh->getUVs()[l + 1];
                }

                // set the indices, 2 triangles forming a quad
                indices = quadIndices;
            }
            // Draw the mesh we created for the attachment
            //engine_drawMesh(vertices, 0, vertexIndex, texture, engineBlendMode);
        }
    }

};