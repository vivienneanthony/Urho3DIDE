#pragma once
#include "../Core/Object.h"

#include "../Container/Vector.h"
#include "../Core/Variant.h"


namespace Urho3D
{
	class MenuBarUI;
	class ToolBarUI;
	class MiniToolBarUI;
	class UIElement;
	class ProjectManager;
	class ResourceCache;
	class UI;
	class Graphics;
	class ProjectWindow;
	class HierarchyWindow;
	class Scene;
	class View3D;
	class Window;
	class Node;
	class Component;
	class Resource;
	class Editor;
	class XMLFile;
	class AttributeWindowUI;
	class AttributeInspector;
	class FileSelector;
	class Camera;
	class EditorPlugin;


	class EditorData : public Object
	{
		friend class Editor;

		OBJECT(EditorData);
	public:
		EditorData(Context* context, Editor* editor = NULL);
		virtual ~EditorData();
		static void RegisterObject(Context* context);
		void Load();
		
		void			SetGlobalVarNames(const String& name);
		const Variant&	GetGlobalVarNames(StringHash& name);

		/// Get the editable scene.
		Scene*	GetEditorScene();
		/// Set the editable scene.
		void	SetEditorScene(Scene* scene);
		// Editor Plugin handling.
		/// return an editor plugin that can handle this object and has the main screen (middle frame).
		EditorPlugin* GetEditor(Object *object);
		/// return an editor plugin that can handle this object and is not in the middle frame tabs.
		EditorPlugin* GetSubeditor(Object *object);
		/// return an editor plugin by name.
		EditorPlugin* GetEditor(const String& name);
		/// add editor plugin
		void AddEditorPlugin(EditorPlugin* plugin);
		/// remove editor plugin
		void RemoveEditorPlugin(EditorPlugin* plugin);

		
		XMLFile*	GetDefaultStyle(){ return defaultStyle_; }
		XMLFile*	GetIconStyle() { return iconStyle_; }
		UIElement*	GetEdiorRootUI() { return rootUI_; }
		Editor*		GetEditor();

		Vector<String> uiSceneFilters;
		Vector<String> uiElementFilters;
		Vector<String> uiAllFilters;
		Vector<String> uiScriptFilters;
		Vector<String> uiParticleFilters;
		Vector<String> uiRenderPathFilters;
		unsigned int uiSceneFilter;
		unsigned int  uiElementFilter;
		unsigned int  uiNodeFilter;
		unsigned int  uiImportFilter;
		unsigned int  uiScriptFilter;
		unsigned int  uiParticleFilter;
		unsigned int  uiRenderPathFilter;
		String uiScenePath;
		String uiElementPath;
		String uiNodePath;
		String uiImportPath;
		String uiScriptPath;
		String uiParticlePath;
		String uiRenderPathPath;
		Vector<String> uiRecentScenes;
		String screenshotDir;

	protected:

		SharedPtr<Scene> scene_;
		/// loaded 
		SharedPtr<XMLFile> defaultStyle_;
		SharedPtr<XMLFile> iconStyle_;
		SharedPtr<UIElement> rootUI_;
		Editor* editor_;
		// Node or UIElement hash-to-varname reverse mapping
		VariantMap globalVarNames_;
		/// Editor plugin handling
		Vector<SharedPtr<EditorPlugin> > editorPlugins_;
	};

}