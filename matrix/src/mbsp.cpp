#include "mbsp.hpp"
#include <mapp.hpp>
#include <mdev.hpp>
#include <mmaterial.hpp>
#include <algorithm>
#include <cstring>
#include <Bullet3Geometry/b3GeometryUtil.h>

namespace mtx
{
    void* BSPDirentry::loadData(std::FILE* bsp)
    {
        size_t fpos = std::ftell(bsp);
        std::fseek(bsp, offset, SEEK_SET);
        void* d = malloc(length);
        std::fread(d, 1, length, bsp);
        std::fseek(bsp, fpos, SEEK_SET);
        return d;
    }

    BSPFile::BSPFile(const char* bsp)
    {
        m_name = bsp;
        m_gfxEnabled = false;
        std::FILE* bspfile = App::getFileSystem()->open(bsp);
        m_currentClusterIndex = 0;
        m_useVis = true;
	m_skybox = NULL;
        if(bspfile)
        {
            std::fread(&m_header, sizeof(BSPHeader), 1, bspfile);
            DEV_MSG("bsp version %02x", m_header.version);
            DEV_ASSERT2(m_header.version, ==, BSP_VERSION);
            
            // preload all dirent entries
            for(int i = 0; i < __BSP_DIRENT_MAX; i++)
            {
                BSPDirentry* ent = &m_header.dirents[i];
                void* ent_data = ent->loadData(bspfile);
                direntData[i] = ent_data;
                DEV_MSG("loaded dirent %i, size %i", i, ent->length);
            }

            BSPDirentry* f = &m_header.dirents[BSP_ENTITIES];
            readEntitesLump(f);

            BSPNode* root = (BSPNode*)direntData[BSP_NODES];
            parseTreeNode(root, true, false);
        }
    }

    void BSPFile::readEntitesLump(BSPDirentry* dirent)
    {

    }

    void BSPFile::addLeafFaces(BSPLeaf* leaf, bool brushen, bool leaffaceen)
    {
        int leafcount = m_header.dirents[BSP_LEAFS].length / sizeof(BSPLeaf);
        int brushcount = m_header.dirents[BSP_BRUSHES].length / sizeof(BSPBrush);

        // DEV_MSG("leaf %p cluster %i area %i n_leaffaces %i", leaf, leaf->cluster, leaf->area, leaf->n_leaffaces);
        int* leaffaces = (int*)direntData[BSP_LEAFFACES];
        int* leafbrushes = (int*)direntData[BSP_LEAFBRUSHES];
        BSPFace* faces = (BSPFace*)direntData[BSP_FACES];
        BSPBrush* brushes = (BSPBrush*)direntData[BSP_BRUSHES];
        BSPBrushSide* brushsides = (BSPBrushSide*)direntData[BSP_BRUSHSIDES];
        BSPPlane* planes = (BSPPlane*)direntData[BSP_PLANES];
        BSPLeafModel m;
        BSPVisdata* visdata = (BSPVisdata*)direntData[BSP_VISDATA];

        m.m_cluster = leaf->cluster;

        if(leaf->cluster > visdata->n_vecs)
            return;

        for(int b = leaf->leafbrush; b < (leaf->leafbrush + leaf->n_leafbrushes); b++)
        {
            if(!brushen)
                break;
            int brushid = leafbrushes[b];
            if(brushid > brushcount || brushid < 0)
                break;
            BSPBrush* brush = &brushes[brushid];
            BSPBrushModel brushmodel;
            for(int s = brush->brushside; s < (brush->brushside + brush->n_brushsides); s++)
            {
                BSPBrushModelSide bmside;
                BSPBrushSide* side = &brushsides[s];
                BSPPlane* plane = &planes[side->plane];
                bmside.plane = *plane;
                brushmodel.brushSides.push_back(bmside);
            }
            m_brushes.push_back(brushmodel);
        }

        if(m.m_cluster < 0)
        {
            // DEV_MSG("leaf is out of world/invalid\n");
            return;
        }
        else
        {
            if(m_gfxEnabled && leaffaceen)
            {
                for(int f = leaf->leafface; f < (leaf->leafface + leaf->n_leaffaces); f++)
                {
                    BSPFace* face = &faces[leaffaces[f]];
                    BSPFaceModel facemodel = addFaceModel(face);

                    m.m_models.push_back(facemodel);
                }
            }
        }
        
        std::memcpy(m.mins,leaf->mins,sizeof(m.mins)+sizeof(m.maxs));

        m_leafs.push_back(m);
    }

