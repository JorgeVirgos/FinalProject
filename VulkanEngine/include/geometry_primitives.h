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
		static void Heighmap(RenderContext* render_ctx, RenderComponent* rend, int64 width = 100, int64 height = 100, float64 vertex_density = 1.0f, int64 u_repeats = 1, int64 v_repeats = 1);


	private:
		GeoPrimitives();
		GeoPrimitives(const GeoPrimitives& other);
		~GeoPrimitives() {};
	};

}

#endif //__GEOMETRY_PRIMITIVES_H__