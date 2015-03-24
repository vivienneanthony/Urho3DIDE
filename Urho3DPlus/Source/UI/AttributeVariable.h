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

/*!
 * \file AttributeVariableUI.h
 *
 * \author vitali
 * \date März 2015
 *
 * 
 */

#pragma once



#include "../UI/UIElement.h"
#include "../Core/Attribute.h"

namespace Urho3D
{

	class BorderImage;
	class ScrollBar;
	class XMLFile;
	class Editor;
	class ProjectManager;
	class ResourceCache;
	class FileSystem;
	class Window;
	class UIElement;
	class EditorSelection;
	class ResourcePickerManager;
	class ListView;
	class Serializable;
	class Text;
	class Button;
	class CheckBox;
	class LineEdit;
	class DropDownList;

	class BasicAttributeUI;



	BasicAttributeUI* CreateAttributeUI(Serializable* serializable,const AttributeInfo& info, unsigned int index, XMLFile* defaultstyle, unsigned int subIndex = 0);

	class BasicAttributeUI : public UIElement
	{
		OBJECT(BasicAttributeUI);

	public:
		/// Construct.
		BasicAttributeUI(Context* context);
		/// Destruct.
		virtual ~BasicAttributeUI();
		/// Register object factory.
		static void RegisterObject(Context* context);

		virtual void	SetVarValue(Variant& var);
		virtual Variant GetVariant();

		void UpdateVar(Serializable* serializable);

		bool IsInUpdated(){ return inUpdated_; }

		void			SetVarName(const String& name);
		const String&	GetVarName();
		Text*			GetVarNameUI();
		
		void			SetIndex(unsigned int i) { index_ = i; }
		unsigned int	GetIndex() const { return index_; }
		
		void			SetSubIndex(unsigned int si){ subIndex_ = si; }
		unsigned int	GetSubIndex() const { return subIndex_; }

	protected:
		/// Attribute Index in the Serializable
		unsigned int index_;
		/// Used for VariantMap/VariantVector/ResourceList Attribute Types
		unsigned int subIndex_;
		bool inUpdated_;
		SharedPtr<Text>	varName_;
	};

	/// see the Create function to see how to use it 
	class BoolAttributeUI : public BasicAttributeUI
	{
		OBJECT(BoolAttributeUI);

	public:
		/// Construct.
		BoolAttributeUI(Context* context);
		/// Destruct.
		virtual ~BoolAttributeUI();
		/// Register object factory.
		static void RegisterObject(Context* context);

		static BoolAttributeUI* Create(Serializable* serializable, const String& name,
			unsigned int index, XMLFile* defaultstyle= NULL, unsigned int subIndex = 0);

		virtual void	SetVarValue(Variant& var);
		virtual Variant GetVariant();

		void SetVarValue(bool b);
		bool GetVarValue();
		/// get the old value, it will be updated on UpdateVar and after the Toggled event 
		bool GetOldValue();

		CheckBox* GetVarValueUI();

	protected:
		void HandleToggled(StringHash eventType, VariantMap& eventData);
		SharedPtr<CheckBox>	varEdit_;
		bool oldValue_;
	};

	/// see the Create function to see how to use it 
	class StringAttributeUI : public BasicAttributeUI
	{
		OBJECT(BoolAttributeUI);

	public:
		/// Construct.
		StringAttributeUI(Context* context);
		/// Destruct.
		virtual ~StringAttributeUI();
		/// Register object factory.
		static void RegisterObject(Context* context);

		static StringAttributeUI* Create(Serializable* serializable, const String& name, 
			unsigned int index, XMLFile* defaultstyle = NULL, unsigned int subIndex = 0);

		virtual void SetVarValue(Variant& var);
		virtual Variant GetVariant();

		void SetVarValue(const String& b);
		const String& GetVarValue();
		/// get the old value, it will be updated on UpdateVar and after the TextChange events
		const String& GetOldValue();

		LineEdit* GetVarValueUI();

	protected:
		void HandleTextChange(StringHash eventType, VariantMap& eventData);
		SharedPtr<LineEdit>	varEdit_;
		String oldValue_;

	};

	/// see the Create function to see how to use it 
	class NumberAttributeUI : public BasicAttributeUI
	{
		OBJECT(NumberAttributeUI);

