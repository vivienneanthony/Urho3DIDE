

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
#include "../Core/Context.h"
#include "TabWindow.h"
#include "../UI/Cursor.h"
#include "../Input/InputEvents.h"
#include "../UI/UI.h"
#include "../UI/UIEvents.h"
#include "../UI/Window.h"
#include "../UI/Button.h"
#include "../UI/Text.h"
#include "../UI/BorderImage.h"


namespace Urho3D
{
	const StringHash TAB_INDEX("Tab_Index");
	extern const char* UI_CATEGORY;

	TabWindow::TabWindow(Context* context) : Window(context)
	{
		SetEnabled(true);
		bringToFront_ = true;
		clipChildren_ = true;

		SetLayout(LM_VERTICAL);

		buttonContainer_ = CreateChild<UIElement>("TW_ButtonContainer");
		buttonContainer_->SetInternal(true);
		buttonContainer_->SetLayout(LM_HORIZONTAL);
		buttonContainer_->SetFixedHeight(25);

// 		BorderImage* img = CreateChild<BorderImage>();
// 		img->SetInternal(true);
// 		img->SetFixedHeight(11);

		contentContainer_ = CreateChild<UIElement>("TW_ContentContainer");
		contentContainer_->SetInternal(true);
	}

	TabWindow::~TabWindow()
	{
	
	}

	void TabWindow::RegisterObject(Context* context)
	{
		context->RegisterFactory<TabWindow>(UI_CATEGORY);
		COPY_BASE_ATTRIBUTES(Window);

	}

	unsigned TabWindow::AddTab(const String& name, UIElement* content)
	{
		StringHash id(name);
		HashMap<StringHash, unsigned>::Iterator it = nameTabMapping_.Find(id);
		if (it == nameTabMapping_.End())
		{
			
			/// create new tab
			unsigned index = tabsContent_.Size();
			nameTabMapping_[id] = index;

			tabsContent_.Push(SharedPtr<UIElement>(content));
			content->SetFixedHeight(GetHeight() - buttonContainer_->GetHeight());
			int layoutwidth = GetLayoutBorder().left_ + GetLayoutBorder().right_;
			content->SetFixedWidth(GetWidth() - layoutwidth);

			SharedPtr<Button> tabbutton( buttonContainer_->CreateChild<Button>(name));
			tabbutton->SetStyle("TabButton");
			tabbutton->SetVar(TAB_INDEX,index);

			SubscribeToEvent(tabbutton.Get(), E_RELEASED, HANDLER(TabWindow, HandleButton));
		
			Text* tabButtonLabel = tabbutton->CreateChild<Text>(name+"_Text");
			tabButtonLabel->SetText(name);
			tabButtonLabel->SetStyle("Text");
			tabButtonLabel->SetAlignment(HA_CENTER, VA_CENTER);
			tabsButtons_.Push(tabbutton);
			tabbutton->SetFixedWidth(tabButtonLabel->GetRowWidth(0) + tabbutton->GetLayoutBorder().left_ + tabbutton->GetLayoutBorder().right_);

			if (activeContent_.Null())
				SetActiveTab(index);
			

			return index;
		}
		else
		{
			if (activeContent_.Null())
				SetActiveTab(it->second_);
			
			/// return found tab
			return it->second_;
		}
	}

	void TabWindow::RemoveTab(const String& name)
	{
		StringHash id(name);
		HashMap<StringHash, unsigned>::Iterator it = nameTabMapping_.Find(id);

		if (it != nameTabMapping_.End())
		{
			RemoveTab(it->second_);
		}
	
	}

	void TabWindow::RemoveTab(unsigned index)
	{
		if (index < tabsContent_.Size())
		{
			UIElement* temp = tabsContent_[index];
			Button* tempB = tabsButtons_[index];
			tempB->Remove();
			tabsButtons_.Erase(index);

			if (temp == activeContent_.Get())
			{			
				activeContent_->Remove();
				activeContent_ = NULL;
			}
			tabsContent_.Erase(index);

			if (tabsContent_.Size() > 0)
			{
				SetActiveTab(0);
			}
			HashMap<StringHash, unsigned>::Iterator it;
			for (it = nameTabMapping_.Begin(); it != nameTabMapping_.End();it++)
			{
				if (it->second_ == index)
				{
					nameTabMapping_.Erase(it);
					break;
				}
			}
		}
	}

	bool TabWindow::SetActiveTab(const String& name)
	{
		StringHash id(name);
		HashMap<StringHash, unsigned>::Iterator it = nameTabMapping_.Find(id);

		if (it != nameTabMapping_.End())
		{
			
			return SetActiveTab(it->second_);
		}
		return false;
	}

	bool TabWindow::SetActiveTab(unsigned index)
	{
		if (index < tabsContent_.Size() )
		{
			UIElement* temp = tabsContent_[index];
			if (temp != activeContent_.Get())
			{
				if (activeContent_.NotNull())
					activeContent_->Remove();
				
				activeContent_ = temp;
				contentContainer_->AddChild(temp);

				activeContent_->SetFixedHeight(GetHeight() - buttonContainer_->GetHeight());
				int layoutwidth = GetLayoutBorder().left_ + GetLayoutBorder().right_;
				activeContent_->SetFixedWidth(GetWidth() - layoutwidth);

				using namespace ActiveTabChanged;
				VariantMap& eventData_ = GetEventDataMap();
				eventData_[P_TABINDEX] = index;
				SendEvent(E_ACTIVETABCHANGED, eventData_);

			}
			return true;
		}
		return false;
	}

	void TabWindow::HandleButton(StringHash eventType, VariantMap& eventData)
	{
		using namespace Released;

		UIElement* element = (UIElement*)eventData[P_ELEMENT].GetPtr();
		if (element)
		{
			const Variant& indexVar = element->GetVar(TAB_INDEX);
			if (indexVar != Variant::EMPTY)
			{
				SetActiveTab(indexVar.GetUInt());
			}
		}
	}

	void TabWindow::OnResize()
	{
		if (activeContent_.NotNull())
		{
			int layoutwidth = GetLayoutBorder().left_ + GetLayoutBorder().right_;
			activeContent_->SetFixedWidth(GetWidth() - layoutwidth);
		}
	}

}