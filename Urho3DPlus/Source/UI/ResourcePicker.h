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
#include "../Core/Object.h"
#include "../Scene/Serializable.h"
#include "../Container/Vector.h"


namespace Urho3D
{


	// Resource picker functionality
	const unsigned int  ACTION_PICK = 1;
	const unsigned int  ACTION_OPEN = 2;
	const unsigned int  ACTION_EDIT = 4;
	const unsigned int  ACTION_TEST = 8;

	class ResourcePicker
	{
	private:

	public:
		ResourcePicker(const String& typeName_, const String& filter_, unsigned int  actions_ = ACTION_PICK | ACTION_OPEN)
		{
			typeName = typeName_;
			type = StringHash(typeName_);
			actions = actions_;
			filters.Push(filter_);
			filters.Push("*.*");
			lastFilter = 0;
		}

		ResourcePicker(const String& typeName_, const Vector<String>& filters_, unsigned int  actions_ = ACTION_PICK | ACTION_OPEN)
		{
			typeName = typeName_;
			type =StringHash(typeName_);
			filters = filters_;
			actions = actions_;
			filters.Push("*.*");
			lastFilter = 0;
		}

		String typeName;
		StringHash type;
		String lastPath;
		unsigned int  lastFilter;
		Vector<String> filters;
		unsigned int  actions;
	};

	// VariantVector decoding & editing for certain components
	class VectorStruct
	{
	private:

	public:
		VectorStruct(const String& componentTypeName_, const String& attributeName_, const Vector<String>& variableNames_, unsigned int restartIndex_)
		{
			componentTypeName = componentTypeName_;
			attributeName = attributeName_;
			variableNames = variableNames_;
			restartIndex = restartIndex_;
		}
		String componentTypeName;
		String attributeName;
		Vector<String> variableNames;
		unsigned int restartIndex;
	};



	class ResourcePickerManager : public Object
	{
		OBJECT(ResourcePickerManager);
	public:
		ResourcePickerManager(Context* context);
		virtual ~ResourcePickerManager();
		/// Register object factory.
		static void RegisterObject(Context* context);

		void Init();

		void Clear();
		VectorStruct* GetVectorStruct(Vector<Serializable*>& serializables, unsigned int index);
		VectorStruct* GetVectorStruct(Serializable* serializable, unsigned int index);

		ResourcePicker* GetResourcePicker(StringHash resourceType);

		Vector<Serializable*>& GetresourceTargets();
		ResourcePicker* GetCurrentResourcePicker();
		void SetCurrentResourcePicker(ResourcePicker* picker);

		unsigned int GetresourcePickIndex()
		{
			return resourcePickIndex;
		}
		unsigned int GetresourcePickSubIndex()
		{
			return resourcePickSubIndex;
		}
		void SetresourcePickSubIndex(unsigned int val)
		{
			resourcePickSubIndex = val;
		}
		void SetresourcePickIndex(unsigned int val)
		{
			resourcePickIndex = val;
		}
	private:


		void InitResourcePicker();

		void InitVectorStructs();

		bool initialized_;
		Vector<ResourcePicker*> resourcePickers;
		Vector<Serializable*> resourceTargets;
		unsigned int  resourcePickIndex;
		unsigned int  resourcePickSubIndex;
		ResourcePicker* resourcePicker;

		Vector<VectorStruct*> vectorStructs;
	};

}
