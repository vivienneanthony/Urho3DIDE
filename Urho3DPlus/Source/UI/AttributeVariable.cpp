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
#include "../UI/BorderImage.h"
#include "../Input/InputEvents.h"
#include "../UI/ScrollBar.h"
#include "../UI/ScrollView.h"
#include "../UI/Slider.h"
#include "../UI/UI.h"
#include "../UI/UIEvents.h"
#include "../UI/Text.h"
#include "../UI/UIEvents.h"
#include "../UI/ListView.h"
#include "../UI/Button.h"
#include "../UI/LineEdit.h"
#include "../UI/CheckBox.h"
#include "../UI/DropDownList.h"
#include "../Scene/Node.h"
#include "../Scene/Component.h"
#include "../UI/DropDownList.h"
#include "../Resource/XMLFile.h"
#include "../Core/StringUtils.h"

#include "AttributeVariable.h"
#include "AttributeVariableEvents.h"

#include "UIGlobals.h"
#include "../DebugNew.h"


// Resource picker functionality
const unsigned int  ACTION_PICK = 1;
const unsigned int  ACTION_OPEN = 2;
const unsigned int  ACTION_EDIT = 4;
const unsigned int  ACTION_TEST = 8;

namespace Urho3D
{


	BasicAttributeUI* CreateAttributeUI(Serializable* serializable,const AttributeInfo& info, unsigned int index, XMLFile* defaultstyle, unsigned int subIndex)
	{
		if (!serializable)
			return NULL;

		if (index >= serializable->GetNumAttributes())
			return NULL;
		BasicAttributeUI* attrui = NULL;

		VariantType type = info.type_;
		if (type == VAR_STRING || type == VAR_BUFFER)
		{
			StringAttributeUI* attr = StringAttributeUI::Create(serializable, info.name_, index, defaultstyle, subIndex);
			attrui = attr;
		}
		else if (type == VAR_BOOL)
		{
			BoolAttributeUI* attr = BoolAttributeUI::Create(serializable, info.name_, index, defaultstyle, subIndex);
			attrui = attr;
		}
		else if ((type >= VAR_FLOAT && type <= VAR_VECTOR4) || type == VAR_QUATERNION || type == VAR_COLOR || type == VAR_INTVECTOR2 || type == VAR_INTRECT)
		{
			NumberAttributeUI* attr = NumberAttributeUI::Create(serializable, info.name_, index, type, defaultstyle, subIndex);

			attrui = attr;
		}
		else if (type == VAR_INT)
		{
			// get  enums names
			if (info.enumNames_ != NULL)
			{
				Vector<String> enumnames;
				const char** enumPtr = info.enumNames_;
				while (*enumPtr)
				{
					enumnames.Push(String(*enumPtr));
					++enumPtr;
				}

				EnumAttributeUI* attr = EnumAttributeUI::Create(serializable, info.name_, index, enumnames, defaultstyle);
				attrui = attr;
			}
			else
			{
				NumberAttributeUI* attr = NumberAttributeUI::Create(serializable, info.name_, index, type, defaultstyle, subIndex);
				attrui = attr;
			}
		}
		else if (type == VAR_RESOURCEREF)
		{
			StringHash resourceType;

			// Get the real attribute info from the serializable for the correct resource type
			AttributeInfo attrInfo = serializable->GetAttributes()->At(index);
			if (attrInfo.type_ == VAR_RESOURCEREF)
				resourceType = serializable->GetAttribute(index).GetResourceRef().type_;
			else if (attrInfo.type_ == VAR_RESOURCEREFLIST)
				resourceType = serializable->GetAttribute(index).GetResourceRefList().type_;
			else if (attrInfo.type_ == VAR_VARIANTVECTOR)
				resourceType = serializable->GetAttribute(index).GetVariantVector()[subIndex].GetResourceRef().type_;

			ResourceRefAttributeUI* attr = ResourceRefAttributeUI::Create(serializable, info.name_, attrInfo.type_,
				resourceType, index, subIndex, defaultstyle, 0);
			attrui = attr;
		}
		else if (type == VAR_RESOURCEREFLIST)
		{
		}
		else if (type == VAR_VARIANTVECTOR)
		{
		}
		else if (type == VAR_VARIANTMAP)
		{
		}

		return attrui;
	}

