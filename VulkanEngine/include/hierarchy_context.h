#ifndef __COMPONENT_CONTEXT_H__
#define __COMPONENT_CONTEXT_H__ 1

#include "platform_types.h"
#include <vector>
#include <map>
#include <functional>
#include <glm/matrix.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "internal_buffer.h"
#include "internal_texture.h"
#include "internal_material.h"


namespace VKE{

	class HierarchyContext;
	class RenderContext;
	template<class Component> class Manager;

	class BaseComponent {
	public:
		virtual void reset();
		virtual void init(VKE::RenderContext* render_ctx) = 0;
	protected:
		BaseComponent();
		~BaseComponent();

		uint32 id_;
		bool in_use_;

	};

	class TransformComponent : public BaseComponent {
	public:
		TransformComponent();
		~TransformComponent();


		void init(VKE::RenderContext* render_ctx) override;

		void setPosition(glm::vec3 pos);
		void setPosition(float x, float y, float z);

		void setRotation(glm::vec3 euler_rot);
		void setRotation(float x, float y, float z);

		void setScale(glm::vec3 scale);
		void setScale(float x, float y, float z);

		glm::vec3 getPosition();
		glm::vec3 getRotation();
		glm::vec3 getScale();

		glm::mat4 getLocalMatrix();
		glm::mat4 getGlobalMatrix();

		VKE::InternalBuffer& getUBMBuffer();

		void reset() override;
		void setLambda(std::function<void(HierarchyContext*,TransformComponent&)>);

	private:
		std::function<void(VKE::HierarchyContext*, TransformComponent&)> update_lambda = nullptr;


		glm::vec3 pos_;
		glm::quat quat_;
		glm::vec3 scale_;

		glm::mat4 local_matrix_;
		glm::mat4 world_matrix_;

		VKE::RenderContext *render_ctx_;
		VKE::Buffer ubm_buffer_;

		friend class Manager<TransformComponent>;
	};

	class RenderComponent : public BaseComponent {
	public:
		RenderComponent();
		~RenderComponent();


		void reset() override;
		void init(VKE::RenderContext* render_ctx) override;
		VKE::InternalBuffer& getVertexBuffer();
		VKE::InternalBuffer& getIndexBuffer();
		VKE::InternalMaterial& getMaterial();

		void RecreateShader();

	private:
		VKE::Buffer vertex_buffer_;
		VKE::Buffer index_buffer_;
		VKE::Material material_;

		VKE::RenderContext* render_ctx_;

		friend class Manager<RenderComponent>;
	};

	template<class Component>
	class Manager {
	public:
		Manager();
		~Manager();

		void expand_storage();
		void Update(VKE::HierarchyContext* hier_ctx_, VKE::RenderContext* render_ctx_);
		void copyComponent(const Component& src_comp, Component& dst_comp);
		void reset();
		Component& getComponent();

	protected:
		std::vector<Component> comp_vec;
	};


	class Entity {
	public:
		Entity();
		~Entity();

		void reset(HierarchyContext* hier_ctx);

		template<class Component> Component* AddComponent(HierarchyContext* hier_ctx);
		template<class Component> Component* AddComponent(HierarchyContext* hier_ctx, const Component& comp);

		template<class Component> Component* GetFirstComponent();
		template<class Component> std::vector<Component*> GetComponents();

		bool isInUse() { return in_use_; }

	protected:

		char name_[64];
		uint32 tag_;
		uint32 id_;
		bool in_use_;

		std::vector<uint32> child_ids_;
		std::vector<BaseComponent*> components_;

		friend class HierarchyContext;
	};



	class HierarchyContext {
	public:
		HierarchyContext(RenderContext* render_ctx);
		~HierarchyContext();

		Entity& getEntity(std::string name);
		RenderContext* getRenderContext() { return render_ctx_; }
		template<class Component> Manager<Component>* GetComponentManager();
		void UpdateManagers();

		std::vector<VKE::ui_data>& getUIData();

	private:
		HierarchyContext();

		std::vector<VKE::ui_data> entities_ui_data;


		std::vector<Entity> entities_;
		std::map<uint64, void*> component_managers_;
		uint32 entities_in_use = 0;
		
		RenderContext* render_ctx_;

		friend class RenderContext;
	};
}

#endif //__COMPONENT_CONTEXT_H__