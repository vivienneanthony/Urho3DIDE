#include "EditorView.h"
#include "../Resource/ResourceCache.h"
#include "../UI/UI.h"
#include "../Graphics/Graphics.h"
#include "../Input/InputEvents.h"
#include "../UI/Window.h"
#include "../UI/UIElement.h"
#include "../UI/UIEvents.h"
#include "../Scene/Scene.h"
#include "../UI/View3D.h"
#include "../Scene/Node.h"
#include "../Graphics/Camera.h"
#include "../Graphics/Zone.h"
#include "../Math/BoundingBox.h"
#include "../Math/Color.h"
#include "../Math/Quaternion.h"
#include "../Graphics/StaticModel.h"
#include "../Graphics/Model.h"
#include "../Graphics/Material.h"
#include "../Graphics/Light.h"
#include "../Graphics/Octree.h"
#include "../UI/UIElement.h"
#include "../Resource/Resource.h"
#include "../Container/Str.h"
#include "../UI/FileSelector.h"
#include "../Resource/XMLFile.h"

#include "UI/HierarchyWindow.h"
#include "UI/AttributeInspector.h"
#include "UI/MenuBarUI.h"
#include "UI/ToolBarUI.h"
#include "UI/MiniToolBarUI.h"
#include "UI/UIUtils.h"
#include "../Core/CoreEvents.h"
#include "TabWindow.h"
#include "EditorData.h"

namespace Urho3D
{
	EditorView::EditorView(Context* context, EditorData* data) : Object(context)
	{
		if (!data)
			return;

		cache_ = GetSubsystem<ResourceCache>();
		graphics_ = GetSubsystem<Graphics>();
		ui_ = GetSubsystem<UI>();

		XMLFile* styleFile = NULL;
		XMLFile* iconstyle = NULL;
		if (data)
		{
			styleFile = data->GetDefaultStyle();
			iconstyle = data->GetIconStyle();
		}
		else
		{
			styleFile = cache_->GetResource<XMLFile>("UI/IDEStyle.xml");
			iconstyle = cache_->GetResource<XMLFile>("UI/IDEIcons.xml");
		}

		uiRoot_ = data->GetEdiorRootUI();

		menubar_ = MenuBarUI::Create(uiRoot_, "EditorMenuBar");

		toolbar_ = ToolBarUI::Create(uiRoot_, "EditorToolBar", iconstyle);

		int minitoolheight = ui_->GetRoot()->GetHeight() - toolbar_->GetMinHeight() - menubar_->GetMinHeight();

		UIElement* mainFrameUI_ = uiRoot_->CreateChild<UIElement>("MainFrame");
		mainFrameUI_->SetDefaultStyle(styleFile);
		mainFrameUI_->SetFixedSize(ui_->GetRoot()->GetWidth(), minitoolheight);

		miniToolbar_ = MiniToolBarUI::Create(mainFrameUI_, "EditorMiniToolBar", iconstyle, 28, minitoolheight, styleFile);
	

		leftFrame_ = mainFrameUI_->CreateChild<TabWindow>("LeftFrame");

		leftFrame_->SetStyle("TabWindow");
		leftFrame_->SetWidth(200);
		leftFrame_->SetFixedHeight(minitoolheight);
		leftFrame_->SetResizable(true);
		leftFrame_->SetResizeBorder(IntRect(6, 6, 6, 6));
		leftFrame_->SetLayoutBorder(IntRect(4, 4, 4, 4));
		leftFrame_->SetPosition(miniToolbar_->GetPosition() + IntVector2(miniToolbar_->GetWidth(), 0));

		middleFrame_ = mainFrameUI_->CreateChild<TabWindow>("MiddleFrame");
		middleFrame_->SetPosition(leftFrame_->GetPosition() + IntVector2(leftFrame_->GetWidth(), 0));
		middleFrame_->SetStyle("TabWindow");
		middleFrame_->SetHeight(minitoolheight);
		middleFrame_->SetWidth(ui_->GetRoot()->GetWidth() - 400);
		middleFrame_->SetLayoutBorder(IntRect(4, 4, 4, 4));

		rightFrame_ = mainFrameUI_->CreateChild<TabWindow>("LeftFrame");
		rightFrame_->SetPosition(middleFrame_->GetPosition() + IntVector2(middleFrame_->GetWidth(), 0));
		rightFrame_->SetStyle("TabWindow");
		rightFrame_->SetLayout(LM_VERTICAL);
		rightFrame_->SetWidth(200);
		rightFrame_->SetResizable(true);
		rightFrame_->SetFixedHeight(minitoolheight);
		rightFrame_->SetResizeBorder(IntRect(6, 6, 6, 6));
		rightFrame_->SetLayoutBorder(IntRect(4, 4, 4, 4));
		SubscribeToEvent(rightFrame_, E_RESIZED, HANDLER(EditorView, HandlePost));
		SubscribeToEvent(leftFrame_, E_RESIZED, HANDLER(EditorView, HandlePost));

		minLeftFrameWidth_ = 100;
		minMiddleFrameWidth_ = 200; /// \todo use minMiddleFrameWidth_
		minRightFrameWidth_ = 100;
	}

	EditorView::~EditorView()
	{
	}

	void EditorView::RegisterObject(Context* context)
	{
		context->RegisterFactory<EditorView>();
	}

