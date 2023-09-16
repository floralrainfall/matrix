#pragma once

// implementation of Quake 3 BSP format
#include <mhwabs.hpp>
#include <mscene.hpp>

namespace mtx
{
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

        char* loadData(std::FILE* bsp);
    };

    struct BSPHeader
    {
        char magic[4];
        int version;
        BSPDirentry dirents[__BSP_DIRENT_MAX];
    };

    struct BSPModel
    {
        HWTextureReference* m_texture;
        HWTextureReference* m_lightmap;
        HWBufferReference* m_buffer;
        HWBufferReference* m_index;
        int m_indexCount;
        HWLayoutReference* m_layout;
    };

    class BSPFile
    {
        BSPHeader m_header;
        std::vector<BSPModel> m_models;
        std::vector<HWTextureReference*> m_textures;

        void readEntitesLump(BSPDirentry* dirent);
    public:
        BSPFile(const char* bsp);
        HWLayoutReference* createModelLayout();
        void hwapiDraw(HWProgramReference* program);
    };

    // wrapper for BSPFiles to be rendered within a scene
    class BSPComponent : public SceneComponent
    {
        BSPFile* m_bspFile;
    public:
        BSPComponent(BSPFile* file);
        virtual void renderComponent();
        virtual std::string className() { return "BSPComponent"; }
    };
}