/*!
 * \file ToolBarUI.h
 *
 * \author vitali
 * \date Februar 2015
 *
 *
 */

#pragma once

#include "../UI/UIElement.h"
#include "../UI/BorderImage.h"

namespace Urho3D
{

	class BorderImage;
	class XMLFile;
	class Menu;
	class Window;
	class Text;
	class ScrollBar;
	class CheckBox;

	/// \todo use dirty masks
	class ToolBarUI : public BorderImage
	{
		OBJECT(ToolBarUI);
	public:
		ToolBarUI(Context* context);
		virtual ~ToolBarUI();
		static void RegisterObject(Context* context);
		static ToolBarUI* Create(UIElement* context, const String& idname, XMLFile* iconStyle, const String& baseStyle ="ToolBarToggle", int width = 0, int height = 41, XMLFile* defaultstyle = NULL);

		UIElement*	CreateGroup( const String& name, LayoutMode layoutmode);
		CheckBox*	CreateToolBarToggle(const String& groupname, const String& title);
		CheckBox*	CreateToolBarToggle(const String& title);
		UIElement*	CreateToolBarIcon(UIElement* element);
		UIElement*	CreateToolTip(UIElement* parent, const String& title, const IntVector2& offset);
		UIElement*  CreateToolBarSpacer(int width);
		void SetIconStyle(XMLFile* iconStyle) { iconStyle_ = iconStyle; }
		void SetBaseStyle(const String& baseStyle) { baseStyle_ = baseStyle; }
	protected:
		void FinalizeGroupHorizontal(UIElement* group, const String& baseStyle);

		SharedPtr< XMLFile> iconStyle_;
		/// Horizontal scroll bar.
		SharedPtr<ScrollBar> horizontalScrollBar_;
		String baseStyle_;
	private:
	};
}

