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



#include "../UI/Window.h"

namespace Urho3D
{
	EVENT(E_ACTIVETABCHANGED, ActiveTabChanged)
	{
		PARAM(P_TABINDEX, TabIndex);              // unsigned
	}

	class Window;
	class Button;
	class UIElement;
	/// %TabWindow %UI element.
	class  TabWindow : public Window
	{
		OBJECT(TabWindow);

	public:
		/// Construct.
		TabWindow(Context* context);
		/// Destruct.
		virtual ~TabWindow();
		/// Register object factory.
		static void RegisterObject(Context* context);

		/// return index to tab 
		/// changes the fixed size of content !
		unsigned AddTab(const String& name, UIElement* content);

		void RemoveTab(unsigned index);
		void RemoveTab(const String& name);

		bool SetActiveTab(unsigned index);
		bool SetActiveTab(const String& name);
	protected:

		void HandleButton(StringHash eventType, VariantMap& eventData);

		virtual void OnResize();

		SharedPtr<UIElement> buttonContainer_;
		SharedPtr<UIElement> contentContainer_;
		SharedPtr<UIElement> activeContent_;

		Vector<SharedPtr<UIElement> >	tabsContent_;
		Vector<SharedPtr<Button> >		tabsButtons_;
		HashMap<StringHash, unsigned>	nameTabMapping_;
	};

}

