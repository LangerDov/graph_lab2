#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "renderinterface.h"
#include "list.h"
#include "optional.h"
#include "matvec.h"
#include "vulkanbuffer.h"

VkInstance hGlobalInstance = nullptr;

#pragma warning(disable:4018)
#pragma warning(disable:4838)

const char* pExtensions[] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_MAINTENANCE1_EXTENSION_NAME
};
constexpr uint32_t uiExtCount = sizeof(pExtensions) / sizeof(const char*);

const char* validationLayers[] = {
	"VK_LAYER_KHRONOS_validation"
};

const int iFramesInFlight = 2;

struct vertex_t
{
	Vector pos;
	Vector color;
};

//VkVertexInputBindingDescription vertDesc = { 0, sizeof(vertex_t), VK_VERTEX_INPUT_RATE_VERTEX };
//
//VkVertexInputAttributeDescription attrDesc[3] = {
//	{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex_t, pos)},
//	{1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex_t, color)},
//};

float Q_rsqrt(float number)
{
	const float x2 = number * 0.5F;
	const float threehalfs = 1.5F;

	union {
		float f;
		uint32_t i;
	} conv = { number }; // member 'f' set to value of 'number'.
	conv.i = 0x5f3759df - (conv.i >> 1);
	conv.f *= threehalfs - x2 * conv.f * conv.f;
	return conv.f;
}



VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	((msgcallback_t)pUserData)(pCallbackData->pMessage);
	return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

VkResult DestroyDebugMessenger(VkInstance instance, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, *pDebugMessenger, pAllocator);
		return VK_SUCCESS;
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

VkDebugUtilsMessengerEXT debugMessenger = nullptr;
bool InitVulkan(msgcallback_t pCallbackFunc)
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Timure loh";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "piskaengine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
	if (pCallbackFunc)
	{
		debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugInfo.pfnUserCallback = debugCallback;
		debugInfo.pUserData = pCallbackFunc; // Optional
	}

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	uint32_t uiCount = 0;
	const char** pTemp = glfwGetRequiredInstanceExtensions(&uiCount);
	List<const char*> pExts(uiCount + 1);
	memcpy(pExts.Data(), pTemp, sizeof(const char*) * uiCount);
	pExts[uiCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	createInfo.enabledExtensionCount = uiCount + 1;
	createInfo.ppEnabledExtensionNames = pExts.Data();
	createInfo.ppEnabledLayerNames = validationLayers;
	createInfo.enabledLayerCount = 1;
	if (pCallbackFunc)
		createInfo.pNext = (const void*)&debugInfo;

	if (vkCreateInstance(&createInfo, nullptr, &hGlobalInstance))
		return false;

	if (pCallbackFunc)
		CreateDebugUtilsMessengerEXT(hGlobalInstance, &debugInfo, nullptr, &debugMessenger);

	return true;
}

void DestroyVulkan()
{
	DestroyDebugMessenger(hGlobalInstance, nullptr, &debugMessenger);
	vkDestroyInstance(hGlobalInstance, nullptr);
}

class CVulkanInterface : public IRenderInterface
{
public:
	CVulkanInterface();
	~CVulkanInterface();

	virtual bool Init(void* pWindow);
	virtual void Shutdown();

	virtual void SetViewport(int width, int height)
	{
		extent.width = width, extent.height = height;
		viewport.x = width, viewport.y = height;

		for (int i = 0; i < framebuffers.Size(); i++)
			vkDestroyFramebuffer(hDevice, framebuffers[i], nullptr);
		for (int i = 0; i < imageViews.Size(); i++)
			vkDestroyImageView(hDevice, imageViews[i], nullptr);
		vkDestroySwapchainKHR(hDevice, hSwapChain, nullptr);

		CreateSwapChains();
		CreateImageViews();
		CreateFramebuffers();
	}