	//////////////////////////////////////////////////////////////////////////
	/// BasicAttributeUI
	//////////////////////////////////////////////////////////////////////////
	void BasicAttributeUI::RegisterObject(Context* context)
	{
		context->RegisterFactory<BoolAttributeUI>();
		COPY_BASE_ATTRIBUTES(UIElement);
	}

	BasicAttributeUI::BasicAttributeUI(Context* context) : UIElement(context)
	{
		inUpdated_ = false;
		index_ = 0;
		subIndex_ = 0;
		SetEnabled(true);

		SetLayout(LM_HORIZONTAL);
		SetFixedHeight(19);

		varName_ = CreateChild<Text>("A_VarName");
		varName_->SetInternal(true);
		varName_->SetStyle("EditorAttributeText");
		varName_->SetText("Variable");
		varName_->SetFixedWidth(150);
	}

	BasicAttributeUI::~BasicAttributeUI()
	{
	}

	void BasicAttributeUI::UpdateVar(Serializable* serializable)
	{
		if (!serializable)
			return;
		inUpdated_ = true;
		Variant var = serializable->GetAttribute(index_);
		if (var.GetType() == VAR_VARIANTMAP)
		{
			VariantMap map = var.GetVariantMap();
			Vector<StringHash> keys = map.Keys();

			var = map[keys[subIndex_]];
		}

		SetVarValue(var);
		inUpdated_ = false;
	}

	void BasicAttributeUI::SetVarName(const String& name)
	{
		varName_->SetText(name);
	}

	const String& BasicAttributeUI::GetVarName()
	{
		return varName_->GetText();
	}

	void BasicAttributeUI::SetVarValue(Variant& var)
	{
	}

	Urho3D::Variant BasicAttributeUI::GetVariant()
	{
		return Variant::EMPTY;
	}

	Text* BasicAttributeUI::GetVarNameUI()
	{
		return varName_;
	}

	//////////////////////////////////////////////////////////////////////////
	/// BoolAttributeUI
	//////////////////////////////////////////////////////////////////////////
	void BoolAttributeUI::RegisterObject(Context* context)
	{
		context->RegisterFactory<BoolAttributeUI>();
		COPY_BASE_ATTRIBUTES(BasicAttributeUI);
	}

	BoolAttributeUI::~BoolAttributeUI()
	{
	}

	BoolAttributeUI::BoolAttributeUI(Context* context) : BasicAttributeUI(context)
	{
		varEdit_ = CreateChild<CheckBox>("A_VarValue");
		varEdit_->SetInternal(true);
		SubscribeToEvent(varEdit_, E_TOGGLED, HANDLER(BoolAttributeUI, HandleToggled));
		oldValue_ = false;
	}

	void BoolAttributeUI::SetVarValue(bool b)
	{
		varEdit_->SetChecked(b);
	}

	void BoolAttributeUI::SetVarValue(Variant& var)
	{
		if (var.GetType() == VAR_BOOL)
		{
			varEdit_->SetChecked(var.GetBool());
			oldValue_ = var.GetBool();
		}
	}

	bool BoolAttributeUI::GetVarValue()
	{
		return varEdit_->IsChecked();
	}

	BoolAttributeUI* BoolAttributeUI::Create(Serializable* serializable, const String& name, unsigned int index, XMLFile* defaultstyle , unsigned int subIndex)
	{
		if (!serializable)
			return NULL;
		BoolAttributeUI* boolattr = new BoolAttributeUI(serializable->GetContext());
		boolattr->SetIndex(index);
		boolattr->SetSubIndex(subIndex);
		boolattr->SetVarName(name);
		if (defaultstyle)
		{
			boolattr->SetDefaultStyle(defaultstyle);
			boolattr->SetStyle("BoolAttributeUI");
		}
		boolattr->UpdateVar(serializable);
		return boolattr;
	}

	CheckBox* BoolAttributeUI::GetVarValueUI()
	{
		return varEdit_;
	}

