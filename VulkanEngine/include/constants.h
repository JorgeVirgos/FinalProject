#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__ 1

#include <array>
#include <vector>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <fstream>
#include "platform_types.h"

namespace VKE{

	enum DescriptorType {
		DescriptorType_Matrices = 0,
		DescriptorType_Textures,
		DescriptorType_MAX
	};

	

	enum BufferType {
		BufferType_Vertex = 0,
		BufferType_Index,
		BufferType_Uniform,
		BufferType_Staging,
		BufferType_MAX
	};

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

	struct UniformBufferMatrices {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};
}

//const std::vector<VKE::Vertex> triangle_vertices = {
//	{{0.0f,-0.5f,0.0f},{1.0f,0.0f,0.0f}},
//	{{0.5f,0.5f,0.0f},{0.0f,1.0f,0.0f}},
//	{{-0.5f,0.5f,0.0f},{0.0f,0.0f,1.0f}}
//};
//
//const std::vector<VKE::Vertex> square_vertices = {
//	{{-0.5f,-0.5f,0.0f},{1.0f,0.0f,0.0f}},
//	{{0.5f,-0.5f,0.0f},{0.0f,1.0f,0.0f}},
//	{{-0.5f,0.5f,0.0f},{0.0f,0.0f,1.0f}},
//
//	{{0.5f,-0.5f,0.0f},{0.0f,1.0f,0.0f}},
//	{{0.5f,0.5f,0.0f},{1.0f,0.0f,0.0f}},
//	{{-0.5f,0.5f,0.0f},{0.0f,0.0f,1.0f}}
//};

const std::vector<VKE::Vertex> indexed_square_vertices = {
	{{-0.5f,-0.5f,0.0f},	{1.0f,0.0f,0.0f},	{1.0f, 0.0f}},
	{{0.5f,-0.5f,0.0f},		{0.0f,1.0f,0.0f},	{0.0f, 0.0f}},
	{{0.5f,0.5f,0.0f},		{0.0f,0.0f,1.0f},	{0.0f, 1.0f}},
	{{-0.5f,0.5f,0.0f},		{1.0f,1.0f,1.0f},	{1.0f, 1.0f}}
};

const std::vector<ushort16> indexed_square_indices = {
	0,1,2,2,3,0
};

static inline std::vector<char8> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char8> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

#endif //__CONSTANTS_H__