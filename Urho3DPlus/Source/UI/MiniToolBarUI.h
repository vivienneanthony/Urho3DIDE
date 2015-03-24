/*!
 * \file MiniToolBarUI.h
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

	/// \todo use dirty masks
	class MiniToolBarUI : public BorderImage
	{
		OBJECT(MiniToolBarUI);
	public:
		MiniToolBarUI(Context* context);
		virtual ~MiniToolBarUI();
		static void RegisterObject(Context* context);
		static MiniToolBarUI* Create(UIElement* context, const String& idname, XMLFile* iconStyle, int width = 28, int height = 0, XMLFile* defaultstyle = NULL);


		void SetIconStyle(XMLFile* iconStyle) { iconStyle_ = iconStyle; }
		UIElement* CreateSmallToolBarButton(const String& title, const String& toolTipTitle = String::EMPTY);
		UIElement* CreateSmallToolBarSpacer(unsigned int width);
	protected:
		void CreateSmallToolBarIcon( UIElement* element);
		UIElement* CreateToolTip(UIElement* parent, const String& title, const IntVector2& offset);
		SharedPtr< XMLFile> iconStyle_;

	private:
	};
}

