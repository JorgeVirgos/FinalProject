#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__ 1

#include <array>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace VKE{
	struct Vertex {
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 uv;

		static VkVertexInputBindingDescription getBindingDescription() {
			static VkVertexInputBindingDescription bindingDescription;
			static bool has_been_initalised = false;

			if (!has_been_initalised) {
				bindingDescription = {};
				bindingDescription.binding = 0;
				bindingDescription.stride = sizeof(Vertex);
				bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				has_been_initalised = true;
			}

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
			static std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions;
			static bool has_been_initalised = false;

			if (!has_been_initalised) {
				attributeDescriptions = {};
				attributeDescriptions[0].binding = 0;
				attributeDescriptions[0].location = 0;
				attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[0].offset = offsetof(Vertex, pos);

				attributeDescriptions[1].binding = 0;
				attributeDescriptions[1].location = 1;
				attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[1].offset = offsetof(Vertex, color);

				attributeDescriptions[2].binding = 0;
				attributeDescriptions[2].location = 2;
				attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
				attributeDescriptions[2].offset = offsetof(Vertex, uv);

				has_been_initalised = true;
			}

			return attributeDescriptions;
		}

	};
}
#endif //__CONSTANTS_H__