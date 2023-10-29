#pragma once
#include "core/core.h"
#include "core/application.h"

namespace ec {

	struct RendererData;

	class Renderer {

	public:

		void init(const Window& window);
		void draw();
		void destroy();

		Renderer();
		~Renderer();

	private:

		std::unique_ptr<RendererData> m_data;

	};

}

