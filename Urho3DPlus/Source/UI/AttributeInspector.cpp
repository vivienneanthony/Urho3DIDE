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
#include "../UI/Window.h"
#include "../Resource/ResourceCache.h"
#include "../IO/FileSystem.h"
#include "../Scene/Scene.h"
#include "../Scene/Node.h"
#include "../UI/UIElement.h"
#include "../UI/DropDownList.h"
#include "../UI/LineEdit.h"
#include "../UI/FileSelector.h"
#include "../Resource/Resource.h"
#include "../Resource/ResourceCache.h"
#include "../IO/Log.h"
#include "../Graphics/StaticModel.h"

#include "AttributeVariableEvents.h"
#include "AttributeInspector.h"
#include "ResourcePicker.h"
#include "AttributeContainer.h"
#include "UIGlobals.h"
#include "UIUtils.h"
#include "AttributeVariable.h"
#include "../Graphics/Graphics.h"
#include "../UI/Button.h"

#include "../DebugNew.h"












namespace Urho3D
{
	
	void AttributeInspector::RegisterObject(Context* context)
	{
		context->RegisterFactory<AttributeInspector>();
	}

	AttributeInspector::~AttributeInspector()
	{
	}

	AttributeInspector::AttributeInspector(Context* context) : Object(context)
	{
		parentContainer_ = NULL;
		editorResourcePicker_ = NULL;
		applyMaterialList_ = true;
		attributesDirty_ = false;
		attributesFullDirty_ = false;

		inLoadAttributeEditor_ = false;
		inEditAttribute_ = false;
		showNonEditableAttribute_ = false;
		numEditableComponentsPerNode_ = 1;

		normalTextColor_ = Color(1.0f, 1.0f, 1.0f);
		modifiedTextColor_ = Color(1.0f, 0.8f, 0.5f);
		nonEditableTextColor_ = Color(0.7f, 0.7f, 0.7f);

		// Exceptions for string attributes that should not be continuously edited
		noTextChangedAttrs_.Push("Script File");
		noTextChangedAttrs_.Push("Class Name");
		noTextChangedAttrs_.Push("Script Object Type");
		noTextChangedAttrs_.Push("Script File Name");
	}

	UIElement* AttributeInspector::Create()
	{
		cache_ = GetSubsystem<ResourceCache>();
		fileSystem_ = GetSubsystem<FileSystem>();


		styleFile_ = cache_->GetResource<XMLFile>("UI/IDEStyle.xml");
		iconStyle_ = cache_->GetResource<XMLFile>("UI/EditorIcons.xml");
		editorResourcePicker_ = GetSubsystem<ResourcePickerManager>();
		editorResourcePicker_->Init();


		attributewindow_ = new Window(context_);
		attributewindow_->SetName("attributewindow");
		attributewindow_->LoadXML(cache_->GetResource<XMLFile>("UI/EditorInspectorWindow.xml")->GetRoot(), styleFile_);
	
		parentContainer_ = attributewindow_->GetChild("ParentContainer", true);

		String resources[] = { "UI/EditorInspector_Attribute.xml", "UI/EditorInspector_Variable.xml", "UI/EditorInspector_Style.xml" };
		for (unsigned int i = 0; i < 3; ++i)
			xmlResources_.Push(SharedPtr<XMLFile>(cache_->GetResource<XMLFile>(resources[i])));

		Update(); 

		SubscribeToEvent(dynamic_cast<Button*>(attributewindow_->GetChild("CloseButton", true)), E_RELEASED, HANDLER(AttributeInspector, HideWindow));

		SubscribeToEvent(AEE_PICKRESOURCE, HANDLER(AttributeInspector, PickResource));
		SubscribeToEvent(AEE_OPENRESOURCE, HANDLER(AttributeInspector, OpenResource));
		SubscribeToEvent(AEE_EDITRESOURCE, HANDLER(AttributeInspector, EditResource));
		SubscribeToEvent(AEE_TESTRESOURCE, HANDLER(AttributeInspector, TestResource));

		return attributewindow_;
	}

