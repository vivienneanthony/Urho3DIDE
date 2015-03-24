//
// Copyright (c) 2008-2015 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#include "../Urho3D.h"
#include "UIUtils.h"
#include <Urho3D/IO/Log.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/DropDownList.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/ListView.h>
#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/CheckBox.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/Menu.h>


#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Scene/Component.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Graphics/GraphicsEvents.h>

#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/UI/ToolTip.h>
#include <Urho3D/DebugNew.h>
#include "../UI/Button.h"
#include "UIGlobals.h"
#include "../Scene/Serializable.h"

namespace Urho3D
{

	namespace UIUtils
	{

		// UIElement does not have unique ID, so use a running number to generate a new ID each time an item is inserted into hierarchy list
		static unsigned int g_uiElementNextID = UI_ELEMENT_BASE_ID;


		void CreateDir(const Object* obj, const String& pathName, String baseDir)
		{
			FileSystem* filesystem = obj->GetSubsystem<FileSystem>();
			if (baseDir.Empty())
			{
				baseDir = filesystem->GetUserDocumentsDir();
			}

			Vector<String> dirs = pathName.Split('/');
			String subdir = baseDir;
			for (unsigned int i = 0; i < dirs.Size(); ++i)
			{
				subdir += dirs[i] + "/";
				filesystem->CreateDir(subdir);
			}
		}

		Menu* CreateMenu(Context* context, XMLFile* uiStyle, const String& title)
		{
			Menu* menu = CreateMenuItem(context, uiStyle, title);
			menu->SetFixedWidth(menu->GetWidth());
			CreatePopup(context, uiStyle, menu);

			return menu;
		}

		Menu* CreateMenuItem(Context* context, XMLFile* uiStyle, const String& title, EventHandler* handler /*= NULL*/, int accelKey /*= 0*/, int accelQual /*= 0*/, bool addToQuickMenu /*= true*/, String quickMenuText /*= ""*/)
		{
			Menu* menu = new Menu(context);
			menu->SetName(title);
			menu->SetDefaultStyle(uiStyle);
			menu->SetStyle(AUTO_STYLE);
			menu->SetLayout(LM_HORIZONTAL, 0, IntRect(8, 2, 8, 2));

			if (accelKey > 0)
				menu->SetAccelerator(accelKey, accelQual);
			if (handler != NULL)
			{
				// TODO:
				// 			Variant callb(menuCallbacks_.Size());
				// 			menu->SetVar(CALLBACK_VAR, callb);
				// 			menuCallbacks_.Push(callback);
			}

			Text* menuText = new Text(context);
			menu->AddChild(menuText);
			menuText->SetStyle("EditorMenuText");
			menuText->SetText(title);
			//TODO:
			// 		if (addToQuickMenu)
			// 			AddQuickMenuItem(callback, quickMenuText.Empty() ? title : quickMenuText);

			if (accelKey != 0)
			{
				UIElement* spacer = new UIElement(context);
				spacer->SetMinWidth(menuText->GetIndentSpacing());
				spacer->SetHeight(menuText->GetHeight());

				menu->AddChild(spacer);
				menu->AddChild(CreateAccelKeyText(context, uiStyle, accelKey, accelQual));
			}

			return menu;
		}

		Text* CreateAccelKeyText(Context* context, XMLFile* uiStyle, int accelKey, int accelQual)
		{
			Text* accelKeyText = new Text(context);
			accelKeyText->SetDefaultStyle(uiStyle);
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
			if (accelQual && QUAL_ALT > 0)
				text = "Alt+" + text;
			if (accelQual && QUAL_SHIFT > 0)
				text = "Shift+" + text;
			if (accelQual && QUAL_CTRL > 0)
				text = "Ctrl+" + text;
			accelKeyText->SetText(text);

			return accelKeyText;
		}

