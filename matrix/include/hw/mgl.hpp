#pragma once
#ifndef GL_ENABLED
#error "hw/mgl.hpp in use, but GL_ENABLED not defined"
#endif
#include <mhwabs.hpp>
#include <glad/glad.h>
#include <vector>
#include <source_location>

namespace mtx::gl
{
    class GLBuffer : public HWBufferReference 
    {
        GLuint m_glBufferId;
    public:
        GLBuffer();
        virtual ~GLBuffer();

        virtual void upload(int size, void* data);

        GLuint getId() { return m_glBufferId; }
    };

    class GLTexture : public HWTextureReference
    {
        GLuint m_glTextureId;
	int m_uploadedTextures;
    public:
        GLTexture();
        virtual ~GLTexture();

        virtual void upload(glm::ivec2 size, void* data, bool genMipMaps = true);
        virtual void uploadRGB(glm::ivec2 size, void* data, bool genMipMaps = true);
	virtual void uploadCubemap(glm::ivec2 size,
				   void* data,
				   HWTextureCubemapDirection direction);

        void setFilter(GLenum min, GLenum mag);

        GLuint getId() { return m_glTextureId; }
	GLenum getBuffer();
    };
    
    class GLProgram : public HWProgramReference
    {
        GLuint m_glProgramId;
        std::vector<GLuint> m_unlinkedShaders;
    public:
        GLProgram();
        virtual ~GLProgram();

        virtual void addShader(ShaderType type,
			       const char* code,
			       size_t code_size);
        virtual void bind();
        virtual void link();

        GLuint getId() { return m_glProgramId; }
    };

    class GLLayout : public HWLayoutReference
    {
        GLuint m_glLayoutId;
    public:
        GLLayout();
        virtual ~GLLayout();

        virtual void upload();

        GLuint getId() { return m_glLayoutId; }
    };

    // GL3API is **NOT** meant to be used on its own, and is meant to be further subclassed to manage windows/events/etc
    // look at SDLGLAPI in hw/msdl.hpp and hw/msdl.cpp for an example
    // of a full HWAPI 
    class GL3API : public virtual HWAPI
    {
        bool m_glReady;
    public:
        GL3API();

        void bindToProgram(GLuint program);
        void checkError(const std::source_location location = std::source_location::current());
        
        virtual void applyParamsToProgram(HWProgramReference* program);
        virtual void gfxViewport(glm::vec4 viewport);
        virtual void gfxClear(glm::vec4 color);
        virtual void gfxClearDepth(float depth);
        virtual void gfxDrawElements(PrimitiveType type, HWLayoutReference* vertexLayout, int indice_count, HWBufferReference* indexBuffer, HWProgramReference* program = 0);
        virtual void gfxDrawArrays(PrimitiveType type, HWLayoutReference* vertexLayout, int vertex_count, HWProgramReference* program = 0);
        virtual void gfxUseLayout(HWLayoutReference* layout);

        virtual HWBufferReference* newBuffer();
        virtual HWTextureReference* newTexture();
        virtual HWProgramReference* newProgram();
        virtual HWLayoutReference* newLayout();
	
	virtual void gfxBeginFrame(Viewport* viewport);

        bool getGlReady() { return m_glReady; }
        void setGlReady(bool gl) { m_glReady = gl; }

	virtual std::string getShaderPrefix() { return "gl3"; }
    };
};