	void BoolAttributeUI::HandleToggled(StringHash eventType, VariantMap& eventData)
	{
		if (inUpdated_)
			return;
		using namespace BoolVarChanged;
		VariantMap& eventData_ = GetEventDataMap();
		eventData_[P_ATTEDIT] = this;
		SendEvent(AEE_BOOLVARCHANGED, eventData_);

		oldValue_ = varEdit_->IsChecked();
	}

	bool BoolAttributeUI::GetOldValue()
	{
		return oldValue_;
	}

	Urho3D::Variant BoolAttributeUI::GetVariant()
	{
		return Variant(varEdit_->IsChecked());
	}

	//////////////////////////////////////////////////////////////////////////
	/// StringAttributeUI
	//////////////////////////////////////////////////////////////////////////
	StringAttributeUI::StringAttributeUI(Context* context) : BasicAttributeUI(context)
	{
		varEdit_ = CreateChild<LineEdit>("A_VarValue");
		varEdit_->SetInternal(true);
		varEdit_->SetFixedHeight(17);
		varEdit_->SetDragDropMode(DD_TARGET);

		// Do not subscribe to continuous edits of certain attributes (script class names) to prevent unnecessary errors getting printed

		SubscribeToEvent(varEdit_, E_TEXTCHANGED, HANDLER(StringAttributeUI, HandleTextChange));
		SubscribeToEvent(varEdit_, E_TEXTFINISHED, HANDLER(StringAttributeUI, HandleTextChange));
	}

	StringAttributeUI::~StringAttributeUI()
	{
	}

	void StringAttributeUI::RegisterObject(Context* context)
	{
		context->RegisterFactory<StringAttributeUI>();
		COPY_BASE_ATTRIBUTES(BasicAttributeUI);
	}

	StringAttributeUI* StringAttributeUI::Create(Serializable* serializable, const String& name, unsigned int index, XMLFile* defaultstyle, unsigned int subIndex)
	{
		if (!serializable)
			return NULL;
		StringAttributeUI* attr = new StringAttributeUI(serializable->GetContext());
		attr->SetIndex(index);
		attr->SetSubIndex(subIndex);
		attr->SetVarName(name);
		if (defaultstyle)
		{
			attr->SetDefaultStyle(defaultstyle);
			attr->SetStyle("StringAttributeUI");
		}
		attr->UpdateVar(serializable);
		return attr;
	}

	void StringAttributeUI::SetVarValue(Variant& var)
	{
		if (var.GetType() == VAR_STRING)
		{
			varEdit_->SetText(var.GetString());
			oldValue_ = var.GetString();
			varEdit_->SetCursorPosition(0);
		}
		else if (var.GetType() == VAR_BUFFER)
		{
			///  \todo
			varEdit_->SetText("VAR_BUFFER");
			varEdit_->SetCursorPosition(0);
		}
	}

	void StringAttributeUI::SetVarValue(const String& b)
	{
		varEdit_->SetText(b);
	}

	const String& StringAttributeUI::GetVarValue()
	{
		return varEdit_->GetText();
	}

	LineEdit* StringAttributeUI::GetVarValueUI()
	{
		return varEdit_;
	}

	void StringAttributeUI::HandleTextChange(StringHash eventType, VariantMap& eventData)
	{
		if (inUpdated_)
			return;
		using namespace StringVarChanged;
		VariantMap& eventData_ = GetEventDataMap();
		eventData_[P_ATTEDIT] = this;
		SendEvent(AEE_STRINGVARCHANGED, eventData_);
		oldValue_ = varEdit_->GetText();
	}

	const String& StringAttributeUI::GetOldValue()
	{
		return oldValue_;
	}

	Urho3D::Variant StringAttributeUI::GetVariant()
	{
		return Variant(varEdit_->GetText());
	}

	//////////////////////////////////////////////////////////////////////////
	/// NumberAttributeUI
	//////////////////////////////////////////////////////////////////////////
	NumberAttributeUI::NumberAttributeUI(Context* context) : BasicAttributeUI(context)
	{
		type_ = VAR_NONE;
	}

	NumberAttributeUI::~NumberAttributeUI()
	{
	}

