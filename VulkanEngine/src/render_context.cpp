#include "render_context.h"

#include "window.h"
#include <GLFW/glfw3.h>


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "constants.h"
#include <glm/gtc/matrix_transform.hpp>
#include <set>
#include <cstdint>
#include <algorithm>
#include <string>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <array>
#include <chrono>
#include "platform_types.h"
#include "hierarchy_context.h"
#include "geometry_primitives.h"


VKE::RenderContext::RenderContext() {

}

VKE::RenderContext::~RenderContext() {
	cleanup();
}


VkResult VKE::RenderContext::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void VKE::RenderContext::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, debugMessenger, pAllocator);
	}
}


void VKE::RenderContext::init(VKE::Window *window, HierarchyContext* hier_context) {

	window_ = window;
	hier_context_ = hier_context;

	createInstance();
	setupDebugMessenger();
	createSurface(window_->window);
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createCommandPool();
	createDepthResources();
	createRenderPass();
	createFramebuffers();
	createDescriptorSetlayout();
	createDescriptorPool();
	setUpResources();
	createQuadscreenEntity();
	//createBuffer();
	//createUniformBuffers();
	//createTexture();
	//createGraphicsPipeline();
	createCommandBuffers();
	createSyncObjects();
}

void VKE::RenderContext::cleanup() {
	vkDeviceWaitIdle(device_);

	cleanupSwapChain();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device_, imageAvailableSemaphore[i], nullptr);
		vkDestroySemaphore(device_, renderFinishedSemaphore[i], nullptr);
		vkDestroyFence(device_, inFlightFences[i], nullptr);
	}

	std::for_each(internal_buffers_.begin(), internal_buffers_.end(), [this](VKE::InternalBuffer buffer) { if(buffer.isInUse())buffer.reset(this); });
	std::for_each(internal_textures_.begin(), internal_textures_.end(), [this](VKE::InternalTexture tex) { if(tex.isInUse())tex.reset(this); });
	std::for_each(internal_materials_.begin(), internal_materials_.end(), [this](VKE::InternalMaterial mat) { if (mat.isInUse())mat.reset(this); });
	staging_buffer_.reset(this);
	VKE::InternalTexture::resetSampler(this);

	std::for_each(descriptorSetLayouts.begin(), descriptorSetLayouts.end(), [this](VkDescriptorSetLayout layout) { vkDestroyDescriptorSetLayout(this->getDevice(), layout, nullptr); });
	vkDestroyDescriptorPool(device_, descriptorPool, nullptr);
	vkDestroyCommandPool(device_, commandPool, nullptr);
	vkDestroyDevice(device_, nullptr);

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance_, debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(instance_, surface, nullptr);
	vkDestroyInstance(instance_, nullptr);
	glfwTerminate();
}

