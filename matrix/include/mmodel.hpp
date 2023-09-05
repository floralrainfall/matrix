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
        HWBufferReference* m_modelBuffer;
        HWBufferReference* m_indexBuffer;
        HWLayoutReference* m_modelLayout;
        std::vector<HWTextureReference*> m_modelTextures;
        int m_modelIndicesCount;

        ModelData(const char* model);
        static ModelData* loadCached(const char* model);
    };

    class ModelComponent : public SceneComponent
    {
        Material* m_material;
        ModelData* m_modelData;
    public:
        ModelComponent(const char* model);

        Material* getMaterial() { return m_material; }
        void setMaterial(Material* material) { m_material = material; }

        virtual void renderComponent();
        virtual std::string className() { return "ModelComponent"; }
    };
}