	void NumberAttributeUI::RegisterObject(Context* context)
	{
		context->RegisterFactory<NumberAttributeUI>();
		COPY_BASE_ATTRIBUTES(BasicAttributeUI);
	}

	NumberAttributeUI* NumberAttributeUI::Create(Serializable* serializable, const String& name, unsigned int index, VariantType type, XMLFile* defaultstyle, unsigned int subIndex)
	{
		if (!serializable)
			return NULL;

		NumberAttributeUI* attr = new NumberAttributeUI(serializable->GetContext());
		attr->SetIndex(index);
		attr->SetSubIndex(subIndex);
		attr->SetVarName(name);
		attr->SetType(type);
		if (defaultstyle)
		{
			attr->SetDefaultStyle(defaultstyle);
			attr->SetStyle("BasicAttributeUI");
			for (int i = 0; i < attr->GetNumCoords(); ++i)
			{
				attr->GetVarValueUI()[i]->SetStyle("EditorAttributeEdit");
				attr->GetVarValueUI()[i]->SetMinWidth(32);
			}
		}

		attr->UpdateVar(serializable);
		return attr;
	}

	void NumberAttributeUI::SetVarValue(Variant& var)
	{
		if (type_ != var.GetType())
			return;

		oldValue_ = var;

		if (type_ == VAR_INT)
		{
			varEdit_[0]->SetText(String(var.GetInt()));
			varEdit_[0]->SetCursorPosition(0);
		}
		else if (type_ == VAR_FLOAT)
		{
			varEdit_[0]->SetText(String(var.GetFloat()));
			varEdit_[0]->SetCursorPosition(0);
		}
		else if (type_ == VAR_VECTOR2)
		{
			const Vector2& v = var.GetVector2();

			varEdit_[0]->SetText(String(v.x_));
			varEdit_[1]->SetText(String(v.y_));
			varEdit_[0]->SetCursorPosition(0);
			varEdit_[1]->SetCursorPosition(0);
		}
		else if (type_ == VAR_VECTOR3)
		{
			const Vector3& v = var.GetVector3();

			varEdit_[0]->SetText(String(v.x_));
			varEdit_[1]->SetText(String(v.y_));
			varEdit_[2]->SetText(String(v.z_));
			varEdit_[0]->SetCursorPosition(0);
			varEdit_[1]->SetCursorPosition(0);
			varEdit_[2]->SetCursorPosition(0);
		}
		else if (type_ == VAR_VECTOR4)
		{
			const Vector4& v = var.GetVector4();

			varEdit_[0]->SetText(String(v.x_));
			varEdit_[1]->SetText(String(v.y_));
			varEdit_[2]->SetText(String(v.z_));
			varEdit_[3]->SetText(String(v.w_));
			varEdit_[0]->SetCursorPosition(0);
			varEdit_[1]->SetCursorPosition(0);
			varEdit_[2]->SetCursorPosition(0);
			varEdit_[3]->SetCursorPosition(0);
		}
		else if (type_ == VAR_QUATERNION)
		{
			const Quaternion& q = var.GetQuaternion();
			Vector3 euler = q.EulerAngles();
			varEdit_[0]->SetText(String(euler.x_));
			varEdit_[1]->SetText(String(euler.y_));
			varEdit_[2]->SetText(String(euler.z_));
			varEdit_[0]->SetCursorPosition(0);
			varEdit_[1]->SetCursorPosition(0);
			varEdit_[2]->SetCursorPosition(0);
		}
		else if (type_ == VAR_COLOR)
		{
			const Color& v = var.GetColor();

			varEdit_[0]->SetText(String(v.r_));
			varEdit_[1]->SetText(String(v.g_));
			varEdit_[2]->SetText(String(v.b_));
			varEdit_[3]->SetText(String(v.a_));
			varEdit_[0]->SetCursorPosition(0);
			varEdit_[1]->SetCursorPosition(0);
			varEdit_[2]->SetCursorPosition(0);
			varEdit_[3]->SetCursorPosition(0);
		}
		else if (type_ == VAR_INTVECTOR2)
		{
			const IntVector2& v = var.GetIntVector2();

			varEdit_[0]->SetText(String(v.x_));
			varEdit_[1]->SetText(String(v.y_));
			varEdit_[0]->SetCursorPosition(0);
			varEdit_[1]->SetCursorPosition(0);
		}
		else if (type_ == VAR_INTRECT)
		{
			const IntRect& v = var.GetIntRect();

			varEdit_[0]->SetText(String(v.left_));
			varEdit_[1]->SetText(String(v.top_));
			varEdit_[2]->SetText(String(v.right_));
			varEdit_[3]->SetText(String(v.bottom_));
			varEdit_[0]->SetCursorPosition(0);
			varEdit_[1]->SetCursorPosition(0);
			varEdit_[2]->SetCursorPosition(0);
			varEdit_[3]->SetCursorPosition(0);
		}
	}