void VKE::RenderContext::cleanupSwapChain() {

	depth_image_.reset(this);

	for (auto framebuffer : swapChainFramebuffers)
		vkDestroyFramebuffer(device_, framebuffer, nullptr);
	vkDestroyFramebuffer(device_, sceneFramebuffer, nullptr);


	vkFreeCommandBuffers(device_, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	vkDestroyRenderPass(device_, renderPass, nullptr);
	vkDestroyRenderPass(device_, quadRenderPass, nullptr);

	for (auto& tex : swapChainTextures)
		vkDestroyImageView(device_, tex.image_view_, nullptr);
	offscreen_image_.reset(this);

	vkDestroySwapchainKHR(device_, swapChain, nullptr);
}

void VKE::RenderContext::recreateSwapChain() {

	int32 width, height;
	glfwGetFramebufferSize(window_->window, &width, &height);

	//While minimized, wait
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window_->window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(device_);
	cleanupSwapChain();

	createSwapChain();
	createImageViews();
	createDepthResources();
	quadscreen_entity_->GetFirstComponent<VKE::RenderComponent>()->getMaterial().UpdateTextures(offscreen_image_);
	createRenderPass();
	createFramebuffers();
	createCommandBuffers();
}


void VKE::RenderContext::draw() {

	vkWaitForFences(device_, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	screen_viewport_.x = 0.0f;
	screen_viewport_.y = 0.0f;
	screen_viewport_.width = (float32)getSwapChainExtent().width;
	screen_viewport_.height = (float32)getSwapChainExtent().height;
	screen_viewport_.minDepth = 0.0f;
	screen_viewport_.maxDepth = 1.0f;

	screen_scissors_.offset = { 0,0 };
	screen_scissors_.extent = getSwapChainExtent();

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device_, swapChain, UINT64_MAX, imageAvailableSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
		vkWaitForFences(device_, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	}

	imagesInFlight[imageIndex] = inFlightFences[currentFrame];

	//updateUniformBuffer(uniformBuffers[0]);
	recordNextFrameCommands(commandBuffers[imageIndex], swapChainFramebuffers[imageIndex]);
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphore[currentFrame] };
	VkPipelineStageFlags waiStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waiStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(device_, 1, &inFlightFences[currentFrame]);

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	presentInfo.pResults = nullptr; //Optional

	result = vkQueuePresentKHR(presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
		recreateSwapChain();
		framebufferResized = false;
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}


VKAPI_ATTR VkBool32 VKAPI_CALL VKE::RenderContext::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::string sev = "[UNSPECIFIED] ";
	if( VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT >= messageSeverity) sev = "[ERROR] ";
	else if(VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT >= messageSeverity) sev = "[WARNING] ";
	else if(VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT >= messageSeverity) sev = "[INFO] ";
	else if(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT >= messageSeverity) sev = "[VERBOSE] ";
	
	std::cerr << sev << "validation layer: "  << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

void VKE::RenderContext::createInstance() {

	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, one or more unavailable");
	}

	//Optional, possible hardware optimization
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan Engine";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "LightingInABottle";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;

	//NOT optional, layers and extensions
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions_vec = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32>(extensions_vec.size());
	createInfo.ppEnabledExtensionNames = extensions_vec.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		debugCreateInfo = {};
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = debugCallback;
		debugCreateInfo.pUserData = nullptr; // Optional

		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}


	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance_);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}

	// Debug: Listing of all extensions
	/*uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	std::function <std::string(const char8[256])> checkIfIncluded =

		[glfwExtensionCount, glfwExtensions](const char8 ext_name[256]) {
		for (int32 i = 0; i < glfwExtensionCount; ++i) {
			if (*(glfwExtensions + i) == ext_name) return std::string(" Included: YES");
		}

		return std::string(" Included : NO");
	};

	std::cout << "available extensions" << std::endl;
	for (const auto& extension : extensions) {
		std::string is_included = checkIfIncluded(extension.extensionName);
		std::cout << "\t" << extension.extensionName << is_included << std::endl;
	}*/

}

void VKE::RenderContext::setupDebugMessenger() {
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr; // Optional

	if (CreateDebugUtilsMessengerEXT(instance_, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}


void VKE::RenderContext::createSurface(GLFWwindow* window) {
	if (glfwCreateWindowSurface(instance_, window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}

}

void VKE::RenderContext::pickPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());

	for (auto &device : devices) {
		if (isDeviceSuitable(device)) {
			physicalDevice = device;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

bool VKE::RenderContext::isDeviceSuitable(VkPhysicalDevice device) {
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	bool discrete_gpu = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
	VKE::RenderContext::QueueFamilyIndices indices = findQueueFamilies(device);
	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		VKE::RenderContext::SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return discrete_gpu && indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;

}

VKE::RenderContext::QueueFamilyIndices VKE::RenderContext::findQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	uint32 it = 0;
	for (it = 0; it < queueFamilyCount; ++it) {

		if (queueFamilies[it].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = it;
		}

		VkBool32 presentSupport = false;
		VkResult res = vkGetPhysicalDeviceSurfaceSupportKHR(device, it, surface, &presentSupport);
		if ((bool)presentSupport == true) {
			indices.presentFamily = it;
		}

		if (indices.isComplete()) break;
	}

	return indices;
}

VKE::RenderContext::SwapChainSupportDetails VKE::RenderContext::querySwapChainSupport(VkPhysicalDevice device) {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}


void VKE::RenderContext::createLogicalDevice() {
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};

	float32 queuePriority = 1.0f;
	for (auto& queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};

		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}


	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(device_, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device_, indices.presentFamily.value(), 0, &presentQueue);

}

void VKE::RenderContext::createSwapChain() {
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapChainFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device_, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(device_, swapChain, &imageCount, nullptr);
	std::vector<VkImage> swapChainImages;
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device_, swapChain, &imageCount, swapChainImages.data());

	swapChainTextures.resize(imageCount);
	for (uint32 i = 0; i < imageCount; ++i) {
		swapChainTextures[i].image_ = swapChainImages[i];
		swapChainTextures[i].format_ = surfaceFormat.format;
	}

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	screen_viewport_.x = 0.0f;
	screen_viewport_.y = 0.0f;
	screen_viewport_.width = (float32)getSwapChainExtent().width;
	screen_viewport_.height = (float32)getSwapChainExtent().height;
	screen_viewport_.minDepth = 0.0f;
	screen_viewport_.maxDepth = 1.0f;

	screen_scissors_.offset = { 0,0 };
	screen_scissors_.extent = getSwapChainExtent();
}