	bool AttributeInspector::DeleteNodeContainer(Serializable* serializable)
	{
		if (!serializable)
			return NULL;
		HashMap< StringHash, SharedPtr<AttributeContainer> >::Iterator it = nodeContainers_.Find(serializable->GetType());
		if (it != nodeContainers_.End())
		{
			nodeContainers_.Erase(it);
			return true;
		}
		return false;
	}

	AttributeContainer* AttributeInspector::CreateNodeContainer(Serializable* serializable)
	{
		if (!serializable)
			return NULL;
		HashMap< StringHash, SharedPtr<AttributeContainer> >::Iterator it = nodeContainers_.Find(serializable->GetType());
		if (it != nodeContainers_.End())
			return it->second_;

		AttributeContainer* nodeContainer = new AttributeContainer(context_);
		nodeContainer->SetTitle("Select editable objects");
		nodeContainer->SetNoTextChangedAttrs(noTextChangedAttrs_);
		nodeContainer->LoadChildXML(xmlResources_[1]->GetRoot(), styleFile_);

		nodeContainer->SetIcon(iconStyle_, Node::GetTypeNameStatic());

		SubscribeToEvent(nodeContainer->GetChild("ResetToDefault", true), E_RELEASED, HANDLER(AttributeInspector, HandleResetToDefault));
		SubscribeToEvent(nodeContainer->GetChild("NewVarDropDown", true), E_ITEMSELECTED, HANDLER(AttributeInspector, CreateNodeVariable));
		SubscribeToEvent(nodeContainer->GetChild("DeleteVarButton", true), E_RELEASED, HANDLER(AttributeInspector, DeleteNodeVariable));

		// Resize the node editor according to the number of variables, up to a certain maximum
		unsigned int maxAttrs = Clamp((int)nodeContainer->GetAttributeList()->GetContentElement()->GetNumChildren(), MIN_NODE_ATTRIBUTES, MAX_NODE_ATTRIBUTES);
		nodeContainer->GetAttributeList()->SetFixedHeight(maxAttrs * ATTR_HEIGHT + 4);
		nodeContainer->SetFixedHeight(maxAttrs * ATTR_HEIGHT + 58);

		parentContainer_->AddChild(nodeContainer);

		nodeContainer->SetStyleAuto();

		nodeContainers_[serializable->GetType()] = nodeContainer;

		return nodeContainer;
	}

	AttributeContainer* AttributeInspector::CreateComponentContainer(Serializable* serializable)
	{
		if (!serializable)
			return NULL;
		HashMap< StringHash, SharedPtr<AttributeContainer> >::Iterator it = componentContainers_.Find(serializable->GetType());
		if (it != componentContainers_.End())
			return it->second_;

		SharedPtr<AttributeContainer> componentContainer(new AttributeContainer(context_));

		componentContainer->SetStyleAuto(styleFile_);
		componentContainer->SetNoTextChangedAttrs(noTextChangedAttrs_);
		componentContainer->SetTitle(serializable->GetTypeName());
		componentContainer->SetSerializableAttributes(serializable);

		componentContainers_[serializable->GetType()] = componentContainer;

		// Resize the node editor according to the number of variables, up to a certain maximum
		unsigned int maxAttrs = componentContainer->GetAttributeList()->GetContentElement()->GetNumChildren();
		componentContainer->GetAttributeList()->SetHeight(maxAttrs * ATTR_HEIGHT + 4);
		componentContainer->SetHeight(maxAttrs * ATTR_HEIGHT + 58);

		parentContainer_->AddChild(componentContainer);
		componentContainer->SetIcon(iconStyle_, serializable->GetTypeName());

		parentContainer_->UpdateLayout();

		return componentContainer;
	}