	virtual IBuffer* CreateBuffer(size_t ulBufferSize, unsigned int iUsage, unsigned int iMemoryType)
	{
		// first check usage and memory flags
		VkBufferUsageFlags usageFlags = 0;
		uint32_t memoryFlags = 0;
		if (iUsage & vertex_buffer_bit)
			usageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		if (iUsage & index_buffer_bit)
			usageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		if (iUsage & uniform_buffer_bit)
			usageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		if (iUsage & transfer_src_bit)
			usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		if (iUsage & transfer_dst_bit)
			usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		if (iMemoryType & memory_ram)
			memoryFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		if (iMemoryType & memory_gpu)
			memoryFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		VkBuffer vkBuff = nullptr;
		VkDeviceMemory vkMemory = nullptr;
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.usage = usageFlags;
		bufferInfo.size = ulBufferSize;
		bufferInfo.queueFamilyIndexCount = 1;
		bufferInfo.pQueueFamilyIndices = (uint32_t*)&sFamilyIndices.cGraphicsFamilyIdx;
		if (vkCreateBuffer(hDevice, &bufferInfo, nullptr, &vkBuff))
			return nullptr;

		VkMemoryRequirements memReq = {};
		vkGetBufferMemoryRequirements(hDevice, vkBuff, &memReq);

		auto getMemIdx = [](VkMemoryRequirements& memReq, VkPhysicalDeviceMemoryProperties& memProp, uint32_t flags)
		{
			for (int i = 0; i < memProp.memoryTypeCount; i++)
			{
				if (memReq.memoryTypeBits & (1 << i) && (memProp.memoryTypes[i].propertyFlags & flags) == flags)
				{
					return i;
				}
			}

			return -1;
		};
		uint32_t uiMemIdx = getMemIdx(memReq, vkMemProperties, memoryFlags);
		if (uiMemIdx == (uint32_t)-1)
		{
			vkDestroyBuffer(hDevice, vkBuff, nullptr);
			return nullptr;
		}

		VkMemoryAllocateInfo memInfo = {};
		memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memInfo.allocationSize = memReq.size;
		memInfo.memoryTypeIndex = uiMemIdx;

		if (vkAllocateMemory(hDevice, &memInfo, nullptr, &vkMemory))
		{
			vkDestroyBuffer(hDevice, vkBuff, nullptr);
			return nullptr;
		}

		vkBindBufferMemory(hDevice, vkBuff, vkMemory, 0);

		CVulkanBuffer* pBuffer = new CVulkanBuffer(vkBuff, vkMemory, ulBufferSize, hDevice);
		buffers.Add(pBuffer);
		return pBuffer;
	}

	virtual void DeleteBuffer(IBuffer* pBuffer)
	{
		if (!pBuffer)
			return;

		vkFreeMemory(hDevice, ((CVulkanBuffer*)pBuffer)->GetVkMemory(), nullptr);
		vkDestroyBuffer(hDevice, ((CVulkanBuffer*)pBuffer)->GetVkBuffer(), nullptr);
		delete (CVulkanBuffer*)pBuffer;
	}

