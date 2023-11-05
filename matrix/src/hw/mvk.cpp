#include <hw/mvk.hpp>
#include <mapp.hpp>
#include <mdev.hpp>

#define HWAPI_VK (dynamic_cast<VKAPI*>(App::getHWAPI()))

namespace mtx::vk
{
    ConVar vk_applicationname = ConVar("vk_applicationname", "",
				       "Matrix Game Window");
    ConVar vk_enginename = ConVar("vk_enginename", "",
				  "Matrix");
    
    VKAPI::VKAPI()
    {
	m_vkApp = {
	    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
	    .pNext = NULL,
	    .pApplicationName = vk_applicationname.getString().c_str(),
	    .applicationVersion = 0,
	    .pEngineName = vk_enginename.getString().c_str(),
	    .engineVersion = 0,
	    .apiVersion = VK_API_VERSION_1_0,
	};
	m_initialized = false;
    }

    unsigned int VKAPI::getSurfacePresentQueue(VkSurfaceKHR surface)
    {
	VkBool32 support = 0;
	for(int i = 0; i < m_queueCount; i++) {
	    vkGetPhysicalDeviceSurfaceSupportKHR(m_vkPhysicalDevice,
						 i,
						 surface,
						 &support);
	    if(support)
	    {
		return i;
	    }
	}

	DEV_WARN("could not find the present queue family");

	return UINT32_MAX;
    }

    // TODO: add thing that creates the debug messenger handle 
    static VKAPI_ATTR VkBool32 VKAPI_CALL __debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
    {
	DEV_MSG("VULKAN: validation layer: %s", pCallbackData->pMessage);
	return VK_FALSE;
    }

    
    void VKAPI::firstTimeInit(const char** extension_names, unsigned int count)
    {
	if(m_initialized)
	    return;

	VkResult result;

	const char* wanted_validation[] = {
	    "VK_LAYER_KHRONOS_validation",
	};
	unsigned int wanted_validation_count =
	    sizeof(wanted_validation) / sizeof(char*);
	
	const char* wanted_extension[] = {
	    VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	unsigned int wanted_extension_count =
	    sizeof(wanted_extension) / sizeof(char*);
	
	unsigned int layer_count;
	vkEnumerateInstanceLayerProperties(&layer_count, NULL);
	std::vector<VkLayerProperties> available_layers(layer_count);
	vkEnumerateInstanceLayerProperties(&layer_count,
					   available_layers.data());

	std::vector<const char*> existing_validation;
	
	for(int i = 0; i < wanted_validation_count; i++)
	{
	    bool found = false;
	    for(int j = 0; j < layer_count; j++)
	    {
		if(strcmp(wanted_validation[i],
			  available_layers[j].layerName) == 0) {
		    found = true;
		    existing_validation.push_back(wanted_validation[i]);
		    break;
		}
	    }

	    if(!found)
		DEV_WARN("could not load layer %s", wanted_validation[i]);
	}

	unsigned int extension_loaded_count;
	vkEnumerateInstanceExtensionProperties(NULL,
					       &extension_loaded_count,
					       NULL);
	std::vector<VkExtensionProperties>
	    extension_properties(extension_loaded_count);
	vkEnumerateInstanceExtensionProperties(NULL,
					       &extension_loaded_count,
					       extension_properties.data());

	for(int i = 0; i < extension_loaded_count; i++)
	    DEV_MSG("Vulkan has instance extension %s",
		    extension_properties[i].extensionName);
	
	m_initialized = true;
	VkInstanceCreateInfo inst_info = {
	    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
	    .pNext = NULL,
	    .pApplicationInfo = HWAPI_VK->getApplicationInfo(),
	    .enabledLayerCount = existing_validation.size(),
	    .ppEnabledLayerNames = existing_validation.data(),
	    .enabledExtensionCount = count,
	    .ppEnabledExtensionNames = extension_names
	};
	
	result = vkCreateInstance(&inst_info, NULL, &m_vkInstance);
	
	unsigned int gpu_count = 16;
	vkEnumeratePhysicalDevices(m_vkInstance,
				   &gpu_count,
				   NULL);
	std::vector<VkPhysicalDevice> physical_devices(gpu_count);
	vkEnumeratePhysicalDevices(m_vkInstance,
				   &gpu_count,
				   physical_devices.data());

	bool success = false;
	for(int di = 0; di < gpu_count; di++)
	{
	    m_vkPhysicalDevice = physical_devices[di];
	
	    unsigned int queue_count;
	    vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice,
						     &queue_count, NULL);
	    DEV_ASSERT(queue_count >= 1);

	    std::vector<VkQueueFamilyProperties> queue_props(queue_count);
	    vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice,
						     &queue_count,
						     queue_props.data());
	
	    m_queueCount = queue_count;
	
	    VkPhysicalDeviceFeatures features{};

	    uint32_t graphics_node_idx = UINT32_MAX;
	    for(int i = 0; i < queue_count; i++) {
		if((queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		   != 0)
		{
		    graphics_node_idx = i;
		    break;
		}
	    }

	    if(graphics_node_idx == UINT32_MAX)
		DEV_WARN("could not find the graphics queue family");

	    m_graphicsQueue = graphics_node_idx;
		    
	    unsigned int dev_extension_loaded_count;
	    vkEnumerateDeviceExtensionProperties(m_vkPhysicalDevice,
						 NULL,
						 &dev_extension_loaded_count,
						 NULL);
	    std::vector<VkExtensionProperties>
		dev_extension_properties(extension_loaded_count);
	    vkEnumerateDeviceExtensionProperties(m_vkPhysicalDevice,
						 NULL,
						 &dev_extension_loaded_count,
						 dev_extension_properties.data());

	    bool next = false;
	    for(int i = 0; i < wanted_extension_count; i++)
	    {
		bool found = false;
		for(int j = 0; j < dev_extension_loaded_count; j++)
		    if(strcmp(wanted_extension[i],
			      dev_extension_properties[j].extensionName)==0)
		    {
			found = true;
			break;
		    }
		if(!found)
		{
		    DEV_MSG("could not find extension %s\n", wanted_extension[i]);
		    next = true;
		    break;
		}
	    }

	    if(next)
	    {
		DEV_MSG("gpu %i doesnt have extensions");
		continue;
	    }

	    DEV_MSG("creating device on gpu %i", di);
		
	    float queue_priorities[1] = {1.0};
	    const VkDeviceQueueCreateInfo queue = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.pNext = NULL,
		.queueFamilyIndex = graphics_node_idx,
		.queueCount = 1,
		.pQueuePriorities  = queue_priorities
	    };

	    VkDeviceCreateInfo device = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = NULL,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queue,	    
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = 0,
		.enabledExtensionCount = wanted_extension_count,
		.ppEnabledExtensionNames = wanted_extension,
		.pEnabledFeatures = &features,
	    };
	    
