#include "application.h"
#include "log.h"

#include <iostream>
#include <chrono>

namespace ec {

	Application* Application::s_app = nullptr;

	void Application::create(ApplicationCreateInfo& createInfo)
	{

		s_app = createInfo.app;

		Log::create();
		EC_LOG("Application: created the logger");
		m_window.create(createInfo.windowCreateInfo);

		EC_LOG("Application: created the window");

		// render, asset loader thread
		m_jobSystem.create(2);
		EC_LOG("Application: created the jobsystem (2 + 1 threads)");

		m_assetManager.create(m_jobSystem);
		EC_LOG("Application: created the asset manager");

		RendererCreateInfo rendererCreateInfo;
		rendererCreateInfo.window = &m_window;
		
		if (createInfo.flags & APPLICATION_USE_CUSTOM_RENDERER_CREATE_INFO) {
			createInfo.customRendererCreateInfo.window = &m_window;
			m_renderer.create(createInfo.customRendererCreateInfo);
			EC_LOG("Application: created the renderer (with custom create info)");
		}
		else {
			m_renderer.create(rendererCreateInfo);
			EC_LOG("Application: created the renderer");
		}

		m_eventSystem.create();
		EC_LOG("Application: created the event system");
		m_eventSystem.addEventListener([&](const Event& event) {return handleEvents(event); });
		m_eventSystem.addEventListener([&](const Event& event) { return m_renderer.handleEvents(event);});
		m_eventSystem.addEventListener(createInfo.eventCallback);

		createInfo.createCallback(*this);
		EC_LOG("Application: called the create callback");

		m_updateCallback = std::move(createInfo.updateCallback);
		m_synchronizedUpdateCallback = std::move(createInfo.synchronizedUpdateCallback);

		while (!m_windowShouldClose) {
			frame();
		}
		
		m_renderer.waitDeviceIdle();
		createInfo.terminateCallback();
		m_renderer.destroy();

		m_assetManager.destroy();

		m_jobSystem.destroy();

		m_eventSystem.destroy();

		m_window.terminate();

		Log::terminate();
	}

	const Window& Application::getWindow() const
	{
		return m_window;
	}

	Renderer& Application::getRenderer() {
		return m_renderer;
	}

	EventSystem& Application::getEventSystem()
	{
		return m_eventSystem;
	}

	AssetManager& Application::getAssetManager()
	{
		return m_assetManager;
	}

	bool Application::handleEvents(const Event& event)
	{
		
		if (event.eventType == EventType::ApplicationRecreateEvent) {
			EC_LOG("Application: ApplicationRecreateEvent");
		}
		
		return false;
	}

	Application& Application::getInstance()
	{
		return *s_app;
	}

	void Application::frame()
	{

		m_window.update(m_windowShouldClose);

		bool recreateApplication = false;

		RendererBeginFrameInfo beginInfo;
		beginInfo.recreateSwapchain = &recreateApplication;
		m_renderer.beginFrame(beginInfo);

		if (recreateApplication) {
			m_eventSystem.triggerEvent({ EventType::ApplicationRecreateEvent, nullptr, 0 });
		}

		m_jobSystem.queueJob([&] {

			m_renderer.drawFrame();
			m_renderer.submitFrame();
		});

		m_updateCallback();
	
		m_jobSystem.waitIdle();

		m_assetManager.handleStates();

		m_synchronizedUpdateCallback();
		
		m_eventSystem.handleDeferredEvents();

		RendererPresentFrameInfo presentInfo;
		presentInfo.recreateSwapchain = &recreateApplication;
		m_renderer.presentFrame(presentInfo);

		if (recreateApplication) {
			m_eventSystem.triggerEvent({ EventType::ApplicationRecreateEvent, nullptr, 0 });
		}
	}

}