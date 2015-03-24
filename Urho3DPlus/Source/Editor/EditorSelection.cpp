

#include "../Urho3D.h"
#include "../Core/Context.h"
#include "EditorSelection.h"
#include "../Core/CoreEvents.h"
#include "../UI/DropDownList.h"
#include "../Engine/EngineEvents.h"
#include "../UI/Font.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/GraphicsEvents.h"
#include "../Input/Input.h"
#include "../Input/InputEvents.h"
#include "../IO/IOEvents.h"
#include "../UI/LineEdit.h"
#include "../UI/ListView.h"
#include "../IO/Log.h"
#include "../Resource/ResourceCache.h"
#include "../UI/ScrollBar.h"
#include "../UI/Text.h"
#include "../UI/UI.h"
#include "../UI/UIEvents.h"
#include "../Scene/Scene.h"
#include "../Graphics/Texture.h"
#include "../Graphics/Texture2D.h"

#include "UIGlobals.h"
#include "MenuBarUI.h"
#include "ToolBarUI.h"
#include "MiniToolBarUI.h"
#include "HierarchyWindow.h"
#include "AttributeInspector.h"
#include "ResourcePicker.h"
#include "../Graphics/Camera.h"
#include "Editor.h"

namespace Urho3D
{


	EditorSelection::EditorSelection(Context* context, Editor* editor) : Object(context),
		editUIElement_(NULL),
		editNode_(NULL),
		numEditableComponentsPerNode_(1),
		editor_(editor)
	{

	}

	EditorSelection::~EditorSelection()
	{

	}

	void EditorSelection::RegisterObject(Context* context)
	{
		context->RegisterFactory<EditorSelection>();
	}

	void EditorSelection::ClearSelection()
	{
		selectedNodes_.Clear();
		selectedComponents_.Clear();
		selectedUIElements_.Clear();

		editUIElement_ = NULL;
		editNode_ = NULL;

		editNodes_.Clear();
		editComponents_.Clear();
		editUIElements_.Clear();

		numEditableComponentsPerNode_ = 1;
	}



	void EditorSelection::AddSelectedComponent(Component* comp)
	{
		if (comp != NULL)
			selectedComponents_.Push(comp);
	}

	void EditorSelection::AddSelectedNode(Node* node)
	{
		if (node != NULL)
			selectedNodes_.Push(node);
	}

	void EditorSelection::AddEditComponent(Component* comp)
	{
		if (comp != NULL)
			editComponents_.Push(comp);
	}

	void EditorSelection::AddEditNode(Node* node)
	{
		if (node != NULL)
			editNodes_.Push(node);
	}



	void EditorSelection::AddSelectedUIElement(UIElement* element)
	{
		if (element != NULL)
			selectedUIElements_.Push(element);
	}

	unsigned EditorSelection::GetNumSelectedUIElements()
	{
		return selectedUIElements_.Size();
	}

	unsigned EditorSelection::GetNumSelectedComponents()
	{
		return selectedComponents_.Size();
	}

	unsigned EditorSelection::GetNumSelectedNodes()
	{
		return selectedNodes_.Size();
	}

	unsigned EditorSelection::GetNumEditComponents()
	{
		return editComponents_.Size();
	}

	unsigned EditorSelection::GetNumEditNodes()
	{
		return editNodes_.Size();
	}

	Node* EditorSelection::GetEditNode()
	{
		return editNode_;
	}

	void EditorSelection::SetEditNode(Node* node)
	{
		editNode_ = node;
	}

	void EditorSelection::SetEditUIElement(UIElement* element)
	{
		editUIElement_ = element;
	}

	UIElement* EditorSelection::GetEditUIElement()
	{
		return editUIElement_;
	}

	Vector<UIElement*> EditorSelection::GetEditUIElements()
	{
		return editUIElements_;
	}

	void EditorSelection::SetNumEditableComponentsPerNode(unsigned int num)
	{
		numEditableComponentsPerNode_ = num;
	}

	unsigned int EditorSelection::GetNumEditableComponentsPerNode()
	{
		return numEditableComponentsPerNode_;
	}

	void EditorSelection::AddEditUIElement(UIElement* element)
	{
		if (element != NULL)
			editUIElements_.Push(element);
	}


	void EditorSelection::SetSelectedNodes(Vector<Node*>& nodes)
	{
		selectedNodes_ = nodes;
	}

	void EditorSelection::SetSelectedComponents(Vector<Component*>& comps)
	{
		selectedComponents_ = comps;
	}

	void EditorSelection::SetSelectedUIElements(Vector<UIElement*>& elemets)
	{
		selectedUIElements_ = elemets;
	}

	void EditorSelection::SetEditNodes(Vector<Node*>& nodes)
	{
		editNodes_ = nodes;
	}

	void EditorSelection::SetEditComponents(Vector<Component*>& comps)
	{
		editComponents_ = comps;
	}