    BSPFaceModel BSPFile::addFaceModel(BSPFace* face)
    {
        BSPFaceModel m;
        m.m_buffer = App::getHWAPI()->newBuffer();
        m.m_index = App::getHWAPI()->newBuffer();
        m.m_layout = App::getHWAPI()->newLayout();
        std::vector<BSPVertex> vertices;
        std::vector<int> indices;
        HWLayoutReference* layout = m.m_layout;

        int* meshverts = (int*)direntData[BSP_MESHVERTS];
        BSPVertex* verts = (BSPVertex*)direntData[BSP_VERTICES];
        BSPLightmap* lightmaps = (BSPLightmap*)direntData[BSP_LIGHTMAPS];

        switch(face->type)
        {
        default:
            DEV_WARN("unknown face type in bsp %i", face->type);
            break;
        case 1: // type 1, polygon
            for(int v = face->meshvert; v < (face->meshvert + face->n_meshverts); v++)
                indices.push_back(meshverts[v]);
            for(int v = face->vertex; v < (face->vertex + face->n_vertices); v++)
                vertices.push_back(verts[v]);
            std::reverse(indices.begin(), indices.end()); // the indices must be flipped so it renders the inside of the mesh

            layout->addEntry({m.m_buffer, 3, HWT_FLOAT, false, sizeof(BSPVertex), (void*)offsetof(BSPVertex, position)});
            layout->addEntry({m.m_buffer, 3, HWT_FLOAT, false, sizeof(BSPVertex), (void*)offsetof(BSPVertex, normal)});
            layout->addEntry({m.m_buffer, 2, HWT_FLOAT, false, sizeof(BSPVertex), (void*)offsetof(BSPVertex, surface_uv)});
            layout->addEntry({m.m_buffer, 2, HWT_FLOAT, false, sizeof(BSPVertex), (void*)offsetof(BSPVertex, lm_uv)});
            layout->addEntry({m.m_buffer, 4, HWT_BYTE,  false, sizeof(BSPVertex), (void*)offsetof(BSPVertex, color)});
            break;
        }

        BSPLightmap* lm = &lightmaps[face->lm_index];
        char tname[64];
        snprintf(tname,64,"lm:%s%i",m_name.c_str(),face->lm_index);
        HWTextureReference* lmt = App::getHWAPI()->loadCachedTexture(tname,false);
        if(!lmt)
        {
            lmt = App::getHWAPI()->newTexture();
            lmt->uploadRGB(glm::ivec2(128, 128), lm->lightmap, true);
            App::getHWAPI()->addTextureToCache(lmt, tname);
        }
        m.m_lightmap = lmt;


        // first part is designed to fit with ModelComponent layout so i can reuse materials

        m.m_buffer->upload(vertices.size() * sizeof(BSPVertex), vertices.data());
        m.m_indexCount = indices.size() * sizeof(int);
        m.m_index->upload(m.m_indexCount, indices.data());

        BSPTexture texture = ((BSPTexture*)direntData[BSP_TEXTURES])[face->texture];

        if(texture.name == std::string("textures/skies/skybox")) // TODO: make this not hardcoded
            m.m_program = Material::getMaterial("materials/bsp/sky.mmf")->getProgram();
        else
        {
            m.m_program = Material::getMaterial("materials/bsp/bsp.mmf")->getProgram();

            std::string txpath = texture.name + std::string(".tga");
            HWTextureReference* r = App::getHWAPI()->loadCachedTexture(txpath.c_str());
            if(!r)
                r = App::getHWAPI()->loadCachedTexture("textures/badbsp.png");
            m.m_texture = r;
        }
        
        layout->upload();

        m_models.push_back(m);
        
        return m;
    }

