#pragma once
#include <mscene.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <mmaterial.hpp>
#include <glm/glm.hpp>

namespace mtx
{
    struct ModelVertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    class ModelData
    {
        static std::map<std::string, ModelData*> m_modelDataCache;
    public:
        aiScene* m_modelScene;
        aiMesh* m_usedMesh;
        HWBufferReference* m_modelBuffer;
        HWBufferReference* m_indexBuffer;
        HWLayoutReference* m_modelLayout;
        std::vector<HWTextureReference*> m_modelTextures;
        int m_modelSceneMeshIndex;
        int m_modelIndicesCount;
        bool m_modelSkinned;

        ModelData(const char* model);
        static ModelData* loadCached(const char* model);

    };

    class ModelComponent : public SceneComponent
    {
        ModelData* m_modelData;
    public:
        ModelComponent(const char* model = 0);
        void setModel(const char* model);

        ModelData* getModelData() { return m_modelData; }

        virtual void renderComponent();
        virtual std::string className() { return "ModelComponent"; }
    };
}