	Urho3D::Variant NumberAttributeUI::GetVarValue()
	{
		if (type_ == VAR_INT)
		{
			return Variant(ToInt(varEdit_[0]->GetText()));
		}
		else if (type_ == VAR_FLOAT)
		{
			return Variant(ToFloat(varEdit_[0]->GetText()));
		}
		else if (type_ == VAR_VECTOR2)
		{
			return Variant(ToVector2(varEdit_[0]->GetText() + " " + varEdit_[1]->GetText()));
		}
		else if (type_ == VAR_VECTOR3)
		{
			return Variant(ToVector3(varEdit_[0]->GetText() + " " + varEdit_[1]->GetText() + " " + varEdit_[2]->GetText()));
		}
		else if (type_ == VAR_VECTOR4)
		{
			return Variant(ToVector4(varEdit_[0]->GetText() + " " + varEdit_[1]->GetText() + " " + varEdit_[2]->GetText() + " " + varEdit_[3]->GetText()));
		}
		else if (type_ == VAR_QUATERNION)
		{
			return Variant(ToQuaternion(varEdit_[0]->GetText() + " " + varEdit_[1]->GetText() + " " + varEdit_[2]->GetText()));
		}
		else if (type_ == VAR_COLOR)
		{
			return Variant(ToColor(varEdit_[0]->GetText() + " " + varEdit_[1]->GetText() + " " + varEdit_[2]->GetText() + " " + varEdit_[3]->GetText()));
		}
		else if (type_ == VAR_INTVECTOR2)
		{
			return Variant(ToIntVector2(varEdit_[0]->GetText() + " " + varEdit_[1]->GetText()));
		}
		else if (type_ == VAR_INTRECT)
		{
			return Variant(ToIntRect(varEdit_[0]->GetText() + " " + varEdit_[1]->GetText() + " " + varEdit_[2]->GetText() + " " + varEdit_[3]->GetText()));
		}
		return Variant();
	}

	const Vector<SharedPtr<LineEdit> >& NumberAttributeUI::GetVarValueUI()
	{
		return varEdit_;
	}

	void NumberAttributeUI::SetType(VariantType type)
	{
		/// if VAR_NONE remove all
		if (type == VAR_NONE)
		{
			varEdit_.Clear();
			numCoords_ = 0;
			type_ = type;
			return;
		}

		/// if same type, dont do anything
		if (type_ == type)
			return;

		type_ = type;

		if (type_ == VAR_INT)
		{
			varEdit_.Clear();
			numCoords_ = 1;
			LineEdit* attrEdit = CreateChild<LineEdit>("A_VarValue");

			attrEdit->SetStyle("EditorAttributeEdit");
			attrEdit->SetFixedHeight(17);
			attrEdit->SetVar(ATTR_COORDINATE, 1);
			SubscribeToEvent(attrEdit, E_TEXTCHANGED, HANDLER(NumberAttributeUI, HandleTextChange));
			SubscribeToEvent(attrEdit, E_TEXTFINISHED, HANDLER(NumberAttributeUI, HandleTextChange));
			varEdit_.Push(SharedPtr<LineEdit>(attrEdit));
			return;
		}

		unsigned int numCoords = type - VAR_FLOAT + 1;
		if (type == VAR_QUATERNION)
			numCoords = 3;
		else if (type == VAR_COLOR || type == VAR_INTRECT)
			numCoords = 4;
		else if (type == VAR_INTVECTOR2)
			numCoords = 2;

		varEdit_.Clear();
		numCoords_ = numCoords;
		for (unsigned int i = 0; i < numCoords; ++i)
		{
			LineEdit* attrEdit = CreateChild<LineEdit>("A_VarValue_" + String(i));

			attrEdit->SetStyle("EditorAttributeEdit");
			attrEdit->SetFixedHeight(17);
			attrEdit->SetVar(ATTR_COORDINATE, i);
			SubscribeToEvent(attrEdit, E_TEXTCHANGED, HANDLER(NumberAttributeUI, HandleTextChange));
			SubscribeToEvent(attrEdit, E_TEXTFINISHED, HANDLER(NumberAttributeUI, HandleTextChange));
			varEdit_.Push(SharedPtr<LineEdit>(attrEdit));
		}
	}

