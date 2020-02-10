#ifndef __RENDER_CONTEXT_H__
#define __RENDER_CONTEXT_H__ 1


#include <vulkan/vulkan.h>
#include "platform_types.h"
#include "internal_buffer.h"
#include <vector>
#include <optional>
#include <iostream>
#include <fstream>


class GLFWwindow;

namespace VKE {

	class InternalBuffer;

	//VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	//	const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {

	//	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	//	if (func != nullptr) {
	//		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	//	}
	//	else {
	//		return VK_ERROR_EXTENSION_NOT_PRESENT;
	//	}
	//}

	//void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	//	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	//	if (func != nullptr) {
	//		return func(instance, debugMessenger, pAllocator);
	//	}
	//}


	class RenderContext {
	public:
		RenderContext();
		~RenderContext();

		void init(GLFWwindow* window);

		void draw();

		VkDevice getDevice() { return device; }
		VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
		uint32 findMemoryType(uint32 type_filter, VkMemoryPropertyFlags properties);

		Buffer getBuffer();
		InternalBuffer& accessInternalBuffer(Buffer buffer);

	private:

		struct QueueFamilyIndices {
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			bool isComplete() {
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
		};

		struct SwapChainSupportDetails {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		void cleanup();

		//Main methods called in init()
		void createInstance();
		void setupDebugMessenger();
		void createSurface(GLFWwindow* window);
		void pickPhysicalDevice();
		void createLogicalDevice();
		void createSwapChain();
		void createImageViews();
		void createRenderPass();
		void createGraphicsPipeline();
		void createFramebuffers();
		void createVertexBuffer();
		void createCommandPool();
		void createCommandBuffers();
		void createSyncObjects();

		//Internal methods called almost exclusively by the previous ones
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		bool isDeviceSuitable(VkPhysicalDevice device);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR chooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		VkShaderModule VKE::RenderContext::createShaderModule(const std::vector<char>& code);
		bool checkValidationLayerSupport();
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		std::vector<const char*> getRequiredExtensions();
		void cleanupSwapChain();
		void recreateSwapChain();
		void recordCommands();
		void rerecordSingleCommand(VkCommandBuffer command_buffer, VkFramebuffer spawchain_framebuffer);

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData) {

			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

			return VK_FALSE;
		}

		static std::vector<char> readFile(const std::string& filename) {
			std::ifstream file(filename, std::ios::ate | std::ios::binary);

			if (!file.is_open()) {
				throw std::runtime_error("failed to open file!");
			}

			size_t fileSize = (size_t)file.tellg();
			std::vector<char> buffer(fileSize);
			file.seekg(0);
			file.read(buffer.data(), fileSize);

			file.close();

			return buffer;
		}

		//static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		//	auto render_ctx = reinterpret_cast<RenderContext*>(glfwGetWindowUserPointer(window));
		//	if(render_ctx != nullptr) render_ctx->framebufferResized = true;
		//}

		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};
		const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		const int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef _NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif

		GLFWwindow* glfw_window_;

		VkInstance instance;

		VkDevice device;

		VkSwapchainKHR swapChain;
		std::vector<VkImage> swapChainImages;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;

		VkQueue graphicsQueue;
		VkQueue presentQueue;

		std::vector<VkImageView> swapChainImageViews;
		std::vector<VkFramebuffer> swapChainFramebuffers;

		VkRenderPass renderPass;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;

		std::vector<InternalBuffer> internal_buffers_;
		//VkBuffer vertexBuffer;
		//VkDeviceMemory vertexBufferMemory;

		VkCommandPool commandPool;
		std::vector<VkCommandBuffer> commandBuffers;

		size_t currentFrame = 0;
		bool framebufferResized = false;
		std::vector<VkSemaphore> imageAvailableSemaphore;
		std::vector<VkSemaphore> renderFinishedSemaphore;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;

		VkDebugUtilsMessengerEXT debugMessenger;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	};
}

#endif //__RENDER_CONTEXT_H__