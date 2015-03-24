



#include "../Urho3D.h"
#include "../Core/Context.h"
#include "../UI/BorderImage.h"
#include "../UI/UI.h"
#include "../UI/Menu.h"
#include "../Math/Rect.h"
#include "../UI/Text.h"
#include "../UI/Window.h"
#include "../Input/InputEvents.h"
#include "../UI/UIEvents.h"
#include "ToolBarUI.h"
#include "../UI/CheckBox.h"
#include "../UI/ToolTip.h"
#include "../UI/ScrollBar.h"

namespace Urho3D
{


	ToolBarUI* ToolBarUI::Create(UIElement* parent, const String& idname, XMLFile* iconStyle, const String& baseStyle, int width/*=0*/, int height /*= 21*/, XMLFile* defaultstyle /*= NULL*/)
	{
		ToolBarUI* menubar = parent->CreateChild<ToolBarUI>(idname);
		//menubar->SetStyle("Window",styleFile);
		if (defaultstyle)
			menubar->SetDefaultStyle(defaultstyle);
		menubar->SetStyleAuto();
		if (width > 0)
			menubar->SetFixedWidth(width);
		else
			menubar->SetFixedWidth(parent->GetMinWidth());
		menubar->SetFixedHeight(height);
		menubar->iconStyle_ = iconStyle;
		menubar->baseStyle_ = baseStyle;
		return menubar;
	}

	void ToolBarUI::RegisterObject(Context* context)
	{
		context->RegisterFactory<ToolBarUI>();
		COPY_BASE_ATTRIBUTES(BorderImage);
		UPDATE_ATTRIBUTE_DEFAULT_VALUE("Is Enabled", true);

	}

	ToolBarUI::~ToolBarUI()
	{

	}

	ToolBarUI::ToolBarUI(Context* context) : BorderImage(context)
	{
		bringToFront_ = true;
		clipChildren_ = true;
		SetEnabled(true);
		SetLayout(LM_HORIZONTAL, 4, IntRect(8, 4, 4, 8));
		SetAlignment(HA_LEFT, VA_TOP);
// 		horizontalScrollBar_ = CreateChild<ScrollBar>("TB_HorizontalScrollBar");
// 		horizontalScrollBar_->SetInternal(true);
// 		horizontalScrollBar_->SetAlignment(HA_LEFT, VA_BOTTOM);
// 		horizontalScrollBar_->SetOrientation(O_HORIZONTAL);

	}

	UIElement* ToolBarUI::CreateGroup(const String& name, LayoutMode layoutmode)
	{
		UIElement* group = GetChild(name);
		if (group)
			return group;

		group = new UIElement(context_);
		group->SetName(name);
		group->SetDefaultStyle(GetDefaultStyle());
		group->SetLayoutMode(layoutmode);
		group->SetAlignment(HA_LEFT,VA_CENTER);
		AddChild(group);
		return group;
	}

	CheckBox* ToolBarUI::CreateToolBarToggle(const String& groupname, const String& title)
	{
		UIElement* group = GetChild(groupname);
		if (group)
		{
			CheckBox* toggle = new CheckBox(context_);
			toggle->SetName(title);
			toggle->SetDefaultStyle(GetDefaultStyle());
			toggle->SetStyle(baseStyle_);
			toggle->SetOpacity(0.7f);

			CreateToolBarIcon(toggle);
			CreateToolTip(toggle, title, IntVector2(toggle->GetWidth() + 10, toggle->GetHeight() - 10));

			group->AddChild(toggle);
			FinalizeGroupHorizontal(group, baseStyle_);

			return toggle;
		}
		return NULL;
	}

	CheckBox* ToolBarUI::CreateToolBarToggle(const String& title)
	{
		CheckBox* toggle = new CheckBox(context_);
		toggle->SetName(title);
		toggle->SetDefaultStyle(GetDefaultStyle());
		toggle->SetStyle(baseStyle_);
		toggle->SetOpacity(0.7f);

		CreateToolBarIcon(toggle);
		CreateToolTip(toggle, title, IntVector2(toggle->GetWidth() + 10, toggle->GetHeight() - 10));
		AddChild(toggle);

		return toggle;
	}

	UIElement* ToolBarUI::CreateToolBarIcon(UIElement* element)
	{
		BorderImage* icon = new BorderImage(context_);
		icon->SetName("Icon");
		icon->SetDefaultStyle(iconStyle_);
		icon->SetStyle(element->GetName());
		icon->SetFixedSize(GetHeight() - 11, GetHeight() - 11);
		icon->SetAlignment(HA_CENTER, VA_CENTER);
		element->AddChild(icon);
		return icon;
	}

	UIElement* ToolBarUI::CreateToolTip(UIElement* parent, const String& title, const IntVector2& offset)
	{
		ToolTip* toolTip = parent->CreateChild<ToolTip>("ToolTip");
		toolTip->SetPosition(offset);

		BorderImage* textHolder = toolTip->CreateChild<BorderImage>("BorderImage");
		textHolder->SetStyle("ToolTipBorderImage");

		Text* toolTipText = textHolder->CreateChild<Text>("Text");
		toolTipText->SetStyle("ToolTipText");
		toolTipText->SetText(title);

		return toolTip;
	}

	void ToolBarUI::FinalizeGroupHorizontal(UIElement* group, const String& baseStyle)
	{
		int width = 0;
		for (unsigned int i = 0; i < group->GetNumChildren(); ++i)
		{
			UIElement* child = group->GetChild(i);
			width += child->GetMinWidth();
			if (i == 0 && i < group->GetNumChildren() - 1)
				child->SetStyle(baseStyle + "GroupLeft");
			else if (i < group->GetNumChildren() - 1)
				child->SetStyle(baseStyle + "GroupMiddle");
			else
				child->SetStyle(baseStyle + "GroupRight");
			child->SetFixedSize(GetHeight() - 6, GetHeight() - 6);
		}
		/// \todo SetMaxSize(group->GetSize()) does not work !?
		//group->SetMaxSize(group->GetSize());
		group->SetFixedWidth(width);
	}

	UIElement* ToolBarUI::CreateToolBarSpacer(int width)
	{
		UIElement* spacer = new UIElement(context_);
		spacer->SetFixedWidth(width);
		AddChild(spacer);
		return spacer;
	}







}