	void NumberAttributeUI::HandleTextChange(StringHash eventType, VariantMap& eventData)
	{
		if (inUpdated_)
			return;
		using namespace NumberVarChanged;
		VariantMap& eventData_ = GetEventDataMap();
		eventData_[P_ATTEDIT] = this;
		SendEvent(AEE_NUMBERVARCHANGED, eventData_);
		oldValue_ = GetVarValue();
	}

	Urho3D::Variant NumberAttributeUI::GetOldValue()
	{
		return oldValue_;
	}

	Urho3D::Variant NumberAttributeUI::GetVariant()
	{
		return GetVarValue();
	}

	//////////////////////////////////////////////////////////////////////////
	/// EnumAttributeUI
	//////////////////////////////////////////////////////////////////////////
	EnumAttributeUI::EnumAttributeUI(Context* context) : BasicAttributeUI(context)
	{
		varEdit_ = CreateChild<DropDownList>("A_VarValue");

		varEdit_->SetInternal(true);
		varEdit_->SetFixedHeight(17);
		varEdit_->SetResizePopup(true);
		varEdit_->SetPlaceholderText("--");
		varEdit_->SetLayout(LM_HORIZONTAL, 0, IntRect(4, 1, 4, 1));
		SubscribeToEvent(varEdit_, E_ITEMSELECTED, HANDLER(EnumAttributeUI, HandleItemSelected));
		oldValue_ = 0;
	}

	EnumAttributeUI::~EnumAttributeUI()
	{
	}

	void EnumAttributeUI::RegisterObject(Context* context)
	{
		context->RegisterFactory<EnumAttributeUI>();
		COPY_BASE_ATTRIBUTES(BasicAttributeUI);
	}

	EnumAttributeUI* EnumAttributeUI::Create(Serializable* serializable, const String& name, unsigned int index, const Vector<String>& enums, XMLFile* defaultstyle)
	{
		if (!serializable)
			return NULL;
		EnumAttributeUI* attr = new EnumAttributeUI(serializable->GetContext());
		attr->SetIndex(index);
		attr->SetVarName(name);
		if (defaultstyle)
		{
			attr->SetDefaultStyle(defaultstyle);
			attr->SetStyle("EnumAttributeUI");
		}
		attr->SetEnumNames(enums);
		attr->UpdateVar(serializable);
		return attr;
	}

	void EnumAttributeUI::SetVarValue(Variant& var)
	{
		if (var.GetType() == VAR_INT)
		{
			varEdit_->SetSelection(var.GetUInt());
			oldValue_ = var.GetInt();
		}
	}

	DropDownList* EnumAttributeUI::GetVarValueUI()
	{
		return varEdit_;
	}

	void EnumAttributeUI::SetEnumNames(const Vector<String>& enums)
	{
		enumNames_ = enums;
		varEdit_->RemoveAllItems();

		for (unsigned int i = 0; i < enums.Size(); i++)
		{
			Text* choice = new Text(context_);
			varEdit_->AddItem(choice);
			choice->SetStyle("EditorEnumAttributeText");
			choice->SetText(enums[i]);
		}
	}

