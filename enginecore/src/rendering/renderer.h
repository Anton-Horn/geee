#pragma once
#include "core/core.h"
#include "core/application.h"

namespace ec {

	struct vec3 {

		float x, y, z;

	};

	struct vec4 {

		float x, y, z, w;

	};

	struct RendererData;

	struct RendererCreateInfo {

		const Window* window;
		std::function<void()> quadRendererDrawCallback;
		std::function<void()> bezierRendererDrawCallback;

	};

	class Renderer {

	public:

		void init(RendererCreateInfo& rendererCreateInfo);
		void draw();
		void destroy();

		void drawBezierCurve(const vec3& p1, const vec3& p2, const vec3& c1, const vec3& c2, const vec4& color);

		Renderer();
		~Renderer();

		const RendererData& getData() const;

	private:

		std::function<void()> m_quadRendererDrawCallback;
		std::function<void()> m_bezierRendererDrawCallback;

		std::unique_ptr<RendererData> m_data;

	};

}

