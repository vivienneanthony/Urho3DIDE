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
#include "../Core/Object.h"

#include "ResourcePicker.h"
#include "Utils/Helpers.h"

#include "../DebugNew.h"



namespace Urho3D
{
	ResourcePickerManager::ResourcePickerManager(Context* context) : Object(context)
	{
		resourcePickIndex = 0;
		resourcePickSubIndex = 0;
		resourcePicker = NULL;
		initialized_ = false;
	}

	ResourcePickerManager::~ResourcePickerManager()
	{
		Clear();
	}

	void ResourcePickerManager::RegisterObject(Context* context)
	{
		context->RegisterFactory<ResourcePickerManager>();
	}

	void ResourcePickerManager::Init()
	{
		if (!initialized_)
		{
			InitVectorStructs();
			InitResourcePicker();
			initialized_ = true;
		}

	}

	void ResourcePickerManager::InitVectorStructs()
	{
		// Fill vector structure data
		Vector<String> billboardVariables;
		billboardVariables.Push("Billboard Count");
		billboardVariables.Push("   Position");
		billboardVariables.Push("   Size");
		billboardVariables.Push("   UV Coordinates");
		billboardVariables.Push("   Color");
		billboardVariables.Push("   Rotation");
		billboardVariables.Push("   Is Enabled");

		vectorStructs.Push(new VectorStruct("BillboardSet", "Billboards", billboardVariables, 1));

		Vector<String> animationStateVariables;
		animationStateVariables.Push("Anim State Count");
		animationStateVariables.Push("   Animation");
		animationStateVariables.Push("   Start Bone");
		animationStateVariables.Push("   Is Looped");
		animationStateVariables.Push("   Weight");
		animationStateVariables.Push("   Time");
		animationStateVariables.Push("   Layer");

		vectorStructs.Push(new VectorStruct("AnimatedModel", "Animation States", animationStateVariables, 1));

		Vector<String> particleColorVariables;
		particleColorVariables.Push("Color Animation Frames");
		particleColorVariables.Push("   Color");
		particleColorVariables.Push("   Time");

		vectorStructs.Push(new VectorStruct("ParticleEmitter", "Particle Colors", particleColorVariables, 1));

		Vector<String> particleUVAnimVariables;
		particleUVAnimVariables.Push("UV Animation Frames");
		particleUVAnimVariables.Push("   UV Coords");
		particleUVAnimVariables.Push("   Time");

		vectorStructs.Push(new VectorStruct("ParticleEmitter", "UV Animation", particleUVAnimVariables, 1));

		Vector<String> staticModelGroupInstanceVariables;
		staticModelGroupInstanceVariables.Push("Instance Count");
		staticModelGroupInstanceVariables.Push("   NodeID");

		vectorStructs.Push(new VectorStruct("StaticModelGroup", "Instance Nodes", staticModelGroupInstanceVariables, 1));

		Vector<String> splinePathInstanceVariables;
		splinePathInstanceVariables.Push("Control Point Count");
		splinePathInstanceVariables.Push("   NodeID");

		vectorStructs.Push(new VectorStruct("SplinePath", "Control Points", splinePathInstanceVariables, 1));
	}

	void ResourcePickerManager::Clear()
	{
		if (!resourcePickers.Empty())
		{
			Vector<ResourcePicker*>::Iterator it;
			for (it = resourcePickers.Begin(); it != resourcePickers.End(); it++)
			{
				SAFE_DELETE(*it);
			}
			resourcePickers.Clear();
		}

		if (!vectorStructs.Empty())
		{
			Vector<VectorStruct*>::Iterator it;
			for (it = vectorStructs.Begin(); it != vectorStructs.End(); it++)
			{
				SAFE_DELETE(*it);
			}
			vectorStructs.Clear();
		}
	}