	void EnumAttributeUI::HandleItemSelected(StringHash eventType, VariantMap& eventData)
	{
		if (inUpdated_)
			return;
		using namespace EnumVarChanged;

		VariantMap& eventData_ = GetEventDataMap();
		eventData_[P_ATTEDIT] = this;
		SendEvent(AEE_ENUMVARCHANGED, eventData_);

		oldValue_ = varEdit_->GetSelection();
	}

	int EnumAttributeUI::GetVarValue()
	{
		return varEdit_->GetSelection();
	}

	int EnumAttributeUI::GetOldValue()
	{
		return oldValue_;
	}

	Urho3D::Variant EnumAttributeUI::GetVariant()
	{
		return Variant(varEdit_->GetSelection());
	}

	ResourceRefAttributeUI::ResourceRefAttributeUI(Context* context) : BasicAttributeUI(context)
	{
		SetLayout(LM_VERTICAL, 2);
		container_ = CreateChild<UIElement>();

		container_->SetInternal(true);
		container_->SetLayout(LM_HORIZONTAL, 4, IntRect(10, 0, 4, 0));    // Left margin is indented more when the name is so
		container_->SetHeight(19);

		varEdit_ = container_->CreateChild<LineEdit>("A_VarValue");
		varEdit_->SetInternal(true);
		varEdit_->SetFixedHeight(17);
		varEdit_->SetDragDropMode(DD_TARGET);

		SubscribeToEvent(varEdit_, E_TEXTFINISHED, HANDLER(ResourceRefAttributeUI, HandleTextChange));
		//SubscribeToEvent(varEdit_, E_TEXTCHANGED, HANDLER(ResourceRefAttributeUI, HandleTextChange));
		
	}

	ResourceRefAttributeUI::~ResourceRefAttributeUI()
	{
	}

	void ResourceRefAttributeUI::RegisterObject(Context* context)
	{
		context->RegisterFactory<ResourceRefAttributeUI>();
		COPY_BASE_ATTRIBUTES(BasicAttributeUI);
	}

	ResourceRefAttributeUI* ResourceRefAttributeUI::Create(Serializable* serializable, const String& name, VariantType type, StringHash resourceType, unsigned int index, unsigned int subindex, XMLFile* defaultstyle, unsigned int action)
	{
		if (!serializable)
			return NULL;
		ResourceRefAttributeUI* attr = new ResourceRefAttributeUI(serializable->GetContext());
		attr->SetDefaultStyle(defaultstyle);
		attr->SetIndex(index);
		attr->SetType(type);
		attr->SetSubIndex(subindex);
		attr->SetResourceType(resourceType);
		attr->SetVarName(name);
		attr->SetStyle("ResourceRefAttributeUI");
		attr->SetActions(action);
		attr->UpdateVar(serializable);
		attr->SetFixedHeight(36);
		return attr;
	}

	void ResourceRefAttributeUI::SetVarValue(Variant& var)
	{
		if (type_ != var.GetType())
			return;

		if (var.GetType() == VAR_RESOURCEREF)
		{
			varEdit_->SetText(var.GetResourceRef().name_);
			varEdit_->SetCursorPosition(0);
			oldValue_ = var.GetResourceRef().name_;
		}
		else if (var.GetType() == VAR_RESOURCEREFLIST)
		{
			varEdit_->SetText(var.GetResourceRefList().names_[subIndex_]);
			varEdit_->SetCursorPosition(0);
			oldValue_ = var.GetResourceRefList().names_[subIndex_];
		}
	}

	void ResourceRefAttributeUI::SetVarValue(const String& b)
	{
		varEdit_->SetText(b);
	}

	Urho3D::Variant ResourceRefAttributeUI::GetVariant()
	{
		if (type_ == VAR_RESOURCEREF)
		{
			ResourceRef ref;
			ref.name_ = varEdit_->GetText().Trimmed();
			ref.type_ = resType_;
			return Variant(ref);
		}
		else if (type_ == VAR_RESOURCEREFLIST)
		{
			ResourceRefList ref;
			ref.type_ = resType_;
			ref.names_.Push(varEdit_->GetText().Trimmed());
			return Variant(ref);
		}

		return Variant::EMPTY;
	}

	LineEdit* ResourceRefAttributeUI::GetVarValueUI()
	{
		return varEdit_;
	}

