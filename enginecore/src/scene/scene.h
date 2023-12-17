#pragma once

#include <entt/entt.hpp>

#include "core/core.h"
#include "components.h"

namespace ec {

	class Entity {

	public:

		Entity(entt::entity e, entt::registry* reg) : m_entity(e), m_reg(reg) {}

		Entity() = default;
		~Entity() = default;

		operator entt::entity() { return m_entity; };

		template<typename T, typename... Args>
		T& addComponent(Args&&... args) {
			EC_ASSERT(m_reg);
			return m_reg->emplace<T>(m_entity, std::forward<Args>(args)...);
		}

		template<typename T, typename... Args>
		T& replaceComponent(Args&&... args) {
			EC_ASSERT(m_reg);
			return m_reg->replace<T>(m_entity, std::forward<Args>(args...));
		}

		template<typename... T>
		void removeComponents() {
			EC_ASSERT(m_reg);
			m_reg->remove<T...>(m_entity);
		}

		template<typename T> 
		T& getComponent() {
			EC_ASSERT(m_reg);
			EC_ASSERT(hasComponents<T>())
			return m_reg->get<T>(m_entity);
		}

		template<typename... T>
		bool hasComponents() {
			EC_ASSERT(m_reg);
			return m_reg->all_of<T...>(m_entity);
		}

		bool valid() {
			if (!m_reg) return false;
			return m_reg->valid(m_entity);
		}

		std::string& getTag() {
			return getComponent<IDComponent>().tag;
		}

		uint64_t getID() {
			return getComponent<IDComponent>().id;
		}

		bool operator==(Entity& other) { return m_entity == other.m_entity; }

	private:

		entt::entity m_entity = entt::null;
		entt::registry* m_reg = nullptr;

	};

	using NullEntity = Entity;

	class Scene {

	public:

		Scene() = default;
		~Scene() = default;

		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;

		Scene(const Scene&&) = delete;
		Scene& operator=(const Scene&&) = delete;

		Entity createEntity();
		void destroyEntity(Entity e);

		std::vector<Entity> getEntities();

		const entt::registry& raw() const;

	private:

		entt::registry m_reg;

	};

}