	public:
		/// Construct.
		NumberAttributeUI(Context* context);
		/// Destruct.
		virtual ~NumberAttributeUI();
		/// Register object factory.
		static void RegisterObject(Context* context);

		static NumberAttributeUI* Create(Serializable* serializable, const String& name, 
			unsigned int index, VariantType type, XMLFile* defaultstyle = NULL, unsigned int subIndex = 0);
	
		virtual void SetVarValue(Variant& var);
		virtual Variant GetVariant();

		void SetType(VariantType type);

		Variant		GetVarValue();
		/// get the old value, it will be updated on UpdateVar and after the TextChange events
		Variant		GetOldValue();

		const Vector<SharedPtr<LineEdit> >& GetVarValueUI();
		int			GetNumCoords() { return numCoords_; }
		VariantType GetType() { return type_; }
	protected:
		void HandleTextChange(StringHash eventType, VariantMap& eventData);
		Vector<SharedPtr<LineEdit> >	varEdit_;
		int numCoords_;
		VariantType type_;
		Variant oldValue_;
	};

	/// see the Create function to see how to use it 
	class EnumAttributeUI : public BasicAttributeUI
	{
		OBJECT(EnumAttributeUI);

	public:
		/// Construct.
		EnumAttributeUI(Context* context);
		/// Destruct.
		virtual ~EnumAttributeUI();
		/// Register object factory.
		static void RegisterObject(Context* context);

		static EnumAttributeUI* Create(Serializable* serializable, const String& name, unsigned int index, const Vector<String>& enums, XMLFile* defaultstyle);

		virtual void	SetVarValue(Variant& var);
		virtual Variant GetVariant();

		int		GetVarValue();
		/// get the old value, it will be updated on UpdateVar and after the TextChange events
		int		GetOldValue();

		DropDownList*	GetVarValueUI();
		void			SetEnumNames(const Vector<String>& enums);
	protected:
		void HandleItemSelected(StringHash eventType, VariantMap& eventData);
		SharedPtr<DropDownList> 	varEdit_;
		Vector<String> enumNames_;
		int oldValue_;
	};

	/// see the Create function to see how to use it 
	class ResourceRefAttributeUI : public BasicAttributeUI
	{
		OBJECT(ResourceRefAttributeUI);

	public:
		/// Construct.
		ResourceRefAttributeUI(Context* context);
		/// Destruct.
		virtual ~ResourceRefAttributeUI();
		/// Register object factory.
		static void RegisterObject(Context* context);

		static ResourceRefAttributeUI* Create(Serializable* serializable, const String& name, VariantType type, StringHash resourceType, unsigned int index, unsigned int subindex, XMLFile* defaultstyle, unsigned int action = 0);

		virtual void	SetVarValue(Variant& var);
		virtual Variant GetVariant();

		LineEdit* GetVarValueUI();

		void			SetActions(unsigned int action);
		unsigned int	GetActions() { return action_; }

		void		SetType(VariantType type);
		VariantType GetType() { return type_; }

		void		SetResourceType(StringHash restype) { resType_ = restype; }
		StringHash	GetResourceType() { return resType_; }

		void SetVarValue(const String& b);
		const String& GetVarValue();
		/// get the old value, it will be updated on UpdateVar and after the TextChange events
		const String& GetOldValue();
	protected:
		void HandleTextChange(StringHash eventType, VariantMap& eventData);
		void HandlePick(StringHash eventType, VariantMap& eventData);
		void HandleOpen(StringHash eventType, VariantMap& eventData); 
		void HandleEdit(StringHash eventType, VariantMap& eventData);
		void HandleTest(StringHash eventType, VariantMap& eventData);

		Button* CreateResourcePickerButton(UIElement* container, const String& text);

		SharedPtr<LineEdit> 	varEdit_;
		SharedPtr<UIElement>	container_;
		Vector<String>			enumNames_;
		VariantType			type_;
		SharedPtr<Button> 	pick_;
		SharedPtr<Button>	open_;
		SharedPtr<Button> 	edit_;
		SharedPtr<Button>	test_;
		unsigned int	action_;
		StringHash		resType_;
		String			oldValue_;
	};

}