	void EditorView::HandlePost(StringHash eventType, VariantMap& eventData)
	{
		using namespace Resized;

		UIElement* element = (UIElement*)eventData[P_ELEMENT].GetPtr();
		int width = eventData[P_WIDTH].GetInt();
		int height = eventData[P_HEIGHT].GetInt();

		if (element == leftFrame_)
		{
			if (width < minLeftFrameWidth_)
			{
				leftFrame_->SetWidth(minLeftFrameWidth_);
			}
			middleFrame_->SetPosition(leftFrame_->GetPosition() + IntVector2(leftFrame_->GetWidth(), 0));
			if (rightFrame_->IsVisible())
			{
				middleFrame_->SetWidth(ui_->GetRoot()->GetWidth() - leftFrame_->GetWidth() - rightFrame_->GetWidth() - miniToolbar_->GetWidth());
			}
			else
				middleFrame_->SetWidth(ui_->GetRoot()->GetWidth() - leftFrame_->GetWidth() - miniToolbar_->GetWidth());
		}
		else if (element == rightFrame_)
		{
			if (width < minRightFrameWidth_)
			{
				rightFrame_->SetWidth(minRightFrameWidth_);
			}
			if (leftFrame_->IsVisible())
			{
				middleFrame_->SetWidth(ui_->GetRoot()->GetWidth() - leftFrame_->GetWidth() - rightFrame_->GetWidth() - miniToolbar_->GetWidth());
			}
			else
				middleFrame_->SetWidth(ui_->GetRoot()->GetWidth() - rightFrame_->GetWidth() - miniToolbar_->GetWidth());

			rightFrame_->SetPosition(middleFrame_->GetPosition() + IntVector2(middleFrame_->GetWidth(), 0));
		}
	}

	void EditorView::UpdateLayout()
	{
		int minitoolheight = ui_->GetRoot()->GetHeight() - menubar_->GetMinHeight();
		if (toolbar_->IsVisible())
			minitoolheight -= toolbar_->GetMinHeight();
		miniToolbar_->SetFixedHeight(minitoolheight);
		middleFrame_->SetFixedHeight(minitoolheight);
		if (leftFrame_->IsVisible())
		{
			if (miniToolbar_->IsVisible())
				leftFrame_->SetPosition(miniToolbar_->GetPosition() + IntVector2(miniToolbar_->GetWidth(), 0));
			else
				leftFrame_->SetPosition(IntVector2::ZERO);

			middleFrame_->SetPosition(leftFrame_->GetPosition() + IntVector2(leftFrame_->GetWidth(), 0));
			leftFrame_->SetFixedHeight(minitoolheight);
			if (rightFrame_->IsVisible())
			{
				rightFrame_->SetFixedHeight(minitoolheight);
				middleFrame_->SetWidth(ui_->GetRoot()->GetWidth() - leftFrame_->GetWidth() - rightFrame_->GetWidth() - miniToolbar_->GetWidth());
				rightFrame_->SetPosition(middleFrame_->GetPosition() + IntVector2(middleFrame_->GetWidth(), 0));
			}
			else
				middleFrame_->SetWidth(ui_->GetRoot()->GetWidth() - leftFrame_->GetWidth() - miniToolbar_->GetWidth());
		}
		else
		{
			if (miniToolbar_->IsVisible())
				middleFrame_->SetPosition(miniToolbar_->GetPosition() + IntVector2(miniToolbar_->GetWidth(), 0));
			else
				middleFrame_->SetPosition(IntVector2::ZERO);

			if (rightFrame_->IsVisible())
			{
				rightFrame_->SetFixedHeight(minitoolheight);
				middleFrame_->SetWidth(ui_->GetRoot()->GetWidth() - rightFrame_->GetWidth() - miniToolbar_->GetWidth());
				rightFrame_->SetPosition(middleFrame_->GetPosition() + IntVector2(middleFrame_->GetWidth(), 0));
			}
			else
				middleFrame_->SetWidth(ui_->GetRoot()->GetWidth() - miniToolbar_->GetWidth());
		}
	}

	void EditorView::SetToolBarVisible(bool enable)
	{
		toolbar_->SetVisible(enable);
		UpdateLayout();
	}

	bool EditorView::IsToolBarVisible() const
	{
		return toolbar_->IsVisible();
	}

	void EditorView::SetLeftFrameVisible(bool enable)
	{
		leftFrame_->SetVisible(enable);
		UpdateLayout();
	}

	bool EditorView::IsLeftFrameVisible() const
	{
		return leftFrame_->IsVisible();
	}

	void EditorView::SetRightFrameVisible(bool enable)
	{
		rightFrame_->SetVisible(enable);
		UpdateLayout();
	}

	bool EditorView::IsRightFrameVisible() const
	{
		return rightFrame_->IsVisible();
	}

// 	void EditorView::SetMiniToolBarVisible(bool enable)
// 	{
// 		miniToolbar_->SetVisible(enable);
// 		UpdateLayout();
// 	}
// 
// 	bool EditorView::IsMiniToolBarVisible() const
// 	{
// 		return miniToolbar_->IsVisible();
// 	}

	TabWindow* EditorView::GetLeftFrame()
	{
		return leftFrame_;
	}

	TabWindow* EditorView::GetRightFrame()
	{
		return rightFrame_;
	}

	TabWindow* EditorView::GetMiddleFrame()
	{
		return middleFrame_;
	}

	ToolBarUI* EditorView::GetToolBar()
	{
		return toolbar_;
	}

	MiniToolBarUI* EditorView::GetMiniToolBar()
	{
		return miniToolbar_;
	}

	MenuBarUI* EditorView::GetGetMenuBar()
	{
		return menubar_;
	}
}