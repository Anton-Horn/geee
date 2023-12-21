#include "application.h"
#include "log.h"

#include <iostream>

#include <chrono>

namespace ec {

	void Application::create(ApplicationCreateInfo& createInfo)
	{

		Log::create();
		m_window.create(createInfo.windowCreateInfo);

		m_eventSystem.addEventListener(std::bind(&Application::handleEvents, this, std::placeholders::_1));
		m_eventSystem.addEventListener(createInfo.eventCallback);

		// render, asset loader thread
		m_jobSystem.create(2);

		RendererCreateInfo rendererCreateInfo;
		rendererCreateInfo.window = &m_window;

		m_renderer.create(rendererCreateInfo);

		createInfo.createCallback(*this);
		m_updateCallback = std::move(createInfo.updateCallback);

		while (!m_windowShouldClose) {
			frame();
		}

		
		m_renderer.waitDeviceIdle();
		createInfo.terminateCallback();
		m_renderer.destroy();

		m_jobSystem.destroy();

		m_window.terminate();

		Log::terminate();
	}

	const Window& Application::getWindow() const
	{
		return m_window;
	}

	const Renderer& Application::getRenderer() const
	{
		return m_renderer;
	}

	Renderer& Application::getRenderer() {
		return m_renderer;
	}

	bool Application::handleEvents(const Event& event)
	{
		return false;
	}

	const Scene& Application::getScene() const
	{
		return m_scene;
	}

	Scene& Application::getScene()
	{
		return m_scene;
	}

	void Application::frame()
	{
		
		m_window.update(m_windowShouldClose);

		bool recreateApplication;

		// Thread scene synchronisation
		m_renderer.setSceneData(m_scene);

		RendererBeginFrameInfo beginInfo;
		beginInfo.recreateSwapchain = &recreateApplication;
		m_renderer.beginFrame(beginInfo);

		if (recreateApplication) {
			m_eventSystem.triggerEvent({ EventType::ApplicationRecreateEvent, nullptr, 0 });
		}

		m_jobSystem.queueJob([this] {

			m_renderer.drawFrame();

		});

		m_updateCallback();
	
		m_jobSystem.waitIdle();
		
		/*m_renderer.draw();
		m_updateCallback();*/

		m_renderer.submitFrame();

		RendererPresentFrameInfo presentInfo;
		presentInfo.recreateSwapchain = &recreateApplication;
		m_renderer.presentFrame(presentInfo);

		if (recreateApplication) {
			m_eventSystem.triggerEvent({ EventType::ApplicationRecreateEvent, nullptr, 0 });
		}
	}

}