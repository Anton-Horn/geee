#pragma once


#include <string>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

#include "core/core.h"

namespace ec {

	struct IDComponent {

		std::string tag;
		uint64_t id;

		IDComponent(const std::string& tag, uint64_t id) : tag(tag), id(id) {};

	};

	struct TransformComponent {
		
		glm::vec3 positon;
		glm::vec3 scale;
		glm::vec3 rotation;
		glm::mat4 transform;

		void calculateTransform() {
			transform = glm::translate(glm::mat4(1.0f), positon) * glm::toMat4(glm::quat(rotation)) * glm::scale(glm::mat4(1.0f), scale);
		}

		TransformComponent(const glm::vec3& positon, const glm::vec3& scale, const glm::vec3& rotation) : positon(positon), scale(scale), rotation(rotation) {}

		TransformComponent() = default;

	};

	struct QuadRenderComponent {

		glm::vec4 color;

		QuadRenderComponent(const glm::vec4& color) : color(color) {}

		QuadRenderComponent() = default;

	};


}