#pragma once


#include "../Urho3D.h"
#include "../Core/Object.h"
#include "../Core/Context.h"
#include "Utils/Macros.h"


namespace Urho3D
{
	class UIElement;
	class ResourceCache;
	class UI;
	class Graphics;
	class Scene;
	class View3D;
	class Window;
	class Node;
	class Component;
	class Resource;
	class XMLFile;
	class FileSelector;

	class MenuBarUI;
	class ToolBarUI;
	class MiniToolBarUI;

	class TabWindow;
	class EditorData;

	class EditorView : public Object
	{
		OBJECT(EditorView);
	public:
		/// Construct.
		EditorView(Context* context,EditorData* data= NULL);
		/// Destruct.
		virtual ~EditorView();
		static void RegisterObject(Context* context);

		void HandlePost(StringHash eventType, VariantMap& eventData);
		void UpdateLayout();

		void SetToolBarVisible(bool enable);
		bool IsToolBarVisible() const;

		void SetLeftFrameVisible(bool enable);
		bool IsLeftFrameVisible() const;

		void SetRightFrameVisible(bool enable);
		bool IsRightFrameVisible() const;

// 	
// 		void SetMiniToolBarVisible(bool enable);
// 		bool IsMiniToolBarVisible() const;
		
		TabWindow* GetLeftFrame();
		TabWindow* GetRightFrame();
		TabWindow* GetMiddleFrame();
		ToolBarUI* GetToolBar();
		MiniToolBarUI* GetMiniToolBar();
		MenuBarUI* GetGetMenuBar();

		U_PROPERTY_IMP(int, minLeftFrameWidth_, MinLeftFrameWidth);
		U_PROPERTY_IMP(int, minMiddleFrameWidth_, MinMiddleFrameWidth);
		U_PROPERTY_IMP(int, minRightFrameWidth_, MinRightFrameWidth);
	protected:
		/// Cache Subsystems
		ResourceCache* cache_;
		Graphics* graphics_;
		UI* ui_;

		SharedPtr<UIElement> uiRoot_;
		SharedPtr<MenuBarUI> menubar_;
		SharedPtr<ToolBarUI> toolbar_;
		SharedPtr<MiniToolBarUI> miniToolbar_;
		SharedPtr<TabWindow> leftFrame_;
		SharedPtr<TabWindow> middleFrame_;
		SharedPtr<TabWindow> rightFrame_;

	
	};
}