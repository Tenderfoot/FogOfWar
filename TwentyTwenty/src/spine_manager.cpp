
#include "spine_manager.h"

std::map<std::string, spine::SkeletonData*> SpineManager::skeletonData;
spine::TextureLoader* SpineManager::textureLoader = new MyTextureLoader();
std::map <std::string, spine::Atlas*> SpineManager::atlas;
std::map <std::string, spine::AnimationStateData*> SpineManager::stateData;

spine::SpineExtension* spine::getDefaultExtension() {
    return new spine::DefaultSpineExtension();
}

SpineManager::SpineManager()
{
}

void SpineManager::LoadData(std::string spine_folder)
{
    std::map<std::string, spine::SkeletonData*>::iterator it;
    it = skeletonData.find(spine_folder.c_str());
    if (it == skeletonData.end())
    {
        std::string atlas_path = std::string("data/").append(spine_folder).append("/skeleton.atlas");
        atlas[spine_folder] = new spine::Atlas(atlas_path.c_str(), textureLoader);

        // Create a SkeletonJson used for loading and set the scale
        // to make the loaded data two times as big as the original data
        spine::SkeletonJson json(atlas[spine_folder]);

        // fix this please
        if(strcmp(spine_folder.c_str(), "spine") == 0)
            json.setScale(0.002);
        if (strcmp(spine_folder.c_str(), "buildings") == 0)
            json.setScale(0.02);

        // Load the skeleton .json file into a SkeletonData
        std::string skeleton_path = std::string("data/").append(spine_folder).append("/skeleton.json");
        skeletonData[spine_folder] = json.readSkeletonDataFile(skeleton_path.c_str());
        stateData[spine_folder] = new spine::AnimationStateData(skeletonData[spine_folder]);
        stateData[spine_folder]->setDefaultMix(0.2);
        // If loading failed, print the error and exit the app

        if (!skeletonData[spine_folder]) {
            printf("%s\n", json.getError().buffer());
            exit(0);
        }
    }
}
// stuff for VBOs...
PFNGLGENBUFFERSARBPROC      glGenBuffersARB = NULL;
PFNGLBUFFERDATAARBPROC      glBufferDataARB = NULL;
PFNGLBINDBUFFERARBPROC      glBindBufferARB = NULL;