		Window* CreatePopup(Context* context, XMLFile* uiStyle, Menu* baseMenu)
		{
			Window* popup = new  Window(context);
			popup->SetDefaultStyle(uiStyle);
			popup->SetStyle(AUTO_STYLE);
			popup->SetLayout(LM_VERTICAL, 1, IntRect(2, 6, 2, 6));
			baseMenu->SetPopup(popup);
			baseMenu->SetPopupOffset(IntVector2(0, baseMenu->GetHeight()));

			return popup;
		}

		void FinalizedPopupMenu(Window* popup)
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

		void CreateChildDivider(UIElement* parent)
		{
			BorderImage* divider = parent->CreateChild<BorderImage>("Divider");
			divider->SetStyle("EditorDivider");
		}

		UIElement* CreateGroup(Context* context, XMLFile* uiStyle, const String& title, LayoutMode layoutMode)
		{
			UIElement* group = new UIElement(context);
			group->SetName(title);
			group->SetDefaultStyle(uiStyle);
			group->SetLayoutMode(layoutMode);
			return group;
		}

		CheckBox* CreateToolBarToggle(Context* context, XMLFile* uiStyle, XMLFile* iconStyle_, const String& title)
		{
			CheckBox* toggle = new CheckBox(context);
			toggle->SetName(title);
			toggle->SetDefaultStyle(uiStyle);
			toggle->SetStyle("ToolBarToggle");

			CreateToolBarIcon(context, iconStyle_, toggle);
			CreateToolTip(toggle, title, IntVector2(toggle->GetWidth() + 10, toggle->GetHeight() - 10));

			return toggle;
		}

		void CreateToolBarIcon(Context* context, XMLFile* iconStyle_, UIElement* element)
		{
			BorderImage* icon = new BorderImage(context);
			icon->SetName("Icon");
			icon->SetDefaultStyle(iconStyle_);
			icon->SetStyle(element->GetName());
			icon->SetFixedSize(30, 30);
			element->AddChild(icon);
		}

		UIElement* CreateToolTip(UIElement* parent, const String& title, const IntVector2& offset)
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

		void FinalizeGroupHorizontal(UIElement* group, const String& baseStyle)
		{
			for (unsigned int i = 0; i < group->GetNumChildren(); ++i)
			{
				UIElement* child = group->GetChild(i);

				if (i == 0 && i < group->GetNumChildren() - 1)
					child->SetStyle(baseStyle + "GroupLeft");
				else if (i < group->GetNumChildren() - 1)
					child->SetStyle(baseStyle + "GroupMiddle");
				else
					child->SetStyle(baseStyle + "GroupRight");
			}
			group->SetMaxSize(group->GetSize());
		}

		Button* CreateSmallToolBarButton(Context* context, XMLFile* uiStyle, XMLFile* iconStyle_, const String& title, String toolTipTitle /*= ""*/)
		{
			Button* button = new Button(context);
			button->SetName(title);
			button->SetDefaultStyle(uiStyle);
			button->SetStyle("ToolBarButton");
			button->SetFixedSize(20, 20);
			CreateSmallToolBarIcon(context, iconStyle_, button);

			if (toolTipTitle.Empty())
				toolTipTitle = title;
			CreateToolTip(button, toolTipTitle, IntVector2(button->GetWidth() + 10, button->GetHeight() - 10));

			return button;
		}

		void CreateSmallToolBarIcon(Context* context, XMLFile* iconStyle_, UIElement* element)
		{
			BorderImage* icon = new BorderImage(context);
			icon->SetName("Icon");
			icon->SetDefaultStyle(iconStyle_);
			icon->SetStyle(element->GetName());
			icon->SetFixedSize(14, 14);
			element->AddChild(icon);
		}

		String GetUIElementTitle(UIElement* element)
		{
			String ret;

			// Only top level UI-element has this variable
			String modifiedStr = element->GetVar(MODIFIED_VAR).GetBool() ? "*" : "";
			ret = (element->GetName().Empty() ? element->GetTypeName() : element->GetName()) + modifiedStr + " [" + GetUIElementID(element).ToString() + "]";

			if (element->IsTemporary())
				ret += " (Temp)";

			return ret;
		}

