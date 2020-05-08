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

	glm::vec3 norm[] = {
	{+0.0f,+0.0f,+1.0f},
	{+0.0f,+0.0f,+1.0f},
	{+0.0f,+0.0f,+1.0f},
	{+0.0f,+0.0f,+1.0f},

	{+1.0f,+0.0f,+0.0f},
	{+1.0f,+0.0f,+0.0f},
	{+1.0f,+0.0f,+0.0f},
	{+1.0f,+0.0f,+0.0f},

	{+0.0f,+1.0f,+0.0f},
	{+0.0f,+1.0f,+0.0f},
	{+0.0f,+1.0f,+0.0f},
	{+0.0f,+1.0f,+0.0f},

	{+0.0f,+0.0f,-1.0f},
	{+0.0f,+0.0f,-1.0f},
	{+0.0f,+0.0f,-1.0f},
	{+0.0f,+0.0f,-1.0f},

	{-1.0f,+0.0f,+0.0f},
	{-1.0f,+0.0f,+0.0f},
	{-1.0f,+0.0f,+0.0f},
	{-1.0f,+0.0f,+0.0f},

	{+0.0f,-1.0f,+0.0f},
	{+0.0f,-1.0f,+0.0f},
	{+0.0f,-1.0f,+0.0f},
	{+0.0f,-1.0f,+0.0f},
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
		cube_vertex_data[i].normal = norm[i];
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
	{{-1.0f, -1.0f, +0.0f},	{1.0f,0.0f,0.0f},	{1.0f, 0.0f}},
	{{+1.0f, -1.0f, +0.0f},	{0.0f,1.0f,0.0f},	{0.0f, 0.0f}},
	{{+1.0f, +1.0f, +0.0f},	{0.0f,0.0f,1.0f},	{0.0f, 1.0f}},
	{{-1.0f, +1.0f, +0.0f},	{1.0f,1.0f,1.0f},	{1.0f, 1.0f}}
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

void VKE::GeoPrimitives::Cubemap(VKE::RenderContext* render_ctx, VKE::RenderComponent* rend) {

	std::vector<VKE::Vertex> cubemap_vertex_data = {
		{ { -1.0f, -1.0f,  1.0f }, { 0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f } },
		{ {  1.0f, -1.0f,  1.0f }, { 0.0f, 0.0f, 0.0f}, { 1.0f, 0.0f } },
		{ {  1.0f,  1.0f,  1.0f }, { 0.0f, 0.0f, 0.0f}, { 1.0f, 1.0f } },
		{ { -1.0f,  1.0f,  1.0f }, { 0.0f, 0.0f, 0.0f}, { 0.0f, 1.0f } },

		{ {  1.0f,  1.0f,  1.0f }, { 0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f } },
		{ {  1.0f,  1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f}, { 1.0f, 0.0f } },
		{ {  1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f}, { 1.0f, 1.0f } },
		{ {  1.0f, -1.0f,  1.0f }, { 0.0f, 0.0f, 0.0f}, { 0.0f, 1.0f } },

		{ { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f } },
		{ {  1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f}, { 1.0f, 0.0f } },
		{ {  1.0f,  1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f}, { 1.0f, 1.0f } },
		{ { -1.0f,  1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f}, { 0.0f, 1.0f } },

		{ { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f } },
		{ { -1.0f, -1.0f,  1.0f }, { 0.0f, 0.0f, 0.0f}, { 1.0f, 0.0f } },
		{ { -1.0f,  1.0f,  1.0f }, { 0.0f, 0.0f, 0.0f}, { 1.0f, 1.0f } },
		{ { -1.0f,  1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f}, { 0.0f, 1.0f } },

		{ {  1.0f,  1.0f,  1.0f }, { 0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f } },
		{ { -1.0f,  1.0f,  1.0f }, { 0.0f, 0.0f, 0.0f}, { 1.0f, 0.0f } },
		{ { -1.0f,  1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f}, { 1.0f, 1.0f } },
		{ {  1.0f,  1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f}, { 0.0f, 1.0f } },

		{ { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f } },
		{ {  1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f}, { 1.0f, 0.0f } },
		{ {  1.0f, -1.0f,  1.0f }, { 0.0f, 0.0f, 0.0f}, { 1.0f, 1.0f } },
		{ { -1.0f, -1.0f,  1.0f }, { 0.0f, 0.0f, 0.0f}, { 0.0f, 1.0f } },
	};
	std::vector<ushort16> cubemap_index_data = {
		0,1,2, 0,2,3, 4,5,6,  4,6,7, 8,9,10, 8,10,11, 12,13,14, 12,14,15, 16,17,18, 16,18,19, 20,21,22, 20,22,23
	};

	VKE::InternalBuffer& vertex_internal_buffer = rend->getVertexBuffer();
	vertex_internal_buffer.init(render_ctx, sizeof(VKE::Vertex), cubemap_vertex_data.size(), VKE::BufferType_Vertex);
	vertex_internal_buffer.uploadData((void*)cubemap_vertex_data.data());

	VKE::InternalBuffer& index_internal_buffer = rend->getIndexBuffer();
	index_internal_buffer.init(render_ctx, sizeof(ushort16), cubemap_index_data.size(), VKE::BufferType_Index);
	index_internal_buffer.uploadData((void*)cubemap_index_data.data());

}

void VKE::GeoPrimitives::Heighmap(RenderContext* render_ctx, RenderComponent* rend, int64 width, int64 height, float64 vertex_density, int64 u_repeats, int64 v_repeats) {

	float64 pos_step = 1.0f / vertex_density;
	float64 u_step = 1.0f / (static_cast<float64>(width) / u_repeats);
	float64 v_step = 1.0f / (static_cast<float64>(height) / v_repeats);

	std::vector<VKE::Vertex> hm_vertex_data_;
	hm_vertex_data_.resize(width*height);

	uint64 vert_it = 0;
	for (int64 y = -(height/2); y < height/2; ++y) {
		for (int64 x = -(width/2); x < width/2; ++x, ++vert_it) {
			VKE::Vertex vertex;

			vertex.pos = glm::vec3(x * pos_step, 0.0f, y * pos_step);
			vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
			vertex.uv = glm::vec2((x+width/2) * u_step, (y+height/2) * v_step);

			hm_vertex_data_[vert_it] = vertex;
		}
	}

	std::vector<ushort16> hm_index_data_;
	hm_index_data_.resize((width-1)*(height-1)*6);

	uint64 index_it = 0;
	for (uint32 y = 0; y < height; ++y) {
		for (uint32 x = 0; x < width; ++x) {

			if (x == width - 1 || y == height - 1) continue;
			uint32 v = (x + y * height);

			hm_index_data_[index_it + 0] = v;
			hm_index_data_[index_it + 1] = v + 1 + height;
			hm_index_data_[index_it + 2] = v + 1;

			hm_index_data_[index_it + 3] = v + 1 + height;
			hm_index_data_[index_it + 4] = v;
			hm_index_data_[index_it + 5] = v + height;

			index_it += 6;
		}
	}

	VKE::InternalBuffer& vertex_internal_buffer = rend->getVertexBuffer();
	vertex_internal_buffer.init(render_ctx, sizeof(VKE::Vertex), hm_vertex_data_.size(), VKE::BufferType_Vertex);
	vertex_internal_buffer.uploadData((void*)hm_vertex_data_.data());

	VKE::InternalBuffer& index_internal_buffer = rend->getIndexBuffer();
	index_internal_buffer.init(render_ctx, sizeof(ushort16), hm_index_data_.size(), VKE::BufferType_Index);
	index_internal_buffer.uploadData((void*)hm_index_data_.data());

}
