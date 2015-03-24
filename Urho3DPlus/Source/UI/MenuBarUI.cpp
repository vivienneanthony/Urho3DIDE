#include "../Urho3D.h"
#include "../Core/Context.h"
#include "MenuBarUI.h"
#include "../UI/BorderImage.h"
#include "../UI/UI.h"
#include "../UI/Menu.h"
#include "../Math/Rect.h"
#include "../UI/Text.h"
#include "../UI/Window.h"
#include "../Input/InputEvents.h"
#include "../UI/UIEvents.h"
#include "UIGlobals.h"




namespace Urho3D
{


	MenuBarUI* MenuBarUI::Create(UIElement* parent, const String& idname, int width/*=0*/, int height /*= 21*/, XMLFile* defaultstyle /*= NULL*/)
	{
		MenuBarUI* menubar = parent->CreateChild<MenuBarUI>(idname);
		//menubar->SetStyle("Window",styleFile);
		if (defaultstyle)
			menubar->SetDefaultStyle(defaultstyle);
		menubar->SetStyleAuto();
		if (width > 0)
			menubar->SetFixedWidth(width);
		else
			menubar->SetFixedWidth(parent->GetWidth());
		menubar->SetFixedHeight(height);

		return menubar;
	}

	void MenuBarUI::RegisterObject(Context* context)
	{
		context->RegisterFactory<MenuBarUI>();
		COPY_BASE_ATTRIBUTES(BorderImage);
		UPDATE_ATTRIBUTE_DEFAULT_VALUE("Is Enabled", true);

	}

	MenuBarUI::~MenuBarUI()
	{

	}

	MenuBarUI::MenuBarUI(Context* context) : BorderImage(context)
	{
		bringToFront_ = true;
		clipChildren_ = true;
		SetEnabled(true);
		SetLayout(LM_HORIZONTAL);
		SetAlignment(HA_LEFT, VA_TOP);
	}

	Menu* MenuBarUI::CreateMenu(const String& title)
	{
		Menu* menu = (Menu*)GetChild(title);
		if (menu)
			return menu;

		menu = new Menu(context_);
		menu->SetName(title);
		menu->SetStyleAuto(GetDefaultStyle());
		menu->SetLayout(LM_HORIZONTAL, 0, IntRect(8, 2, 8, 2));

		// Create Text Label
		Text* menuText =new Text(context_);
		menuText->SetName(title + "_text");
		menu->AddChild(menuText);
		menuText->SetStyle("EditorMenuText");
		menuText->SetText(title);

		// set menubutton size
		menu->SetFixedWidth(menu->GetWidth());

		// create popup
		Window* popup = new  Window(context_);
		popup->SetName(title + "_popup");
		popup->SetLayout(LM_VERTICAL, 1, IntRect(2, 6, 2, 6));
		popup->SetStyleAuto(GetDefaultStyle());
		menu->SetPopup(popup);
		menu->SetPopupOffset(IntVector2(0, menu->GetHeight()));

		AddChild(menu);
		return menu;
	}

	Menu* MenuBarUI::CreateMenuItem(const String& menuTitle, const String& title, const StringHash& action, int accelKey, int accelQual, bool addToQuickMenu, String quickMenuText)
	{
		Menu* menu = (Menu*)GetChild(menuTitle);
		if (!menu)
			return NULL;

		Window*  popup = (Window*)menu->GetPopup();
		if (!popup)
			return NULL;

		// create Menu item
		Menu* menuItem = new Menu(context_);
		menuItem->SetName(title);
		menuItem->SetStyleAuto(GetDefaultStyle());
		menuItem->SetLayout(LM_HORIZONTAL, 0, IntRect(8, 2, 8, 2));
		if (action != StringHash::ZERO)
		{
			menuItem->SetVar(ACTION_VAR, action);
		}
		if (accelKey > 0)
			menuItem->SetAccelerator(accelKey, accelQual);

		// Create Text Label
		Text* menuText = new Text(context_);
		menuText->SetName(title + "_text");
		menuItem->AddChild(menuText);
		menuText->SetStyle("EditorMenuText");
		menuText->SetText(title);

		if (accelKey != 0)
		{
			UIElement* spacer = new UIElement(context_);
			spacer->SetMinWidth(menuText->GetIndentSpacing());
			spacer->SetHeight(menuText->GetHeight());

			menuItem->AddChild(spacer);
			menuItem->AddChild(CreateAccelKeyText(accelKey, accelQual));
		}

		popup->AddChild(menuItem);
		if (action != StringHash::ZERO)
			SubscribeToEvent(menuItem, E_MENUSELECTED, HANDLER(MenuBarUI, HandleMenuSelected));
		/// \todo use dirty masks
		FinalizedPopupMenu(popup);

		return menuItem;
	}

	Window* MenuBarUI::CreatePopupMenu(Menu* menu)
	{
		if (!menu)
		{
			return NULL;
		}
		// create popup
		Window* popup = new  Window(context_);

		popup->SetLayout(LM_VERTICAL, 1, IntRect(2, 6, 2, 6));
		popup->SetDefaultStyle(GetDefaultStyle());
		popup->SetStyleAuto();
		menu->SetPopup(popup);
		menu->SetPopupOffset(IntVector2(0, menu->GetHeight()));
		return popup;
	}

