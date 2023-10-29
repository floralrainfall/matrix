#pragma once
#include <deque>
#include <glm/glm.hpp>
#include <ctime>
#include <vector>
#include <string>
#include <functional>
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

    enum HWTextureType
    {
	HWTT_TEXTURE2D,
	HWTT_TEXTURE_CUBEMAP
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
            struct {
                HWTextureReference* tx;
                int slot;
            } tx;
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

	// the data should be copied to the gpu, you can deallocate data
        virtual void upload(int size, void* data) = 0;

    };

    class HWTextureReference
    {
    protected:
        glm::ivec2 m_textureSize;
	HWTextureType m_type;
    public:
        virtual ~HWTextureReference();

        glm::ivec2 getTextureSize() { return m_textureSize; }

        virtual void upload(glm::ivec2 size, void* data, bool genMipMaps = true) = 0;
        virtual void uploadRGB(glm::ivec2 size, void* data, bool genMipMaps = true) = 0;
        bool uploadCompressedTexture(int size, void* data);
    	void setTextureType(HWTextureType type) { m_type = type; };
    };

    class Window;

    class HWWindowReference
    {
    protected:
        glm::ivec2 m_windowSize;
        HWTextureReference* m_hwTexture;
        Window* m_engineWindow;
        std::clock_t m_frameBegin;
        float m_deltaTime;
    public:
        virtual ~HWWindowReference();

        void setEngineWindow(Window* window) { m_engineWindow = window; }

        glm::ivec2 getWindowSize() { return m_windowSize; }
        HWTextureReference* getHwTexture() { return m_hwTexture; }
        Window* getEngineWindow() { return m_engineWindow; }
        float getDeltaTime() { return m_deltaTime; }

        virtual void createWindow(glm::ivec2 size, int type) = 0;
        virtual void setWindowTitle(const char* title) {}
        virtual void pumpOSEvents(Window* ewnd) {}
        virtual void beginFrame() = 0;
        virtual void endFrame() = 0;
        virtual void setGrab(bool grab) {};
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

        int m_drawnVertices;
        int m_drawCalls;
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

        enum MessageBoxType
        {
            HWMBT_ERROR,
            HWMBT_WARNING,
            HWMBT_INFORMATION,
        };

	/**
	 * Shows a simple OK message box to the user.
	 *
	 * @param title Title of the message box
	 * @param message Message within the box
	 * @param type Which 'type' (picture of message icon) is used
	 */
        virtual void showMessageBox(const char* title,
				    const char* message,
				    MessageBoxType type =
				    HWMBT_INFORMATION) = 0;

	/**
	 * Sets the viewport in which the renderer renders into.
	 *
	 * @param viewport The rectangle, z being width and w being
	 * height
	 */
        virtual void gfxViewport(glm::vec4 viewport) =0;

	/**
	 * Clears the color buffer with specified color
	 */
        virtual void gfxClear(glm::vec4 color) = 0;
	/**
	 * Clears the depth buffer with the specified depth
	 */
        virtual void gfxClearDepth(float depth) = 0;
	/**
	 * Draws a list of elements to the screen
	 *
	 * @param type Type of primitive used
	 * @param vertexLayout The layout the vertex buffer is
	 * arranged in
	 * @param indice_count The number of elements to render
	 * @param indexBuffer The buffer of indices to use
	 * @param program The program to render with
	 */
        virtual void gfxDrawElements(PrimitiveType type,
				     HWLayoutReference* vertexLayout, int indice_count,
				     HWBufferReference* indexBuffer, HWProgramReference* program
				     = 0) = 0;
	/**
	 * Draws an array of primitives to the screen
	 *
	 * It is recommended you use mtx::HWAPI::gfxDrawElements instead.
	 *
	 * @param type The type of primitive used
	 * @param vertexLayout The layout of the vertex buffer
	 * @param vertex_count The number of vertices within the
	 * buffer
	 * @param program The program to use
	 */
        virtual void gfxDrawArrays(PrimitiveType type, HWLayoutReference* vertexLayout, int vertex_count, HWProgramReference* program = 0) = 0;
        virtual void gfxUseLayout(HWLayoutReference* layout) = 0;

	/**
	 * Creates a new GPU buffer reference.
	 */
        virtual HWBufferReference* newBuffer() = 0;
	/**
	 * Creates a new GPU texture reference
	 *
	 * It is recommended you use mtx::HWAPI::loadCachedTexture instead, if
	 * you're simply trying to use a texture stored on disk. Even
	 * if its not cached yet, it will cache it for you. This
	 * creates a texture reference only.
	 */
        virtual HWTextureReference* newTexture() = 0;
	/**
	 * Creates a new GPU program reference
	 *
	 * It is recommended you use the mtx::Material system. That caches
	 * program's and automatically deals with linking them together
	 * for you. See mtx::Material::getMaterial
	 */
        virtual HWProgramReference* newProgram() = 0;
	/**
	 * Creates a new GPU layout reference
	 */
        virtual HWLayoutReference* newLayout() = 0;

        enum WindowType
        {
            HWWT_NORMAL,
            HWWT_NORMAL_RESIZABLE,
            HWWT_FULLSCREEN,
        };

	/**
	 * Creates a new window visible to the user for rendering in.
	 * You probably want to use mtx::App::newWindow instead, as that
	 * manages initializing the viewport automatically for you,
	 * and giving you an easier to work with mtx::Window class,
	 * rather then a mtx::HWWindowReference.
	 * 
	 * @param resX The X resolution of the window (can be changed)
	 * @param resY The Y resolution of the window (can be changed)
	 * @param type The type of the window (can be resizable, or fullscreen)
	 */
        virtual HWWindowReference* newWindow(int resX, int resY, WindowType type = HWWT_NORMAL) = 0;

	/**
	 * Loads a texture from the cache
	 *
	 * @param texture Texture name. If autoload is true, it will
	 * load this texture from the disk
	 * @param autoload Automatically loads the texture if it
	 * doesnt exist. Otherwise, return NULL if not found
	 */
        HWTextureReference* loadCachedTexture(const char* texture,
					      bool autoload = true);
	/**
	 * Adds a texture to the cache. This is if you load a texture
	 * from somewhere other then disk, but still want
	 * mtx::HWAPI::loadCachedTexture to work.
	 */
        void addTextureToCache(HWTextureReference* texture, const char* name);

        class EventListener
        {
        public:
            virtual void onQuit() = 0;
            virtual void onKeyDown(int key) = 0;
            virtual void onKeyUp(int key) = 0;

            virtual void onWindowClose(Window* window) {};
            virtual void onWindowSize(int w, int h, Window* window) {};

            virtual void onMouseDown(int x, int y, Window* window) {};
            virtual void onMouseUp(int x, int y, Window* window) {};
            virtual void onMouseMove(int x, int y, Window* window) {};
            virtual void onMouseMoveRel(int x, int y, Window* window) {};
        };

        std::vector<EventListener*> getListeners() { return m_listeners; }
        void addListener(EventListener* listener);
        virtual void pumpOSEvents() = 0;

        int getDrawnVertices() { return m_drawnVertices; }
        int getDrawCalls() { return m_drawCalls; }
    private:
        std::vector<EventListener*> m_listeners;
    };

    typedef std::function<HWAPI*()> HWAPIConstructor;
    template<typename T>
    T* HWAPIConstructorDefault()
    {
	return new T();
    }
};