    void BSPFile::updatePosition(glm::vec3 new_pos)
    {
        m_currentClusterIndex = getCluster(new_pos);
    }

    void BSPFile::parseTreeNode(BSPNode* node, bool brush, bool leafface)
    {
        // DEV_MSG("node %p, a: %i, b: %i", node, node->children[0], node->children[1]);
        BSPLeaf* leafs = (BSPLeaf*)direntData[BSP_LEAFS];
        BSPNode* nodes = (BSPNode*)direntData[BSP_NODES];
        int leafcount = m_header.dirents[BSP_LEAFS].length;

        if(node->children[0] < 0)
        {
            int leaf = -(node->children[0]);
            BSPLeaf* leafNode = &leafs[leaf];
            if(leaf < leafcount)
                addLeafFaces(leafNode, brush, leafface);
        }
        else
        {
            BSPNode* cnode = &nodes[node->children[0]];
            parseTreeNode(cnode, brush, leafface);
        }

        if(node->children[1] < 0)
        {
            int leaf = -(node->children[1]);
            BSPLeaf* leafNode = &leafs[leaf];
            if(leaf < leafcount)
                addLeafFaces(leafNode, brush, leafface);
        }
        else
        {            
            BSPNode* cnode = &nodes[node->children[1]];
            parseTreeNode(cnode, brush, leafface);
        }
    }

    ConVar r_bsptexel("r_bsptexel", "", "1");
    ConVar r_bspluxel("r_bspluxel", "", "1");
    ConVar r_vis("r_vis", "", "1");

    void BSPFile::renderFaceModel(BSPFaceModel* model, HWProgramReference* program)
    {
        if(!program)
            return;
        if(!m_gfxEnabled)
            return;

	HWTextureReference* tex = model->m_texture;
	HWTextureReference* light = model->m_lightmap;

	if(!r_bsptexel.getBool())
	    tex = 0;
	if(!r_bspluxel.getBool())
	    light = 0;
	
        HWRenderParameter rp;
        rp.name = "surface";
        rp.type = HWT_TEXTURE;
        rp.data.tx.tx = tex;
        rp.data.tx.slot = 0;
        App::getHWAPI()->pushParam(rp);
        rp.name = "lightmap";
        rp.type = HWT_TEXTURE;
        rp.data.tx.tx = light;
        rp.data.tx.slot = 1;
        App::getHWAPI()->pushParam(rp);
	if(m_skybox)
	{
	    rp.name = "skybox";
	    rp.type = HWT_TEXTURE;
	    rp.data.tx.tx = m_skybox;
	    rp.data.tx.slot = 1;
	    App::getHWAPI()->pushParam(rp);
	}
        App::getHWAPI()->gfxDrawElements(HWAPI::HWPT_TRIANGLES, 
                                        model->m_layout, 
                                        model->m_indexCount, 
                                        model->m_index, 
                                        program);
	if(m_skybox)
	    App::getHWAPI()->popParam();
        App::getHWAPI()->popParam();
        App::getHWAPI()->popParam();
    }