t_VBO SpineManager::make_vbo(spine::Skeleton* skeleton)
{
    glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)
        uglGetProcAddress("glGenBuffersARB");
    glBufferDataARB = (PFNGLBUFFERDATAARBPROC)
        uglGetProcAddress("glBufferDataARB");
    glBindBufferARB = (PFNGLBINDBUFFERARBPROC)
        uglGetProcAddress("glBindBufferARB");

    spine::Vector<float> worldVertices;
    unsigned short quadIndices[] = { 0, 1, 2, 2, 3, 0 };
    spine::Vector<float>* vertices = &worldVertices;
    spine::Vector<float>* uvs = NULL;
    spine::Vector<unsigned short>* indices = NULL;
    int indicesCount = 0;
    int verticesCount = 0;
    GLuint* texture = nullptr;

    t_VBO new_vbo;

    int num_verts=0;

    // For each slot in the draw order array of the skeleton
    for (size_t i = 0, n = skeleton->getSlots().size(); i < n; ++i) {
        spine::Slot* slot = skeleton->getDrawOrder()[i];

        spine::Attachment* attachment = slot->getAttachment();
        if (!attachment) continue;


        if (attachment->getRTTI().isExactly(spine::MeshAttachment::rtti)) {
            spine::MeshAttachment* mesh = (spine::MeshAttachment*)attachment;
            worldVertices.setSize(mesh->getWorldVerticesLength(), 0);
            mesh->computeWorldVertices(*slot, 0, mesh->getWorldVerticesLength(), worldVertices, 0, 2);

            texture = (GLuint*)((spine::AtlasRegion*)mesh->getRendererObject())->page->getRendererObject();
            new_vbo.texture = *texture;

            num_verts += worldVertices.size();
        }
    }

    new_vbo.num_faces = num_verts / 3;
    new_vbo.verticies = new float[new_vbo.num_faces * 3 * 3];
    new_vbo.colors = new float[new_vbo.num_faces * 3 * 3];
    new_vbo.texcoords = new float[new_vbo.num_faces * 3 * 2];

    int vert_index = 0;

    // For each slot in the draw order array of the skeleton
    for (size_t i = 0, n = skeleton->getSlots().size(); i < n; ++i) {
        spine::Slot* slot = skeleton->getDrawOrder()[i];

        spine::Attachment* attachment = slot->getAttachment();
        if (!attachment) continue;


        if (attachment->getRTTI().isExactly(spine::MeshAttachment::rtti)) {
            spine::MeshAttachment* mesh = (spine::MeshAttachment*)attachment;

            worldVertices.setSize(mesh->getWorldVerticesLength(), 0);
            texture = (GLuint*)((spine::AtlasRegion*)mesh->getRendererObject())->page->getRendererObject();
            mesh->computeWorldVertices(*slot, 0, mesh->getWorldVerticesLength(), worldVertices, 0, 2);
            verticesCount = mesh->getWorldVerticesLength() >> 1;
            uvs = &mesh->getUVs();
            indices = &mesh->getTriangles();
            indicesCount = mesh->getTriangles().size();

            // This draw section should be removed and these should be batched and drawn as an arraylist

            for (size_t j = 0, l = 0; j < worldVertices.size(); j += 2, l += 2) {
                for (int ii = 0; ii < indicesCount; ++ii) {
                    int index = (*indices)[ii] << 1;

                    new_vbo.verticies[index + vert_index] = (*vertices)[index];
                    new_vbo.verticies[index + 1 + vert_index] = (*vertices)[index+1];
                    new_vbo.verticies[index + 2 + vert_index] = 0.0f;
                    new_vbo.texcoords[index + vert_index] = (*uvs)[index];
                    new_vbo.texcoords[index+1 + vert_index] = (*uvs)[index+1];
                    new_vbo.colors[index + vert_index] = 1.0f;
                    new_vbo.colors[index+1 + vert_index] = 1.0f;
                    new_vbo.colors[index+2 + vert_index] = 1.0f;
                }
            }
        }
        vert_index += worldVertices.size();
    }


    glGenBuffersARB(1, &new_vbo.vertex_buffer);
    glBindBufferARB(GL_ARRAY_BUFFER, new_vbo.vertex_buffer);
    glBufferDataARB(GL_ARRAY_BUFFER, sizeof(float) * new_vbo.num_faces * 3 * 3, new_vbo.verticies,
        GL_STATIC_DRAW);
    //tri_vbo_elem = sizeof(new_vbo.verticies) / sizeof(*new_vbo.verticies) / 3;
    glBindBufferARB(GL_ARRAY_BUFFER, 0);

    //Make the new VBO active
    glGenBuffersARB(1, &new_vbo.texcoord_buffer);
    glBindBufferARB(GL_ARRAY_BUFFER, new_vbo.texcoord_buffer);
    //Upload vertex data to the video device
    glBufferDataARB(GL_ARRAY_BUFFER, sizeof(float) * new_vbo.num_faces * 3 * 2, new_vbo.texcoords, GL_STATIC_DRAW);

    glGenBuffersARB(1, &new_vbo.color_buffer);
    glBindBufferARB(GL_ARRAY_BUFFER, new_vbo.color_buffer);
    glBufferDataARB(GL_ARRAY_BUFFER, sizeof(float) * new_vbo.num_faces * 3 * 3, new_vbo.colors,
        GL_STATIC_DRAW);
    
    /*if (tri_vbo_elem != (sizeof(tri_color) / s izeof(*tri_color) / 3)) {
        fprintf(stderr, "ERROR:tri_color[] does not have the same number of elements as tri_data[]\n");
        // TODO: bail
    }*/
    glBindBufferARB(GL_ARRAY_BUFFER, 0); /* release binding */


    return new_vbo;
}