		String GetComponentTitle(Component* component)
		{
			String ret = component->GetTypeName();

			if (component->GetID() >= FIRST_LOCAL_ID)
				ret += " (Local)";

			if (component->IsTemporary())
				ret += " (Temp)";

			return ret;
		}

		String GetNodeTitle(Node* node)
		{
			String ret;

			if (node->GetName().Empty())
				ret = node->GetTypeName();
			else
				ret = node->GetName();

			if (node->GetID() >= FIRST_LOCAL_ID)
				ret += " (Local " + String(node->GetID()) + ")";
			else
				ret += " (" + String(node->GetID()) + ")";

			if (node->IsTemporary())
				ret += " (Temp)";

			return ret;
		}

		Variant GetUIElementID(UIElement* element)
		{
			Variant elementID = element->GetVar(UI_ELEMENT_ID_VAR);
			if (elementID.IsEmpty())
			{
				// Generate new ID
				elementID = g_uiElementNextID++;
				// Store the generated ID
				element->SetVar(UI_ELEMENT_ID_VAR, elementID);
			}

			return elementID;
		}

		unsigned int GetID(Serializable* serializable, int itemType /*= ITEM_NONE*/)
		{
			// If item type is not provided, auto detect it
			if (itemType == ITEM_NONE)
				itemType = GetType(serializable);

			switch (itemType)
			{
			case ITEM_NODE:
				return static_cast<Node*>(serializable)->GetID();

			case ITEM_COMPONENT:
				return static_cast<Component*>(serializable)->GetID();

			case ITEM_UI_ELEMENT:
				return GetUIElementID(static_cast<UIElement*>(serializable)).GetUInt();
			}

			return M_MAX_UNSIGNED;
		}

		int GetType(Serializable* serializable)
		{
			if (dynamic_cast<Node*>(serializable) != NULL)
				return ITEM_NODE;
			else if (dynamic_cast<Component*>(serializable) != NULL)
				return ITEM_COMPONENT;
			else if (dynamic_cast<UIElement*>(serializable) != NULL)
				return ITEM_UI_ELEMENT;
			else
				return ITEM_NONE;
		}

		bool MatchID(UIElement* element, const Variant& id, int itemType)
		{
			return element->GetVar(TYPE_VAR).GetInt() == itemType && element->GetVar(ID_VARS[itemType]) == id;
		}

		void SetIconEnabledColor(UIElement* element, bool enabled, bool partial /*= false*/)
		{
			BorderImage* icon = (BorderImage*)element->GetChild(String("Icon"));
			if (icon != NULL)
			{
				if (partial)
				{
					icon->SetColor(C_TOPLEFT, Color(1, 1, 1, 1));
					icon->SetColor(C_BOTTOMLEFT, Color(1, 1, 1, 1));
					icon->SetColor(C_TOPRIGHT, Color(1, 0, 0, 1));
					icon->SetColor(C_BOTTOMRIGHT, Color(1, 0, 0, 1));
				}
				else
					icon->SetColor(enabled ? Color(1, 1, 1, 1) : Color(1, 0, 0, 1));
			}
		}

		void IconizeUIElement(Context* context, XMLFile* iconStyle_, UIElement* element, const String& iconType)
		{
			// Check if the icon has been created before
			BorderImage* icon = (BorderImage*)element->GetChild(String("Icon"));

			// If iconType is empty, it is a request to remove the existing icon
			if (iconType.Empty())
			{
				// Remove the icon if it exists
				if (icon != NULL)
					icon->Remove();

				// Revert back the indent but only if it is indented by this function
				if (element->GetVar(INDENT_MODIFIED_BY_ICON_VAR).GetBool())
					element->SetIndent(0);

				return;
			}

			// The UI element must itself has been indented to reserve the space for the icon
			if (element->GetIndent() == 0)
			{

				element->SetIndent(1);
				element->SetVar(INDENT_MODIFIED_BY_ICON_VAR, true);
			}

			// If no icon yet then create one with the correct indent and size in respect to the UI element
			if (icon == NULL)
			{
				// The icon is placed at one indent level less than the UI element
				icon = new BorderImage(context);
				icon->SetName("Icon");
				icon->SetIndent(element->GetIndent() - 1);
				icon->SetFixedSize(element->GetIndentWidth() - 2, 14);
				element->InsertChild(0, icon);   // Ensure icon is added as the first child
			}

			// Set the icon type
			if (!icon->SetStyle(iconType, iconStyle_))
				icon->SetStyle("Unknown", iconStyle_);    // If fails then use an 'unknown' icon type
			icon->SetColor(Color(1, 1, 1, 1)); // Reset to enabled color
		}

