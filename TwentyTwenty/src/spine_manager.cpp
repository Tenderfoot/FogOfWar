
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
            texture = (GLuint*)((spine::AtlasRegion*)mesh->getRendererObject())->page->getRendererObject();
            mesh->computeWorldVertices(*slot, 0, mesh->getWorldVerticesLength(), worldVertices, 0, 2);
            verticesCount = mesh->getWorldVerticesLength() >> 1;
            uvs = &mesh->getUVs();
            indices = &mesh->getTriangles();
            indicesCount = mesh->getTriangles().size();

            texture = (GLuint*)((spine::AtlasRegion*)mesh->getRendererObject())->page->getRendererObject();
            new_vbo.texture = *texture;


            printf("WorldVerticies Size: %d\n", worldVertices.size());
            printf("indiciesCount: %d\n", indicesCount);

            for (int ii = 0; ii < indicesCount; ++ii) {
                num_verts ++;
            }
        }
    }

    printf("num verts: %d\n", num_verts);

    new_vbo.num_faces = num_verts;
    new_vbo.verticies = new float[new_vbo.num_faces * 3];
    new_vbo.colors = new float[new_vbo.num_faces * 3];
    new_vbo.texcoords = new float[new_vbo.num_faces * 2];

    int tri_count = 0;
    int uv_count = 0;

    // For each slot in the draw order array of the skeleton
    for (size_t i = 0, n = skeleton->getSlots().size(); i < n; ++i) {
        spine::Slot* slot = skeleton->getDrawOrder()[i];

        spine::Attachment* attachment = slot->getAttachment();
        if (!attachment) continue;


        if (attachment->getRTTI().isExactly(spine::MeshAttachment::rtti)) {
            spine::MeshAttachment* mesh = (spine::MeshAttachment*)attachment;

            worldVertices.setSize(mesh->getWorldVerticesLength(), 0);
            mesh->computeWorldVertices(*slot, 0, mesh->getWorldVerticesLength(), worldVertices, 0, 2);
            uvs = &mesh->getUVs();
            indices = &mesh->getTriangles();
            indicesCount = mesh->getTriangles().size();

            // This draw section should be removed and these should be batched and drawn as an arraylist

            for (int ii = 0; ii < indicesCount; ++ii) {
                int index = (*indices)[ii] << 1;

                new_vbo.verticies[tri_count] = (*vertices)[index];
                new_vbo.verticies[tri_count + 1] = (*vertices)[index+1];
                new_vbo.verticies[tri_count + 2] = 0.0f;
                new_vbo.texcoords[uv_count] = (*uvs)[index];
                new_vbo.texcoords[uv_count + 1] = (*uvs)[index+1];
                new_vbo.colors[tri_count] = 1.0f;
                new_vbo.colors[tri_count +1] = 1.0f;
                new_vbo.colors[tri_count +2] = 1.0f;

                tri_count += 3;
                uv_count += 2;
            }
        }
    }


    glGenBuffersARB(1, &new_vbo.vertex_buffer);
    glBindBufferARB(GL_ARRAY_BUFFER, new_vbo.vertex_buffer);
    glBufferDataARB(GL_ARRAY_BUFFER, sizeof(float) * new_vbo.num_faces * 3, new_vbo.verticies,
        GL_DYNAMIC_DRAW);
    glBindBufferARB(GL_ARRAY_BUFFER, 0);

    glGenBuffersARB(1, &new_vbo.texcoord_buffer);
    glBindBufferARB(GL_ARRAY_BUFFER, new_vbo.texcoord_buffer);
    glBufferDataARB(GL_ARRAY_BUFFER, sizeof(float) * new_vbo.num_faces * 2, new_vbo.texcoords, GL_STATIC_DRAW);

    glGenBuffersARB(1, &new_vbo.color_buffer);
    glBindBufferARB(GL_ARRAY_BUFFER, new_vbo.color_buffer);
    glBufferDataARB(GL_ARRAY_BUFFER, sizeof(float) * new_vbo.num_faces * 3, new_vbo.colors,
        GL_STATIC_DRAW);
    
    glBindBufferARB(GL_ARRAY_BUFFER, 0);

    return new_vbo;
}

void SpineManager::update_vbo(spine::Skeleton* skeleton, t_VBO* vbo)
{
    spine::Vector<float> worldVertices;
    unsigned short quadIndices[] = { 0, 1, 2, 2, 3, 0 };
    spine::Vector<float>* vertices = &worldVertices;
    spine::Vector<float>* uvs = NULL;
    spine::Vector<unsigned short>* indices = NULL;
    int indicesCount = 0;
    int verticesCount = 0;

    int tri_count = 0;
    int uv_count = 0;

    skeleton->updateWorldTransform();

    // For each slot in the draw order array of the skeleton
    for (size_t i = 0, n = skeleton->getSlots().size(); i < n; ++i) {
        spine::Slot* slot = skeleton->getDrawOrder()[i];

        spine::Attachment* attachment = slot->getAttachment();
        if (!attachment) continue;


        if (attachment->getRTTI().isExactly(spine::MeshAttachment::rtti)) {
            spine::MeshAttachment* mesh = (spine::MeshAttachment*)attachment;

            worldVertices.setSize(mesh->getWorldVerticesLength(), 0);
            mesh->computeWorldVertices(*slot, 0, mesh->getWorldVerticesLength(), worldVertices, 0, 2);
            uvs = &mesh->getUVs();
            indices = &mesh->getTriangles();
            indicesCount = mesh->getTriangles().size();

            // This draw section should be removed and these should be batched and drawn as an arraylist

            for (int ii = 0; ii < indicesCount; ++ii) {
                int index = (*indices)[ii] << 1;

                vbo->verticies[tri_count] = (*vertices)[index];
                vbo->verticies[tri_count + 1] = (*vertices)[index + 1];
                vbo->verticies[tri_count + 2] = 0.0f;
                vbo->texcoords[uv_count] = (*uvs)[index];
                vbo->texcoords[uv_count + 1] = (*uvs)[index + 1];
                vbo->colors[tri_count] = 1.0f;
                vbo->colors[tri_count + 1] = 1.0f;
                vbo->colors[tri_count + 2] = 1.0f;

                tri_count += 3;
                uv_count += 2;
            }
        }
    }

   glBindBufferARB(GL_ARRAY_BUFFER, vbo->vertex_buffer);
   glBufferDataARB(GL_ARRAY_BUFFER, sizeof(float) * vbo->num_faces * 3, vbo->verticies, GL_DYNAMIC_DRAW);
   glBindBufferARB(GL_ARRAY_BUFFER, 0);

}

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
            for (int ii = 0; ii < indicesCount; ++ii) {
                int index = quadIndices[ii] << 1;
                glTexCoord2f((*uvs)[index], (*uvs)[index + 1]);
                glVertex3f((*vertices)[index], (*vertices)[index + 1], 0.0f);
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
            for (int ii = 0; ii < indicesCount; ++ii) {
                int index = (*indices)[ii] << 1;
                glTexCoord2f((*uvs)[index], (*uvs)[index + 1]);
                glVertex3f((*vertices)[index], (*vertices)[index + 1], 0.0f);
            }
            glEnd();
        }
    }
}