	void ResourcePickerManager::InitResourcePicker()
	{
		// Fill resource picker data
		Vector<String> fontFilters;
		fontFilters.Push("*.ttf");
		fontFilters.Push("*.fnt");
		fontFilters.Push("*.xml");
		Vector<String> imageFilters;
		imageFilters.Push("*.png");
		imageFilters.Push("*.jpg");
		imageFilters.Push("*.bmp");
		imageFilters.Push("*.tga");

		Vector<String> luaFileFilters;
		luaFileFilters.Push("*.lua");
		luaFileFilters.Push("*.luc");

		Vector<String> scriptFilters;
		scriptFilters.Push("*.as");
		scriptFilters.Push("*.asc");

		Vector<String> soundFilters;
		soundFilters.Push("*.wav");
		soundFilters.Push("*.ogg");

		Vector<String> textureFilters;
		textureFilters.Push("*.dds");
		textureFilters.Push("*.png");
		textureFilters.Push("*.jpg");
		textureFilters.Push("*.bmp");
		textureFilters.Push("*.tga");
		textureFilters.Push("*.ktx");
		textureFilters.Push("*.pvr");

		Vector<String> materialFilters;
		materialFilters.Push("*.xml");
		materialFilters.Push("*.material");

		Vector<String> pexFilters;
		pexFilters.Push("*.pex");

		Vector<String> anmFilters;
		anmFilters.Push("*.anm");

		resourcePickers.Push(new ResourcePicker("Animation", "*.ani", ACTION_PICK | ACTION_TEST));
		resourcePickers.Push(new ResourcePicker("Font", fontFilters));
		resourcePickers.Push(new ResourcePicker("Image", imageFilters));
		resourcePickers.Push(new ResourcePicker("LuaFile", luaFileFilters));
		resourcePickers.Push(new ResourcePicker("Material", materialFilters, ACTION_PICK | ACTION_OPEN | ACTION_EDIT));
		resourcePickers.Push(new ResourcePicker("Model", "*.mdl", ACTION_PICK | ACTION_EDIT));
		resourcePickers.Push(new ResourcePicker("ScriptFile", scriptFilters));
		resourcePickers.Push(new ResourcePicker("Sound", soundFilters));
		resourcePickers.Push(new ResourcePicker("Technique", "*.xml"));
		resourcePickers.Push(new ResourcePicker("Texture2D", textureFilters));
		resourcePickers.Push(new ResourcePicker("TextureCube", "*.xml"));
		resourcePickers.Push(new ResourcePicker("XMLFile", "*.xml"));
		resourcePickers.Push(new ResourcePicker("Sprite2D", textureFilters, ACTION_PICK | ACTION_OPEN));
		resourcePickers.Push(new ResourcePicker("Animation2D", anmFilters, ACTION_PICK | ACTION_OPEN));
		resourcePickers.Push(new ResourcePicker("ParticleEffect2D", pexFilters, ACTION_PICK | ACTION_OPEN));
	}

	VectorStruct* ResourcePickerManager::GetVectorStruct(Vector<Serializable*>& serializables, unsigned int index)
	{
		AttributeInfo info = serializables[0]->GetAttributes()->At(index);
		for (unsigned int i = 0; i < vectorStructs.Size(); ++i)
		{
			if (vectorStructs[i]->componentTypeName == serializables[0]->GetTypeName() && vectorStructs[i]->attributeName == info.name_)
				return vectorStructs[i];
		}
		return NULL;
	}

	VectorStruct* ResourcePickerManager::GetVectorStruct(Serializable* serializable, unsigned int index)
	{
		AttributeInfo info = serializable->GetAttributes()->At(index);
		for (unsigned int i = 0; i < vectorStructs.Size(); ++i)
		{
			if (vectorStructs[i]->componentTypeName == serializable->GetTypeName() && vectorStructs[i]->attributeName == info.name_)
				return vectorStructs[i];
		}
		return NULL;
	}

	ResourcePicker* ResourcePickerManager::GetResourcePicker(StringHash resourceType)
	{
		for (unsigned int i = 0; i < resourcePickers.Size(); ++i)
		{
			if (resourcePickers[i]->type == resourceType)
				return resourcePickers[i];
		}
		return NULL;
	}

	Vector<Serializable*>& ResourcePickerManager::GetresourceTargets()
	{
		return resourceTargets;
	}

	ResourcePicker* ResourcePickerManager::GetCurrentResourcePicker()
	{
		return resourcePicker;
	}

	void ResourcePickerManager::SetCurrentResourcePicker(ResourcePicker* picker)
	{
		resourcePicker = picker;
	}
}
