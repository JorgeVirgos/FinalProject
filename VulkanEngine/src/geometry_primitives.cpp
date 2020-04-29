#include "geometry_primitives.h"
#include "hierarchy_context.h"

void VKE::GeoPrimitives::Cube(VKE::RenderContext* render_ctx, VKE::RenderComponent* rend) {

	glm::vec3 pos[] = {
		{+1.0f,+1.0f,+1.0f},	//FRONT
		{-1.0f,+1.0f,+1.0f},
		{-1.0f,-1.0f,+1.0f},
		{+1.0f,-1.0f,+1.0f},

		{+1.0f,+1.0f,+1.0f},
		{+1.0f,-1.0f,+1.0f},
		{+1.0f,-1.0f,-1.0f},
		{+1.0f,+1.0f,-1.0f},

		{+1.0f,+1.0f,+1.0f},
		{+1.0f,+1.0f,-1.0f},
		{-1.0f,+1.0f,-1.0f},
		{-1.0f,+1.0f,+1.0f},

		{-1.0f,+1.0f,-1.0f}, //BACK
		{+1.0f,+1.0f,-1.0f},
		{+1.0f,-1.0f,-1.0f},
		{-1.0f,-1.0f,-1.0f},

		{-1.0f,-1.0f,-1.0f},
		{-1.0f,-1.0f,+1.0f},
		{-1.0f,+1.0f,+1.0f},
		{-1.0f,+1.0f,-1.0f},

		{-1.0f,-1.0f,-1.0f},
		{+1.0f,-1.0f,-1.0f},
		{+1.0f,-1.0f,+1.0f},
		{-1.0f,-1.0f,+1.0f}
	};  

	glm::vec2 uv[]{
		{1.0f,0.0f},
		{0.0f,0.0f},
		{0.0f,1.0f},
		{1.0f,1.0f},

		{0.0f,0.0f},
		{0.0f,1.0f},
		{1.0f,1.0f},
		{1.0f,0.0f},

		{1.0f,1.0f},
		{1.0f,0.0f},
		{0.0f,0.0f},
		{0.0f,1.0f},

		{1.0f,0.0f},
		{0.0f,0.0f},
		{0.0f,1.0f},
		{1.0f,1.0f},

		{1.0f,1.0f},
		{0.0f,1.0f},
		{0.0f,0.0f},
		{1.0f,0.0f},

		{0.0f,0.0f},
		{1.0f,0.0f},
		{1.0f,1.0f},
		{0.0f,1.0f},
	};

	std::vector<VKE::Vertex> cube_vertex_data;
	std::vector<ushort16> cube_index_data;

	cube_vertex_data.resize(24);
	cube_index_data.resize(36);

	for (uint32 i = 0; i < 24; ++i) {
		cube_vertex_data[i].pos = pos[i];
		cube_vertex_data[i].uv = uv[i];
	}

	ushort16 indices[] = { 0,1,3,3,1,2 };
	for (uint32 i = 0; i < 36; ++i) {
		cube_index_data[i] = indices[i%6] + static_cast<ushort16>(i/6 * 4);
	}

	VKE::InternalBuffer& vertex_internal_buffer = rend->getVertexBuffer();
	vertex_internal_buffer.init(render_ctx, sizeof(VKE::Vertex), cube_vertex_data.size(), VKE::BufferType_Vertex);
	vertex_internal_buffer.uploadData((void*)cube_vertex_data.data());

	VKE::InternalBuffer& index_internal_buffer = rend->getIndexBuffer();
	index_internal_buffer.init(render_ctx, sizeof(ushort16), cube_index_data.size(), VKE::BufferType_Index);
	index_internal_buffer.uploadData((void*)cube_index_data.data());

}
void VKE::GeoPrimitives::Sphere(VKE::RenderContext* render_ctx, VKE::RenderComponent* rend) {
	// OpenGL sphere
	// http://www.songho.ca/opengl/gl_sphere.html

	uint32 stacks = 24;
	uint32 sectors = 72;

	float64 radius = 1.0f;
	float64 sectorStep = 2.0 * glm::pi<float64>() / sectors;
	float64 stackStep = glm::pi<float64>() / stacks;
	float64 length_inverse = 1.0f / radius;

	std::vector<VKE::Vertex> sphere_vertex_data;
	std::vector<ushort16> sphere_index_data;

	for (uint32 i = 0; i <= stacks; i++) {

		float64 stack_angle = glm::half_pi<float64>() - static_cast<float64>(i) * stackStep;
		float64 xy = radius * glm::cos(stack_angle);
		float64 z = radius * glm::sin(stack_angle);

		for (uint32 j = 0; j <= sectors; j++) {
			float64 sector_angle = static_cast<float64>(j) * sectorStep;

			VKE::Vertex vertex;
			float64 x, y;
			x = xy * glm::sin(sector_angle);
			y = xy * glm::cos(sector_angle);

			vertex.pos = glm::vec3(x, z, y);
			vertex.normal = vertex.pos * glm::vec3(length_inverse, length_inverse, length_inverse);
			vertex.uv = glm::vec2(
				static_cast<float64>(j) / static_cast<float64>(sectors),
				static_cast<float64>(i) / static_cast<float64>(stacks));

			sphere_vertex_data.push_back(vertex);
		}
	}

	for (uint32 i = 0; i < stacks; i++) {

		ushort16 k1 = i * (sectors + 1);
		ushort16 k2 = k1 + sectors + 1;

		for (uint32 j = 0; j < sectors; ++j, ++k1, ++k2) {

			if (i != 0) {
				sphere_index_data.push_back(k1);
				sphere_index_data.push_back(k2);
				sphere_index_data.push_back(k1 + 1);
			}

			if (i != (stacks - 1)) {
				sphere_index_data.push_back(k1 + 1);
				sphere_index_data.push_back(k2);
				sphere_index_data.push_back(k2 + 1);
			}

		}
	}

	VKE::InternalBuffer& vertex_internal_buffer = rend->getVertexBuffer();
	vertex_internal_buffer.init(render_ctx, sizeof(VKE::Vertex), sphere_vertex_data.size(), VKE::BufferType_Vertex);
	vertex_internal_buffer.uploadData((void*)sphere_vertex_data.data());

	VKE::InternalBuffer& index_internal_buffer = rend->getIndexBuffer();
	index_internal_buffer.init(render_ctx, sizeof(ushort16), sphere_index_data.size(), VKE::BufferType_Index);
	index_internal_buffer.uploadData((void*)sphere_index_data.data());

}

