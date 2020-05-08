#include "hierarchy_context.h"
#include "render_context.h"
#include <algorithm>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <typeinfo>
#include <memory>

VKE::BaseComponent::BaseComponent() {
	id_ = UINT32_MAX;
	in_use_ = false;
}
VKE::BaseComponent::~BaseComponent(){

}
void VKE::BaseComponent::reset() {
	id_ = UINT32_MAX;
	in_use_ = false;
}



VKE::TransformComponent::TransformComponent() {
	pos_ = glm::vec3();
	quat_ = glm::quat();
	scale_ = glm::vec3(1.0f, 1.0f, 1.0f);
	local_matrix_ = glm::identity<glm::mat4>();
	world_matrix_ = glm::identity<glm::mat4>();
	render_ctx_ = nullptr;
}
VKE::TransformComponent::~TransformComponent() {

}

void VKE::TransformComponent::init(VKE::RenderContext* render_ctx) {
	render_ctx_ = render_ctx;
	ubm_buffer_ = render_ctx_->getResource<VKE::Buffer>();
	getUBMBuffer().init(render_ctx_, sizeof(VKE::UniformBufferMatrices), 1, BufferType_Uniform);
}

void VKE::TransformComponent::reset() {
	pos_ = glm::vec3();
	quat_ = glm::quat();
	scale_ = glm::vec3(1.0f, 1.0f, 1.0f);

	local_matrix_ = glm::identity<glm::mat4>();
	world_matrix_ = glm::identity<glm::mat4>();
	render_ctx_ = nullptr;

}

void VKE::TransformComponent::setLambda(std::function<void(VKE::HierarchyContext*, VKE::TransformComponent&)> new_lambda) {
	update_lambda = new_lambda;
}

void VKE::TransformComponent::setPosition(glm::vec3 pos) {
	pos_ = pos;
}
void VKE::TransformComponent::setPosition(float x, float y, float z) {
	pos_ = glm::vec3(x, y, z);
}

void VKE::TransformComponent::setRotation(glm::vec3 euler_rot) {
	quat_ = glm::toQuat(glm::orientate3(euler_rot));
}
void VKE::TransformComponent::setRotation(float x, float y, float z) {
	quat_ = glm::toQuat(glm::orientate3(glm::vec3(x,z,y)));
}

void VKE::TransformComponent::setScale(glm::vec3 scale) {
	scale_ = scale;
}
void VKE::TransformComponent::setScale(float x, float y, float z) {
	scale_ = glm::vec3(x, y, z);
}

glm::vec3 VKE::TransformComponent::getPosition() { return pos_; }
glm::vec3 VKE::TransformComponent::getRotation() { return glm::eulerAngles(quat_) * 3.14159f / 180.f; }
glm::vec3 VKE::TransformComponent::getScale() { return scale_; }

glm::mat4 VKE::TransformComponent::getLocalMatrix() { return local_matrix_; }
glm::mat4 VKE::TransformComponent::getGlobalMatrix() { return world_matrix_; }



VKE::InternalBuffer& VKE::TransformComponent::getUBMBuffer() {
	if (ubm_buffer_.id() == UINT32_MAX) throw std::runtime_error("Uninitialised Transform Component asking for UBM buffer");
	return render_ctx_->getInternalRsc<VKE::Buffer::internal_class>(ubm_buffer_);
}

VKE::RenderComponent::RenderComponent() {
	render_ctx_ = nullptr;
}

VKE::RenderComponent::~RenderComponent() {

}

void VKE::RenderComponent::reset() {

	getVertexBuffer().reset(render_ctx_);
	vertex_buffer_ = VKE::Buffer();

	getIndexBuffer().reset(render_ctx_);
	index_buffer_ = VKE::Buffer();

	getMaterial().reset(render_ctx_);
	material_ = VKE::Material();

	render_ctx_ = nullptr;
}

void VKE::RenderComponent::init(VKE::RenderContext* render_ctx) {
	render_ctx_ = render_ctx;

	vertex_buffer_ = render_ctx_->getResource<VKE::Buffer>();
	index_buffer_ = render_ctx_->getResource<VKE::Buffer>();
	material_ = render_ctx_->getResource<VKE::Material>();

}