VkSurfaceFormatKHR VKE::RenderContext::chooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	//default if preferred is not available
	return availableFormats[0];
}

VkPresentModeKHR VKE::RenderContext::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {

	// triple buffer (or more), replaces images when full
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	//blocks app when full
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VKE::RenderContext::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}
	else {
		int32 width, height;
		glfwGetFramebufferSize(window_->window, &width, &height);
		VkExtent2D actualExtent = { static_cast<uint32>(width), static_cast<uint32>(height) };

		//Clamp between the actual max and min supported
		actualExtent.width = std::max(
			capabilities.minImageExtent.width,
			std::min(
				capabilities.maxImageExtent.width,
				actualExtent.width));

		actualExtent.height = std::max(
			capabilities.minImageExtent.height,
			std::min(
				capabilities.maxImageExtent.height,
				actualExtent.height));

		return actualExtent;

	}
}


void VKE::RenderContext::createImageViews() {

	for (size_t i = 0; i < swapChainTextures.size(); ++i) {
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainTextures[i].image_;

		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device_, &createInfo, nullptr, &swapChainTextures[i].image_view_) != VK_SUCCESS) {
			throw::std::runtime_error("failed to create image views!");
		}
	}
}

void VKE::RenderContext::createDepthResources() {

	std::vector<VkFormat> candidates;
	candidates.push_back(VK_FORMAT_D32_SFLOAT);
	candidates.push_back(VK_FORMAT_D32_SFLOAT_S8_UINT);
	candidates.push_back(VK_FORMAT_D24_UNORM_S8_UINT);

	VKE::InternalTexture::depth_format_ = findSupportedFormat(candidates, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	depth_image_.init(this, nullptr, swapChainExtent.width, swapChainExtent.height, VKE::TextureType_DepthAttachment);
	transitionImageLayout(&depth_image_, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	offscreen_image_.init(this, nullptr, swapChainExtent.width, swapChainExtent.height, VKE::TextureType_ColorAttachment);
	transitionImageLayout(&offscreen_image_, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

VkFormat VKE::RenderContext::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("Failed to find supported format");
}


void VKE::RenderContext::createDescriptorSetlayout() {

	VkDescriptorSetLayoutBinding  ubmLayoutBinding = {};
	ubmLayoutBinding.binding = 0;
	ubmLayoutBinding.descriptorCount = 1;
	ubmLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubmLayoutBinding.pImmutableSamplers = nullptr; // Optional
	ubmLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::vector<std::vector<VkDescriptorSetLayoutBinding>> set_descriptors_vec;
	set_descriptors_vec.push_back(std::vector<VkDescriptorSetLayoutBinding>(1, ubmLayoutBinding));
	set_descriptors_vec.push_back(std::vector<VkDescriptorSetLayoutBinding>(1, samplerLayoutBinding));

	for (auto binding_vec : set_descriptors_vec) {
		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32>(binding_vec.size());
		layoutInfo.pBindings = binding_vec.data();

		VkDescriptorSetLayout temp_layout;
		if (vkCreateDescriptorSetLayout(device_, &layoutInfo, nullptr, &temp_layout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
		descriptorSetLayouts.push_back(temp_layout);
	}

}


//void VKE::RenderContext::createGraphicsPipeline() {
//
//}


VkShaderModule VKE::RenderContext::createShaderModule(const std::vector<char8>& code) {
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device_, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;

}


void VKE::RenderContext::createRenderPass() {

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = depth_image_.format_;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription offscreen_subpass = {};
	offscreen_subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	offscreen_subpass.colorAttachmentCount = 1;
	offscreen_subpass.pColorAttachments = &colorAttachmentRef;
	offscreen_subpass.pDepthStencilAttachment = &depthAttachmentRef;
	offscreen_subpass.pInputAttachments = nullptr;

	VkSubpassDependency dependencies[2] = {
		{}, {}
	};

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkAttachmentDescription attachments[] = {
	colorAttachment,
	depthAttachment,
	};

	VkRenderPassCreateInfo sceneRenderPassInfo = {};
	sceneRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	sceneRenderPassInfo.attachmentCount = 2;
	sceneRenderPassInfo.pAttachments = attachments;
	sceneRenderPassInfo.subpassCount = 1;
	sceneRenderPassInfo.pSubpasses = &offscreen_subpass;
	sceneRenderPassInfo.dependencyCount = 2;
	sceneRenderPassInfo.pDependencies = dependencies;


	if (vkCreateRenderPass(device_, &sceneRenderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}



	VkAttachmentDescription postprocessColorAttachment = colorAttachment;
	postprocessColorAttachment.format = swapChainImageFormat;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	postprocessColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference postprocessColorAttachmentRef = {};
	postprocessColorAttachmentRef.attachment = 0;
	postprocessColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription postprocess_subpass = {};
	postprocess_subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	postprocess_subpass.colorAttachmentCount = 1;
	postprocess_subpass.pColorAttachments = &postprocessColorAttachmentRef;
	postprocess_subpass.pDepthStencilAttachment = nullptr;
	postprocess_subpass.pInputAttachments = nullptr;

	VkRenderPassCreateInfo QuadRenderPassInfo = {};
	QuadRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	QuadRenderPassInfo.attachmentCount = 1;
	QuadRenderPassInfo.pAttachments = &postprocessColorAttachment;
	QuadRenderPassInfo.subpassCount = 1;
	QuadRenderPassInfo.pSubpasses = &postprocess_subpass;
	QuadRenderPassInfo.dependencyCount = 1;
	QuadRenderPassInfo.pDependencies = dependencies;


	if (vkCreateRenderPass(device_, &QuadRenderPassInfo, nullptr, &quadRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}

}


void VKE::RenderContext::createFramebuffers() {

	VkImageView attachments[] = {
		offscreen_image_.image_view_,
		depth_image_.image_view_,
	};

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass;
	framebufferInfo.attachmentCount = 2;
	framebufferInfo.pAttachments = attachments;
	framebufferInfo.width = swapChainExtent.width;
	framebufferInfo.height = swapChainExtent.height;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(device_, &framebufferInfo, nullptr, &sceneFramebuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create framebuffer!");
	}


	swapChainFramebuffers.resize(swapChainTextures.size());
	for (size_t i = 0; i < swapChainTextures.size(); ++i) {

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = quadRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &swapChainTextures[i].image_view_;
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device_, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}


void VKE::RenderContext::createCommandPool() {
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = indices.graphicsFamily.value();
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}

}

void VKE::RenderContext::createQuadscreenEntity()
{
	quadscreen_entity_ = &(hier_context_->getEntity());
	auto quadscreen_trans = quadscreen_entity_->AddComponent<VKE::TransformComponent>(hier_context_);
	auto quadscreen_rend = quadscreen_entity_->AddComponent<VKE::RenderComponent>(hier_context_);
	
	quadscreen_trans->init(this);
	VKE::GeoPrimitives::Quad(this, quadscreen_rend);

	VKE::InternalMaterial& quadscreen_mat = quadscreen_rend->getMaterial();
	VKE::MaterialInfo screen_quad_mat_info;
	screen_quad_mat_info.cull_mode_ = VK_CULL_MODE_NONE;
	screen_quad_mat_info.subpass_num_ = 0;
	quadscreen_mat.init(this, "quadscreen_vert.spv", "quadscreen_frag.spv", screen_quad_mat_info);
	quadscreen_mat.UpdateTextures(offscreen_image_);

}

uint32 VKE::RenderContext::findMemoryType(uint32 type_filter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if (type_filter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}


void VKE::RenderContext::setUpResources() {

	system("D:/WORKSPACE/Vulkan/FinalProjectRepo/VulkanEngine/build/compile_shaders_code.bat");

	resource_type_map.insert({ typeid(VKE::Buffer).hash_code(), &internal_buffers_ });
	resource_type_map.insert({ typeid(VKE::Texture).hash_code(), &internal_textures_ });
	resource_type_map.insert({ typeid(VKE::Material).hash_code(), &internal_materials_ });

	std::fill_n(std::back_inserter(internal_buffers_), 100, VKE::InternalBuffer());
	std::fill_n(std::back_inserter(internal_textures_), 100, VKE::InternalTexture());
	std::fill_n(std::back_inserter(internal_materials_), 100, VKE::InternalMaterial());

	createStagingBuffer();
	createDefaultTexture();
}


void VKE::RenderContext::createStagingBuffer() {
	staging_buffer_.init(this, static_cast<uint32>(staging_buffer_size_), 1, VKE::BufferType_Staging);
	staging_buffer_.uploadData(nullptr);
}

void VKE::RenderContext::createDefaultTexture() {

	placeholder_texture_ = getResource<VKE::Texture>();
	
	VKE::InternalTexture& tex = getInternalRsc<VKE::Texture::internal_class>(placeholder_texture_);
	tex.init(this, "./../../resources/textures/default_placeholder.jpg", TextureType_Sampler);
}


void VKE::RenderContext::createDescriptorPool() {

	std::array< VkDescriptorPoolSize, 2> pool_sizes;
	pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_sizes[0].descriptorCount = 100;
	pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pool_sizes[1].descriptorCount = 100;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32>(pool_sizes.size());
	poolInfo.pPoolSizes = pool_sizes.data();
	poolInfo.maxSets = 200;

	if (vkCreateDescriptorPool(device_, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayouts[VKE::DescriptorType_Matrices];

	desc_set_allocation_info_[VKE::DescriptorType_Matrices] = allocInfo;
	allocInfo.pSetLayouts = &descriptorSetLayouts[VKE::DescriptorType_Textures];
	desc_set_allocation_info_[VKE::DescriptorType_Textures] = allocInfo;
}


void VKE::RenderContext::createCommandBuffers() {
	commandBuffers.resize(swapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	if (vkAllocateCommandBuffers(device_, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	recordCommands();
}


void VKE::RenderContext::createSyncObjects() {
	imageAvailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	imagesInFlight.resize(swapChainTextures.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		if (vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &imageAvailableSemaphore[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &renderFinishedSemaphore[i]) != VK_SUCCESS ||
			vkCreateFence(device_, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create sync objects for a frame!");
		}
	}


}

bool VKE::RenderContext::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char8* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) return false;
	}

	return true;

}

bool VKE::RenderContext::checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

std::vector<const char8*> VKE::RenderContext::getRequiredExtensions() {
	uint32_t glfwExtensionCount = 0;
	const char8** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char8*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}


void VKE::RenderContext::recordCommands() {
	for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) recordNextFrameCommands(commandBuffers[i], swapChainFramebuffers[i]);
}

void VKE::RenderContext::recordNextFrameCommands(VkCommandBuffer command_buffer, VkFramebuffer spawchain_framebuffer) {
	
	std::vector<std::pair<VKE::TransformComponent*, VKE::RenderComponent*>> valid_entities;

	for (VKE::Entity& entity : hier_context_->entities_) {
		if (!entity.isInUse() || &entity == quadscreen_entity_) continue;
		std::vector<VKE::TransformComponent*> transforms = entity.GetComponents<VKE::TransformComponent>();
		std::vector<VKE::RenderComponent*> renderers = entity.GetComponents<VKE::RenderComponent>();

		if (transforms.size() && renderers.size()) {
			for (VKE::TransformComponent* transform : transforms) {
				for (VKE::RenderComponent* renderer : renderers) {
					valid_entities.push_back(std::make_pair(transform, renderer));
				}
			}
		}

	}

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(command_buffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkClearValue clear_values[3];
	clear_values[0].color = { 0.3f,0.25f,0.85f,1.0f };	//Color
	clear_values[1].depthStencil = { 1.0, 0 };							//Depth
	clear_values[2].color = { 1.0f,0.5f,0.5f,1.0f };	//offscreen color

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = sceneFramebuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapChainExtent;
	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = clear_values;

	VkRenderPassBeginInfo quadRenderPassInfo = {};
	quadRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	quadRenderPassInfo.renderPass = quadRenderPass;
	quadRenderPassInfo.framebuffer = spawchain_framebuffer;
	quadRenderPassInfo.renderArea.offset = { 0, 0 };
	quadRenderPassInfo.renderArea.extent = swapChainExtent;
	quadRenderPassInfo.clearValueCount = 1;
	quadRenderPassInfo.pClearValues = &clear_values[2];

	VkDeviceSize offsets[] = { 0 };




	vkCmdBeginRenderPass(command_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	for (std::pair<VKE::TransformComponent*, VKE::RenderComponent*> entry : valid_entities) {

		VKE::InternalBuffer& uniform_buffer = entry.first->getUBMBuffer();
		VKE::InternalBuffer& vertex_buf = entry.second->getVertexBuffer();
		VKE::InternalBuffer& index_buf = entry.second->getIndexBuffer();
		VKE::InternalMaterial& mat = entry.second->getMaterial();

		vkCmdSetLineWidth(command_buffer, static_cast<float>(mat.mat_info_.dynamic_line_width_));
		vkCmdSetViewport(command_buffer, 0, 1, &mat.mat_info_.dynamic_viewport_);
		vkCmdSetScissor(command_buffer, 0, 1, &mat.mat_info_.dynamic_scissors_);

		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mat.graphical_pipeline_);

		vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buf.buffer_, offsets);
		vkCmdBindIndexBuffer(command_buffer, index_buf.buffer_, 0, VK_INDEX_TYPE_UINT16);

		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mat.pipeline_layout_, VKE::DescriptorType_Matrices, 1, &uniform_buffer.uniform_desc_set_, 0, nullptr);
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mat.pipeline_layout_, VKE::DescriptorType_Textures, 1, &mat.textures_desc_set_, 0, nullptr);

		vkCmdDrawIndexed(command_buffer, index_buf.getElementCount(), 1, 0, 0, 0);
	}

	vkCmdEndRenderPass(command_buffer);
	vkCmdBeginRenderPass(command_buffer, &quadRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	//transitionImageLayout(&offscreen_image_, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, command_buffer);
	//vkCmdNextSubpass(command_buffer, VK_SUBPASS_CONTENTS_INLINE);

	if (quadscreen_entity_ != nullptr) {

		VKE::InternalBuffer& uniform_buffer = quadscreen_entity_->GetFirstComponent<VKE::TransformComponent>()->getUBMBuffer();
		if (uniform_buffer.uniform_desc_set_ != VK_NULL_HANDLE) {	//First frame, before TransformComponent->Update()

			VKE::InternalBuffer& vertex_buf = quadscreen_entity_->GetFirstComponent<VKE::RenderComponent>()->getVertexBuffer();
			VKE::InternalBuffer& index_buf = quadscreen_entity_->GetFirstComponent<VKE::RenderComponent>()->getIndexBuffer();

			VKE::InternalMaterial& mat = quadscreen_entity_->GetFirstComponent<VKE::RenderComponent>()->getMaterial();

			vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mat.graphical_pipeline_);

			vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buf.buffer_, offsets);
			vkCmdBindIndexBuffer(command_buffer, index_buf.buffer_, 0, VK_INDEX_TYPE_UINT16);

			vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mat.pipeline_layout_, VKE::DescriptorType_Matrices, 1, &uniform_buffer.uniform_desc_set_, 0, nullptr);
			vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mat.pipeline_layout_, VKE::DescriptorType_Textures, 1, &mat.textures_desc_set_, 0, nullptr);

			vkCmdDrawIndexed(command_buffer, index_buf.getElementCount(), 1, 0, 0, 0);
		}

	}
	vkCmdEndRenderPass(command_buffer);

	if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}

}

template<class T> T VKE::RenderContext::getResource() {
	std::vector<T::internal_class> *vec = static_cast<std::vector<T::internal_class>*>(resource_type_map[typeid(T).hash_code()]);

	T rsc;
	rsc.id_ = UINT32_MAX;
	for (uint32 i = 0; i < vec->size(); ++i) {
		if (!vec->at(i).isInitialised() && !vec->at(i).isAllocated() && !vec->at(i).in_use_) {
			vec->at(i).in_use_ = true;
			rsc.id_ = i;
			break;
		}
	}

	if (rsc.id_ == UINT32_MAX) {
		std::fill_n(std::back_inserter(*vec), 100, T::internal_class());
		return getResource<T>();
	}
	else {
		return rsc;
	}
}

template VKE::Buffer VKE::RenderContext::getResource<VKE::Buffer>();
template VKE::Texture VKE::RenderContext::getResource<VKE::Texture>();
template VKE::Material VKE::RenderContext::getResource<VKE::Material>();


template<class InternalT, class T> InternalT& VKE::RenderContext::getInternalRsc(T rsc) {
	std::vector<T::internal_class> *vec = static_cast<std::vector<T::internal_class>*>(resource_type_map[typeid(T).hash_code()]);
	return vec->at(rsc.id_);
}

template VKE::Buffer::internal_class& VKE::RenderContext::getInternalRsc<VKE::Buffer::internal_class, VKE::Buffer>(VKE::Buffer rsc);
template VKE::Texture::internal_class& VKE::RenderContext::getInternalRsc<VKE::Texture::internal_class, VKE::Texture>(VKE::Texture rsc);
template VKE::Material::internal_class& VKE::RenderContext::getInternalRsc<VKE::Material::internal_class, VKE::Material>(VKE::Material rsc);



void VKE::RenderContext::uploadToStaging(void* data, size_t size) {
	// TEST WHETHER THE STAGING BUFFER IS BIG ENOUGH

	if (data == nullptr) return; // ERROR
	if (size == 0) return; // WARN

	void* mapped_data = nullptr;
	vkMapMemory(device_, staging_buffer_.buffer_memory_, 0, VK_WHOLE_SIZE, 0, &mapped_data);
	memcpy(mapped_data, data, size);
	vkUnmapMemory(device_, staging_buffer_.buffer_memory_);
}

void VKE::RenderContext::clearStaging()
{
	//Clears data on the staging buffer
	void* mapped_data = nullptr;
	vkMapMemory(getDevice(), staging_buffer_.buffer_memory_, 0, staging_buffer_size_, 0, &mapped_data);
	memset(mapped_data, 0, staging_buffer_size_);
	vkUnmapMemory(getDevice(), staging_buffer_.buffer_memory_);
}

VkCommandBuffer beginCommands(VkCommandPool& command_pool, VkDevice& device) {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = command_pool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void endCommands(VkCommandPool& command_pool, VkDevice& device, VkQueue& queue, VkCommandBuffer& command_buffer) {
	vkEndCommandBuffer(command_buffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &command_buffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
}

void VKE::RenderContext::copyBuffer(const InternalBuffer& srcBuffer, const InternalBuffer& dstBuffer, size_t size) {

	VkCommandBuffer copyCommandBuffer = beginCommands(commandPool, device_);

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = size;

	vkCmdCopyBuffer(copyCommandBuffer, 
		srcBuffer.buffer_,
		dstBuffer.buffer_,
		1, &copyRegion);

	endCommands(commandPool, device_, graphicsQueue, copyCommandBuffer);

}

void VKE::RenderContext::copyBufferToImage(const InternalBuffer& srcBuffer, const InternalTexture& dstTexture, size_t height, size_t width) {

	VkCommandBuffer copyToImageCommandBuffer = beginCommands(commandPool, device_);

	std::vector<VkBufferImageCopy> regions;

	VkBufferImageCopy def_reg = {};
	def_reg.bufferOffset = 0;
	def_reg.bufferRowLength = 0;
	def_reg.bufferImageHeight = 0;
	def_reg.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	def_reg.imageSubresource.mipLevel = 0;
	def_reg.imageSubresource.baseArrayLayer = 0;
	def_reg.imageSubresource.layerCount = 1;
	def_reg.imageOffset = { 0, 0, 0 };
	def_reg.imageExtent = {
		static_cast<uint32>(width),
		static_cast<uint32>(height),
		1 };
	if (dstTexture.tex_type_ != TextureType_Cubemap) {
		regions.push_back(def_reg);
	}
	else {
		uint64 data_offset = 0;
		for (uint32 i = 0; i < 6; ++i) {	
			def_reg.imageSubresource.baseArrayLayer = i;
			def_reg.bufferOffset = data_offset;
			data_offset += sizeof(uchar8) * width * height * 4;
			regions.push_back(def_reg);
		}
	}

	vkCmdCopyBufferToImage(copyToImageCommandBuffer, srcBuffer.buffer_, dstTexture.image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regions.size(), regions.data());

	endCommands(commandPool, device_, graphicsQueue, copyToImageCommandBuffer);
}

void VKE::RenderContext::transitionImageLayout(InternalTexture* tex, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer optional_external_cmd_buf) {

	VkCommandBuffer layoutBuffer;

	if (optional_external_cmd_buf == VK_NULL_HANDLE)
		layoutBuffer = beginCommands(commandPool, device_);
	else
		layoutBuffer = optional_external_cmd_buf;

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = nullptr;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = tex->image_;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if(hasStencilComponent(VKE::InternalTexture::depth_format_))
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	switch (oldLayout) {
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_UNDEFINED:
		barrier.srcAccessMask = 0;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	default:
		throw std::invalid_argument("unsupported src image layout type transition");
	}

	switch (newLayout) {
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
		break;
	default:
		throw std::invalid_argument("unsupported dst image layout type transition");
	}

	VkDependencyFlags dependency_flags = {};
	if (optional_external_cmd_buf != VK_NULL_HANDLE)
		dependency_flags |= VK_DEPENDENCY_BY_REGION_BIT;

	vkCmdPipelineBarrier(
		layoutBuffer,
		sourceStage, destinationStage,
		dependency_flags,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	if(optional_external_cmd_buf == VK_NULL_HANDLE)
		endCommands(commandPool, device_, graphicsQueue, layoutBuffer);

}


void VKE::RenderContext::UpdateDescriptor(VkDescriptorSet desc, VKE::DescriptorType type, void* info_) {

	VkWriteDescriptorSet desc_write = {};

	if(type == VKE::DescriptorType_Textures){

		VKE::InternalTexture* internal_tex = ((VKE::InternalTexture*)info_);
		VkDescriptorImageInfo imageInfo = {};

		//if(internal_tex->tex_type_ == TextureType_ColorAttachment)
		//	imageInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		//else
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		imageInfo.imageView = internal_tex->image_view_;

		if(internal_tex->has_created_custom_sampler_)
			imageInfo.sampler = internal_tex->custom_sampler_;
		else
			imageInfo.sampler = VKE::InternalTexture::default_sampler_;

		desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		desc_write.dstSet = desc;
		desc_write.dstBinding = 0;
		desc_write.dstArrayElement = 0;
		desc_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		desc_write.descriptorCount = 1;
		desc_write.pImageInfo = &imageInfo;

	} else if (type == VKE::DescriptorType_Matrices) {

		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = *((VkBuffer*)info_);
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(VKE::UniformBufferMatrices);

		desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		desc_write.dstSet = desc;
		desc_write.dstBinding = 0;
		desc_write.dstArrayElement = 0;
		desc_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		desc_write.descriptorCount = 1;
		desc_write.pBufferInfo = &bufferInfo;

	}

	vkUpdateDescriptorSets(device_, 1, &desc_write, 0, nullptr);
}

void  VKE::RenderContext::updateUniformBuffer(VKE::Buffer buffer, glm::mat4 model_mat) {

	VKE::UniformBufferMatrices ubm;

	ubm.model = model_mat;
	ubm.view =	camera_.viewMatrix();
	ubm.proj = camera_.projectionMatrix();

	//ubm.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//ubm.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float32)swapChainExtent.height, 0.1f, 10.0f);
	//ubm.proj[1][1] *= -1.0f; // Reversal of Y axis

	getInternalRsc<VKE::Buffer::internal_class>(buffer).uploadData((void*)&ubm);
}