	void ResourceRefAttributeUI::HandleTextChange(StringHash eventType, VariantMap& eventData)
	{
		if (inUpdated_)
			return;
		using namespace ResourceRefVarChanged;

		VariantMap& eventData_ = GetEventDataMap();
		eventData_[P_ATTEDIT] = this;
		SendEvent(AEE_RESREFVARCHANGED, eventData_);

		oldValue_ = varEdit_->GetText();
	}

	void ResourceRefAttributeUI::SetActions(unsigned int action)
	{
		if (action != action_)
		{
			if (pick_.NotNull())
			{
				pick_->Remove();
				pick_.Reset();
			}
			if (open_.NotNull())
			{
				open_->Remove();
				open_.Reset();
			}
			if (edit_.NotNull())
			{
				edit_->Remove();
				edit_.Reset();
			}
			if (test_.NotNull())
			{
				test_->Remove();
				test_.Reset();
			}
		}

		action_ = action;

		if ((action_ & ACTION_PICK) != 0)
		{
			pick_ = CreateResourcePickerButton(container_, "Pick");
			SubscribeToEvent(pick_, E_RELEASED, HANDLER(ResourceRefAttributeUI, HandlePick));
		}
		if ((action_ & ACTION_OPEN) != 0)
		{
			open_ = CreateResourcePickerButton(container_, "Open");
			SubscribeToEvent(open_, E_RELEASED, HANDLER(ResourceRefAttributeUI, HandleOpen));
		}
		if ((action_ & ACTION_EDIT) != 0)
		{
			edit_ = CreateResourcePickerButton(container_, "Edit");
			SubscribeToEvent(edit_, E_RELEASED, HANDLER(ResourceRefAttributeUI, HandleEdit));
		}
		if ((action_ & ACTION_TEST) != 0)
		{
			test_ = CreateResourcePickerButton(container_, "Test");
			SubscribeToEvent(test_, E_RELEASED, HANDLER(ResourceRefAttributeUI, HandleTest));
		}
	}

	Button* ResourceRefAttributeUI::CreateResourcePickerButton(UIElement* container, const String& text)
	{
		Button* button = new Button(context_);
		container->AddChild(button);
		button->SetStyleAuto();
		button->SetFixedSize(36, 17);

		Text* buttonText = new Text(context_);
		button->AddChild(buttonText);
		buttonText->SetStyle("EditorAttributeText");
		buttonText->SetAlignment(HA_CENTER, VA_CENTER);
		buttonText->SetText(text);

		return button;
	}

	void ResourceRefAttributeUI::HandlePick(StringHash eventType, VariantMap& eventData)
	{
		using namespace PickResource;

		VariantMap& eventData_ = GetEventDataMap();
		eventData_[P_ATTEDIT] = this;
		SendEvent(AEE_PICKRESOURCE, eventData_);
	}

	void ResourceRefAttributeUI::HandleOpen(StringHash eventType, VariantMap& eventData)
	{
		using namespace OpenResource;

		VariantMap& eventData_ = GetEventDataMap();
		eventData_[P_ATTEDIT] = this;
		SendEvent(AEE_OPENRESOURCE, eventData_);
	}

	void ResourceRefAttributeUI::HandleEdit(StringHash eventType, VariantMap& eventData)
	{
		using namespace EditResource;

		VariantMap& eventData_ = GetEventDataMap();
		eventData_[P_ATTEDIT] = this;
		SendEvent(AEE_EDITRESOURCE, eventData_);
	}

	void ResourceRefAttributeUI::HandleTest(StringHash eventType, VariantMap& eventData)
	{
		using namespace TestResource;

		VariantMap& eventData_ = GetEventDataMap();
		eventData_[P_ATTEDIT] = this;
		SendEvent(AEE_TESTRESOURCE, eventData_);
	}

	void ResourceRefAttributeUI::SetType(VariantType type)
	{
		type_ = type;
	}

	const String& ResourceRefAttributeUI::GetVarValue()
	{
		return  varEdit_->GetText();
	}

	const String& ResourceRefAttributeUI::GetOldValue()
	{
		return oldValue_;
	}
}