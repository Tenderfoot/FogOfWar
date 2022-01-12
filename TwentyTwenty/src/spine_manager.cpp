
#include "spine_manager.h"

std::map<std::string, spine::SkeletonData*> SpineManager::skeletonData;
spine::TextureLoader* SpineManager::textureLoader = new MyTextureLoader();
std::map <std::string, spine::Atlas*> SpineManager::atlas;
std::map <std::string, spine::AnimationStateData*> SpineManager::stateData;

// stuff for VBOs...
PFNGLGENBUFFERSARBPROC      glGenBuffersARB = NULL;
PFNGLBUFFERDATAARBPROC      glBufferDataARB = NULL;
PFNGLBINDBUFFERARBPROC      glBindBufferARB = NULL;

spine::SpineExtension* spine::getDefaultExtension() {
    return new spine::DefaultSpineExtension();
}

SpineManager::SpineManager()
{
}

// Pass as const-reference
void SpineManager::LoadData(std::string spine_folder)
{
    // this is already being done in paintbrush I think
    // probably sketchy
    glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)
        uglGetProcAddress("glGenBuffersARB");
    glBufferDataARB = (PFNGLBUFFERDATAARBPROC)
        uglGetProcAddress("glBufferDataARB");
    glBindBufferARB = (PFNGLBINDBUFFERARBPROC)
        uglGetProcAddress("glBindBufferARB");

    // Using "auto" here helps with readability
    std::map<std::string, spine::SkeletonData*>::iterator it;
    it = skeletonData.find(spine_folder.c_str());
    if (it == skeletonData.end())
    {
        std::string atlas_path = std::string("data/").append(spine_folder).append("/skeleton.atlas");
        // If atlas owns this pointer use std::shared_ptr or std::unique_ptr
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

        // If stateData owns this pointer use std::shared_ptr or std::unique_ptr
        stateData[spine_folder] = new spine::AnimationStateData(skeletonData[spine_folder]);
        stateData[spine_folder]->setDefaultMix(0.2);
        // If loading failed, print the error and exit the app

        if (!skeletonData[spine_folder]) {
            printf("%s\n", json.getError().buffer());
            exit(0);
        }
    }
}

t_VBO SpineManager::make_vbo(spine::Skeleton* skeleton)
{
    t_VBO new_vbo;

    get_num_faces(skeleton, &new_vbo);
    
    new_vbo.verticies = std::make_shared<float>(new float[new_vbo.num_faces * 3]);
    new_vbo.colors = std::make_shared<float>(new float[new_vbo.num_faces * 3]);
    new_vbo.texcoords = std::make_shared<float>(new float[new_vbo.num_faces * 2]);

    glGenBuffersARB(1, &new_vbo.vertex_buffer);

    update_vbo(skeleton, &new_vbo);

    glGenBuffersARB(1, &new_vbo.texcoord_buffer);
    glBindBufferARB(GL_ARRAY_BUFFER, new_vbo.texcoord_buffer);
    glBufferDataARB(GL_ARRAY_BUFFER, sizeof(float) * new_vbo.num_faces * 2, new_vbo.texcoords.get(), GL_STATIC_DRAW);

    glGenBuffersARB(1, &new_vbo.color_buffer);
    glBindBufferARB(GL_ARRAY_BUFFER, new_vbo.color_buffer);
    glBufferDataARB(GL_ARRAY_BUFFER, sizeof(float) * new_vbo.num_faces * 3, new_vbo.colors.get(), GL_STATIC_DRAW);
    
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

            for (int ii = 0; ii < indicesCount; ++ii) 
            {
                int index = (*indices)[ii] << 1;

                vbo->verticies.get()[tri_count] = (*vertices)[index];
                vbo->verticies.get()[tri_count + 1] = (*vertices)[index + 1];
                vbo->verticies.get()[tri_count + 2] = 0.0f;
                vbo->texcoords.get()[uv_count] = (*uvs)[index];
                vbo->texcoords.get()[uv_count + 1] = (*uvs)[index + 1];
                vbo->colors.get()[tri_count] = 1.0f;
                vbo->colors.get()[tri_count + 1] = 1.0f;
                vbo->colors.get()[tri_count + 2] = 1.0f;

                tri_count += 3;
                uv_count += 2;
            }
        }
    }

   glBindBufferARB(GL_ARRAY_BUFFER, vbo->vertex_buffer);
   glBufferDataARB(GL_ARRAY_BUFFER, sizeof(float) * vbo->num_faces * 3, vbo->verticies.get(), GL_DYNAMIC_DRAW);
   glBindBufferARB(GL_ARRAY_BUFFER, 0);
}

void SpineManager::get_num_faces(spine::Skeleton* skeleton, t_VBO* vbo)
{
    vbo->num_faces = 0;
    // For each slot in the draw order array of the skeleton
    for (size_t i = 0, n = skeleton->getSlots().size(); i < n; ++i)
    {
        spine::Slot* slot = skeleton->getDrawOrder()[i];
        spine::Attachment* attachment = slot->getAttachment();

        if (!attachment) continue;

        // Is there a way to not need RTTI here?  I prefer to avoid it if possible
        if (attachment->getRTTI().isExactly(spine::MeshAttachment::rtti))
        {
            spine::MeshAttachment* mesh = (spine::MeshAttachment*)attachment;
            vbo->num_faces += mesh->getTriangles().size();
            vbo->texture = *((GLuint*)((spine::AtlasRegion*)mesh->getRendererObject())->page->getRendererObject());
        }
    }
}

void SpineManager::reset_vbo(spine::Skeleton* skeleton, t_VBO* vbo)
{
    get_num_faces(skeleton, vbo);
    
    // delete the old data

    // make space for the new data
    vbo->verticies = std::make_shared<float>(new float[vbo->num_faces * 3]);
    vbo->colors = std::make_shared<float>(new float[vbo->num_faces * 3]);
    vbo->texcoords = std::make_shared<float>(new float[vbo->num_faces * 2]);

    update_vbo(skeleton, vbo);

    // texture and color buffer updated too
    glBindBufferARB(GL_ARRAY_BUFFER, vbo->texcoord_buffer);
    glBufferDataARB(GL_ARRAY_BUFFER, sizeof(float) * vbo->num_faces * 2, vbo->texcoords.get(), GL_STATIC_DRAW);

    glBindBufferARB(GL_ARRAY_BUFFER, vbo->color_buffer);
    glBufferDataARB(GL_ARRAY_BUFFER, sizeof(float) * vbo->num_faces * 3, vbo->colors.get(), GL_STATIC_DRAW);

    glBindBufferARB(GL_ARRAY_BUFFER, 0);
}