VKE::InternalBuffer&  VKE::RenderComponent::getVertexBuffer() {
	return render_ctx_->getInternalRsc<VKE::Buffer::internal_class>(vertex_buffer_);
}
VKE::InternalBuffer&  VKE::RenderComponent::getIndexBuffer() {
	return render_ctx_->getInternalRsc<VKE::Buffer::internal_class>(index_buffer_);

}
VKE::InternalMaterial&  VKE::RenderComponent::getMaterial() {
	return render_ctx_->getInternalRsc<VKE::Material::internal_class>(material_);
}

VKE::Entity::Entity() {
	memset(name_, '/0', 64);
	tag_ = UINT32_MAX;
	id_ = UINT32_MAX;
	in_use_ = false;
}

VKE::Entity::~Entity() {

}

void VKE::Entity::reset(HierarchyContext* hier_ctx){
	memset(name_, '/0', 64);
	tag_ = UINT32_MAX;
	id_ = UINT32_MAX;
	in_use_ = false;

	std::for_each(components_.begin(), components_.end(), [hier_ctx](BaseComponent* comp) {
		comp->reset();
	});
}

template<class Component> Component* VKE::Entity::AddComponent(HierarchyContext* hier_ctx) {
	Manager<Component>* comp_manager = hier_ctx->GetComponentManager<Component>();
	Component& new_comp = comp_manager->getComponent();
	new_comp.init(hier_ctx->getRenderContext());
	components_.push_back(static_cast<BaseComponent*>(&new_comp));
	return &new_comp;
}
template VKE::TransformComponent* VKE::Entity::AddComponent<VKE::TransformComponent>(HierarchyContext* hier_ctx);
template VKE::RenderComponent* VKE::Entity::AddComponent<VKE::RenderComponent>(HierarchyContext* hier_ctx);

template<class Component> Component* VKE::Entity::AddComponent(HierarchyContext* hier_ctx, const Component& comp) {
	Manager<Component>* comp_manager = hier_ctx->GetComponentManager<Component>();
	Component& new_comp = comp_manager->getComponent();
	comp_manager->copyComponent(comp, new_comp);
	components_.push_back(static_cast<BaseComponent*>(&new_comp));
	return &new_comp;
}
template VKE::TransformComponent* VKE::Entity::AddComponent<VKE::TransformComponent>(HierarchyContext* hier_ctx, const VKE::TransformComponent& comp);
template VKE::RenderComponent* VKE::Entity::AddComponent<VKE::RenderComponent>(HierarchyContext* hier_ctx, const VKE::RenderComponent& comp);

template<class Component> Component* VKE::Entity::GetFirstComponent() {

	Component* casted_comp = nullptr;

	for (BaseComponent* comp : components_) {
		casted_comp = dynamic_cast<Component*>(comp);
		if (casted_comp != nullptr) return casted_comp;
	}

	return nullptr;
}
template VKE::TransformComponent* VKE::Entity::GetFirstComponent<VKE::TransformComponent>();
template VKE::RenderComponent* VKE::Entity::GetFirstComponent<VKE::RenderComponent>();

template<class Component> std::vector<Component*> VKE::Entity::GetComponents() {

	std::vector<Component*> valid_comps;
	Component* casted_comp = nullptr;

	for (BaseComponent* comp : components_) {
		casted_comp = dynamic_cast<Component*>(comp);
		if (casted_comp != nullptr) valid_comps.push_back(casted_comp);
	}

	return valid_comps;
}
template std::vector<VKE::TransformComponent*> VKE::Entity::GetComponents<VKE::TransformComponent>();
template std::vector<VKE::RenderComponent*> VKE::Entity::GetComponents<VKE::RenderComponent>();



template<class Component> VKE::Manager<Component>::Manager() {
	expand_storage();
}

template<class Component> VKE::Manager<Component>::~Manager() {

}

template<class Component> void VKE::Manager<Component>::expand_storage() {
	std::fill_n(std::back_inserter(comp_vec), 100, Component());
}

template<class Component> Component& VKE::Manager<Component>::getComponent() {
	
	for (Component& comp : comp_vec) {
		if (comp.in_use_ == false) {
			comp.in_use_ = true;
			return comp;
		}
	}

	throw std::runtime_error("Manager run out of unused components"/*, typeid(Component).name()*/);
	//return Component();
	//expand_storage();
	//return getComponent();
}

