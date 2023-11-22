#include <mhwabs.hpp>
#include <mdev.hpp>
#include <mfile.hpp>
#include <mapp.hpp>

#ifdef __unix__
#include <dlfcn.h>
#endif

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

    void HWAPI::loadResources()
    {
	m_whiteTexture = newTexture();
	unsigned char image[] = {
	    0xff, 0xff, 0xff, 0xff
	};
	m_whiteTexture->upload(glm::ivec2(1,1), &image, false);

	DEV_MSG("loaded HWAPI resources");
    }

    void HWAPI::frameStart()
    {
        m_drawnVertices = 0;
        m_drawCalls = 0;
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

    bool HWTextureReference::uploadCompressedCubemap(int size,
						     void* data,
						     HWTextureCubemapDirection
						     dir)
    {
        int resX;
        int resY;
        int comp;

        stbi_set_flip_vertically_on_load(true);
        stbi_uc* img_data = stbi_load_from_memory((const stbi_uc*)data, size, &resX, &resY, &comp, 4);
        if(!img_data)
            return false;

        DEV_MSG("read compressed %ix%i, c: %i image", resX, resY, comp);
        uploadCubemap(glm::ivec2(resX, resY), img_data, dir);
        return true;
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

    void HWTextureReference::loadCubemap(char** cubemap_textures)
    {
	for(int i = 0; i < __HWTCD_MAX; i++)
	{
	    char* tex = cubemap_textures[i];
	    DEV_MSG("loading cubemap %s into %i", tex, i);
	    std::FILE* texfile = App::getFileSystem()->open(tex);
	    size_t size = FileSystem::getFileSize(texfile);
	    char* filedata = (char*)malloc(size);
	    fread(filedata, 1, size, texfile);
	    uploadCompressedCubemap(size, (void*)filedata,
				    (HWTextureCubemapDirection)i);
	    free(filedata);
	}
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

    HWAPILib::HWAPILib(const char* hwapi_file)
    {
	m_hwnd = dlopen(hwapi_file, RTLD_LAZY | RTLD_GLOBAL);

	if(!m_hwnd)
	{
	    DEV_ERROR("HWAPILib %s does not exist", hwapi_file);
	    m_hlFunc = 0;
	    return;
	}
	
	m_hlFunc =
	    reinterpret_cast<HLFunction>(dlsym(m_hwnd,
					       "__MatrixMain"));

	if(!m_hlFunc)
	    DEV_ERROR("HWAPILib %s has no __MatrixMain defined",
		      hwapi_file);
    }

    std::map<std::string, HWAPIConstructor>
    HWAPILib::getConstructors()
    {
	HLContext ctx;
	ctx.request = HLR_LIST_HWAPIS;
	HLResponse* rsp = m_hlFunc(ctx);
	std::map<std::string, HWAPIConstructor>
			  ctrs;
	if(rsp->status != HLS_SUCCESS)
	{
	    DEV_SOFTWARN("HWAPILib getConstructors error (%i)",
			 rsp->status);
	    return ctrs;
	}
	for(int i = 0; i < rsp->data.listHwapis.entryCount; i++)
	{
	    HLHWAPIEntry entry = rsp->data.listHwapis.entries[i];
	    ctrs[std::string(entry.name)] = entry.ctor;
	}
	free(rsp);
	return ctrs;
    }

    static App* __dummyMain(int argc, char** argv)
    {
	DEV_ERROR("Hi this HWAPILib does not support HLR_GET_MAIN_ROUTINE lol bye");
	exit(-1);
	return 0;
    }	

    MainRoutine HWAPILib::getMainRoutine()
    {
	HLContext ctx;
	ctx.request = HLR_GET_MAIN_ROUTINE;
	HLResponse* rsp = m_hlFunc(ctx);
	if(rsp->status != HLS_SUCCESS)
	{
	    DEV_SOFTWARN("HWAPILib getMainRoutine error (%i)",
			 rsp->status);
	    return __dummyMain;
	}
	MainRoutine routine = rsp->data.mainRoutine.routine;
	free(rsp);
	return routine;
    }
}