		Vector<Serializable*> ToSerializableArray(Vector<Node*> nodes)
		{
			Vector<Serializable*> serializables;
			for (unsigned int i = 0; i < nodes.Size(); ++i)
				serializables.Push(nodes[i]);
			return serializables;
		}



		UIElement* CreateAttributeEditorParent(ListView* list, const String& name, unsigned int index, unsigned int subIndex)
		{
			UIElement* editorParent = new UIElement(list->GetContext());
			editorParent->SetName("Edit" + String(index) + "_" + String(subIndex));
			editorParent->SetVar("Index", index);
			editorParent->SetVar("SubIndex", subIndex);
			editorParent->SetLayout(LM_HORIZONTAL);
			editorParent->SetFixedHeight(ATTR_HEIGHT);
			list->AddItem(editorParent);

			Text* attrNameText = new Text(list->GetContext());
			editorParent->AddChild(attrNameText);
			attrNameText->SetStyle("EditorAttributeText");
			attrNameText->SetText(name);
			attrNameText->SetFixedWidth(ATTRNAME_WIDTH);

			return editorParent;
		}

		UIElement* CreateAttributeEditorParentAsListChild(ListView* list, const String& name, unsigned int index, unsigned int subIndex)
		{
			UIElement* editorParent = new UIElement(list->GetContext());
			editorParent->SetName("Edit" + String(index) + "_" + String(subIndex));
			editorParent->SetVar("Index", index);
			editorParent->SetVar("SubIndex", subIndex);
			editorParent->SetLayout(LM_HORIZONTAL);
			list->AddChild(editorParent);

			UIElement* placeHolder = new UIElement(list->GetContext());
			placeHolder->SetName(name);
			editorParent->AddChild(placeHolder);

			return editorParent;
		}

		LineEdit* CreateAttributeLineEdit(UIElement* parent, Vector<Serializable*>& serializables, unsigned int index, unsigned int subIndex)
		{
			LineEdit* attrEdit = new LineEdit(parent->GetContext());
			parent->AddChild((UIElement*)attrEdit);
			attrEdit->SetStyle("EditorAttributeEdit");
			attrEdit->SetFixedHeight(ATTR_HEIGHT - 2);
			attrEdit->SetVar("Index", index);
			attrEdit->SetVar("SubIndex", subIndex);
			/// \todo
		//	SetAttributeEditorID(attrEdit, serializables);

			return attrEdit;
		}

		UIElement* CreateBoolAttributeEditor(ListView* list, Vector<Serializable*>& serializables, const AttributeInfo& info, unsigned int index, unsigned int subIndex)
		{
			bool isUIElement = dynamic_cast<UIElement*>(serializables[0]) != NULL;
			UIElement* parent;
			if (info.name_ == (isUIElement ? "Is Visible" : "Is Enabled"))
				parent = CreateAttributeEditorParentAsListChild(list, info.name_, index, subIndex);
			else
				parent = CreateAttributeEditorParent(list, info.name_, index, subIndex);

			CheckBox* attrEdit = new CheckBox(list->GetContext());
			parent->AddChild(attrEdit);
			attrEdit->SetStyle(AUTO_STYLE);
			attrEdit->SetVar("Index", index);
			attrEdit->SetVar("SubIndex", subIndex);

			/// \todo
// 			SetAttributeEditorID(attrEdit, serializables);
// 			SubscribeToEvent(attrEdit, E_TOGGLED, HANDLER(EditorAttributeInspectorWindow, EditAttribute));

			return parent;
		}