	Menu* MenuBarUI::CreatePopupMenuItem(Window* window, const String& title, const StringHash& action /*= StringHash::ZERO*/, int accelKey /*= 0*/, int accelQual /*= 0*/, bool addToQuickMenu /*= true*/, String quickMenuText /*= ""*/)
	{
		if (!window)
		{
			return NULL;
		}

		// create Menu item
		Menu* menuItem = new Menu(context_);
		menuItem->SetName(title);
		menuItem->SetStyleAuto(GetDefaultStyle());
		menuItem->SetLayout(LM_HORIZONTAL, 0, IntRect(8, 2, 8, 2));
		if (action != StringHash::ZERO)
		{
			menuItem->SetVar(ACTION_VAR, action);
		}
		if (accelKey > 0)
			menuItem->SetAccelerator(accelKey, accelQual);

		// Create Text Label
		Text* menuText = new Text(context_);
		menuText->SetName(title + "_text");
		menuItem->AddChild(menuText);
		menuText->SetStyle("EditorMenuText");
		menuText->SetText(title);

		if (accelKey != 0)
		{
			UIElement* spacer = new UIElement(context_);
			spacer->SetMinWidth(menuText->GetIndentSpacing());
			spacer->SetHeight(menuText->GetHeight());

			menuItem->AddChild(spacer);
			menuItem->AddChild(CreateAccelKeyText(accelKey, accelQual));
		}

		window->AddChild(menuItem);
		if (action != StringHash::ZERO)
			SubscribeToEvent(menuItem, E_MENUSELECTED, HANDLER(MenuBarUI, HandleMenuSelected));
		/// \todo use dirty masks
		FinalizedPopupMenu(window);
		return menuItem;
	}

	void MenuBarUI::FinalizedPopupMenu(Window* popup)
	{
		// Find the maximum menu text width
		Vector<SharedPtr<UIElement> > children = popup->GetChildren();

		int maxWidth = 0;

		for (unsigned int i = 0; i < children.Size(); ++i)
		{
			UIElement* element = children[i];
			if (element->GetType() != MENU_TYPE)    // Skip if not menu item
				continue;

			int width = element->GetChild(0)->GetWidth();
			if (width > maxWidth)
				maxWidth = width;
		}

		// Adjust the indent spacing to slightly wider than the maximum width
		maxWidth += 20;
		for (unsigned int i = 0; i < children.Size(); ++i)
		{
			UIElement* element = children[i];
			if (element->GetType() != MENU_TYPE)
				continue;
			Menu* menu = (Menu*)element;

			Text* menuText = (Text*)menu->GetChild(0);
			if (menuText->GetNumChildren() == 1)    // Skip if menu text does not have accel
				menuText->GetChild(0)->SetIndentSpacing(maxWidth);

			// Adjust the popup offset taking the indentation into effect
			if (menu->GetPopup() != NULL)
				menu->SetPopupOffset(IntVector2(menu->GetWidth(), 0));
		}
	}

	Text* MenuBarUI::CreateAccelKeyText( int accelKey, int accelQual)
	{
		Text* accelKeyText = new Text(context_);
		accelKeyText->SetDefaultStyle(GetDefaultStyle());
		accelKeyText->SetStyle("EditorMenuText");
		accelKeyText->SetTextAlignment(HA_RIGHT);

		String text;
		if (accelKey == KEY_DELETE)
			text = "Del";
		else if (accelKey == KEY_SPACE)
			text = "Space";
		// Cannot use range as the key constants below do not appear to be in sequence
		else if (accelKey == KEY_F1)
			text = "F1";
		else if (accelKey == KEY_F2)
			text = "F2";
		else if (accelKey == KEY_F3)
			text = "F3";
		else if (accelKey == KEY_F4)
			text = "F4";
		else if (accelKey == KEY_F5)
			text = "F5";
		else if (accelKey == KEY_F6)
			text = "F6";
		else if (accelKey == KEY_F7)
			text = "F7";
		else if (accelKey == KEY_F8)
			text = "F8";
		else if (accelKey == KEY_F9)
			text = "F9";
		else if (accelKey == KEY_F10)
			text = "F10";
		else if (accelKey == KEY_F11)
			text = "F11";
		else if (accelKey == KEY_F12)
			text = "F12";
		else if (accelKey == SHOW_POPUP_INDICATOR)
			text = ">";
		else
			text.AppendUTF8(accelKey);
		if ((accelQual & QUAL_ALT) > 0)
			text = "Alt+" + text;
		if ((accelQual & QUAL_SHIFT ) > 0)
			text = "Shift+" + text;
		if ((accelQual & QUAL_CTRL) > 0)
			text = "Ctrl+" + text;
		accelKeyText->SetText(text);

		return accelKeyText;
	}

	void MenuBarUI::HandleMenuSelected(StringHash eventType, VariantMap& eventData)
	{
		using namespace MenuSelected;

		UIElement* element = static_cast<UIElement*>(eventData[P_ELEMENT].GetPtr());
		if (element && element->GetType() == MENU_TYPE)
		{
			const Variant& action = element->GetVar(ACTION_VAR);
			if (action != Variant::EMPTY)
			{
				using namespace MenuBarAction;

				VariantMap& newEventData = GetEventDataMap();
				newEventData[P_ACTION] = action;
				newEventData[P_UINAME] = element->GetName();
				SendEvent(E_MENUBAR_ACTION, newEventData);
			}

		}

	}







}