template<class Component> void VKE::Manager<Component>::Update(VKE::HierarchyContext* hier_ctx_, VKE::RenderContext* render_ctx_){}
template<> void VKE::Manager<VKE::TransformComponent>::Update(VKE::HierarchyContext* hier_ctx_, VKE::RenderContext* render_ctx_) {

	glm::mat4 translate, rotate, scale;
	for (TransformComponent& transform : comp_vec) {

		if (!transform.in_use_) continue;

		if (transform.update_lambda != nullptr) transform.update_lambda(hier_ctx_, transform);

		translate = glm::translate(glm::mat4(1.0f), transform.pos_);
		rotate = glm::toMat4(transform.quat_);
		scale = glm::scale(glm::mat4(1.0f), transform.scale_);

		transform.local_matrix_ = translate * rotate * scale;
		transform.world_matrix_ = transform.local_matrix_;

		render_ctx_->updateUniformMatrices(transform.ubm_buffer_, transform.world_matrix_);
	}
}
template<> void VKE::Manager<VKE::RenderComponent>::Update(VKE::HierarchyContext* hier_ctx_, VKE::RenderContext* render_ctx_) {
	
	VkViewport vp = render_ctx_->getScreenViewport();
	VkRect2D sc = render_ctx_->getScreenScissors();

	bool should_recreate_shader = render_ctx_->shouldRecreateShaders();
	if(should_recreate_shader) vkDeviceWaitIdle(render_ctx_->getDevice());

	for (RenderComponent& render : comp_vec) {
		if(!render.in_use_) continue;

		render.getMaterial().UpdateDynamicStates(1, vp, sc);
		if (should_recreate_shader) render.getMaterial().recreatePipeline();
	}

	if (should_recreate_shader) render_ctx_->setRecreateShaders(false);
}

template<class Component> void VKE::Manager<Component>::reset() {}
template<> void VKE::Manager<VKE::TransformComponent>::reset() {

	for (TransformComponent& transform : comp_vec) {
		transform.pos_ = glm::vec3();
		transform.quat_ = glm::quat();
		transform.scale_ = glm::vec3();

		transform.local_matrix_ = glm::identity<glm::mat4>();
		transform.world_matrix_ = glm::identity<glm::mat4>();

		transform.ubm_buffer_ = VKE::Buffer();
		transform.render_ctx_ = nullptr;
	}
}
template<> void VKE::Manager<VKE::RenderComponent>::reset() {

	for (RenderComponent& render : comp_vec) {
		render.getVertexBuffer().reset(render.render_ctx_);
		render.vertex_buffer_ = VKE::Buffer();

		render.getIndexBuffer().reset(render.render_ctx_);
		render.index_buffer_ = VKE::Buffer();

		render.getMaterial().reset(render.render_ctx_);
		render.material_ = VKE::Material();

		render.render_ctx_ = nullptr;
	}
}

template<class Component> void VKE::Manager<Component>::copyComponent(const Component& src_comp, Component& dst_comp) {}
template<> void VKE::Manager<VKE::TransformComponent>::copyComponent(const TransformComponent& src_comp, TransformComponent& dst_comp) {
	dst_comp.render_ctx_ = src_comp.render_ctx_;
	dst_comp.init(dst_comp.render_ctx_);

	dst_comp.pos_ = src_comp.pos_;
	dst_comp.quat_ = src_comp.quat_;
	dst_comp.scale_ = src_comp.scale_;
}
template<> void VKE::Manager<VKE::RenderComponent>::copyComponent(const RenderComponent& src_comp, RenderComponent& dst_comp) {
	dst_comp.render_ctx_ = src_comp.render_ctx_;
	dst_comp.init(dst_comp.render_ctx_);

	InternalBuffer& dst_vertex = dst_comp.getVertexBuffer();
	InternalBuffer& dst_index = dst_comp.getIndexBuffer();
	InternalMaterial &dst_mat = dst_comp.getMaterial();

	InternalBuffer& src_vertex = src_comp.render_ctx_->getInternalRsc<VKE::Buffer::internal_class>(src_comp.vertex_buffer_);
	InternalBuffer& src_index = src_comp.render_ctx_->getInternalRsc<VKE::Buffer::internal_class>(src_comp.index_buffer_);
	InternalMaterial &src_mat = src_comp.render_ctx_->getInternalRsc<VKE::Material::internal_class>(src_comp.material_);

	dst_vertex.init(dst_comp.render_ctx_, sizeof(VKE::Vertex), src_vertex.getElementCount(), BufferType_Vertex);
	dst_vertex.uploadData(nullptr);
	dst_comp.render_ctx_->copyBuffer(src_vertex, dst_comp.getVertexBuffer(), src_vertex.getElementCount() * sizeof(VKE::Vertex));

	dst_index.init(dst_comp.render_ctx_, sizeof(ushort16), src_index.getElementCount(), BufferType_Index);
	dst_index.uploadData(nullptr);
	dst_comp.render_ctx_->copyBuffer(src_index, dst_comp.getIndexBuffer(), src_index.getElementCount() * sizeof(ushort16));

	dst_mat.init(dst_comp.render_ctx_, dst_mat.mat_info_);
	dst_mat.UpdateTextures(src_mat.albedo_texture_);
}


