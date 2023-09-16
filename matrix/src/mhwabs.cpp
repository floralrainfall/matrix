#include <mhwabs.hpp>
#include <mdev.hpp>
#include <mfile.hpp>
#include <mapp.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace mtx
{
    void HWAPI::popParam()
    {
        m_hwParams.pop_back();
    }

    void HWAPI::pushParam(HWRenderParameter param)
    {
        m_hwParams.push_back(param);
    }

    HWTextureReference* HWAPI::loadCachedTexture(const char* texture, bool autoload)
    {
        auto it = m_cachedTextures.find(texture);
        if(it != m_cachedTextures.end())
            return m_cachedTextures[texture];
        else
        {
            if(!autoload)
                return NULL;
            std::FILE* texfile = App::getFileSystem()->open(texture);
            if(texfile)
            {
                DEV_MSG("caching new %s", texture);
                size_t filesize = FileSystem::getFileSize(texfile);
                char* filedata = (char*)malloc(filesize);
                fread(filedata, 1, filesize, texfile);

                HWTextureReference* textureref = newTexture();
                textureref->uploadCompressedTexture(filesize, (void*)filedata);
                m_cachedTextures[texture] = textureref;
                return textureref;
            }
            else
                DEV_MSG("could not open texture %s", texture);
            return NULL;
        }
    }

    void HWAPI::addTextureToCache(HWTextureReference* texture, const char* name)
    {
        DEV_MSG("caching pre-existing %s", name);
        m_cachedTextures[name] = texture;
    }

    void HWAPI::addListener(EventListener* listener)
    {
        m_listeners.push_back(listener);
    }

    HWBufferReference::~HWBufferReference()
    {

    }

    HWTextureReference::~HWTextureReference()
    {

    }

    bool HWTextureReference::uploadCompressedTexture(int size, void* data)
    {
        int resX;
        int resY;
        int comp;

        stbi_set_flip_vertically_on_load(true);
        stbi_uc* img_data = stbi_load_from_memory((const stbi_uc*)data, size, &resX, &resY, &comp, 4);
        if(!img_data)
            return false;

        DEV_MSG("read compressed %ix%i, c: %i image", resX, resY, comp);
        upload(glm::ivec2(resX, resY), img_data);
        return true;
    }

    HWProgramReference::~HWProgramReference()
    {

    }

    HWWindowReference::~HWWindowReference()
    {

    }

    HWLayoutReference::~HWLayoutReference()
    {

    }
}