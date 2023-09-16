#include "mbsp.hpp"
#include <mapp.hpp>
#include <mdev.hpp>
#include <mmaterial.hpp>
#include <algorithm>
#include <hw/mgl.hpp>

namespace mtx
{
    struct BSPTexture
    {
        char name[64];
        int flags;
        int contents;
    };

    struct BSPVertex
    {
        glm::vec3 position;
        glm::vec2 surface_uv;
        glm::vec2 lm_uv;
        glm::vec3 normal;
        char color[4];
    };

    struct BSPLightmap
    {
        char lightmap[128][128][3];
    };

    struct BSPFace
    {
        int texture;
        int effect;
        int type;
        int vertex;
        int n_vertices;
        int meshvert;
        int n_meshverts;
        int lm_index;
        glm::ivec2 lm_start;
        glm::ivec2 lm_size;
        glm::vec3 lm_origin;
        glm::vec3 lm_vecs[2];
        glm::vec3 normal;
        glm::ivec2 size;
    };

    char* BSPDirentry::loadData(std::FILE* bsp)
    {
        size_t fpos = std::ftell(bsp);
        std::fseek(bsp, offset, SEEK_SET);
        char* d = (char*)malloc(length);
        std::fread(d, length, 1, bsp);
        std::fseek(bsp, fpos, SEEK_SET);
        return d;
    }

    BSPFile::BSPFile(const char* bsp)
    {
        std::FILE* bspfile = App::getFileSystem()->open(bsp);
        if(bspfile)
        {
            std::fread(&m_header, sizeof(BSPHeader), 1, bspfile);
            DEV_MSG("bsp version %02x", m_header.version);
            DEV_ASSERT2(m_header.version, ==, 0x2d);
            for(int i = 0; i < __BSP_DIRENT_MAX; i++)
            {
                BSPDirentry* ent = &m_header.dirents[i];
                char* ent_data = ent->loadData(bspfile);
                DEV_MSG("loading dirent %i (size: %i bytes)", i, ent->length);
                switch((BSPEntry)i)
                {
                default:
                    break;
                case BSP_ENTITIES:
                    readEntitesLump(ent);
                    break;
                case BSP_TEXTURES:
                    {
                        int texturec = ent->length / sizeof(BSPTexture);
                        for(int i = 0; i < texturec; i++)
                        {
                            BSPTexture* tx = (BSPTexture*)(ent_data + sizeof(BSPTexture) * i);
                            std::string txpath = tx->name + std::string(".tga");
                            HWTextureReference* r = App::getHWAPI()->loadCachedTexture(txpath.c_str());
                            if(!r)
                                r = App::getHWAPI()->loadCachedTexture("textures/badbsp.png");
                            m_textures.push_back(r);
                        }
                    }
                    break;
                case BSP_FACES:
                    {
                        BSPFace* face_array = (BSPFace*)ent_data;
                        BSPVertex* vtx_data = (BSPVertex*)m_header.dirents[BSP_VERTICES].loadData(bspfile);
                        BSPLightmap* lightmaps = (BSPLightmap*)m_header.dirents[BSP_LIGHTMAPS].loadData(bspfile);
                        int* mvtx_data = (int*)m_header.dirents[BSP_MESHVERTS].loadData(bspfile); // mesh verts are just an integer offset
                        int faces = ent->length / sizeof(BSPFace);
                        if(faces == 0)
                            DEV_WARN("no faces in bsp");
                        for(int i = 0; i < faces; i++)
                        {
                            BSPModel m;
                            m.m_buffer = App::getHWAPI()->newBuffer();
                            m.m_index = App::getHWAPI()->newBuffer();
                            m.m_layout = App::getHWAPI()->newLayout();
                            std::vector<BSPVertex> vertices;
                            std::vector<int> indices;
                            BSPFace face = face_array[i];
                            HWLayoutReference* layout = m.m_layout;

                            switch(face.type)
                            {
                            default:
                                DEV_WARN("unknown face type in bsp %i", face.type);
                                break;
                            case 1: // type 1, polygon
                                for(int v = face.meshvert; v < (face.meshvert + face.n_meshverts); v++)
                                    indices.push_back(mvtx_data[v]);
                                for(int v = face.vertex; v < (face.vertex + face.n_vertices); v++)
                                    vertices.push_back(vtx_data[v]);
                                std::reverse(indices.begin(), indices.end()); // the indices must be flipped so it renders the inside of the mesh,

                                layout->addEntry({m.m_buffer, 3, HWT_FLOAT, false, sizeof(BSPVertex), (void*)offsetof(BSPVertex, position)});
                                layout->addEntry({m.m_buffer, 3, HWT_FLOAT, false, sizeof(BSPVertex), (void*)offsetof(BSPVertex, normal)});
                                layout->addEntry({m.m_buffer, 2, HWT_FLOAT, false, sizeof(BSPVertex), (void*)offsetof(BSPVertex, surface_uv)});
                                layout->addEntry({m.m_buffer, 2, HWT_FLOAT, false, sizeof(BSPVertex), (void*)offsetof(BSPVertex, lm_uv)});
                                layout->addEntry({m.m_buffer, 4, HWT_BYTE,  false, sizeof(BSPVertex), (void*)offsetof(BSPVertex, color)});
                                break;
                            }

                            BSPLightmap* lm = &lightmaps[face.lm_index];
                            char tname[64];
                            snprintf(tname,64,"lm:%s%i",bsp,face.lm_index);
                            HWTextureReference* lmt = App::getHWAPI()->loadCachedTexture(tname,false);
                            if(!lmt)
                            {
                                lmt = App::getHWAPI()->newTexture();
                                lmt->uploadRGB(glm::ivec2(128, 128), lm->lightmap, true);
                                gl::GLTexture* lmt_gl = (gl::GLTexture*)lmt; // FIXME: add a generic interface for setting texture filters
                                lmt_gl->setFilter(GL_LINEAR, GL_LINEAR);
                                App::getHWAPI()->addTextureToCache(lmt, tname);
                            }
                            m.m_lightmap = lmt;
                            m.m_texture = m_textures.at(face.texture);
                            
                            // first part is designed to fit with ModelComponent layout so i can reuse materials

                            m.m_buffer->upload(vertices.size() * sizeof(BSPVertex), vertices.data());
                            m.m_indexCount = indices.size() * sizeof(int);
                            m.m_index->upload(m.m_indexCount, indices.data());
                            
                            layout->upload();
                            
                            m_models.push_back(m);
                        }

                        free(lightmaps);
                        free(vtx_data);
                        free(mvtx_data);
                    }
                    break;
                }
                free(ent_data);
            }
        }
    }

