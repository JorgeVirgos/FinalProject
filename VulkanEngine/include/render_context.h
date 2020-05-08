#ifndef __RENDER_CONTEXT_H__
#define __RENDER_CONTEXT_H__ 1


#include <vulkan/vulkan.h>
#include "platform_types.h"
#include "internal_buffer.h"
#include "internal_texture.h"
#include "internal_material.h"
#include "camera.h"
#include <map>
#include <vector>
#include <optional>
#include <iostream>


struct GLFWwindow;


namespace VKE {

	class Window;
	class HierarchyContext;
	class Entity;

	class RenderContext {
		struct QueueFamilyIndices;
		struct SwapChainSupportDetails;
	public:
		RenderContext();
		~RenderContext();

		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

		void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

		void init(Window* window, HierarchyContext* hier_context);

		template<class T> T getResource();
		template<class internalT,class T> internalT& getInternalRsc(T rsc);

		void draw();

		void recreateSwapChain();

		VkDevice getDevice() { return device_; }
		VkDescriptorSetAllocateInfo getDescriptorAllocationInfo(VKE::DescriptorType desc_type) { return desc_set_allocation_info_[desc_type]; }
		VkRenderPass getRenderPass() { return renderPass; }
		VkRenderPass getQuadRenderPass() { return quadRenderPass; }
		VkRenderPass getShadowRenderPass() { return shadowRenderPass; }
		VKE::InternalBuffer& getStagingBuffer() { return staging_buffer_; }
		VKE::InternalBuffer& getLightingDataBuffer() { return world_lighting_data_; }
		VKE::Texture getDefaultTexture() { return placeholder_texture_; }
		VKE::InternalTexture& getDefaultInternalTexture() { return getInternalRsc<VKE::Texture::internal_class>(placeholder_texture_); }
		VkSwapchainKHR getSwapChain() { return swapChain; }
		VkExtent2D getSwapChainExtent() { return swapChainExtent; }
		VKE::InternalTexture& getCurrentSwapChainTexture() { return swapChainTextures[0]; }
		VkViewport getScreenViewport() { return screen_viewport_; }
		VkRect2D getScreenScissors() { return screen_scissors_; }
		std::vector<VkDescriptorSetLayout> getDescSetLayouts() { return descriptorSetLayouts; }
		VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
		bool shouldRecreateShaders() { return recreate_shaders_; }
		void setRecreateShaders(bool recreate) { recreate_shaders_ = recreate; }

		VKE::Camera& getCamera() { return camera_; }
		VKE::Window* getWindow() { return window_; }

		uint32 findMemoryType(uint32 type_filter, VkMemoryPropertyFlags properties);
		VkShaderModule VKE::RenderContext::createShaderModule(const std::vector<char8>& code);
		bool hasStencilComponent(VkFormat format) {return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;}
		void uploadToStaging(void* data, size_t size);
		void clearStaging();
		void copyBuffer(const InternalBuffer& srcBuffer, const InternalBuffer& dstBuffer, size_t size);
		void copyBufferToImage(const InternalBuffer& srcBuffer, const InternalTexture& dstTexture, size_t height, size_t width);
		void transitionImageLayout(InternalTexture* tex, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer optional_external_cmd_buf = VK_NULL_HANDLE);
		void UpdateDescriptor(VkDescriptorSet desc, VKE::DescriptorType type, void* info_);
		void updateUniformMatrices(Buffer buffer, glm::mat4 model_mat);
		void updateLightData();


	private:

		void cleanup();

		//Main methods called in init()
		void createInstance();
		void setupDebugMessenger();
		void createSurface(GLFWwindow* window);
		void pickPhysicalDevice();
		void createLogicalDevice();
		void createSwapChain();
		void createImageViews();
		void createDepthResources();
		void createQuadscreenEntity();
		void createRenderPass();
		void createDescriptorSetlayout();
		void createFramebuffers();
		void setUpResources();
		void createStagingBuffer();
		void createDefaultTexture();
		void createDefaultLightingData();
		void createDescriptorPool();
		void createCommandPool();
		void createCommandBuffers();
		void createSyncObjects();

		//Internal methods called almost exclusively by the previous ones
		bool isDeviceSuitable(VkPhysicalDevice device);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR chooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		bool checkValidationLayerSupport();
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		std::vector<const char8*> getRequiredExtensions();
		void cleanupSwapChain();
		void recordCommands();
		void recordNextFrameCommands(VkCommandBuffer command_buffer, VkFramebuffer spawchain_framebuffer);
		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

#ifdef _NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif

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

		const std::vector<const char8*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};
		const std::vector<const char8*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		const uint32 MAX_FRAMES_IN_FLIGHT = 2;

		Window* window_;
		HierarchyContext* hier_context_;

		VkInstance instance_ = VK_NULL_HANDLE;
		VkDevice device_ = VK_NULL_HANDLE;

		VkSwapchainKHR swapChain;
		std::vector<InternalTexture> swapChainTextures;
		std::vector<VkFramebuffer> swapChainFramebuffers;
		VkFramebuffer sceneFramebuffer;
		VkFramebuffer shadowFramebuffer;

		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;
		VkViewport screen_viewport_;
		VkRect2D screen_scissors_;

		VkQueue graphicsQueue = VK_NULL_HANDLE;
		VkQueue presentQueue = VK_NULL_HANDLE;

		VkRenderPass renderPass = VK_NULL_HANDLE;
		VkRenderPass quadRenderPass = VK_NULL_HANDLE;
		VkRenderPass shadowRenderPass = VK_NULL_HANDLE;

		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		VkDescriptorSetAllocateInfo desc_set_allocation_info_[VKE::DescriptorType_MAX] = {};

		std::map<uint64, void*> resource_type_map;
		std::vector<InternalBuffer> internal_buffers_;
		std::vector<InternalTexture> internal_textures_;
		std::vector<InternalMaterial> internal_materials_;

		Entity* quadscreen_entity_ = nullptr;
		InternalBuffer staging_buffer_;
		InternalBuffer world_lighting_data_;
		InternalTexture depth_image_;
		InternalTexture shadow_mapping_image_;
		InternalTexture offscreen_image_;
		Texture placeholder_texture_;

		InternalMaterial shadow_mapping_material_;

		const size_t staging_buffer_size_ = sizeof(uchar8) * 1024 * 1024 * 100;
		const VkExtent2D scene_shadowmap_size = { 2 << 12, 2 << 12 };

		VkCommandPool commandPool;
		std::vector<VkCommandBuffer> commandBuffers;

		size_t currentFrame = 0;
		bool framebufferResized = false;
		std::vector<VkSemaphore> imageAvailableSemaphore;
		std::vector<VkSemaphore> renderFinishedSemaphore;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;

		VkDebugUtilsMessengerEXT debugMessenger;
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

		VKE::Camera camera_;
		glm::mat4 camera_view_matrix_;
		glm::mat4 camera_projection_matrix_;

		bool recreate_shaders_ = false;


		friend class Window;
	};
}

#endif //__RENDER_CONTEXT_H__