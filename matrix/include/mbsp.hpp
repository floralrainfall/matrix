#pragma once

// implementation of Quake 3 BSP format
#include <mhwabs.hpp>
#include <mscene.hpp>
#include <mphysics.hpp>

namespace mtx
{
    const int BSP_VERSION = 0x2e;

    enum BSPEntry
    {
        BSP_ENTITIES,
        BSP_TEXTURES,
        BSP_PLANES,
        BSP_NODES,
        BSP_LEAFS,
        BSP_LEAFFACES,
        BSP_LEAFBRUSHES,
        BSP_MODELS,
        BSP_BRUSHES,
        BSP_BRUSHSIDES,
        BSP_VERTICES,
        BSP_MESHVERTS,
        BSP_EFFECTS,
        BSP_FACES,
        BSP_LIGHTMAPS,
        BSP_LIGHTVOLS,
        BSP_VISDATA,
        __BSP_DIRENT_MAX,
    };

    struct BSPDirentry
    {
        int offset;
        int length;

        void* loadData(std::FILE* bsp);
    };

    struct BSPHeader
    {
        char magic[4];
        int version;
        BSPDirentry dirents[__BSP_DIRENT_MAX];
    };

    struct BSPFaceModel
    {
        HWTextureReference* m_texture; // dont dealloc this
        HWTextureReference* m_lightmap; // dont dealloc this
        HWBufferReference* m_buffer; 
        HWBufferReference* m_index;
        HWProgramReference* m_program;
        int m_indexCount;
        HWLayoutReference* m_layout;
    };

    // lump types

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

    struct BSPNode
    {
        int plane;
        int children[2];
        int maxs[3];
        int mins[3];
    };

    struct BSPLeaf
    {
        int cluster;
        int area;
        int mins[3];
        int maxs[3];
        int leafface;
        int n_leaffaces;
        int leafbrush;
        int n_leafbrushes;
    };

    struct BSPVisdata
    {
        int n_vecs;
        int sz_vecs;
        unsigned char data[];
    };

    struct BSPPlane
    {
        glm::vec3 normal;
        float dist;
    };

    struct BSPBrush
    {
        int brushside;
        int n_brushsides;
        int texture;
    };

    struct BSPBrushSide
    {
        int plane;
        int texture;
    };

    struct BSPBrushModelSide
    {
        BSPPlane plane;
    };

    struct BSPBrushModel
    {
        std::vector<BSPBrushModelSide> brushSides;
        int texture;
    };

    class BSPFile
    {
        struct BSPLeafModel
        {
            std::vector<BSPFaceModel> m_models; 
            int m_cluster;
            int mins[3];
            int maxs[3];
        };
        void* direntData[__BSP_DIRENT_MAX];
        bool m_useVis;
        bool m_gfxEnabled;
        std::string m_name;
	PhysicsWorld* m_physicsWorld;

        BSPHeader m_header;
        std::vector<BSPLeafModel> m_leafs;
        std::vector<BSPFaceModel> m_models;
        std::vector<BSPBrushModel> m_brushes;
        std::vector<HWTextureReference*> m_textures;
	std::vector<btRigidBody*> m_brushBodies;
        glm::vec3 vecpos;
        int m_currentClusterIndex;
        int m_facesRendered;
        int m_leafsRendered;

	HWTextureReference* m_skybox;

        void readEntitesLump(BSPDirentry* dirent);
        void addLeafFaces(BSPLeaf* leaf, bool brush, bool leafface);
        void parseTreeNode(BSPNode* node, bool brush, bool leafface);
        BSPFaceModel addFaceModel(BSPFace* face);
        void renderFaceModel(BSPFaceModel* model, HWProgramReference* program);
    public:
        BSPFile(const char* bsp);
	~BSPFile();
	
        HWLayoutReference* createModelLayout();
        void addToPhysicsWorld(PhysicsWorld* world);
	void removeFromPhysicsWorld(PhysicsWorld* world);
        bool getUsingVis() { return m_useVis; }
        bool getGfxEnabled() { return m_gfxEnabled; }
        int getVisCluster() { return m_currentClusterIndex; }
        int getFacesRendered() { return m_facesRendered; }
        int getFacesTotal() { return m_models.size(); }
        int getLeafsRendered() { return m_leafsRendered; }
        int getLeafsTotal() { return m_leafs.size(); }
        void hwapiDraw(HWProgramReference* program);
        void updatePosition(glm::vec3 new_pos);
	void setSkybox(HWTextureReference* skybox);
        int getCluster(glm::vec3 p);
        bool canSeeCluster(int x, int y);
        bool canSee(glm::vec3 x, glm::vec3 y);
	std::string getName() { return m_name; }

        void initGfx();
    };

    // wrapper for BSPFiles to be rendered within a scene
    class BSPComponent : public SceneComponent
    {
        BSPFile* m_bspFile;
        SceneNode* m_camera;
    public:
        BSPComponent(BSPFile* file);
        virtual void renderComponent();
        virtual void tick();
	void setBSP(BSPFile* file) { m_bspFile = file; };
        void setCameraNode(SceneNode* node) { m_camera = node; }
        virtual std::string className() { return "BSPComponent"; }
    };
}
