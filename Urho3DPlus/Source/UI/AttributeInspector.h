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
	class Node;
	class Component;
	class UIElement;

	class BorderImage;
	class ScrollBar;
	class XMLFile;
	class ResourceCache;
	class FileSystem;
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
	class Resource;
	class FileSelector;

	class AttributeContainer;
	class ResourcePickerManager;

	/// \todo Serialization
	class AttributeInspector : public Object
	{
		OBJECT(AttributeInspector);

	public:
		/// Construct.
		AttributeInspector(Context* context);
		/// Destruct.
		virtual ~AttributeInspector();
		/// Register object factory.
		static void RegisterObject(Context* context);

		UIElement*  Create();
		/// Update the whole attribute inspector window, when fullUpdate flag is set to true then first delete all
		///	the containers and repopulate them again from scratch.
		/// The fullUpdate flag is usually set to true when the structure of the attributes are different than
		/// the existing attributes in the list.
		void Update(bool fullUpdate = true);
		/// Disable all child containers in the inspector list.
		void DisableAllContainers();

		Window*				GetAttributewindow(); 
		Vector<Node*>&		GetEditNodes();
		Vector<Component*>& GetEditComponents();
		Vector<UIElement*>&	GetEditUIElements();
	protected:
		/// Get node container in the inspector list, create the container if it is not yet available.
		AttributeContainer* CreateNodeContainer(Serializable* serializable);
		bool				DeleteNodeContainer(Serializable* serializable);
		/// Get component container at the specified index, create the container if it is not yet available at the specified index.
		AttributeContainer*		CreateComponentContainer(Serializable* serializable);
		String					ExtractVariableName(VariantMap& eventData, LineEdit* nameEdit);
		Variant					ExtractVariantType(VariantMap& eventData);
		Vector<Serializable*>	GetAttributeEditorTargets(BasicAttributeUI* attrEdit);

		void		StoreResourcePickerPath();
		Resource*	GetPickedResource(String resourceName);
		String		GetResourceNameFromFullName(const String& resourceName);

		/// Call after the attribute values in the target serializables have been edited. 
		void PostEditAttribute(Vector<Serializable*>& serializables, unsigned int index, const Vector<Variant>& oldValues);
		/// Call after the attribute values in the target serializables have been edited. 
		void PostEditAttribute(Serializable* serializable, unsigned int index);
		
		/// AttributeContainerUI Handlers
		void EditAttribute(StringHash eventType, VariantMap& eventData);
		void PickResource(StringHash eventType, VariantMap& eventData);
		void PickResourceDone(StringHash eventType, VariantMap& eventData);
		void OpenResource(StringHash eventType, VariantMap& eventData);
		void EditResource(StringHash eventType, VariantMap& eventData);
		void TestResource(StringHash eventType, VariantMap& eventData);

		/// Handle reset to default event, sent when reset icon in the icon-panel is clicked.
		void HandleResetToDefault(StringHash eventType, VariantMap& eventData);
		/// Handle create new user-defined variable event for node target.
		void CreateNodeVariable(StringHash eventType, VariantMap& eventData);
		/// Handle delete existing user-defined variable event for node target.
		void DeleteNodeVariable(StringHash eventType, VariantMap& eventData);
		/// UI actions
		void HideWindow(StringHash eventType, VariantMap& eventData);

		/// cached subsystem
		ResourceCache*	cache_;
		FileSystem*		fileSystem_;
		UIElement*		parentContainer_;
		ResourcePickerManager* editorResourcePicker_;

		/// Serialized Attributes
		Color	normalTextColor_;
		Color	modifiedTextColor_;
		Color	nonEditableTextColor_;
		bool	applyMaterialList_;
		bool	showNonEditableAttribute_;
		unsigned int numEditableComponentsPerNode_;

		/// other Attributes
		bool inLoadAttributeEditor_;
		bool inEditAttribute_;
		bool attributesDirty_;
		bool attributesFullDirty_;

		SharedPtr<Window>	attributewindow_;
		SharedPtr<XMLFile>	styleFile_;
		SharedPtr<XMLFile>	iconStyle_;
		///	Constants for accessing xmlResources
		Vector< SharedPtr<XMLFile> >	xmlResources_;
		/// Exceptions for string attributes that should not be continuously edited
		Vector<String>		noTextChangedAttrs_;

		/// node container
		HashMap< StringHash, SharedPtr<AttributeContainer> > nodeContainers_;
		/// cached Attribute Containers for different component Types
		HashMap< StringHash, SharedPtr<AttributeContainer> > componentContainers_;

		Vector<Node*>		editNodes_;
		Vector<Component*>	editComponents_;
		Vector<UIElement*>	editUIElements_;

		SharedPtr<FileSelector> uiFileSelector_;
	};
}