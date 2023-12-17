#pragma once
#include <functional>
#include "scene/scene.h"

struct SharedWindowData {

	ec::Entity selectedEntity;
	ec::Scene scene;

	SharedWindowData() = default;
	~SharedWindowData() = default;

};

class SceneHierachyWindow {

public:

	SceneHierachyWindow() = default;
	~SceneHierachyWindow() = default;

	SceneHierachyWindow(const SceneHierachyWindow&) = delete;
	SceneHierachyWindow& operator=(const SceneHierachyWindow&) = delete;

	SceneHierachyWindow(const SceneHierachyWindow&&) = delete;
	SceneHierachyWindow& operator=(const SceneHierachyWindow&&) = delete;

	void create(SharedWindowData* sharedData);
	void update();
	void destroy();

private:

	uint32_t getTreeNodeFlags(ec::Entity& entity);

	SharedWindowData* m_sharedData;

};

class InspectorWindow {

public:

	InspectorWindow() = default;
	~InspectorWindow() = default;

	InspectorWindow(const InspectorWindow&) = delete;
	InspectorWindow& operator=(const InspectorWindow&) = delete;

	InspectorWindow(const InspectorWindow&&) = delete;
	InspectorWindow& operator=(const InspectorWindow&&) = delete;

	void create(SharedWindowData* sharedData);
	void update();
	void destroy();

private:

	SharedWindowData* m_sharedData;

};

class ViewportWindow {

public:

	ViewportWindow() = default;
	~ViewportWindow() = default;

	ViewportWindow(const ViewportWindow&) = delete;
	ViewportWindow& operator=(const ViewportWindow&) = delete;

	ViewportWindow(const ViewportWindow&&) = delete;
	ViewportWindow& operator=(const ViewportWindow&&) = delete;

	void create(SharedWindowData* sharedData, void* imageView, void* sampler);
	void update();
	void destroy();

private:

	void* m_renderImageDescriptorSet;

	SharedWindowData* m_sharedData;

};

struct EditorCreateInfo {

	void* viewportImageView;
	void* sampler;

};

class Editor {

public:

	Editor() = default;
	~Editor() = default;

	Editor(const Editor&) = delete;
	Editor& operator=(const Editor&) = delete;

	Editor(const Editor&&) = delete;
	Editor& operator=(const Editor&&) = delete;

	void update();
	void create(EditorCreateInfo& createInfo);
	void destroy();

	const SharedWindowData& getSharedData() const;

private:

	bool m_demoWindow = false;

	SceneHierachyWindow m_sceneHierachyPanel;
	InspectorWindow m_inspecorWindow;
	ViewportWindow m_viewportWindow;

	SharedWindowData m_sharedData;

};