	virtual void BeginTransfer()
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkResetCommandBuffer(vkLoadBuffer, 0);
		vkBeginCommandBuffer(vkLoadBuffer, &beginInfo);
	}
	
	virtual bool CopyBuffer(IBufferRegion* pSource, IBufferRegion* pDestination,
		unsigned int size, unsigned int srcOffset, unsigned int dstOffset)
	{
		if (!pSource || !pDestination)
			return false;

		VkBufferCopy copyInfo = {};
		copyInfo.size = size;
		copyInfo.srcOffset = srcOffset + pSource->GetOffset();
		copyInfo.dstOffset = dstOffset + pDestination->GetOffset();

		vkCmdCopyBuffer(vkLoadBuffer, ((CVulkanBuffer*)pSource->GetBuffer())->GetVkBuffer(), 
			((CVulkanBuffer*)pDestination->GetBuffer())->GetVkBuffer(), 1, &copyInfo);
		return true;
	}

	virtual void EndTransfer(bool bWait)
	{
		vkEndCommandBuffer(vkLoadBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &vkLoadBuffer;

		vkQueueSubmit(sQueues.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		if (bWait)
			vkQueueWaitIdle(sQueues.graphicsQueue);
	}

	// RENDER COMMAND QUEUE

	// Begin recording command queue
	virtual void StartQueue();

//	virtual void PushConstant(void* pData, size_t ulSize)
//	{
////		vkCmdPushConstants(hCommandBuffs[currFrame], hPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, ulSize, pData);
////		pPipeline->PushConstant(pData, ulSize);
//	}
//
//	// use shaders to draw meshes
//	virtual void Draw(IBufferRegion* pRegion, unsigned int offset, unsigned int vertCount)
//	{
////		pPipeline->Draw(pRegion, offset, vertCount);
//	}
/*	virtual void DrawIndexed(IBufferRegion* pRegion, unsigned int offset, size_t indexOffset, unsigned int idxCount)
	{
		if (!_pBuffer)
			return;

		vulkandata_t* pBuffer = (vulkandata_t*)_pBuffer;
		VkBuffer pVkBuffer = pBuffer->pVkBuffer->GetVkBuffer();
		VkDeviceSize vertexOffset[] = { pBuffer->ulOffset };
		vkCmdBindVertexBuffers(hCommandBuffs[currFrame], 0, 1, &pVkBuffer, vertexOffset);
		vkCmdBindIndexBuffer(hCommandBuffs[currFrame], pVkBuffer, idxoffset, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(hCommandBuffs[currFrame], idxCount, 1, 0, 0, 0);
	}*/

	// End recording command queue
	virtual void EndQueue();
	virtual void WaitForDevice() { vkDeviceWaitIdle(hDevice); }

	virtual void* GetPhysDevice() { return hPhysDevice; }
	virtual void* GetDevice() { return hDevice; }
	virtual void* GetCurrentBuffer() { return hCommandBuffs[currFrame]; }
	virtual void* GetRenderPass() { return hRenderPass; }
	virtual void* GetGraphicsQueue() { return sQueues.graphicsQueue; }

private:
	bool CreateSurface();
	bool ChoosePhysicalDevice();
	bool CheckExtensionsSupport();
	bool GetFamilyIndices();
	bool CreateDevice();
	void GetQueues();
	bool CreateSwapChains();
	bool CreateImageViews();
	bool CreateRenderPass();
	bool CreateFramebuffers();
	bool CreateCommandBuffer();
	bool CreateSyncObjects();

private:
	GLFWwindow* pWindow;

private:
	VkSurfaceKHR hSurface;
	VkPhysicalDevice hPhysDevice;
	VkDevice hDevice;
	VkSwapchainKHR hSwapChain;
	List<VkImage> images;
	List<VkImageView> imageViews;
	VkRenderPass hRenderPass;
	List<VkFramebuffer> framebuffers;
	VkCommandPool hCommandPool;
	List<VkCommandBuffer> hCommandBuffs;
	VkCommandBuffer vkLoadBuffer;
	List<VkSemaphore> imgAvailable;
	List<VkSemaphore> renderFinish;
	List<VkFence> inFlightFences;
	VkPhysicalDeviceMemoryProperties vkMemProperties = {};

	int currFrame, imgIdx;

private:
	VkSurfaceFormatKHR format = {};
	VkPresentModeKHR presentMode = {};
	VkExtent2D extent = {};
	Vector2D viewport;

	struct
	{
		Optional<int> cGraphicsFamilyIdx;
		Optional<int> cPresentFamilyIdx;
	} sFamilyIndices;
	struct
	{
		VkQueue graphicsQueue = nullptr;
		VkQueue presentQueue = nullptr;
	} sQueues;

	struct
	{
		VkSurfaceCapabilitiesKHR capabilities = {};
		List<VkSurfaceFormatKHR> formats;
		List<VkPresentModeKHR> presentModes;
	} details;

private:
	List<CVulkanBuffer*> buffers;
};

CVulkanInterface::CVulkanInterface()
{
	pWindow = nullptr;
	hSurface = nullptr;
	hPhysDevice = nullptr;
	hDevice = nullptr;
	hSwapChain = nullptr;
	hRenderPass = nullptr;
	hCommandPool = nullptr;
	vkLoadBuffer = nullptr;
	currFrame = imgIdx = 0;
}

CVulkanInterface::~CVulkanInterface()
{
}

bool CVulkanInterface::Init(void* _pWindow)
{
	pWindow = (GLFWwindow*)_pWindow;

	if (!CreateSurface())
		return false;
	if (!ChoosePhysicalDevice())
		return false;
	if (!CheckExtensionsSupport())
		return false;
	if (!GetFamilyIndices())
		return false;
	if (!CreateDevice())
		return false;
	GetQueues();
	if (!CreateSwapChains())
		return false;
	if (!CreateImageViews())
		return false;
	if (!CreateRenderPass())
		return false;
	if (!CreateFramebuffers())
		return false;
	if (!CreateCommandBuffer())
		return false;
	if (!CreateSyncObjects())
		return false;

	return true;
}

void CVulkanInterface::Shutdown()
{
	for (int i = 0; i < iFramesInFlight; i++)
	{
		vkDestroyFence(hDevice, inFlightFences[i], nullptr);
		vkDestroySemaphore(hDevice, imgAvailable[i], nullptr);
		vkDestroySemaphore(hDevice, renderFinish[i], nullptr);
	}
	vkFreeCommandBuffers(hDevice, hCommandPool, 1, &vkLoadBuffer);
	vkFreeCommandBuffers(hDevice, hCommandPool, 2, hCommandBuffs.Data());
	vkDestroyCommandPool(hDevice, hCommandPool, nullptr);
	for (int i = 0; i < framebuffers.Size(); i++)
		vkDestroyFramebuffer(hDevice, framebuffers[i], nullptr);
//	vkDestroyDescriptorSetLayout(hDevice, hDescLayout, nullptr);
	vkDestroyRenderPass(hDevice, hRenderPass, nullptr);
	for (int i = 0; i < imageViews.Size(); i++)
		vkDestroyImageView(hDevice, imageViews[i], nullptr);
	vkDestroySwapchainKHR(hDevice, hSwapChain, nullptr);
	vkDestroyDevice(hDevice, nullptr);
	vkDestroySurfaceKHR(hGlobalInstance, hSurface, nullptr);
}

void CVulkanInterface::StartQueue()
{
	vkWaitForFences(hDevice, 1, &inFlightFences[currFrame], true, UINT64_MAX);
	vkResetFences(hDevice, 1, &inFlightFences[currFrame]);
	VkResult result = vkAcquireNextImageKHR(hDevice, hSwapChain, UINT64_MAX, imgAvailable[currFrame], nullptr, ((unsigned int*)&imgIdx));
	vkResetCommandBuffer(hCommandBuffs[currFrame], 0);

	VkCommandBuffer comBuf = hCommandBuffs[currFrame];
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional
	if (vkBeginCommandBuffer(comBuf, &beginInfo))
		return;

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = hRenderPass;
	renderPassInfo.framebuffer = framebuffers[imgIdx];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = extent;
	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(comBuf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	VkViewport viewport = {};
	viewport.width = this->viewport.x;
	viewport.height = -this->viewport.y;
	viewport.x = 0.f;
	viewport.y = this->viewport.y;
	viewport.maxDepth = 1.f;
	viewport.minDepth = 0.f;
	vkCmdSetViewport(comBuf, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.extent = extent;
	scissor.offset = { 0, 0 };
	vkCmdSetScissor(comBuf, 0, 1, &scissor);
}

void CVulkanInterface::EndQueue()
{
	vkCmdEndRenderPass(hCommandBuffs[currFrame]);
	if (vkEndCommandBuffer(hCommandBuffs[currFrame]))
		return;

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imgAvailable[currFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &hCommandBuffs[currFrame];

	VkSemaphore signalSemaphores[] = { renderFinish[currFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(sQueues.graphicsQueue, 1, &submitInfo, inFlightFences[currFrame]))
		exit(1);

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	VkSwapchainKHR swapChains[] = { hSwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = (unsigned int*)&imgIdx;
	vkQueuePresentKHR(sQueues.presentQueue, &presentInfo);

	currFrame = (currFrame + 1) % iFramesInFlight;
}

bool CVulkanInterface::CreateSurface()
{
	return !glfwCreateWindowSurface(hGlobalInstance, pWindow, nullptr, &hSurface);
}

bool CVulkanInterface::ChoosePhysicalDevice()
{
	// iterate gpus and choose discrete one
	uint32_t uiGpuCount = 0;
	hPhysDevice = nullptr;
	vkEnumeratePhysicalDevices(hGlobalInstance, &uiGpuCount, nullptr);
	// create list for those gpus
	List<VkPhysicalDevice> devices(uiGpuCount);
	vkEnumeratePhysicalDevices(hGlobalInstance, &uiGpuCount, devices.Data());
	if (!devices.Size())
		return false;

	for (int i = 0; i < uiGpuCount; i++)
	{
		VkPhysicalDevice device = devices[i];
		VkPhysicalDeviceProperties properties = {};
		VkPhysicalDeviceFeatures features = {};
		vkGetPhysicalDeviceProperties(device, &properties);
		vkGetPhysicalDeviceFeatures(device, &features);
		hPhysDevice = device;
		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
			return true;
	}

	if (!hPhysDevice)
		hPhysDevice = devices[0];
	return true;
}

bool CVulkanInterface::CheckExtensionsSupport()
{
	// check extension support
	uint32_t count = 0;
	uint32_t uiCount = 0;
	vkEnumerateDeviceExtensionProperties(hPhysDevice, nullptr, &uiCount, nullptr);
	List<VkExtensionProperties> properties(uiCount);
	vkEnumerateDeviceExtensionProperties(hPhysDevice, nullptr, &uiCount, properties.Data());
	for (int i = 0; i < uiCount; i++)
	{
		for (int j = 0; j < uiExtCount; j++)
		{
			if (!strcmp(properties[i].extensionName, pExtensions[j]))
				count++;
		}
	}

	if (count < uiExtCount)
		return false;

	vkGetPhysicalDeviceMemoryProperties(hPhysDevice, &vkMemProperties);

	// query swap chain details
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(hPhysDevice, hSurface, &details.capabilities);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(hPhysDevice, hSurface, &formatCount, nullptr);
	if (!formatCount)
		return false;
	details.formats.Resize(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(hPhysDevice, hSurface, &formatCount, details.formats.Data());

	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(hPhysDevice, hSurface, &presentModeCount, nullptr);
	if (!presentModeCount)
		return false;
	details.presentModes.Resize(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(hPhysDevice, hSurface, &presentModeCount, details.presentModes.Data());

	// first choose format and present mode
	for (int i = 0; i < details.formats.Size(); i++)
	{
		if (details.formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
			details.formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			format = details.formats[i];
	}

	for (int i = 0; i < details.presentModes.Size(); i++)
		if (details.presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
			presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

	extent = details.capabilities.currentExtent;
	viewport.x = extent.width, viewport.y = extent.height;

	return true;
}

bool CVulkanInterface::GetFamilyIndices()
{
	uint32_t uiCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(hPhysDevice, &uiCount, nullptr);
	List<VkQueueFamilyProperties> list(uiCount);
	vkGetPhysicalDeviceQueueFamilyProperties(hPhysDevice, &uiCount, list.Data());
	for (int i = 0; i < uiCount; i++)
	{
		if (list[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			sFamilyIndices.cGraphicsFamilyIdx = i;

		VkBool32 support = 0;
		vkGetPhysicalDeviceSurfaceSupportKHR(hPhysDevice, i, hSurface, &support);
		if (support)
			sFamilyIndices.cPresentFamilyIdx = i;
	}

	if (sFamilyIndices.cGraphicsFamilyIdx.HasValue() && sFamilyIndices.cPresentFamilyIdx.HasValue())
		return true;

	return false;
}

bool CVulkanInterface::CreateDevice()
{
	VkPhysicalDeviceFeatures features = {};

	int indices[] = { sFamilyIndices.cGraphicsFamilyIdx.GetValue(), 
		sFamilyIndices.cPresentFamilyIdx.GetValue()};
	List<VkDeviceQueueCreateInfo> queueInfos(2);
	float queuePriority = 1.0f;
	for (int i = 0; i < queueInfos.Size(); i++)
	{
		VkDeviceQueueCreateInfo& queueCreateInfo = queueInfos[i];
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices[i];
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
	}

	VkDeviceCreateInfo deviceInfo = {};

	vkGetPhysicalDeviceFeatures(hPhysDevice, &features);

	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pQueueCreateInfos = queueInfos.Data();
	deviceInfo.queueCreateInfoCount = queueInfos.Size();
	deviceInfo.pEnabledFeatures = &features;
	deviceInfo.enabledExtensionCount = uiExtCount;
	deviceInfo.ppEnabledExtensionNames = pExtensions;
	deviceInfo.enabledLayerCount = 0;
	if (!vkCreateDevice(hPhysDevice, &deviceInfo, nullptr, &hDevice))
		return true;

	return false;
}

void CVulkanInterface::GetQueues()
{
	vkGetDeviceQueue(hDevice, sFamilyIndices.cGraphicsFamilyIdx.GetValue(), 0, &sQueues.graphicsQueue);
	vkGetDeviceQueue(hDevice, sFamilyIndices.cPresentFamilyIdx.GetValue(), 0, &sQueues.presentQueue);
}

bool CVulkanInterface::CreateSwapChains()
{
	uint32_t minImgCount = details.capabilities.minImageCount + 1;
	VkSwapchainCreateInfoKHR swapChainInfo = {};
	uint32_t indices[] = { sFamilyIndices.cGraphicsFamilyIdx.GetValue(), sFamilyIndices.cPresentFamilyIdx.GetValue() };
	swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainInfo.surface = hSurface;
	swapChainInfo.minImageCount = minImgCount;
	swapChainInfo.imageFormat = format.format;
	swapChainInfo.imageColorSpace = format.colorSpace;
	swapChainInfo.imageExtent = extent;
	swapChainInfo.imageArrayLayers = 1;
	swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
	swapChainInfo.queueFamilyIndexCount = 2;
	swapChainInfo.pQueueFamilyIndices = indices;
	swapChainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainInfo.presentMode = presentMode;
	swapChainInfo.clipped = VK_TRUE;
	swapChainInfo.oldSwapchain = nullptr;

	if (vkCreateSwapchainKHR(hDevice, &swapChainInfo, nullptr, &hSwapChain))
		return false;

	uint32_t imageCount = 0;
	vkGetSwapchainImagesKHR(hDevice, hSwapChain, &imageCount, nullptr);
	images.Resize(imageCount);
	vkGetSwapchainImagesKHR(hDevice, hSwapChain, &imageCount, images.Data());

	return true;
}

bool CVulkanInterface::CreateImageViews()
{
	imageViews.Resize(images.Size());
	for (int i = 0; i < imageViews.Size(); i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = format.format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(hDevice, &createInfo, nullptr, &imageViews[i]))
			return false;
	}

	return true;
}

bool CVulkanInterface::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = format.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(hDevice, &renderPassInfo, nullptr, &hRenderPass))
		return false;

	return true;
}
bool CVulkanInterface::CreateFramebuffers()
{
	framebuffers.Resize(imageViews.Size());
	for (int i = 0; i < framebuffers.Size(); i++)
	{
		VkImageView attachments[] = {
			imageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = hRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(hDevice, &framebufferInfo, nullptr, &framebuffers[i]))
			return false;
	}

	return true;
}

bool CVulkanInterface::CreateCommandBuffer()
{
	hCommandBuffs.Resize(iFramesInFlight);
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = sFamilyIndices.cGraphicsFamilyIdx.GetValue();

	if (vkCreateCommandPool(hDevice, &poolInfo, nullptr, &hCommandPool))
		return false;

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = hCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = iFramesInFlight;

	if (vkAllocateCommandBuffers(hDevice, &allocInfo, hCommandBuffs.Data()) != VK_SUCCESS)
		return false;

	allocInfo.commandBufferCount = 1;
	if (vkAllocateCommandBuffers(hDevice, &allocInfo, &vkLoadBuffer))
		return false;

	return true;
}

bool CVulkanInterface::CreateSyncObjects()
{
	imgAvailable.Resize(iFramesInFlight);
	renderFinish.Resize(iFramesInFlight);
	inFlightFences.Resize(iFramesInFlight);

	VkFenceCreateInfo fence = {};
	fence.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkSemaphoreCreateInfo semaphore = {};
	semaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	for (int i = 0; i < iFramesInFlight; i++)
	{
		if (vkCreateSemaphore(hDevice, &semaphore, nullptr, &imgAvailable[i]) ||
			vkCreateSemaphore(hDevice, &semaphore, nullptr, &renderFinish[i]) ||
			vkCreateFence(hDevice, &fence, nullptr, &inFlightFences[i]))
			return false;
	}

	return true;
}

IRenderInterface* CreateVulkanInterface()
{
	return new CVulkanInterface();
}