	void AttributeInspector::Update(bool fullUpdate /*= true*/)
	{
		attributesDirty_ = false;
		if (fullUpdate)
			attributesFullDirty_ = false;

		DisableAllContainers();

		if (!editNodes_.Empty())
		{
		//	Vector<Serializable*> nodes = UIUtils::ToSerializableArray(editorData_->GetEditNodes());
			AttributeContainer* nodeContainer = CreateNodeContainer(editNodes_[0]);
			nodeContainer->SetVisible(true);
			nodeContainer->SetEnabled(true);

			Node* editNode = editNodes_[0];
			if (editNode != NULL)
			{
				String idStr;
				if (editNode->GetID() >= FIRST_LOCAL_ID)
					idStr = " (Local ID " + String(editNode->GetID()) + ")";
				else
					idStr = " (ID " + String(editNode->GetID()) + ")";

				nodeContainer->SetTitle(editNode->GetTypeName() + idStr);
			}
			else
			{
				nodeContainer->SetTitle(editNodes_[0]->GetTypeName() + " (ID -- : " + String(editNodes_.Size()) + "x)");
			}

			nodeContainer->SetSerializableAttributes(editNodes_[0]);
		}

		if (!editComponents_.Empty())
		{
	
			for (unsigned int j = 0; j < editComponents_.Size(); ++j)
			{
				Component* comp = editComponents_[j ];

				AttributeContainer* container = CreateComponentContainer(comp);

				container->SetVisible(true);
				container->SetEnabled(true);

				container->SetTitle(UIUtils::GetComponentTitle(comp) );

				container->SetSerializableAttributes(comp);
			}
		}

		if (parentContainer_->GetNumChildren() == 0)
		{
			// No editables, insert a dummy component container to show the information
		}
	}

	void AttributeInspector::PickResource(StringHash eventType, VariantMap& eventData)
	{
		using namespace PickResource;
		//TODO:
		ResourceRefAttributeUI* attrEdit = dynamic_cast<ResourceRefAttributeUI*>(eventData[P_ATTEDIT].GetPtr());

		Vector<Serializable*> targets = GetAttributeEditorTargets(attrEdit);
		if (targets.Empty())
			return;

		editorResourcePicker_->SetresourcePickIndex( attrEdit->GetIndex());
		editorResourcePicker_->SetresourcePickSubIndex ( attrEdit->GetSubIndex());
		AttributeInfo info = targets[0]->GetAttributes()->At(editorResourcePicker_->GetresourcePickIndex());

		StringHash resourceType;
		if (info.type_ == VAR_RESOURCEREF)
			resourceType = targets[0]->GetAttribute(editorResourcePicker_->GetresourcePickIndex()).GetResourceRef().type_;
		else if (info.type_ == VAR_RESOURCEREFLIST)
			resourceType = targets[0]->GetAttribute(editorResourcePicker_->GetresourcePickIndex()).GetResourceRefList().type_;
		else if (info.type_ == VAR_VARIANTVECTOR)
			resourceType = targets[0]->GetAttribute(editorResourcePicker_->GetresourcePickIndex()).GetVariantVector()[editorResourcePicker_->GetresourcePickSubIndex()].GetResourceRef().type_;

		editorResourcePicker_->SetCurrentResourcePicker(editorResourcePicker_->GetResourcePicker(resourceType));
		ResourcePicker* picker = editorResourcePicker_->GetCurrentResourcePicker();
		if (picker == NULL)
			return;

		editorResourcePicker_->GetresourceTargets().Clear();
		for (unsigned int i = 0; i < targets.Size(); ++i)
			editorResourcePicker_->GetresourceTargets().Push(targets[i]);
		
		String lastPath = picker->lastPath;
		ResourceCache* cache = GetSubsystem<ResourceCache>();
		if (lastPath.Empty())	
			lastPath = cache->GetResourceDirs()[0];
		
		

		uiFileSelector_ = new  FileSelector(context_);
		uiFileSelector_->SetDefaultStyle(cache_->GetResource<XMLFile>("UI/DefaultStyle.xml"));
		uiFileSelector_->SetTitle("Pick " + picker->typeName);
		uiFileSelector_->SetPath(lastPath);
		uiFileSelector_->SetButtonTexts("OK", "Cancel");
		uiFileSelector_->SetFilters(picker->filters, picker->lastFilter);

		IntVector2 size = uiFileSelector_->GetWindow()->GetSize();
		Graphics* graphics = GetSubsystem<Graphics>();
		uiFileSelector_->GetWindow()->SetPosition((graphics->GetWidth() - size.x_) / 2, (graphics->GetHeight() - size.y_) / 2);
	
		SubscribeToEvent(uiFileSelector_, E_FILESELECTED, HANDLER(AttributeInspector, PickResourceDone));
	}

