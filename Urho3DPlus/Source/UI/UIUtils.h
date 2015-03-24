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
#pragma once

#include "../Urho3D.h"
#include "../Core/Object.h"
#include "../Core/Context.h"
#include "../UI/UIElement.h"
#include "../Math/StringHash.h"
#include "UIGlobals.h"
#include "../Core/Attribute.h"

namespace Urho3D
{
	class FileSystem;
	class Menu;
	class XMLFile;
	class Text;
	class Window;
	class UIElement;
	class CheckBox;
	class Component;
	class Node;
	class Button;
	class ListView;
	class Serializable;
	class LineEdit;

	namespace UIUtils
	{
		//////////////////////////////////////////////////////////////////////////
		///  Editor UI Creation Function
		//////////////////////////////////////////////////////////////////////////
		void		CreateDir(const Object* obj, const String& pathName, String baseDir = String::EMPTY);
		Menu*		CreateMenu(Context* context, XMLFile* uiStyle, const String& title);
		Menu*		CreateMenuItem(Context* context, XMLFile* uiStyle, const String& title, EventHandler* handler = NULL, int accelKey = 0, int accelQual = 0, bool addToQuickMenu = true, String quickMenuText = "");
		Text*		CreateAccelKeyText(Context* context, XMLFile* uiStyle, int accelKey, int accelQual);
		Window*		CreatePopup(Context* context, XMLFile* uiStyle, Menu* baseMenu);
		void		FinalizedPopupMenu(Window* popup);
		void		CreateChildDivider(UIElement* parent);
		UIElement*	CreateGroup(Context* context, XMLFile* uiStyle, const String& title, LayoutMode layoutMode);
		CheckBox*	CreateToolBarToggle(Context* context, XMLFile* uiStyle, XMLFile* iconStyle_, const String& title);
		void		CreateToolBarIcon(Context* context, XMLFile* iconStyle_, UIElement* element);
		UIElement*	CreateToolTip(UIElement* parent, const String& title, const IntVector2& offset);
		void		FinalizeGroupHorizontal(UIElement* group, const String& baseStyle);
		Button*		CreateSmallToolBarButton(Context* context, XMLFile* uiStyle, XMLFile* iconStyle_, const String& title, String toolTipTitle = "");
		void		CreateSmallToolBarIcon(Context* context, XMLFile* iconStyle_, UIElement* element);
		UIElement*	CreateSmallToolBarSpacer(unsigned int width);

		//////////////////////////////////////////////////////////////////////////
		///  Hierarchy Window Function
		//////////////////////////////////////////////////////////////////////////
		String	GetUIElementTitle(UIElement* element);
		String	GetComponentTitle(Component* component);
		String	GetNodeTitle(Node* node);
		Variant		GetUIElementID(UIElement* element);
		unsigned int	GetID(Serializable* serializable, int itemType = ITEM_NONE);
		int		GetType(Serializable* serializable);
		bool	MatchID(UIElement* element, const Variant& id, int itemType);
		void	SetIconEnabledColor(UIElement* element, bool enabled, bool partial = false);

		void	IconizeUIElement(Context* context, XMLFile* iconStyle_, UIElement* element, const String& iconType);
		Vector<Serializable*> ToSerializableArray(Vector<Node*> nodes);

		//////////////////////////////////////////////////////////////////////////
		///  Attribute Window Function
		//////////////////////////////////////////////////////////////////////////
		UIElement*	CreateAttributeEditorParent(ListView* list, const String& name, unsigned int index, unsigned int subIndex);
		UIElement*	CreateAttributeEditorParentAsListChild(ListView* list, const String& name, unsigned int index, unsigned int subIndex);
		LineEdit*	CreateAttributeLineEdit(UIElement* parent, Vector<Serializable*>& serializables, unsigned int index, unsigned int subIndex);
		void		SetAttributeEditorID(UIElement* attrEdit, Vector<Serializable*>& serializables);
		void		SetAttributeEditorID(UIElement* attrEdit, Serializable* serializables);
		UIElement*	CreateBoolAttributeEditor(ListView* list, Vector<Serializable*>& serializables, const AttributeInfo& info, unsigned int index, unsigned int subIndex);
		UIElement*	CreateStringAttributeEditor(ListView* list, Vector<Serializable*>& serializables, const AttributeInfo& info, unsigned int index, unsigned int subIndex);
		UIElement*	CreateNumAttributeEditor(ListView* list, Vector<Serializable*>& serializables, const AttributeInfo& info, unsigned int index, unsigned int subIndex);
		UIElement*	CreateIntAttributeEditor(ListView* list, Vector<Serializable*>& serializables, const AttributeInfo& info, unsigned int index, unsigned int subIndex);

		/// scene Editor, menu bar actions
		String ExtractFileName(VariantMap& eventData, bool forSave = false);

	}
}
