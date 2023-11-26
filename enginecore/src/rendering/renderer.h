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

	struct MandelbrotSpec {

		float csX;
		float csY;
		float iterations;
		float zoom;

	};

	struct RendererData;

	struct RendererCallbacks {

		std::optional<std::function<void()>> drawCallback;

		std::optional<std::function<void()>> recreateCallback;

		std::optional<std::function<void()>> goochRendererDrawCallback;
		std::optional<std::function<void()>> quadRendererDrawCallback;
		std::optional<std::function<void()>> bezierRendererDrawCallback;

	};

	struct RendererCreateInfo {

		const Window* window;
		RendererCallbacks callbacks;
		

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
		RendererData& getData();

	private:

		void recreate();
		
		RendererCallbacks m_callbacks;
		std::unique_ptr<RendererData> m_data;

	};

}