    void BSPFile::hwapiDraw(HWProgramReference* program)
    {
        if(!m_gfxEnabled)
            return;

        m_facesRendered = 0;
        m_leafsRendered = 0;
        if(r_vis.getBool())
        {
            for(int i = 0; i < m_leafs.size(); i++)
            {
                BSPLeafModel leaf = m_leafs.at(i);

                if(!leaf.m_models.size())
                    continue;

                int y = m_currentClusterIndex;
                int x = leaf.m_cluster;

                if(!canSeeCluster(x, y))
                    continue;

                for(int j = 0; j < leaf.m_models.size(); j++)
                {
                    BSPFaceModel model = leaf.m_models.at(j);
                    renderFaceModel(&model, model.m_program);
                    m_facesRendered++;
                }
                m_leafsRendered++;
            }
        }
        else
        {
            for(int i = 0; i < m_models.size(); i++)
            {
                BSPFaceModel model = m_models.at(i);
                renderFaceModel(&model, model.m_program);                    
                m_facesRendered++;
            }
        }
    }

    void BSPFile::addToPhysicsWorld(PhysicsWorld* world)
    {
        int rb_added = 0;
        for(auto brush : m_brushes)
        {
            b3AlignedObjectArray<b3Vector3> planeeqs;

            for(auto brushside : brush.brushSides)
            {
                b3Vector3 planeEq;
                planeEq.setValue(
                    brushside.plane.normal[0],
                    brushside.plane.normal[1],
                    brushside.plane.normal[2]
                );
                planeEq[3] = -brushside.plane.dist;
                planeeqs.push_back(planeEq);
            }

            b3AlignedObjectArray<b3Vector3> verts;
            b3GeometryUtil::getVerticesFromPlaneEquations(planeeqs, verts);
            btVector3 etg;

            // i dont know why i must do this but i have to
            std::vector<btVector3> intermediate;
            for(int i = 0; i < verts.size(); i++)
            {
                b3Vector3 vert = verts.at(i);
                intermediate.push_back(btVector3(
                    vert.x,
                    vert.y,
                    vert.z
                ));
            }

            if(intermediate.size() != 0)
            {
                btCollisionShape* brushshape = new btConvexHullShape((btScalar*)intermediate.data(), intermediate.size());
                btRigidBody* brushbody = new btRigidBody(0.f, NULL, brushshape);
                brushbody->setUserPointer(this);
                world->addRigidBody(brushbody);
                rb_added++;
            }
        }
        DEV_MSG("added %i brushes to physicsworld", rb_added);
    }

    int BSPFile::getCluster(glm::vec3 p)
    {
        glm::ivec3 posi = glm::ivec3(
            (int)roundf(p.x),
            (int)roundf(p.y),
            (int)roundf(p.z)
        );
        for(BSPLeafModel model : m_leafs)
        {
            if(posi.x >= model.mins[0] && posi.x <= model.maxs[0] &&
               posi.y >= model.mins[1] && posi.y <= model.maxs[1] &&
               posi.z >= model.mins[2] && posi.z <= model.maxs[2])
            {
                // inside this leaf
                return model.m_cluster;
            }
        }
        return -1;
    }

    bool BSPFile::canSeeCluster(int x, int y)
    {
        if(x == -1)
            return false;
        if(y == -1)
            return false;

        BSPVisdata* vdata = (BSPVisdata*)direntData[BSP_VISDATA];
        return vdata->data[x * vdata->sz_vecs + y / 8] & (1 << y % 8);
    }

    bool BSPFile::canSee(glm::vec3 x, glm::vec3 y)
    {
        return canSeeCluster(
            getCluster(x),
            getCluster(y)
        );
    }

    void BSPFile::initGfx()
    {
        m_gfxEnabled = true;

        BSPNode* root = (BSPNode*)direntData[BSP_NODES];
        parseTreeNode(root, false, true);
    }

    BSPComponent::BSPComponent(BSPFile* file)
    {
        m_bspFile = file;
        m_camera = 0;
    }

    void BSPComponent::renderComponent()
    {
        if(!m_bspFile->getGfxEnabled())
            return;
        if(m_bspFile)
            m_bspFile->hwapiDraw(0);
    }

    void BSPComponent::tick()
    {
        if(m_camera && m_bspFile->getUsingVis())
            m_bspFile->updatePosition(m_camera->getTransform().getPosition());
        m_node->setOccludes(false);
    }
}
