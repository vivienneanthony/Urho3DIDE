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

#include "../UI/UIElement.h"
#include "../Core/Attribute.h"
#include "../Core/Object.h"

namespace Urho3D
{
	class XMLFile;
	class ResourceCache;
	class FileSystem;
	class Resource;

	class BorderImage;
	class ScrollBar;
	class Window;
	class UIElement;
	class ListView;
	class Serializable;
	class Text;
	class Button;
	class CheckBox;
	class LineEdit;
	class DropDownList;

	class BasicAttributeUI;
	class EditorResourcePicker;
	class ResourcePickerManager;

	class AttributeContainer : public UIElement
	{
		OBJECT(AttributeContainer);

	public:
		/// Construct.
		AttributeContainer(Context* context);
		/// Destruct.
		virtual ~AttributeContainer();
		/// Register object factory.
		static void RegisterObject(Context* context);
		/// React to resize.
		virtual void OnResize();

		void SetTitle(const String& title);
		void SetIcon(XMLFile* iconStyle_, const String& iconType);

		void SetNoTextChangedAttrs(const Vector<String>& noTextChangedAttrs);

		void SetSerializableAttributes(Serializable* serializable, bool createNew = false);

		ListView*	GetAttributeList();

		void UpdateVariantMap(Serializable* serializable);
		void UpdateVariantMap(Serializable* serializable, unsigned int index);

		Serializable*	GetSerializable();
		Button*			GetResetToDefault() { return resetToDefault_; }
	protected:

		void CreateSerializableAttributes(Serializable* serializable);
		void UpdateSerializableAttributes(Serializable* serializable);

		UIElement*	CreateAttribute(Serializable* serializable, const AttributeInfo& info, unsigned int index, unsigned int subIndex, bool suppressedSeparatedLabel = false);
		void		UpdateAttribute(Serializable* serializable, const AttributeInfo& info, unsigned int index, unsigned int subIndex, bool suppressedSeparatedLabel = false);

		String	GetVariableName(Serializable* serializable, StringHash hash);

		void EditEnabledAttribute(StringHash eventType, VariantMap& eventData);
		void EditStringAttribute(StringHash eventType, VariantMap& eventData);
		void EditBoolAttribute(StringHash eventType, VariantMap& eventData);
		void EditEnumAttribute(StringHash eventType, VariantMap& eventData);
		void EditNumberAttribute(StringHash eventType, VariantMap& eventData);
		void EditResRefAttribute(StringHash eventType, VariantMap& eventData);

		/// UI Attributes
		SharedPtr<Text>			titleText_;
		SharedPtr<UIElement>	iconsPanel_;
		SharedPtr<UIElement>	spacer_;
		SharedPtr<Button>		resetToDefault_;
		SharedPtr<ListView>		attributeList_;
		Vector< Vector< BasicAttributeUI* > >	attributes_;

		/// other Attributes
		StringHash				serializableType_;
		Serializable*			serializable_;
		ResourcePickerManager*   editorResourcePicker_;

		/// Exceptions for string attributes that should not be continuously edited
		Vector<String>		noTextChangedAttrs_;


		/// Serialized Attributes
		bool	showNonEditableAttribute_;
		int		attrNameWidth_;
		int		attrHeight_;
	};
}