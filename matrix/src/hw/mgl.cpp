#include <hw/mgl.hpp>
#include <mapp.hpp>
#include <mdev.hpp>
#include <sstream>

#define HWAPI_GL ((GL3API*)App::getHWAPI())

namespace mtx::gl
{
    GLTexture::GLTexture()
    {
        glGenTextures(1, &m_glTextureId);
    }

    GLTexture::~GLTexture()
    {
        glDeleteTextures(1, &m_glTextureId);
    }

    void GLTexture::upload(glm::ivec2 size, void* data, bool genMipMaps)
    {
        glBindTexture(GL_TEXTURE_2D, m_glTextureId);
        HWAPI_GL->checkError();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        HWAPI_GL->checkError();

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if(genMipMaps)
            glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        m_textureSize = size;

        DEV_MSG("uploaded %ix%i texture", size.x, size.y);
    }

    GLBuffer::GLBuffer()
    {
        glGenBuffers(1, &m_glBufferId);
        HWAPI_GL->checkError();
    }

    GLBuffer::~GLBuffer()
    {
        glDeleteBuffers(1, &m_glBufferId);
    }

    void GLBuffer::upload(int size, void* data)
    {
        DEV_MSG("uploading %i bytes to buffer %i", size, m_glBufferId);
        glBindBuffer(GL_ARRAY_BUFFER, m_glBufferId);
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
        HWAPI_GL->checkError();
    }

    void GL3API::gfxViewport(glm::vec4 viewport)
    {
        glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
        HWAPI_GL->checkError();
    }

    void GL3API::gfxClear(glm::vec4 color)
    {
        glClearColor(color.x, color.y, color.z, color.w);
        glClear(GL_COLOR_BUFFER_BIT);        
        HWAPI_GL->checkError();
    }

