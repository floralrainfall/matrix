#pragma once
#ifndef VK_ENABLED
#error "hw/mvk.hpp in use, but VK_ENABLED not defined"
#endif
#include <mhwabs.hpp>
#include <vulkan/vulkan.hpp>

namespace mtx::vk
{
    class VKBuffer : public HWBufferReference
    {
    public:
	VKBuffer();
	virtual ~VKBuffer();

	virtual void upload(int size, void* data);
    };

    class VKTexture : public HWTextureReference
    {
    public:
	VKTexture();
	virtual ~VKTexture();

	virtual void upload(glm::ivec2 size, void* data, bool
			    genMipMaps = true);
	virtual void uploadRGB(glm::ivec2 size, void* data, bool
			       genMipMaps = true);	
    };

    class VKProgram : public HWProgramReference
    {
	std::map<ShaderType, VkShaderModule> m_shaderModules;
    public:
	VKProgram();
	virtual ~VKProgram();

	virtual void addShader(ShaderType type,
			       const char* code,
			       size_t code_size);
	virtual void bind();
	virtual void link();
    };

    class VKLayout : public HWLayoutReference
    {
    public:
	VKLayout();
	virtual ~VKLayout();
	virtual void upload();
    };

    struct VKSwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
    };
    
    class VKAPI : public virtual HWAPI
    {
	VkApplicationInfo m_vkApp;
	VkInstance m_vkInstance;
	VkPhysicalDevice m_vkPhysicalDevice;
	VkDevice m_vkDevice;
	VkSurfaceKHR* m_surface;
	unsigned int m_queueCount;
	unsigned int m_graphicsQueue;
	VkQueue m_vkGraphicsQueue;
	bool m_initialized;
    public:
	VKAPI();

	void firstTimeInit(const char** extension_names, unsigned int count);
	void setSurface(VkSurfaceKHR* surface) { m_surface = surface; };

	VkPhysicalDevice getPhysicalDevice()
	{
	    return m_vkPhysicalDevice;
	}

	VkDevice getDevice() { return m_vkDevice; }
	
	unsigned int getSurfacePresentQueue(VkSurfaceKHR surface);
	unsigned int getGraphicsQueue() { return m_graphicsQueue; };
	
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

	VkApplicationInfo* getApplicationInfo() { return &m_vkApp; }

	VkInstance getInstance() { return m_vkInstance; }

	virtual std::string getShaderPrefix() { return "vk"; }
    };
}