	    result = vkCreateDevice(m_vkPhysicalDevice, &device, NULL,
				    &m_vkDevice);

	    if(result != VK_SUCCESS)
	    {
		DEV_MSG("vkCreateDevice failed on device %i", di);
		continue;
	    }

	    success = true;
	    break;
	}

	if(success)
	{
	    vkGetDeviceQueue(m_vkDevice,
			     m_graphicsQueue, 0,
			     &m_vkGraphicsQueue);
	}
	else
	    DEV_WARN("could not find valid VkPhysicalDevice to use");
	
	DEV_MSG("created vulkan device");
    }

    void VKAPI::applyParamsToProgram(HWProgramReference* program)
    {
	
    }

    void VKAPI::gfxViewport(glm::vec4 viewport)
    {

    }

    void VKAPI::gfxClear(glm::vec4 color)
    {

    }

    void VKAPI::gfxClearDepth(float depth)
    {

    }

    void VKAPI::gfxDrawElements(PrimitiveType type,
				HWLayoutReference* layout,
				int indice_count,
				HWBufferReference* index_buffer,
				HWProgramReference* program)
    {
	
    }

    void VKAPI::gfxDrawArrays(PrimitiveType type,
			      HWLayoutReference* layout,
			      int vertex_count,
			      HWProgramReference* program)
    {

    }

    void VKAPI::gfxUseLayout(HWLayoutReference* layout)
    {

    }

    VKBuffer::VKBuffer()
    {

    }

    VKBuffer::~VKBuffer()
    {

    }

    void VKBuffer::upload(int size, void* data)
    {

    }

    HWBufferReference* VKAPI::newBuffer()
    {
	return new VKBuffer();
    }

    VKTexture::VKTexture()
    {

    }

    VKTexture::~VKTexture()
    {

    }

    void VKTexture::upload(glm::ivec2 size, void* data, bool genMipMaps)
    {

    }

    void VKTexture::uploadRGB(glm::ivec2 size, void* data, bool genMipMaps)
    {

    }
	
    HWTextureReference* VKAPI::newTexture()
    {
	return new VKTexture();

    }

    VKProgram::VKProgram()
    {

    }

    VKProgram::~VKProgram()
    {

    }

    void VKProgram::addShader(ShaderType type, const char* code)
    {

    }

    void VKProgram::bind()
    {

    }

    void VKProgram::link()
    {

    }

    HWProgramReference* VKAPI::newProgram()
    {
	return new VKProgram();
    }

    VKLayout::VKLayout()
    {

    }

    VKLayout::~VKLayout()
    {

    }

    void VKLayout::upload()
    {
	
    }
	
    HWLayoutReference* VKAPI::newLayout()
    {
	return new VKLayout();
    }
}