	void AttributeInspector::EditAttribute(StringHash eventType, VariantMap& eventData)
	{
	}

	void AttributeInspector::OpenResource(StringHash eventType, VariantMap& eventData)
	{
	}

	void AttributeInspector::EditResource(StringHash eventType, VariantMap& eventData)
	{
	}

	void AttributeInspector::TestResource(StringHash eventType, VariantMap& eventData)
	{
	}

	void AttributeInspector::HandleResetToDefault(StringHash eventType, VariantMap& eventData)
	{
	}

	void AttributeInspector::CreateNodeVariable(StringHash eventType, VariantMap& eventData)
	{
		if (editNodes_.Empty())
			return;
		LineEdit* editName = NULL;
		String newName = ExtractVariableName(eventData, editName);
		if (newName.Empty())
			return;

		// Create scene variable
		editNodes_[0]->GetScene()->RegisterVar(newName);


		Variant newValue = ExtractVariantType(eventData);

		// If we overwrite an existing variable, must recreate the attribute-editor(s) for the correct type
		bool overwrite = false;
		for (unsigned int i = 0; i < editNodes_.Size(); ++i)
		{
			overwrite = overwrite || editNodes_[i]->GetVars().Contains(newName);
			editNodes_[i]->SetVar(newName, newValue);
		}

		AttributeContainer* nodeContainer = CreateNodeContainer(editNodes_[0]);
		nodeContainer->UpdateVariantMap(editNodes_[0]);

		if (editName)
			editName->SetText("");
	}

	void AttributeInspector::DeleteNodeVariable(StringHash eventType, VariantMap& eventData)
	{
		if (editNodes_.Empty())
			return;
		LineEdit* editName = NULL;

		String delName = ExtractVariableName(eventData, editName);
		if (delName.Empty())
			return;

		// Note: intentionally do not unregister the variable name here as the same variable name may still be used by other attribute list

		bool erased = false;
		for (unsigned int i = 0; i < editNodes_.Size(); ++i)
		{
			// \todo Should first check whether var in question is editable
			//	erased = editorData_->GetEditNodes()[i].GetVars().Erase(delName) || erased;
		}
		if (editName)
			editName->SetText("");
	}

	void AttributeInspector::HideWindow(StringHash eventType, VariantMap& eventData)
	{
		attributewindow_->SetVisible(false);
	}

	void AttributeInspector::DisableAllContainers()
	{
		for (unsigned int i = 0; i < parentContainer_->GetNumChildren(); i++)
		{
			UIElement* e = parentContainer_->GetChild(i);
			if (e)
			{
				e->SetVisible(false);
				e->SetEnabled(false);
			}
		}
	}



	Window* AttributeInspector::GetAttributewindow()
	{
		return attributewindow_;

	}

	Vector<Node*>& AttributeInspector::GetEditNodes()
	{
		return editNodes_;
	}

	Vector<Component*>& AttributeInspector::GetEditComponents()
	{
		return editComponents_;
	}

	Vector<UIElement*>& AttributeInspector::GetEditUIElements()
	{
		return editUIElements_;
	}

	Urho3D::String AttributeInspector::ExtractVariableName(VariantMap& eventData, LineEdit* nameEdit)
	{
		UIElement* element = (UIElement*)eventData[ItemSelected::P_ELEMENT].GetPtr();
		UIElement* parent = element->GetParent();
		nameEdit = (LineEdit*)parent->GetChild("VarNameEdit", true);
		return nameEdit->GetText().Trimmed();
	}

	Urho3D::Variant AttributeInspector::ExtractVariantType(VariantMap& eventData)
	{
		DropDownList* dropDown = (DropDownList*)eventData[ItemSelected::P_ELEMENT].GetPtr();
		switch (dropDown->GetSelection())
		{
		case 0:
			return Variant(0);
		case 1:
			return Variant(false);
		case 2:
			return Variant(0.0f);
		case 3:
			return Variant(String());
		case 4:
			return Variant(Vector3());
		case 5:
			return Variant(Color());
		}

		return Variant::EMPTY;   // This should not happen
	}

