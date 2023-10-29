#include <mmodel.hpp>
#include <mdev.hpp>
#include <mhwabs.hpp>
#include <mapp.hpp>
#include <assimp/postprocess.h>

namespace mtx
{
	std::map<std::string, ModelData*> ModelData::m_modelDataCache;

	ModelData::ModelData(const char* model)
	{
		Assimp::Importer importer;

		m_modelScene = (aiScene*)importer.ReadFile(App::getFileSystem()->fullPath(model), aiProcess_Triangulate);
		DEV_ASSERT(m_modelScene);

		m_modelBuffer = App::getHWAPI()->newBuffer();
		m_indexBuffer = App::getHWAPI()->newBuffer();
		m_modelIndicesCount = 0;
		m_modelSkinned = false;

		if(m_modelScene)
		{        
			DEV_MSG("model has %i meshes", m_modelScene->mNumMeshes);
			aiMesh* mesh = m_modelScene->mMeshes[0];
			m_usedMesh = mesh;
            
			DEV_MSG("model has %i textures", m_modelScene->mNumTextures);
			for(int i = 0; i < m_modelScene->mNumTextures; i++)
			{
				aiTexture* texture = m_modelScene->mTextures[i];

				HWTextureReference* hwtx = App::getHWAPI()->loadCachedTexture(texture->mFilename.C_Str(), false);
				if(!hwtx)
				{
					hwtx = App::getHWAPI()->newTexture();
					DEV_MSG("loading texture %s", texture->mFilename.C_Str());
					if(texture->mHeight == 0)
					{
				                DEV_MSG("texture compressed")
                                                        hwtx->uploadCompressedTexture(texture->mWidth, texture->pcData);
                                        }
                                        else
                                        {
                                                hwtx->upload(glm::ivec2(texture->mWidth, texture->mHeight), texture->pcData, true);
                                        }
                                        App::getHWAPI()->addTextureToCache(hwtx, texture->mFilename.C_Str());
                                }
                                m_modelTextures.push_back(hwtx);
                        }

                        if(mesh->HasBones())
                        {
                                DEV_MSG("mesh has %i bones", mesh->mNumBones);
                                m_modelSkinned = true;
                                for(int i = 0; i < mesh->mNumBones; i++)
                                {
                                        aiBone* bone = mesh->mBones[i];

                                }
                        }


                        std::vector<ModelVertex> meshData;
                        std::vector<int> indices;
                        for(int i = 0; i < mesh->mNumVertices; i++)
                        {
                                ModelVertex v;
                                v.position = glm::vec3(
                                        mesh->mVertices[i].x,
                                        mesh->mVertices[i].y,
                                        mesh->mVertices[i].z
                                        );
                                v.normal = glm::vec3(
                                        mesh->mNormals[i].x,
                                        mesh->mNormals[i].y,
                                        mesh->mNormals[i].z
                                        );
                                v.uv = glm::vec2(
                                        mesh->mTextureCoords[0][i].x,
                                        mesh->mTextureCoords[0][i].y
                                        );
                                meshData.push_back(v);
                        }
                        for(int i = 0; i < mesh->mNumFaces; i++)
                        {
                                aiFace face = mesh->mFaces[i];
                                for(int j = 0; j < face.mNumIndices; j++)
                                {
                                        m_modelIndicesCount++;
                                        indices.push_back(face.mIndices[j]);
                                }
                        }

                        m_modelBuffer->upload(meshData.size() * sizeof(ModelVertex), meshData.data());
                        m_indexBuffer->upload(indices.size() * sizeof(int), indices.data());

                        m_modelLayout = App::getHWAPI()->newLayout();
                        m_modelLayout->addEntry({m_modelBuffer, 3, HWT_FLOAT, false, sizeof(ModelVertex), (void*)offsetof(ModelVertex, position)});
                        m_modelLayout->addEntry({m_modelBuffer, 3, HWT_FLOAT, false, sizeof(ModelVertex), (void*)offsetof(ModelVertex, normal)});
                        m_modelLayout->addEntry({m_modelBuffer, 2, HWT_FLOAT, false, sizeof(ModelVertex), (void*)offsetof(ModelVertex, uv)});
                        m_modelLayout->upload();
                }
                else
                        DEV_MSG("could not open model %s", model);
        }

        ModelData* ModelData::loadCached(const char* model)
        {
                auto it = m_modelDataCache.find(model);
                if(it != m_modelDataCache.end())
                        return m_modelDataCache[model];
                else
                {
                        DEV_MSG("caching new %s", model);
                        ModelData* newmodel = new ModelData(model);
                        m_modelDataCache[model] = newmodel;
                        return newmodel;
                }
        }

        ModelComponent::ModelComponent(const char* model)
        {
                if(model)
                        setModel(model);
        }

        void ModelComponent::setModel(const char* model)
        {
                m_modelData = ModelData::loadCached(model);
        }

        void ModelComponent::renderComponent()
        {
                if(m_modelData && m_modelData->m_modelScene)
                {
                        MaterialComponent* materialcomp = (MaterialComponent*)m_node->getComponent("MaterialComponent");
                        if(materialcomp && materialcomp)
                        {
                                if(!materialcomp->getMaterial())
                                        materialcomp->updateFromModelComponent();

                                HWProgramReference* program = materialcomp->getMaterial()->getProgram();
                                HWRenderParameter rp;

                                for(int i = 0; i < m_modelData->m_modelTextures.size(); i++)
                                {
                                        HWTextureReference* texture = m_modelData->m_modelTextures[i];
                                        rp.name = std::string("texture") + std::to_string(i);
                                        rp.data.tx.tx = texture;
                                        rp.data.tx.slot = i;
                                        rp.type = HWT_TEXTURE;
                                        App::getHWAPI()->pushParam(rp);
                                }

                                rp.name = "model";
                                rp.data.m4 = m_node->getTransform().getWorldMatrix();
                                rp.type = HWT_MATRIX4;
                                App::getHWAPI()->pushParam(rp);

                                materialcomp->addShaderParams();
                
                                App::getHWAPI()->gfxUseLayout(m_modelData->m_modelLayout);
                                App::getHWAPI()->gfxDrawElements(mtx::HWAPI::HWPT_TRIANGLES, 
                                                                 m_modelData->m_modelLayout, 
                                                                 m_modelData->m_modelIndicesCount, 
                                                                 m_modelData->m_indexBuffer, 
                                                                 program);

                                materialcomp->popShaderParams();

                                App::getHWAPI()->popParam();
                
                                for(int i = 0; i < m_modelData->m_modelTextures.size(); i++)
                                        App::getHWAPI()->popParam();
                        }
                }
        }
}