    void GL3API::gfxDrawElements(PrimitiveType type, HWLayoutReference* vertexLayout, int indice_count, HWBufferReference* indexBuffer, HWProgramReference* program)
    {
        GLuint glst = 0;
        switch(type)
        {
        case HWPT_LINES:
            glst = GL_LINES;
            break;
        case HWPT_POINTS:
            glst = GL_POINTS;
            break;  
        case HWPT_TRIANGLES:
            glst = GL_TRIANGLES;
            break;
        }

        if(program)
        {
            program->bind();
            bindToProgram(((GLProgram*)program)->getId());
        }    
        
        gfxUseLayout(vertexLayout);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ((GLBuffer*)indexBuffer)->getId());
        glDrawElements(glst, indice_count, GL_UNSIGNED_INT, 0);
        HWAPI_GL->checkError();
    }

    void GL3API::gfxDrawArrays(PrimitiveType type, HWLayoutReference* vertexLayout, int vertex_count, HWProgramReference* program)
    {
        GLuint glst = 0;
        switch(type)
        {
        case HWPT_LINES:
            glst = GL_LINES;
            break;
        case HWPT_POINTS:
            glst = GL_POINTS;
            break;  
        case HWPT_TRIANGLES:
            glst = GL_TRIANGLES;
            break;
        }

        if(program)
        {
            program->bind();
            bindToProgram(((GLProgram*)program)->getId());
        }    
        
        gfxUseLayout(vertexLayout);
        glDrawArrays(glst, 0, vertex_count);
        HWAPI_GL->checkError();
    }

    GLProgram::GLProgram()
    {
        m_glProgramId = glCreateProgram();
        DEV_ASSERT(m_glProgramId);
        HWAPI_GL->checkError();
    }

    GLProgram::~GLProgram()
    {
        glDeleteProgram(m_glProgramId);
    }

    void GLProgram::addShader(ShaderType type, const char* code)
    {
        GLuint glst = 0;
        switch(type)
        {
        case HWPST_VERTEX:
            glst = GL_VERTEX_SHADER;
            break;
        case HWPST_FRAGMENT:
            glst = GL_FRAGMENT_SHADER;
            break;  
        case HWPST_GEOMETRY:
            glst = GL_GEOMETRY_SHADER;
            break;
        }

        GLuint shader = glCreateShader(glst);
        DEV_ASSERT(shader);
        HWAPI_GL->checkError();
        m_unlinkedShaders.push_back(shader);

        GLint success;
        glShaderSource(shader, 1, &code, NULL);
        glCompileShader(shader);
        HWAPI_GL->checkError();
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            char info_log[512];
            glGetShaderInfoLog(shader, 512, NULL, info_log);
            DEV_MSG("shader compilation FAILED:\n%s", info_log);
        }
        else
            DEV_MSG("compiled shader (%04x)", glst);
    }

    void GLProgram::bind()
    {
        glUseProgram(m_glProgramId);
        HWAPI_GL->applyParamsToProgram(this);
    }

    void GLProgram::link()
    {
        DEV_MSG("linking %i shaders", m_unlinkedShaders.size());

        for(auto shader : m_unlinkedShaders)
            glAttachShader(m_glProgramId, shader);

        glLinkProgram(m_glProgramId);
        HWAPI_GL->checkError();
        
        GLint success;
        glGetProgramiv(m_glProgramId, GL_LINK_STATUS, &success);
        if(!success)
        {
            char info_log[512];
            glGetProgramInfoLog(m_glProgramId, 512, NULL, info_log);
            DEV_MSG("program linking FAILED:\n%s", info_log);
        }

        for(auto shader : m_unlinkedShaders)
            glDeleteShader(shader);
        m_unlinkedShaders.clear();
    }

    GLLayout::GLLayout()
    {
        glGenVertexArrays(1, &m_glLayoutId);
    };

    GLLayout::~GLLayout()
    {
        glDeleteVertexArrays(1, &m_glLayoutId);
    }

    void GLLayout::upload()
    {
        int vaid = 0;
        glBindVertexArray(m_glLayoutId);
        for(auto entry : m_entries)
        {
            glBindBuffer(GL_ARRAY_BUFFER, ((GLBuffer*)entry.buffer)->getId());
            GLenum typegl;
            switch(entry.type)
            {
            case HWT_FLOAT:
                typegl = GL_FLOAT;
                break;
            }
            glVertexAttribPointer(vaid, entry.components, typegl, entry.normalized, entry.stride, entry.offset);
            glEnableVertexArrayAttrib(m_glLayoutId, vaid);
            DEV_MSG("added vap to vao %i, type %i", vaid, typegl);
            vaid++;
        }
    }

    GL3API::GL3API()
    {
        DEV_MSG("initialized GL3API");
    }

    void GL3API::bindToProgram(GLuint program)
    {
        
    }

    void GL3API::checkError(const std::source_location location)
    {
        std::stringstream funcline;
        funcline << location.function_name() << ":" << location.line();
        GLenum gl_error = glGetError();
        switch(gl_error)
        {
        case GL_NO_ERROR:
            return;
        case GL_INVALID_ENUM:
            DEV_MSG("GL error: invalid enum (%04x) @ %s", gl_error, funcline.str().c_str());
            break;
        case GL_INVALID_OPERATION:
            DEV_MSG("GL error: invalid operation (%04x) @ %s", gl_error, funcline.str().c_str());
            break;
        case GL_INVALID_INDEX:
            DEV_MSG("GL error: invalid index (%04x) @ %s", gl_error, funcline.str().c_str());
            break;
        case GL_INVALID_VALUE:
            DEV_MSG("GL error: invalid value (%04x) @ %s", gl_error, funcline.str().c_str());
            break;
        case GL_OUT_OF_MEMORY: // the state is undefined after this, so in the future we should exit
            DEV_MSG("GL out of memory @ %s", funcline.str().c_str()); 
            break;
        case GL_STACK_UNDERFLOW:
            DEV_MSG("GL error: internal stack underflow @ %s", funcline.str().c_str());
            break;
        case GL_STACK_OVERFLOW:
            DEV_MSG("GL error: internal stack overflow @ %s", funcline.str().c_str());
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            DEV_MSG("GL error: invalid framebuffer operation (%04x) @ %s", gl_error, funcline.str().c_str());
            break;
        default:
            DEV_MSG("GL error: %04x @ %s", gl_error, funcline.str().c_str());
            break;
        }
        exit(-1);
    }

    void GL3API::gfxUseLayout(HWLayoutReference* layout)
    {
        GLLayout* gllayout = (GLLayout*)layout;
        glBindVertexArray(gllayout->getId());
    }

    void GL3API::applyParamsToProgram(HWProgramReference* program)
    {
        int usedTextureIds = 0;
        int maxTextureIds;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureIds);
        for(auto prop : m_hwParams)
        {
            int uniform = glGetUniformLocation(((GLProgram*)program)->getId(), prop.name.c_str());
            if(uniform != -1)
            {
                switch(prop.type)
                {
                case HWT_FLOAT:
                    glUniform1f(uniform, prop.data.f);
                    break;
                case HWT_VECTOR2:
                    glUniform2fv(uniform, 1, (float*)&prop.data);
                    break;
                case HWT_VECTOR3:
                    glUniform3fv(uniform, 1, (float*)&prop.data);
                    break;
                case HWT_VECTOR4:
                    glUniform4fv(uniform, 1, (float*)&prop.data);
                    break;
                case HWT_MATRIX2:
                    glUniformMatrix2fv(uniform, 1, GL_FALSE, (float*)&prop.data);
                    break;
                case HWT_MATRIX3:
                    glUniformMatrix3fv(uniform, 1, GL_FALSE, (float*)&prop.data);
                    break;
                case HWT_MATRIX4:
                    glUniformMatrix4fv(uniform, 1, GL_FALSE, (float*)&prop.data);
                    break;
                case HWT_TEXTURE:
                    DEV_ASSERT(usedTextureIds < maxTextureIds)
                    else
                    {
                        glActiveTexture(GL_TEXTURE0 + usedTextureIds);
                        glBindTexture(GL_TEXTURE_2D, ((GLTexture*)prop.data.tx)->getId());
                        glUniform1i(uniform, usedTextureIds++);
                    }
                    break;
                }

                checkError();
            }
        }
    }

    void GL3API::gfxClearDepth(float depth)
    {
        glClearDepth(depth);
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    HWBufferReference* GL3API::newBuffer()
    {
        return new GLBuffer();
    }

    HWTextureReference* GL3API::newTexture()
    {
        return new GLTexture();
    }

    HWProgramReference* GL3API::newProgram()
    {
        return new GLProgram();
    }

    HWLayoutReference* GL3API::newLayout()
    {
        return new GLLayout();
    }
}