    void BSPFile::readEntitesLump(BSPDirentry* dirent)
    {

    }

    void BSPFile::hwapiDraw(HWProgramReference* program)
    {
        for(BSPModel model : m_models)
        {
            HWRenderParameter rp;
            rp.name = "surface";
            rp.type = HWT_TEXTURE;
            rp.data.tx.tx = model.m_texture;
            rp.data.tx.slot = 0;
            App::getHWAPI()->pushParam(rp);
            rp.name = "lightmap";
            rp.type = HWT_TEXTURE;
            rp.data.tx.tx = model.m_lightmap;
            rp.data.tx.slot = 1;
            App::getHWAPI()->pushParam(rp);
            App::getHWAPI()->gfxDrawElements(HWAPI::HWPT_TRIANGLES, 
                                             model.m_layout, 
                                             model.m_indexCount, 
                                             model.m_index, 
                                             program);
            App::getHWAPI()->popParam();
        }
    }

    BSPComponent::BSPComponent(BSPFile* file)
    {
        m_bspFile = file;
    }

    void BSPComponent::renderComponent()
    {
        MaterialComponent* materialcomp = (MaterialComponent*)m_node->getComponent("MaterialComponent");
        if(m_bspFile && materialcomp)
            m_bspFile->hwapiDraw(materialcomp->getMaterial()->getProgram());
    }
}