#pragma once
#include <deque>
#include <glm/glm.hpp>
#include <ctime>
#include <vector>
#include <string>
#include <map>

namespace mtx
{
    enum HWType
    {
        HWT_BYTE,
        HWT_UBYTE,
        HWT_SHORT,
        HWT_USHORT,
        HWT_INT,
        HWT_UINT,
        HWT_FLOAT,

        HWT_VECTOR2,
        HWT_VECTOR3,
        HWT_VECTOR4,
        HWT_MATRIX2,
        HWT_MATRIX3,
        HWT_MATRIX4,
        HWT_TEXTURE
    };

    class HWTextureReference;
    struct HWRenderParameter
    {
        HWType type;
        std::string name;
        union {
            char b;
            unsigned char ub;
            short s;
            unsigned short us;
            int i;
            unsigned int ui;
            float f;
            glm::vec2 v2;
            glm::vec3 v3;
            glm::vec4 v4;
            glm::mat3 m3;
            glm::mat4 m4;
            HWTextureReference* tx;
        } data;
    };

    class HWProgramReference
    {
    public:
        enum ShaderType
        {
            HWPST_VERTEX,
            HWPST_FRAGMENT,
            HWPST_GEOMETRY
        };

        virtual ~HWProgramReference();

        virtual void addShader(ShaderType type, const char* file) = 0;
        virtual void bind() = 0;
        virtual void link() = 0;
    };

    class HWBufferReference
    {
        bool m_allocated;
    public:
        virtual ~HWBufferReference();

        virtual void upload(int size, void* data) = 0;
    };

    class HWTextureReference
    {
    protected:
        glm::ivec2 m_textureSize;
    public:
        virtual ~HWTextureReference();

        glm::ivec2 getTextureSize() { return m_textureSize; }
        
        virtual void upload(glm::ivec2 size, void* data, bool genMipMaps = true) = 0;
        void uploadCompressedTexture(int size, void* data);
    };

    class Window;

    class HWWindowReference
    {
    protected:
        glm::ivec2 m_windowSize;
        HWTextureReference* m_hwTexture;
        std::clock_t m_frameBegin;
        float m_deltaTime;
    public:
        virtual ~HWWindowReference();

        glm::ivec2 getWindowSize() { return m_windowSize; }
        HWTextureReference* getHwTexture() { return m_hwTexture; }

        float getDeltaTime() { return m_deltaTime; }

        virtual void createWindow(glm::ivec2 size) = 0;
        virtual void setWindowTitle(const char* title) {}
        virtual void pumpOSEvents(Window* ewnd) {}
        virtual void beginFrame() = 0;
        virtual void endFrame() = 0;
    };

    // this is modeled after opengl cause idk how directx does it
    struct HWLayoutEntry
    {
        HWBufferReference* buffer;
        int components; // must be 1, 2, 3, or 4
        HWType type;
        bool normalized;
        int stride;
        void* offset;

        HWLayoutEntry(HWBufferReference* buffer, int components, HWType type, bool normalized, int stride, void* offset)
        {
            this->buffer = buffer;
            this->components = components;
            this->type = type;
            this->normalized = normalized;
            this->stride = stride;
            this->offset = offset;
        }
    };

    class HWLayoutReference
    {
    protected:
        std::vector<HWLayoutEntry> m_entries;
    public:
        virtual ~HWLayoutReference();

        void clearEntries() { m_entries.clear(); }
        void addEntry(HWLayoutEntry entry) { m_entries.push_back(entry); };
        virtual void upload() = 0;
    };

    class HWAPI
    {
    protected:
        std::map<std::string, HWTextureReference*> m_cachedTextures;
        std::deque<HWRenderParameter> m_hwParams; // when anything is drawn, these are applied as uniforms in OpenGL's case
    public:
        virtual void shutdown() = 0; // remove for example the GL context

        // make sure after you are finished rendering you pop every parameter you've pushed
        void pushParam(HWRenderParameter param);
        void popParam();
        void clearParams() { m_hwParams.clear(); }

        virtual void applyParamsToProgram(HWProgramReference* program) = 0;

        enum PrimitiveType
        {
            HWPT_POINTS,
            HWPT_LINES,
            HWPT_TRIANGLES
        };

        virtual void gfxViewport(glm::vec4 viewport) =0;
        virtual void gfxClear(glm::vec4 color) = 0;
        virtual void gfxClearDepth(float depth) = 0;
        virtual void gfxDrawElements(PrimitiveType type, HWLayoutReference* vertexLayout, int indice_count, HWBufferReference* indexBuffer, HWProgramReference* program = 0) = 0;
        virtual void gfxDrawArrays(PrimitiveType type, HWLayoutReference* vertexLayout, int vertex_count, HWProgramReference* program = 0) = 0;
        virtual void gfxUseLayout(HWLayoutReference* layout) = 0;

        virtual HWBufferReference* newBuffer() = 0;
        virtual HWTextureReference* newTexture() = 0;
        virtual HWProgramReference* newProgram() = 0;
        virtual HWLayoutReference* newLayout() = 0;
        virtual HWWindowReference*  newWindow(int resX, int resY) = 0;

        HWTextureReference* loadCachedTexture(const char* texture, bool autoload = true);
        void addTextureToCache(HWTextureReference* texture, const char* name);
    };
};