template class VKE::Manager<VKE::TransformComponent>;
template class VKE::Manager<VKE::RenderComponent>;


VKE::HierarchyContext::HierarchyContext() {
	std::fill_n(std::back_inserter(entities_), 100, VKE::Entity());

	component_managers_.insert({ typeid(VKE::TransformComponent).hash_code(), new Manager<TransformComponent>() });
	component_managers_.insert({ typeid(VKE::RenderComponent).hash_code(), new Manager<RenderComponent>() });

}
VKE::HierarchyContext::HierarchyContext(VKE::RenderContext *render_ctx) {

	render_ctx_ = render_ctx;

	std::fill_n(std::back_inserter(entities_), 100, VKE::Entity());

	component_managers_.insert({ typeid(VKE::TransformComponent).hash_code(), new Manager<TransformComponent>() });
	component_managers_.insert({ typeid(VKE::RenderComponent).hash_code(), new Manager<RenderComponent>() });

}

VKE::HierarchyContext::~HierarchyContext() {

	std::for_each(component_managers_.begin(), component_managers_.end(), [](std::pair<uint64, void*> manager) {
		delete manager.second;
	});

}

VKE::Entity& VKE::HierarchyContext::getEntity(std::string name) {
	for (Entity& entity : entities_) {
		if (entity.in_use_ == false) {
			strcpy(entity.name_, name.data());
			entity.in_use_ = true;
			entities_in_use++;
			return entity;
		} 
	}

	throw std::runtime_error("HierarchyContext run out of unused Entities");
	//return Entity();
}

template<class Component> VKE::Manager<Component>* VKE::HierarchyContext::GetComponentManager() {
	return (VKE::Manager<Component>*)component_managers_[typeid(Component).hash_code()];
}
template VKE::Manager<VKE::TransformComponent>* VKE::HierarchyContext::GetComponentManager<VKE::TransformComponent>();
template VKE::Manager<VKE::RenderComponent>* VKE::HierarchyContext::GetComponentManager<VKE::RenderComponent>();

void VKE::HierarchyContext::UpdateManagers() {
	((VKE::Manager<TransformComponent>*)component_managers_[typeid(VKE::TransformComponent).hash_code()])->Update(this, render_ctx_);
	((VKE::Manager<RenderComponent>*)component_managers_[typeid(VKE::RenderComponent).hash_code()])->Update(this, render_ctx_);
}

std::vector<VKE::ui_data>& VKE::HierarchyContext::getUIData() {

	entities_ui_data.resize(entities_in_use);

	uint32 it = 0;
	for (Entity& entity : entities_) {
		if (entity.in_use_ == false) continue;
		memset(entities_ui_data[it].entity_name, '/0', 64);
		strcpy(entities_ui_data[it].entity_name, entity.name_);
		entities_ui_data[it].num_transform = entity.GetComponents<VKE::TransformComponent>().size();
		entities_ui_data[it].num_render = entity.GetComponents<VKE::RenderComponent>().size();
		it++;
	}

	return entities_ui_data;
}