		UIElement* CreateStringAttributeEditor(ListView* list, Vector<Serializable*>& serializables, const AttributeInfo& info, unsigned int index, unsigned int subIndex)
		{
			UIElement* parent = CreateAttributeEditorParent(list, info.name_, index, subIndex);
			LineEdit* attrEdit = CreateAttributeLineEdit(parent, serializables, index, subIndex);
			attrEdit->SetDragDropMode(DD_TARGET);
			/// \todo
			// Do not subscribe to continuous edits of certain attributes (script class names) to prevent unnecessary errors getting printed
// 			if (noTextChangedAttrs.Find(info.name_) == noTextChangedAttrs.End())
// 				SubscribeToEvent(attrEdit, E_TEXTCHANGED, HANDLER(EditorAttributeInspectorWindow, EditAttribute));
// 			SubscribeToEvent(attrEdit, E_TEXTFINISHED, HANDLER(EditorAttributeInspectorWindow, EditAttribute));

			return parent;
		}

		UIElement* CreateNumAttributeEditor(ListView* list, Vector<Serializable*>& serializables, const AttributeInfo& info, unsigned int index, unsigned int subIndex)
		{
			UIElement* parent = CreateAttributeEditorParent(list, info.name_, index, subIndex);
			VariantType type = info.type_;
			unsigned int numCoords = type - VAR_FLOAT + 1;
			if (type == VAR_QUATERNION)
				numCoords = 3;
			else if (type == VAR_COLOR || type == VAR_INTRECT)
				numCoords = 4;
			else if (type == VAR_INTVECTOR2)
				numCoords = 2;

// 			for (unsigned int i = 0; i < numCoords; ++i)
// 			{
// 				LineEdit* attrEdit = CreateAttributeLineEdit(parent, serializables, index, subIndex);
// 				attrEdit->SetVar("Coordinate", i);
// 				SubscribeToEvent(attrEdit, E_TEXTCHANGED, HANDLER(EditorAttributeInspectorWindow, EditAttribute));
// 				SubscribeToEvent(attrEdit, E_TEXTFINISHED, HANDLER(EditorAttributeInspectorWindow, EditAttribute));
// 			}

			return parent;
		}

		UIElement* CreateIntAttributeEditor(ListView* list, Vector<Serializable*>& serializables, const AttributeInfo& info, unsigned int index, unsigned int subIndex)
		{
			UIElement* parent = CreateAttributeEditorParent(list, info.name_, index, subIndex);
			Vector<String> enumnames;

			// get  enums names
			if (info.enumNames_ != NULL)
			{
				const char** enumPtr = info.enumNames_;
				while (*enumPtr)
				{
					enumnames.Push(String(*enumPtr));
					++enumPtr;
				}
			}

			// Check for enums
			if (enumnames.Empty())
			{
				// No enums, create a numeric editor
				LineEdit* attrEdit = CreateAttributeLineEdit(parent, serializables, index, subIndex);
			//	SubscribeToEvent(attrEdit, E_TEXTCHANGED, HANDLER(EditorAttributeInspectorWindow, EditAttribute));
			//	SubscribeToEvent(attrEdit, E_TEXTFINISHED, HANDLER(EditorAttributeInspectorWindow, EditAttribute));
				// If the attribute is a node ID, make it a drag/drop target
				if (info.name_.Contains("NodeID", false) || info.name_.Contains("Node ID", false) || (info.mode_ & AM_NODEID) != 0)
					attrEdit->SetDragDropMode(DD_TARGET);
			}
			else
			{
				DropDownList* attrEdit = new DropDownList(list->GetContext());
				parent->AddChild(attrEdit);
				attrEdit->SetStyle(AUTO_STYLE);
				attrEdit->SetFixedHeight(ATTR_HEIGHT - 2);
				attrEdit->SetResizePopup(true);
				attrEdit->SetPlaceholderText(STRIKED_OUT);
				attrEdit->SetVar("Index", index);
				attrEdit->SetVar("SubIndex", subIndex);
				attrEdit->SetLayout(LM_HORIZONTAL, 0, IntRect(4, 1, 4, 1));
			//	SetAttributeEditorID(attrEdit, serializables);

				for (unsigned i = 0; i < enumnames.Size(); i++)
				{
					Text* choice = new Text(list->GetContext());
					attrEdit->AddItem(choice);
					choice->SetStyle("EditorEnumAttributeText");
					choice->SetText(enumnames[i]);

				}

				// 				for (unsigned int i = 0; i < info.enumNames.Size(); ++i)
				// 				{
				// 					Text* choice = Text();
				// 					attrEdit.AddItem(choice);
				// 					choice.style = "EditorEnumAttributeText";
				// 					choice.text = info.enumNames[i];
				// 				}
				//SubscribeToEvent(attrEdit, E_ITEMSELECTED, HANDLER(EditorAttributeInspectorWindow, EditAttribute));
			}

			return parent;
		}

