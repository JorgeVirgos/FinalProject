#ifndef __GEOMETRY_PRIMITIVES_H__
#define __GEOMETRY_PRIMITIVES_H__ 1

#include "platform_types.h"
#include "constants.h"

namespace VKE {

	class RenderContext;
	class RenderComponent;

	class GeoPrimitives {
	public:
		static void Cube(RenderContext* render_ctx, RenderComponent* rend);
		static void Sphere(RenderContext* render_ctx, RenderComponent* rend);
		static void Quad(RenderContext* render_ctx, RenderComponent* rend);
		static void Cubemap(RenderContext* render_ctx, RenderComponent* rend);


	private:
		GeoPrimitives();
		GeoPrimitives(const GeoPrimitives& other);
		~GeoPrimitives() {};
	};

}

#endif //__GEOMETRY_PRIMITIVES_H__