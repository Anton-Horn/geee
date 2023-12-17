#include "Scene.h"

#include "components.h"
#include "core/random.h"

#include "rendering/renderer.h"

namespace ec {

	Entity Scene::createEntity()
	{

		Entity e(m_reg.create(), &m_reg);
		e.addComponent<IDComponent>("Empty Entity", ec::randomInt<uint64_t>());

		return e;
	}

	void Scene::destroyEntity(Entity e)
	{
		m_reg.destroy(e);
	}

	std::vector<Entity> Scene::getEntities()
	{
		
		std::vector<Entity> result;
		for (auto& e : m_reg.storage<entt::entity>().each()) {
			
			result.emplace_back(std::get<0>(e), &m_reg);
		}
		return result;
	}

	const entt::registry& Scene::raw() const
	{
		return m_reg;
	}


}