void VKE::GeoPrimitives::Quad(RenderContext* render_ctx, RenderComponent* rend) {

	const std::vector<VKE::Vertex> quad_vertex_data = {
	{{-0.5f, -0.5f, +0.0f},	{1.0f,0.0f,0.0f},	{1.0f, 0.0f}},
	{{+0.5f, -0.5f, +0.0f},	{0.0f,1.0f,0.0f},	{0.0f, 0.0f}},
	{{+0.5f, +0.5f, +0.0f},	{0.0f,0.0f,1.0f},	{0.0f, 1.0f}},
	{{-0.5f, +0.5f, +0.0f},	{1.0f,1.0f,1.0f},	{1.0f, 1.0f}}
	};

	const std::vector<ushort16> quad_index_data = {
		0,1,2,2,3,0
	};

	VKE::InternalBuffer& vertex_internal_buffer = rend->getVertexBuffer();
	vertex_internal_buffer.init(render_ctx, sizeof(VKE::Vertex), quad_vertex_data.size(), VKE::BufferType_Vertex);
	vertex_internal_buffer.uploadData((void*)quad_vertex_data.data());

	VKE::InternalBuffer& index_internal_buffer = rend->getIndexBuffer();
	index_internal_buffer.init(render_ctx, sizeof(ushort16), quad_index_data.size(), VKE::BufferType_Index);
	index_internal_buffer.uploadData((void*)quad_index_data.data());

}