	void EditorSelection::SetEditUIElements(Vector<UIElement*>& elements)
	{
		editUIElements_ = elements;
	}

	Vector<Node*>& EditorSelection::GetSelectedNodes()
	{
		return selectedNodes_;
	}

	Vector<Component*>& EditorSelection::GetSelectedComponents()
	{
		return selectedComponents_;
	}

	Vector<UIElement*>& EditorSelection::GetSelectedUIElements()
	{
		return selectedUIElements_;
	}

	Vector<Node*>& EditorSelection::GetEditNodes()
	{
		return editNodes_;
	}

	Vector<Component*>& EditorSelection::GetEditComponents()
	{
		return editComponents_;
	}

	void EditorSelection::SetGlobalVarNames(const String& name)
	{
		globalVarNames_[name] = name;
	}

	const Variant& EditorSelection::GetGlobalVarNames(StringHash& name)
	{
		return globalVarNames_[name];
	}



	void EditorSelection::OnHierarchyListSelectionChange(const PODVector<UIElement*>& items, const PODVector<unsigned>& indices)
	{
		ClearSelection();

		for (unsigned int i = 0; i < indices.Size(); ++i)
		{
			unsigned int index = indices[i];
			UIElement* item = items[index];
			int type = item->GetVar(TYPE_VAR).GetInt();
			if (type == ITEM_COMPONENT)
			{
				Component* comp = editor_->GetListComponent(item);

				AddSelectedComponent(comp);
			}
			else if (type == ITEM_NODE)
			{
				Node* node = editor_->GetListNode(item);

				AddSelectedNode(node);
			}
			else if (type == ITEM_UI_ELEMENT)
			{
				UIElement* element = editor_->GetListUIElement(item);
				AddSelectedUIElement(element);
			}
		}

		// If only one node/UIElement selected, use it for editing
		if (GetNumSelectedNodes() == 1)
			editNode_ = selectedNodes_[0];

		
		if (GetNumSelectedUIElements() == 1)
			editUIElement_ = selectedUIElements_[0];
		

		// If selection contains only components, and they have a common node, use it for editing
		if (selectedNodes_.Empty() && !selectedComponents_.Empty())
		{
			Node* commonNode = NULL;
			for (unsigned int i = 0; i < GetNumSelectedComponents(); ++i)
			{
				if (i == 0)
					commonNode = GetSelectedComponents()[i]->GetNode();
				else
				{
					if (selectedComponents_[i]->GetNode() != commonNode)
						commonNode = NULL;
				}
			}
			editNode_ = commonNode;
		}

		// Now check if the component(s) can be edited. If many selected, must have same type or have same edit node
		if (!selectedComponents_.Empty())
		{
			if (editNode_ == NULL)
			{
				StringHash compType = selectedComponents_[0]->GetType();
				bool sameType = true;
				for (unsigned int i = 1; i < GetNumSelectedComponents(); ++i)
				{
					if (selectedComponents_[i]->GetType() != compType)
					{
						sameType = false;
						break;
					}
				}
				if (sameType)
					editComponents_ = selectedComponents_;
			}
			else
			{
				editComponents_ = selectedComponents_;
				numEditableComponentsPerNode_ = GetNumSelectedComponents();
			}
		}

		// If just nodes selected, and no components, show as many matching components for editing as possible
		if (!selectedNodes_.Empty() && selectedComponents_.Empty() && selectedNodes_[0]->GetNumComponents() > 0)
		{
			unsigned int count = 0;
			for (unsigned int j = 0; j < selectedNodes_[0]->GetNumComponents(); ++j)
			{
				StringHash compType = selectedNodes_[0]->GetComponents()[j]->GetType();
				bool sameType = true;
				for (unsigned int i = 1; i < GetNumSelectedNodes(); ++i)
				{
					if (selectedNodes_[i]->GetNumComponents() <= j || selectedNodes_[i]->GetComponents()[j]->GetType() != compType)
					{
						sameType = false;
						break;
					}
				}

				if (sameType)
				{
					++count;
					for (unsigned int i = 0; i < GetNumSelectedNodes(); ++i)
						AddEditComponent(selectedNodes_[i]->GetComponents()[j]);
				}
			}
			if (count > 1)
				numEditableComponentsPerNode_ = count;
		}

		if (selectedNodes_.Empty() && editNode_ != NULL)
			AddEditNode(editNode_);
		else
		{
			editNodes_ = selectedNodes_;

			// Cannot multi-edit on scene and node(s) together as scene and node do not share identical attributes,
			// editing via gizmo does not make too much sense either
			if (editNodes_.Size() > 1 && editNodes_[0] == editor_->GetScene())
				editNodes_.Erase(0);
		}

		if (selectedUIElements_.Empty() && editUIElement_ != NULL)
			AddEditUIElement(editUIElement_);
		else
			editUIElements_ = selectedUIElements_;


	}

}