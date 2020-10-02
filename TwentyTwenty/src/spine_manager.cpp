
#include "spine_manager.h"

spine::SkeletonData* SpineManager::skeletonData = nullptr;
spine::TextureLoader* SpineManager::textureLoader = nullptr;
spine::Atlas* SpineManager::atlas = nullptr;
spine::AnimationStateData* SpineManager::stateData = nullptr;
t_VBO* SpineManager::vertexbuffer = nullptr;

spine::SpineExtension* spine::getDefaultExtension() {
    return new spine::DefaultSpineExtension();
}

SpineManager::SpineManager()
{
    skeletonData = nullptr;
    textureLoader = nullptr;
    atlas = nullptr;
}

void SpineManager::LoadData()
{
    textureLoader = new MyTextureLoader();
    atlas = new spine::Atlas("data/spine/skeleton.atlas", textureLoader);

    // Create a SkeletonJson used for loading and set the scale
    // to make the loaded data two times as big as the original data
    spine::SkeletonJson json(atlas);
    json.setScale(0.01);

    // Load the skeleton .json file into a SkeletonData
    skeletonData = json.readSkeletonDataFile("data/spine/skeleton.json");
    stateData = new spine::AnimationStateData(skeletonData);
    stateData->setDefaultMix(0.2);
    // If loading failed, print the error and exit the app

    if (!skeletonData) {
        printf("%s\n", json.getError().buffer());
        exit(0);
    }

    vertexbuffer = new t_VBO;
    PaintBrush::build_vbo(vertexbuffer);

}

t_transform SpineManager::getAABB(spine::Skeleton* skeleton)
{
    t_transform aabb;
    aabb.x = 0;
    aabb.y = 0;
    aabb.w = 0;
    aabb.h = 0;

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
            indicesCount = 6;

            for (size_t j = 0, l = 0; j < worldVertices.size(); j += 2, l += 2) {
                for (int ii = 0; ii < indicesCount; ++ii) {
                    int index = quadIndices[ii] << 1;

                    if ((*vertices)[index] > aabb.w)
                        aabb.w = (*vertices)[index];
                    if ((*vertices)[index] < aabb.x)
                        aabb.x = (*vertices)[index];
                    if ((*vertices)[index + 1] > aabb.h)
                        aabb.h = (*vertices)[index];
                    if ((*vertices)[index + 1] < aabb.y)
                        aabb.y = (*vertices)[index];
                }
            }
        }
        else if (attachment->getRTTI().isExactly(spine::MeshAttachment::rtti)) {
            spine::MeshAttachment* mesh = (spine::MeshAttachment*)attachment;
            worldVertices.setSize(mesh->getWorldVerticesLength(), 0);
            mesh->computeWorldVertices(*slot, 0, mesh->getWorldVerticesLength(), worldVertices, 0, 2);
            verticesCount = mesh->getWorldVerticesLength() >> 1;
            uvs = &mesh->getUVs();
            indices = &mesh->getTriangles();
            indicesCount = mesh->getTriangles().size();

            for (size_t j = 0, l = 0; j < worldVertices.size(); j += 2, l += 2) {
                for (int ii = 0; ii < indicesCount; ++ii) {
                    int index = (*indices)[ii] << 1;
                    if ((*vertices)[index] > aabb.w)
                        aabb.w = (*vertices)[index];
                    if ((*vertices)[index] < aabb.x)
                        aabb.x = (*vertices)[index];
                    if ((*vertices)[index + 1] > aabb.h)
                        aabb.h = (*vertices)[index + 1];
                    if ((*vertices)[index + 1] < aabb.y)
                        aabb.y = (*vertices)[index + 1];
                }
            }
        }
    }

    return aabb;
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


            /************* VBO STUFF ****************/
            // make some space for the 3 buffers
            vertexbuffer->verticies = new float[indicesCount*3];
            vertexbuffer->colors = new float[indicesCount*3];
            vertexbuffer->texcoords = new float[indicesCount*2];
            vertexbuffer->num_faces = indicesCount;

            int vert_index_last = 0;

            for (int ii = 0; ii < indicesCount; ++ii) {
                int index = (*indices)[ii] << 1;
                vertexbuffer->verticies[(vert_index_last * 3)] = (*vertices)[index];
                vertexbuffer->verticies[(vert_index_last * 3) + 1] = (*vertices)[index + 1];
                vertexbuffer->verticies[(vert_index_last * 3) + 2] = 0.0f;
                vertexbuffer->colors[(vert_index_last * 3) + 0] = 1.0f;
                vertexbuffer->colors[(vert_index_last * 3) + 1] = 1.0f;
                vertexbuffer->colors[(vert_index_last * 3) + 2] = 1.0f;
                vertexbuffer->texcoords[(vert_index_last * 2)] = (*uvs)[index];
                vertexbuffer->texcoords[(vert_index_last * 2) + 1] = (*uvs)[index + 1];
                vert_index_last++;
            }

            vertexbuffer->texture = *texture;
         
            glPushMatrix();
            glColor3f(1.0f, 1.0f, 1.0f);
                PaintBrush::draw_vbo(vertexbuffer);
            glPopMatrix();
            
            delete vertexbuffer->verticies;
            delete vertexbuffer->colors;
            delete vertexbuffer->texcoords;


        }
    }
}