/*

t_VBO SpineManager::make_vbo(spine::Skeleton* skeleton)
{
    glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)
        uglGetProcAddress("glGenBuffersARB");
    glBufferDataARB = (PFNGLBUFFERDATAARBPROC)
        uglGetProcAddress("glBufferDataARB");
    glBindBufferARB = (PFNGLBINDBUFFERARBPROC)
        uglGetProcAddress("glBindBufferARB");

    spine::Vector<float> worldVertices;
    unsigned short quadIndices[] = { 0, 1, 2, 2, 3, 0 };
    spine::Vector<float>* vertices = &worldVertices;
    spine::Vector<float>* uvs = NULL;
    spine::Vector<unsigned short>* indices = NULL;
    int indicesCount = 0;
    int verticesCount = 0;
    GLuint* texture = nullptr;

    t_VBO new_vbo;

    new_vbo.num_faces = 1;
    new_vbo.verticies = new float[new_vbo.num_faces * 3 * 3];


    new_vbo.verticies[0] = 1;
    new_vbo.verticies[1] = 0;
    new_vbo.verticies[2] = 0.0f;

    new_vbo.verticies[3] = 0;
    new_vbo.verticies[4] = 1;
    new_vbo.verticies[5] = 0.0f;

    new_vbo.verticies[6] = 1;
    new_vbo.verticies[7] = 1;
    new_vbo.verticies[8] = 0.0f;

    new_vbo.colors = new float[new_vbo.num_faces * 3 * 3];

    new_vbo.colors[0] = 1;
    new_vbo.colors[1] = 0;
    new_vbo.colors[2] = 0.0f;

    new_vbo.colors[3] = 0;
    new_vbo.colors[4] = 1;
    new_vbo.colors[5] = 0.0f;

    new_vbo.colors[6] = 1;
    new_vbo.colors[7] = 1;
    new_vbo.colors[8] = 0.0f;

    new_vbo.texcoords = new float[new_vbo.num_faces * 3 * 2];

    new_vbo.texture = NULL;

glGenBuffersARB(1, &new_vbo.vertex_buffer);
glBindBufferARB(GL_ARRAY_BUFFER, new_vbo.vertex_buffer);
glBufferDataARB(GL_ARRAY_BUFFER, sizeof(float)* new_vbo.num_faces * 3 * 3, new_vbo.verticies,
    GL_STATIC_DRAW);
//tri_vbo_elem = sizeof(new_vbo.verticies) / sizeof(*new_vbo.verticies) / 3;
glBindBufferARB(GL_ARRAY_BUFFER, 0);

//Make the new VBO active
glGenBuffersARB(1, &new_vbo.texcoord_buffer);
glBindBufferARB(GL_ARRAY_BUFFER, new_vbo.texcoord_buffer);
//Upload vertex data to the video device
glBufferDataARB(GL_ARRAY_BUFFER, sizeof(float)* new_vbo.num_faces * 3 * 2, new_vbo.texcoords, GL_STATIC_DRAW);

glGenBuffersARB(1, &new_vbo.color_buffer);
glBindBufferARB(GL_ARRAY_BUFFER, new_vbo.color_buffer);
glBufferDataARB(GL_ARRAY_BUFFER, sizeof(float)* new_vbo.num_faces * 3 * 3, new_vbo.colors,
    GL_STATIC_DRAW);

if (tri_vbo_elem != (sizeof(tri_color) / s izeof(*tri_color) / 3)) {
    fprintf(stderr, "ERROR:tri_color[] does not have the same number of elements as tri_data[]\n");
    // TODO: bail
}
glBindBufferARB(GL_ARRAY_BUFFER, 0); 


return new_vbo;
}*/

void SpineManager::drawSkeleton(spine::Skeleton* skeleton) {

    spine::Vector<float> worldVertices;
    unsigned short quadIndices[] = { 0, 1, 2, 2, 3, 0 };
    spine::Vector<float>* vertices = &worldVertices;
    spine::Vector<float>* uvs = NULL;
    spine::Vector<unsigned short>* indices = NULL;
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
            if (texture != nullptr)
                glBindTexture(GL_TEXTURE_2D, *texture);
            glBegin(GL_TRIANGLES);
            for (size_t j = 0, l = 0; j < worldVertices.size(); j += 2, l += 2) {
                for (int ii = 0; ii < indicesCount; ++ii) {
                    int index = quadIndices[ii] << 1;
                    glTexCoord2f((*uvs)[index], (*uvs)[index + 1]);
                    glVertex3f((*vertices)[index], (*vertices)[index + 1], 0.0f);
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
                    glVertex3f((*vertices)[index], (*vertices)[index + 1], 0.0f);
                }
            }
            glEnd();
        }
    }
}