		Urho3D::String ExtractFileName(VariantMap& eventData, bool forSave /*= false*/)
		{
			using namespace FileSelected;
			String fileName;

			// Check for OK
			if (eventData[P_OK].GetBool())
			{
				String filter = eventData[P_FILTER].GetString();
				fileName = eventData[P_FILENAME].GetString();
				// Add default extension for saving if not specified
				if (GetExtension(fileName).Empty() && forSave && filter != "*.*")
					fileName = fileName + filter.Substring(1);
			}
			return fileName;
		}

		void SetAttributeEditorID(UIElement* attrEdit, Vector<Serializable*>& serializables)
		{
			if (serializables.Size() == 0)
				return;

			// All target serializables must be either nodes, ui-elements, or components
			Vector<Variant> ids;
			switch (UIUtils::GetType(serializables[0]))
			{
			case ITEM_NODE:
				for (unsigned int i = 0; i < serializables.Size(); ++i)
					ids.Push(dynamic_cast<Node*>(serializables[i])->GetID());
				attrEdit->SetVar(NODE_IDS_VAR, ids);
				break;

			case ITEM_COMPONENT:
				for (unsigned int i = 0; i < serializables.Size(); ++i)
					ids.Push(dynamic_cast<Component*>(serializables[i])->GetID());
				attrEdit->SetVar(COMPONENT_IDS_VAR, ids);
				break;

			case ITEM_UI_ELEMENT:
				for (unsigned int i = 0; i < serializables.Size(); ++i)
					ids.Push(UIUtils::GetUIElementID(dynamic_cast<UIElement*>(serializables[i])));
				attrEdit->SetVar(UI_ELEMENT_IDS_VAR, ids);
				break;

			default:
				break;
			}
		}

		void SetAttributeEditorID(UIElement* attrEdit, Serializable* serializables)
		{
			if (serializables == NULL)
				return;

			// All target serializables must be either nodes, ui-elements, or components
			Vector<Variant> ids;
			switch (UIUtils::GetType(serializables))
			{
			case ITEM_NODE:

				ids.Push(dynamic_cast<Node*>(serializables)->GetID());
				attrEdit->SetVar(NODE_IDS_VAR, ids);
				break;

			case ITEM_COMPONENT:

				ids.Push(dynamic_cast<Component*>(serializables)->GetID());
				attrEdit->SetVar(COMPONENT_IDS_VAR, ids);
				break;

			case ITEM_UI_ELEMENT:

				ids.Push(UIUtils::GetUIElementID(dynamic_cast<UIElement*>(serializables)));
				attrEdit->SetVar(UI_ELEMENT_IDS_VAR, ids);
				break;

			default:
				break;
			}
		}



	}

}

