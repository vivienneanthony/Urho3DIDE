#include "../Urho3D.h"
#include "../Core/Context.h"
#include "../Core/Variant.h"
#include "IDESettings.h"
#include "../IO/FileSystem.h"
#include "../IO/File.h"
#include "../Resource/XMLFile.h"
#include "../Resource/XMLElement.h"

namespace Urho3D
{
	IDESettings::IDESettings(Context* context) :Serializable(context)

	{
		ResetToDefault();
	}

	IDESettings::~IDESettings()
	{
	}

	void IDESettings::RegisterObject(Context* context)
	{
		context->RegisterFactory<IDESettings>();

		ATTRIBUTE("WindowTitle", String, windowTitle_, String("Urho3D IDE"), AM_FILE);
		ATTRIBUTE("WindowIcon", String, windowIcon_, String("Textures/UrhoIcon.png"), AM_FILE);
		ATTRIBUTE("WindowWidth", int, windowWidth_, 1024, AM_FILE);
		ATTRIBUTE("WindowHeight", int, windowHeight_, 768, AM_FILE);

		ATTRIBUTE("ResourcePaths", String, resourcePaths_, String("Data;CoreData;IDEData"), AM_FILE);
		ATTRIBUTE("ResourcePackages", String, resourcePackages_, String::EMPTY, AM_FILE);
		ATTRIBUTE("AutoloadPaths", String, autoloadPaths_, String("Extra"), AM_FILE);

		ATTRIBUTE("WindowResizable", bool, windowResizable_, false, AM_FILE);
		ATTRIBUTE("FullScreen", bool, fullScreen_, false, AM_FILE);

		ATTRIBUTE("Projects Root Dir", String, projectsRootDir_, String::EMPTY, AM_FILE);
	}

	VariantMap IDESettings::ToVariantMap()
	{
		const Vector<AttributeInfo>* attributes = GetAttributes();
		if (!attributes)
			return VariantMap();
		VariantMap variantMap;
		Variant value;

		for (unsigned i = 0; i < attributes->Size(); ++i)
		{
			const AttributeInfo& attr = attributes->At(i);
			if (!(attr.mode_ & AM_FILE))
				continue;

			OnGetAttribute(attr, value);
			variantMap[attr.name_] = value;
		}
		return variantMap;
	}

	Urho3D::String IDESettings::GetPreferencesFullPath()
	{
		FileSystem* fs = GetSubsystem<FileSystem>();
		String filepath = fs->GetAppPreferencesDir("urho3d", "ide");
		filepath += "IDESettings.xml";
		return filepath;
	}

	bool IDESettings::Save()
	{
		FileSystem* fileSystem = GetSubsystem<FileSystem>();

		File saveFile(context_, GetPreferencesFullPath(), FILE_WRITE);
		XMLFile xmlFile(context_);
		XMLElement rootElem = xmlFile.CreateRoot("IDESettings");
		SaveXML(rootElem);
		return xmlFile.Save(saveFile);
	}

	bool IDESettings::Load()
	{
		FileSystem* fileSystem = GetSubsystem<FileSystem>();

		if (fileSystem->FileExists(GetPreferencesFullPath()))
		{
			File loadFile(context_, GetPreferencesFullPath(), FILE_READ);
			XMLFile loadXML(context_);
			loadXML.BeginLoad(loadFile);
			LoadXML(loadXML.GetRoot("IDESettings"));
			return true;
		}
		else
			return false;
	}

	void IDESettings::LoadConfigFile()
	{
		if (!Load())
		{
			/// file does not exist so create default file
			Save();
		}
	}
}