	Vector<Serializable*> AttributeInspector::GetAttributeEditorTargets(BasicAttributeUI* attrEdit)
	{
		Vector<Serializable*> ret;


		UIElement* elpar = attrEdit->GetParent()->GetParent()->GetParent()->GetParent();
		AttributeContainer* acon = dynamic_cast<AttributeContainer*>(elpar);
		if (acon)
			ret.Push(acon->GetSerializable());
		
// 		const Vector<unsigned int>& ids = attrEdit->GetIDs();
// 		if (attrEdit->GetIDType() == NODE_IDS_VAR)
// 		{
// 			for (unsigned int i = 0; i < ids.Size(); ++i)
// 			{
// 				Node* node = editorData_->GetCurrentScene()->GetNode(ids[i]);
// 				if (node != NULL)
// 					ret.Push(node);
// 			}
// 		}
// 		else if (attrEdit->GetIDType() == COMPONENT_IDS_VAR)
// 		{
// 			for (unsigned int i = 0; i < ids.Size(); ++i)
// 			{
// 				Component* component = editorData_->GetCurrentScene()->GetComponent(ids[i]);
// 				if (component != NULL)
// 					ret.Push(component);
// 			}
// 		}
// 		else	if (attrEdit->GetIDType() == UI_ELEMENT_IDS_VAR)
// 		{
// 			for (unsigned int i = 0; i < ids.Size(); ++i)
// 			{
// 				// TODO:
// 				// 							UIElement* element = editorUIElement.GetChild(UI_ELEMENT_ID_VAR, ids[i], true);
// 				// 							if (element != NULL)
// 				// 								ret.Push(element);
// 			}
// 		}

		return ret;
	}

	void AttributeInspector::PickResourceDone(StringHash eventType, VariantMap& eventData)
	{
		StoreResourcePickerPath();
		uiFileSelector_ = NULL;


		if (!eventData["OK"].GetBool())
		{
			editorResourcePicker_->GetresourceTargets().Clear();
			editorResourcePicker_->SetCurrentResourcePicker(NULL);
			return;
		}

		if (editorResourcePicker_->GetCurrentResourcePicker() == NULL)
			return;

		// Validate the resource. It must come from within a registered resource directory, and be loaded successfully
		String resourceName = eventData["FileName"].GetString();
		Resource* res = GetPickedResource(resourceName);
		if (res == NULL)
		{
			editorResourcePicker_->SetCurrentResourcePicker(NULL);
			return;
		}

		// Store old values so that PostEditAttribute can create undo actions
		Vector<Variant> oldValues;
		for (unsigned int i = 0; i < editorResourcePicker_->GetresourceTargets().Size(); ++i)
			oldValues.Push(editorResourcePicker_->GetresourceTargets()[i]->GetAttribute(editorResourcePicker_->GetresourcePickIndex()));

		for (unsigned int i = 0; i < editorResourcePicker_->GetresourceTargets().Size(); ++i)
		{
			Serializable* target = editorResourcePicker_->GetresourceTargets()[i];

			AttributeInfo info = target->GetAttributes()->At(editorResourcePicker_->GetresourcePickIndex());
			if (info.type_ == VAR_RESOURCEREF)
			{
				ResourceRef ref = target->GetAttribute(editorResourcePicker_->GetresourcePickIndex()).GetResourceRef();
				ref.type_ = res->GetType();
				ref.name_ = res->GetName();
				target->SetAttribute(editorResourcePicker_->GetresourcePickIndex(), Variant(ref));
				target->ApplyAttributes();
			}
			else if (info.type_ == VAR_RESOURCEREFLIST)
			{
				ResourceRefList refList = target->GetAttribute(editorResourcePicker_->GetresourcePickIndex()).GetResourceRefList();
				if (editorResourcePicker_->GetresourcePickSubIndex() < refList.names_.Size())
				{
					refList.names_[editorResourcePicker_->GetresourcePickSubIndex()] = res->GetName();
					target->SetAttribute(editorResourcePicker_->GetresourcePickIndex(), Variant(refList));
					target->ApplyAttributes();
				}
			}
			else if (info.type_ == VAR_VARIANTVECTOR)
			{
				Vector<Variant> attrs = target->GetAttribute(editorResourcePicker_->GetresourcePickIndex()).GetVariantVector();
				ResourceRef ref = attrs[editorResourcePicker_->GetresourcePickSubIndex()].GetResourceRef();
				ref.type_ = res->GetType();
				ref.name_ = res->GetName();
				attrs[editorResourcePicker_->GetresourcePickSubIndex()] = ref;
				target->SetAttribute(editorResourcePicker_->GetresourcePickIndex(), Variant(attrs));
				target->ApplyAttributes();
			}
		}

		PostEditAttribute(editorResourcePicker_->GetresourceTargets(), editorResourcePicker_->GetresourcePickIndex(), oldValues);
		Update(false);

		editorResourcePicker_->GetresourceTargets().Clear();
		editorResourcePicker_->SetCurrentResourcePicker(NULL);
	}

