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
#include "../UI/Button.h"
#include "../Core/Context.h"
#include "../Graphics/Graphics.h"
#include "../IO/Log.h"
#include "../Resource/ResourceCache.h"
#include "../UI/Text.h"
#include "../UI/UI.h"
#include "../UI/UIEvents.h"
#include "../UI/Window.h"
#include "../Resource/XMLFile.h"
#include "ModalWindow.h"
#include "../UI/UIElement.h"


namespace Urho3D
{



	ModalWindow::ModalWindow(Context* context, UIElement* content /*= NULL*/, const String& titleString /*= String::EMPTY*/, XMLFile* layoutFile /*= 0*/, XMLFile* styleFile /*= 0*/)
		:  Object(context),
		titleText_(0),
		okButton_(0)
	{
		// If layout file is not given, use the default message box layout
		if (!layoutFile)
		{
			ResourceCache* cache = GetSubsystem<ResourceCache>();
			layoutFile = cache->GetResource<XMLFile>("UI/ModalWindow.xml");
			if (!layoutFile)    // Error is already logged
				return;         // Note: windowless MessageBox should not be used!
		}

		UI* ui = GetSubsystem<UI>();
		window_ = ui->LoadLayout(layoutFile, styleFile);
		if (!window_)   // Error is already logged
			return;
		ui->GetRoot()->AddChild(window_);

		content_ = content;

		// Set the title and message strings if they are given
		titleText_ = dynamic_cast<Text*>(window_->GetChild("TitleText", true));
		if (titleText_ && !titleString.Empty())
			titleText_->SetText(titleString);

		contentContainer_ = dynamic_cast<UIElement*>(window_->GetChild("ContentContainer", true));
		if (contentContainer_ && content)
			contentContainer_->AddChild(content);

		// Center window after the message is set
		Window* window = dynamic_cast<Window*>(window_.Get());
		if (window)
		{
			Graphics* graphics = GetSubsystem<Graphics>();  // May be null if headless
			if (graphics)
			{
				const IntVector2& size = window->GetSize();
				window->SetPosition((graphics->GetWidth() - size.x_) / 2, (graphics->GetHeight() - size.y_) / 2);
			}
			else
				LOGWARNING("Instantiating a modal window in headless mode!");

			window->SetModal(true);
			SubscribeToEvent(window, E_MODALCHANGED, HANDLER(ModalWindow, HandleMessageAcknowledged));
		}

		// Bind the buttons (if any in the loaded UI layout) to event handlers
		okButton_ = dynamic_cast<Button*>(window_->GetChild("OkButton", true));
		if (okButton_)
		{
			ui->SetFocusElement(okButton_);
			SubscribeToEvent(okButton_, E_RELEASED, HANDLER(ModalWindow, HandleMessageAcknowledged));
		}

		Button* cancelButton = dynamic_cast<Button*>(window_->GetChild("CancelButton", true));
		if (cancelButton)
			SubscribeToEvent(cancelButton, E_RELEASED, HANDLER(ModalWindow, HandleMessageAcknowledged));

		Button* closeButton = dynamic_cast<Button*>(window_->GetChild("CloseButton", true));
		if (closeButton)
			SubscribeToEvent(closeButton, E_RELEASED, HANDLER(ModalWindow, HandleMessageAcknowledged));
	}


	ModalWindow::~ModalWindow()
	{
		if (window_)
			window_->Remove();
	}


	void ModalWindow::RegisterObject(Context* context)
	{
		context->RegisterFactory<ModalWindow>();
	}


	void ModalWindow::SetTitle(const String& text)
	{
		if (titleText_)
			titleText_->SetText(text);
	}


	void ModalWindow::SetContent(UIElement* content) 
	{
		content_ = SharedPtr<UIElement>(content);
	}

	const String& ModalWindow::GetTitle() const
	{
		return titleText_ ? titleText_->GetText() : String::EMPTY;
	}


	UIElement* ModalWindow::GetContentContainer() const
	{
		return contentContainer_;
	}

	void ModalWindow::HandleMessageAcknowledged(StringHash eventType, VariantMap& eventData)
	{
		using namespace MessageACK;

		VariantMap& newEventData = GetEventDataMap();
		newEventData[P_OK] = eventData[Released::P_ELEMENT] == okButton_;
		SendEvent(E_MESSAGEACK, newEventData);

		this->ReleaseRef();
	}

}

