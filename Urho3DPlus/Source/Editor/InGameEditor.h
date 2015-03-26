#pragma once



#include "../Core/Object.h"
#include "Utils/Macros.h"


namespace Urho3D
{

	EVENT(E_QUIT_INGAMEEDITOR_, QuitInGameEditor)
	{

	}
	EVENT(E_START_INGAMEEDITOR_, StartInGameEditor)
	{

	}
	class Node;
	class Component;
	class Camera;

	class Button;
	class BorderImage;
	class DropDownList;
	class Engine;
	class Font;
	class LineEdit;
	class ListView;
	class Text;
	class UIElement;
	class XMLFile;
	class Scene;
	class MenuBarUI;
	class ToolBarUI;
	class MiniToolBarUI;

	class ResourceCache;
	class UI;
	class Graphics;
	class HierarchyWindow;
	class AttributeInspector;
	class EditorSelection;
	class EditorPlugin;
	class Viewport;

	class InGameEditor : public Object
	{
		OBJECT(InGameEditor);
	public:
		/// Construct.
		InGameEditor(Context* context);
		/// Destruct.
		virtual ~InGameEditor();
		/// Register object factory.
		static void RegisterObject(Context* context);

		/// Toggle visibility.
		void Toggle();

		/// Update Attribute Inspector manually.
		void UpdateAttributeInspector();

		/// Register an Editor Plugin
		bool RegisterEditorPlugin(EditorPlugin* plugin);
		/// set main editor plugin
		bool SetMainEditor(const String& name);
		/// set main editor plugin
		bool SetMainEditor(StringHash name);


		/// Set UI elements' style from an XML file.
		void SetDefaultStyle(XMLFile* style);
		/// Show or hide.
		void SetVisible(bool enable);
		/// Set the scene to be edited.
		void SetScene(Scene* scene);
		/// Set the scene to be edited.
		void SetSceneUI(UIElement* sceneUI);

		/// Return the UI style file.
		XMLFile* GetDefaultStyle() const;
		/// Return whether is visible.
		bool IsVisible() const;
		/// Return the edited scene.
		Scene* GetScene();
		/// Return the edited scene.
		UIElement* GetSceneUI();
		/// Return the editor camera node.
		Node* GetCameraNode();

	protected:
		/// Input Events Handler
		void HandleKeyDown(StringHash eventType, VariantMap& eventData);
		void HandleKeyUp(StringHash eventType, VariantMap& eventData);
		void HandleUpdate(StringHash eventType, VariantMap& eventData);


		void HandleMenuBarAction(StringHash eventType, VariantMap& eventData);
		void HandleHierarchyListSelectionChange(StringHash eventType, VariantMap& eventData);

		Component*	GetListComponent(UIElement* item);
		Node*		GetListNode(UIElement* item);
		UIElement*	GetListUIElement(UIElement*  item);
		UIElement*	GetUIElementByID(const Variant& id);

		ResourceCache*	cache_;
		Graphics*		graphics_;
		UI*				ui_;

		SharedPtr<UIElement>	rootUI_;
		SharedPtr<Scene>		scene_;
		SharedPtr<UIElement>	sceneUI_;
		SharedPtr<EditorSelection>	editorData_;

		SharedPtr<XMLFile>			defaultStyle_;
		SharedPtr<XMLFile>			iconStyle_;
		SharedPtr<MenuBarUI>		menubar_;
		SharedPtr<ToolBarUI>		toolbar_;
		SharedPtr<MiniToolBarUI>	minitoolbar_;
		SharedPtr<HierarchyWindow>		hierarchyWindow_;
		SharedPtr<AttributeInspector>	attributeInspector_;

		EditorPlugin*			mainEditorPlugin_;
		Vector<EditorPlugin*>	activeSubEditorPlugins_;
		Vector<EditorPlugin*>	mainEditorPlugins_;
		Vector<SharedPtr<EditorPlugin> > allEditorPlugins_;

		SharedPtr<Camera>	camera_;
		SharedPtr<Node>		cameraNode_;
		SharedPtr<Viewport> viewport_;
		SharedPtr<Viewport> backupViewport_;
	};


	void RegisterInGameEditor(Context* context);
}