	void AttributeInspector::StoreResourcePickerPath()
	{
		// Store filter and directory for next time
		if (editorResourcePicker_->GetCurrentResourcePicker() != NULL && uiFileSelector_.NotNull())
		{
			editorResourcePicker_->GetCurrentResourcePicker()->lastPath = uiFileSelector_->GetPath();
			editorResourcePicker_->GetCurrentResourcePicker()->lastFilter = uiFileSelector_->GetFilterIndex();
		}
	}

	Resource* AttributeInspector::GetPickedResource(String resourceName)
	{
		resourceName = GetResourceNameFromFullName(resourceName);
		ResourceCache* cache = GetSubsystem<ResourceCache>();
		Resource* res = cache->GetResource(editorResourcePicker_->GetCurrentResourcePicker()->typeName, resourceName);

		if (res == NULL)
			LOGWARNINGF("Cannot find resource type: %s  Name: %s ", editorResourcePicker_->GetCurrentResourcePicker()->typeName.CString(), resourceName.CString());

		return res;
	}

	Urho3D::String AttributeInspector::GetResourceNameFromFullName(const String& resourceName)
	{
		ResourceCache* cache = GetSubsystem<ResourceCache>();
		const Vector<String>& resourceDirs = cache->GetResourceDirs();

		for (unsigned int i = 0; i < resourceDirs.Size(); ++i)
		{
			if (!resourceName.ToLower().StartsWith(resourceDirs[i].ToLower()))
				continue;
			return resourceName.Substring(resourceDirs[i].Length());
		}

		return ""; // Not found
	}

	void AttributeInspector::PostEditAttribute(Serializable* serializable, unsigned int index)
	{
		// If a StaticModel/AnimatedModel/Skybox model was changed, apply a possibly different material list
		if (applyMaterialList_ && serializable->GetAttributes()->At(index).name_ == "Model")
		{
			StaticModel* staticModel = dynamic_cast<StaticModel*>(serializable);
			if (staticModel != NULL)
				staticModel->ApplyMaterialList();
		}
	}

	void AttributeInspector::PostEditAttribute(Vector<Serializable*>& serializables, unsigned int index, const Vector<Variant>& oldValues)
	{
		// Create undo actions for the edits
// 		EditActionGroup group;
// 		for (uint i = 0; i < serializables.length; ++i)
// 		{
// 			EditAttributeAction action;
// 			action.Define(serializables[i], index, oldValues[i]);
// 			group.actions.Push(action);
// 		}
// 		SaveEditActionGroup(group);

		// If a UI-element changing its 'Is Modal' attribute, clear the hierarchy list selection
// 		int itemType = UIUtils::GetType(serializables[0]);
// 		if (itemType == ITEM_UI_ELEMENT && serializables[0].attributeInfos[index].name == "Is Modal")
// 			hierarchyList.ClearSelection();

		for (unsigned int i = 0; i < serializables.Size(); ++i)
		{
			PostEditAttribute(serializables[i], index);
// 			if (itemType == ITEM_UI_ELEMENT)
// 				SetUIElementModified(serializables[i]);
		}

// 		if (itemType != ITEM_UI_ELEMENT)
// 			SetSceneModified();
	}

}