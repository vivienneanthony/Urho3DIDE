#include "../Urho3D.h"
#include "../Core/Context.h"
#include "ResourceBrowser.h"
#include "../Resource/ResourceCache.h"
#include "../IO/FileSystem.h"
#include "../IO/File.h"
#include "../Resource/XMLFile.h"
#include "../Resource/XMLElement.h"
#include "../UI/UI.h"
#include "../UI/Window.h"
#include "../UI/Text.h"
#include "../UI/LineEdit.h"
#include "../UI/ListView.h"
#include "../Scene/Scene.h"
#include "../Physics/PhysicsWorld.h"
#include "../Scene/Node.h"
#include "../Graphics/Zone.h"
#include "../Graphics/Camera.h"
#include "../Graphics/Light.h"
#include "../UI/View3D.h"
#include "../Math/Vector3.h"
#include "../UI/CheckBox.h"
#include "../UI/UIEvents.h"
#include "../Graphics/Model.h"
#include "../Graphics/StaticModel.h"
#include "../Graphics/Material.h"
#include "../Resource/Image.h"
#include "../Graphics/Texture2D.h"
#include "../Resource/XMLFile.h"
#include "../Graphics/ParticleEffect.h"
#include "../Resource/Resource.h"
#include "../IO/Deserializer.h"
#include "../Scene/Serializable.h"
#include "../IO/File.h"
#include "../Graphics/Texture.h"
#include "../Graphics/AnimatedModel.h"
#include "../IO/Serializer.h"
#include "../Graphics/ParticleEmitter.h"
#include "../Math/BoundingBox.h"
#include "../UI/UIElement.h"
#include "../Resource/ResourceEvents.h"
#include "../Graphics/Octree.h"
#include "../Input/InputEvents.h"
#include "EditorView.h"
#include "MenuBarUI.h"
#include "UIGlobals.h"
#include "../UI/TabWindow.h"

namespace Urho3D
{
	const unsigned int BROWSER_WORKER_ITEMS_PER_TICK = 10;
	const unsigned int BROWSER_SEARCH_LIMIT = 50;
	const int BROWSER_SORT_MODE_ALPHA = 1;
	const int BROWSER_SORT_MODE_SEARCH = 2;

	const int RESOURCE_TYPE_UNUSABLE = -2;
	const int RESOURCE_TYPE_UNKNOWN = -1;
	const int RESOURCE_TYPE_NOTSET = 0;
	const int RESOURCE_TYPE_SCENE = 1;
	const int RESOURCE_TYPE_SCRIPTFILE = 2;
	const int RESOURCE_TYPE_MODEL = 3;
	const int RESOURCE_TYPE_MATERIAL = 4;
	const int RESOURCE_TYPE_ANIMATION = 5;
	const int RESOURCE_TYPE_IMAGE = 6;
	const int RESOURCE_TYPE_SOUND = 7;
	const int RESOURCE_TYPE_TEXTURE = 8;
	const int RESOURCE_TYPE_FONT = 9;
	const int RESOURCE_TYPE_PREFAB = 10;
	const int RESOURCE_TYPE_TECHNIQUE = 11;
	const int RESOURCE_TYPE_PARTICLEEFFECT = 12;
	const int RESOURCE_TYPE_UIELEMENT = 13;
	const int RESOURCE_TYPE_UIELEMENTS = 14;
	const int RESOURCE_TYPE_ANIMATION_SETTINGS = 15;
	const int RESOURCE_TYPE_RENDERPATH = 16;
	const int RESOURCE_TYPE_TEXTURE_ATLAS = 17;
	const int RESOURCE_TYPE_2D_PARTICLE_EFFECT = 18;
	const int RESOURCE_TYPE_TEXTURE_3D = 19;
	const int RESOURCE_TYPE_CUBEMAP = 20;
	const int RESOURCE_TYPE_PARTICLEEMITTER = 21;
	const int RESOURCE_TYPE_2D_ANIMATION_SET = 22;

	// any resource type > 0 is valid
	const int NUMBER_OF_VALID_RESOURCE_TYPES = 22;

	const StringHash XML_TYPE_SCENE("scene");
	const StringHash XML_TYPE_NODE("node");
	const StringHash XML_TYPE_MATERIAL("material");
	const StringHash XML_TYPE_TECHNIQUE("technique");
	const StringHash XML_TYPE_PARTICLEEFFECT("particleeffect");
	const StringHash XML_TYPE_PARTICLEEMITTER("particleemitter");
	const StringHash XML_TYPE_TEXTURE("texture");
	const StringHash XML_TYPE_ELEMENT("element");
	const StringHash XML_TYPE_ELEMENTS("elements");
	const StringHash XML_TYPE_ANIMATION_SETTINGS("animation");
	const StringHash XML_TYPE_RENDERPATH("renderpath");
	const StringHash XML_TYPE_TEXTURE_ATLAS("TextureAtlas");
	const StringHash XML_TYPE_2D_PARTICLE_EFFECT("particleEmitterConfig");
	const StringHash XML_TYPE_TEXTURE_3D("texture3d");
	const StringHash XML_TYPE_CUBEMAP("cubemap");
	const StringHash XML_TYPE_SPRITER_DATA("spriter_data");

	const StringHash BINARY_TYPE_SCENE("USCN");
	const StringHash BINARY_TYPE_PACKAGE("UPAK");
	const StringHash BINARY_TYPE_COMPRESSED_PACKAGE("ULZ4");
	const StringHash BINARY_TYPE_ANGLESCRIPT("ASBC");
	const StringHash BINARY_TYPE_MODEL("UMDL");
	const StringHash BINARY_TYPE_SHADER("USHD");
	const StringHash BINARY_TYPE_ANIMATION("UANI");

	const StringHash EXTENSION_TYPE_TTF(".ttf");
	const StringHash EXTENSION_TYPE_OTF(".otf");
	const StringHash EXTENSION_TYPE_OGG(".ogg");
	const StringHash EXTENSION_TYPE_WAV(".wav");
	const StringHash EXTENSION_TYPE_DDS(".dds");
	const StringHash EXTENSION_TYPE_PNG(".png");
	const StringHash EXTENSION_TYPE_JPG(".jpg");
	const StringHash EXTENSION_TYPE_JPEG(".jpeg");
	const StringHash EXTENSION_TYPE_TGA(".tga");
	const StringHash EXTENSION_TYPE_OBJ(".obj");
	const StringHash EXTENSION_TYPE_FBX(".fbx");
	const StringHash EXTENSION_TYPE_COLLADA(".dae");
	const StringHash EXTENSION_TYPE_BLEND(".blend");
	const StringHash EXTENSION_TYPE_ANGELSCRIPT(".as");
	const StringHash EXTENSION_TYPE_LUASCRIPT(".lua");
	const StringHash EXTENSION_TYPE_HLSL(".hlsl");
	const StringHash EXTENSION_TYPE_GLSL(".glsl");
	const StringHash EXTENSION_TYPE_FRAGMENTSHADER(".frag");
	const StringHash EXTENSION_TYPE_VERTEXSHADER(".vert");
	const StringHash EXTENSION_TYPE_HTML(".html");

	const StringHash TEXT_VAR_FILE_ID("browser_file_id");
	const StringHash TEXT_VAR_DIR_ID("browser_dir_id");
	const StringHash TEXT_VAR_RESOURCE_TYPE("resource_type");
	const StringHash TEXT_VAR_RESOURCE_DIR_ID("resource_dir_id");

	const int BROWSER_FILE_SOURCE_RESOURCE_DIR = 1;

	unsigned int BrowserFile::browserFileIndex = 1;
	unsigned int BrowserDir::browserDirIndex = 1;

	bool BrowserFileopCmp(BrowserFile* a, BrowserFile* b)
	{
		return a->fullname == (b->fullname);
	}
	bool BrowserFileopCmpScore(BrowserFile* a, BrowserFile* b)
	{
		return a->sortScore > b->sortScore;
	}
	bool ResourceTypeopCmp(ResourceType& a, ResourceType& b)
	{
		return a.name == b.name;
	}
	bool BrowserDiropCmp(BrowserDir* a, BrowserDir* b)
	{
		return a->name == b->name;
	}
	String Join(const Vector<String>& strings, String seperator)
	{
		String ret;
		if (strings.Empty())
			return ret;
		ret.Append(strings[0]);

		for (unsigned i = 1; i < strings.Size(); i++)
		{
			ret.Append(seperator);
			ret.Append(strings[i]);
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	/// get ResourceType functions
	namespace Res
	{
		int GetResourceType(Context* context, String path, StringHash& fileType, ResourceCache* cache = NULL);

		int GetResourceType(StringHash fileType)
		{
			// binary fileTypes
			if (fileType == BINARY_TYPE_SCENE)
				return RESOURCE_TYPE_SCENE;
			else if (fileType == BINARY_TYPE_PACKAGE)
				return RESOURCE_TYPE_UNUSABLE;
			else if (fileType == BINARY_TYPE_COMPRESSED_PACKAGE)
				return RESOURCE_TYPE_UNUSABLE;
			else if (fileType == BINARY_TYPE_ANGLESCRIPT)
				return RESOURCE_TYPE_SCRIPTFILE;
			else if (fileType == BINARY_TYPE_MODEL)
				return RESOURCE_TYPE_MODEL;
			else if (fileType == BINARY_TYPE_SHADER)
				return RESOURCE_TYPE_UNUSABLE;
			else if (fileType == BINARY_TYPE_ANIMATION)
				return RESOURCE_TYPE_ANIMATION;

			// xml fileTypes
			else if (fileType == XML_TYPE_SCENE)
				return RESOURCE_TYPE_SCENE;
			else if (fileType == XML_TYPE_NODE)
				return RESOURCE_TYPE_PREFAB;
			else if (fileType == XML_TYPE_MATERIAL)
				return RESOURCE_TYPE_MATERIAL;
			else if (fileType == XML_TYPE_TECHNIQUE)
				return RESOURCE_TYPE_TECHNIQUE;
			else if (fileType == XML_TYPE_PARTICLEEFFECT)
				return RESOURCE_TYPE_PARTICLEEFFECT;
			else if (fileType == XML_TYPE_PARTICLEEMITTER)
				return RESOURCE_TYPE_PARTICLEEMITTER;
			else if (fileType == XML_TYPE_TEXTURE)
				return RESOURCE_TYPE_TEXTURE;
			else if (fileType == XML_TYPE_ELEMENT)
				return RESOURCE_TYPE_UIELEMENT;
			else if (fileType == XML_TYPE_ELEMENTS)
				return RESOURCE_TYPE_UIELEMENTS;
			else if (fileType == XML_TYPE_ANIMATION_SETTINGS)
				return RESOURCE_TYPE_ANIMATION_SETTINGS;
			else if (fileType == XML_TYPE_RENDERPATH)
				return RESOURCE_TYPE_RENDERPATH;
			else if (fileType == XML_TYPE_TEXTURE_ATLAS)
				return RESOURCE_TYPE_TEXTURE_ATLAS;
			else if (fileType == XML_TYPE_2D_PARTICLE_EFFECT)
				return RESOURCE_TYPE_2D_PARTICLE_EFFECT;
			else if (fileType == XML_TYPE_TEXTURE_3D)
				return RESOURCE_TYPE_TEXTURE_3D;
			else if (fileType == XML_TYPE_CUBEMAP)
				return RESOURCE_TYPE_CUBEMAP;
			else if (fileType == XML_TYPE_SPRITER_DATA)
				return RESOURCE_TYPE_2D_ANIMATION_SET;

			// extension fileTypes
			else if (fileType == EXTENSION_TYPE_TTF)
				return RESOURCE_TYPE_FONT;
			else if (fileType == EXTENSION_TYPE_OTF)
				return RESOURCE_TYPE_FONT;
			else if (fileType == EXTENSION_TYPE_OGG)
				return RESOURCE_TYPE_SOUND;
			else if (fileType == EXTENSION_TYPE_WAV)
				return RESOURCE_TYPE_SOUND;
			else if (fileType == EXTENSION_TYPE_DDS)
				return RESOURCE_TYPE_IMAGE;
			else if (fileType == EXTENSION_TYPE_PNG)
				return RESOURCE_TYPE_IMAGE;
			else if (fileType == EXTENSION_TYPE_JPG)
				return RESOURCE_TYPE_IMAGE;
			else if (fileType == EXTENSION_TYPE_JPEG)
				return RESOURCE_TYPE_IMAGE;
			else if (fileType == EXTENSION_TYPE_TGA)
				return RESOURCE_TYPE_IMAGE;
			else if (fileType == EXTENSION_TYPE_OBJ)
				return RESOURCE_TYPE_UNUSABLE;
			else if (fileType == EXTENSION_TYPE_FBX)
				return RESOURCE_TYPE_UNUSABLE;
			else if (fileType == EXTENSION_TYPE_COLLADA)
				return RESOURCE_TYPE_UNUSABLE;
			else if (fileType == EXTENSION_TYPE_BLEND)
				return RESOURCE_TYPE_UNUSABLE;
			else if (fileType == EXTENSION_TYPE_ANGELSCRIPT)
				return RESOURCE_TYPE_SCRIPTFILE;
			else if (fileType == EXTENSION_TYPE_LUASCRIPT)
				return RESOURCE_TYPE_SCRIPTFILE;
			else if (fileType == EXTENSION_TYPE_HLSL)
				return RESOURCE_TYPE_UNUSABLE;
			else if (fileType == EXTENSION_TYPE_GLSL)
				return RESOURCE_TYPE_UNUSABLE;
			else if (fileType == EXTENSION_TYPE_FRAGMENTSHADER)
				return RESOURCE_TYPE_UNUSABLE;
			else if (fileType == EXTENSION_TYPE_VERTEXSHADER)
				return RESOURCE_TYPE_UNUSABLE;
			else if (fileType == EXTENSION_TYPE_HTML)
				return RESOURCE_TYPE_UNUSABLE;

			return RESOURCE_TYPE_UNKNOWN;
		}

		bool GetExtensionType(String path, StringHash& fileType)
		{
			StringHash type = StringHash(GetExtension(path));
			if (type == EXTENSION_TYPE_TTF)
				fileType = EXTENSION_TYPE_TTF;
			else if (type == EXTENSION_TYPE_OGG)
				fileType = EXTENSION_TYPE_OGG;
			else if (type == EXTENSION_TYPE_WAV)
				fileType = EXTENSION_TYPE_WAV;
			else if (type == EXTENSION_TYPE_DDS)
				fileType = EXTENSION_TYPE_DDS;
			else if (type == EXTENSION_TYPE_PNG)
				fileType = EXTENSION_TYPE_PNG;
			else if (type == EXTENSION_TYPE_JPG)
				fileType = EXTENSION_TYPE_JPG;
			else if (type == EXTENSION_TYPE_JPEG)
				fileType = EXTENSION_TYPE_JPEG;
			else if (type == EXTENSION_TYPE_TGA)
				fileType = EXTENSION_TYPE_TGA;
			else if (type == EXTENSION_TYPE_OBJ)
				fileType = EXTENSION_TYPE_OBJ;
			else if (type == EXTENSION_TYPE_FBX)
				fileType = EXTENSION_TYPE_FBX;
			else if (type == EXTENSION_TYPE_COLLADA)
				fileType = EXTENSION_TYPE_COLLADA;
			else if (type == EXTENSION_TYPE_BLEND)
				fileType = EXTENSION_TYPE_BLEND;
			else if (type == EXTENSION_TYPE_ANGELSCRIPT)
				fileType = EXTENSION_TYPE_ANGELSCRIPT;
			else if (type == EXTENSION_TYPE_LUASCRIPT)
				fileType = EXTENSION_TYPE_LUASCRIPT;
			else if (type == EXTENSION_TYPE_HLSL)
				fileType = EXTENSION_TYPE_HLSL;
			else if (type == EXTENSION_TYPE_GLSL)
				fileType = EXTENSION_TYPE_GLSL;
			else if (type == EXTENSION_TYPE_FRAGMENTSHADER)
				fileType = EXTENSION_TYPE_FRAGMENTSHADER;
			else if (type == EXTENSION_TYPE_VERTEXSHADER)
				fileType = EXTENSION_TYPE_VERTEXSHADER;
			else if (type == EXTENSION_TYPE_HTML)
				fileType = EXTENSION_TYPE_HTML;
			else
				return false;

			return true;
		}

		bool GetBinaryType(Context* context, String path, StringHash & fileType, ResourceCache* cache = NULL)
		{
			StringHash type;
			if (cache)
			{
				SharedPtr<File> file = cache->GetFile(path);
				if (file.Null())
					return false;

				if (file->GetSize() == 0)
					return false;

				type = StringHash(file->ReadFileID());
			}
			else
			{
				File file(context);
				if (!file.Open(path))
					return false;

				if (file.GetSize() == 0)
					return false;

				type = StringHash(file.ReadFileID());
			}

			if (type == BINARY_TYPE_SCENE)
				fileType = BINARY_TYPE_SCENE;
			else if (type == BINARY_TYPE_PACKAGE)
				fileType = BINARY_TYPE_PACKAGE;
			else if (type == BINARY_TYPE_COMPRESSED_PACKAGE)
				fileType = BINARY_TYPE_COMPRESSED_PACKAGE;
			else if (type == BINARY_TYPE_ANGLESCRIPT)
				fileType = BINARY_TYPE_ANGLESCRIPT;
			else if (type == BINARY_TYPE_MODEL)
				fileType = BINARY_TYPE_MODEL;
			else if (type == BINARY_TYPE_SHADER)
				fileType = BINARY_TYPE_SHADER;
			else if (type == BINARY_TYPE_ANIMATION)
				fileType = BINARY_TYPE_ANIMATION;
			else
				return false;

			return true;
		}

		bool GetXmlType(Context* context, String path, StringHash & fileType, ResourceCache* cache = NULL)
		{
			String name;
			if (cache)
			{
				XMLFile* xml = cache->GetResource<XMLFile>(path);
				if (xml == NULL)
					return false;

				name = xml->GetRoot().GetName();
			}
			else
			{
				File file(context);
				if (!file.Open(path))
					return false;

				if (file.GetSize() == 0)
					return false;

				XMLFile xml(context);
				if (xml.Load(file))
					name = xml.GetRoot().GetName();
				else
					return false;
			}

			bool found = false;
			if (!name.Empty())
			{
				found = true;
				StringHash type = StringHash(name);
				if (type == XML_TYPE_SCENE)
					fileType = XML_TYPE_SCENE;
				else if (type == XML_TYPE_NODE)
					fileType = XML_TYPE_NODE;
				else if (type == XML_TYPE_MATERIAL)
					fileType = XML_TYPE_MATERIAL;
				else if (type == XML_TYPE_TECHNIQUE)
					fileType = XML_TYPE_TECHNIQUE;
				else if (type == XML_TYPE_PARTICLEEFFECT)
					fileType = XML_TYPE_PARTICLEEFFECT;
				else if (type == XML_TYPE_PARTICLEEMITTER)
					fileType = XML_TYPE_PARTICLEEMITTER;
				else if (type == XML_TYPE_TEXTURE)
					fileType = XML_TYPE_TEXTURE;
				else if (type == XML_TYPE_ELEMENT)
					fileType = XML_TYPE_ELEMENT;
				else if (type == XML_TYPE_ELEMENTS)
					fileType = XML_TYPE_ELEMENTS;
				else if (type == XML_TYPE_ANIMATION_SETTINGS)
					fileType = XML_TYPE_ANIMATION_SETTINGS;
				else if (type == XML_TYPE_RENDERPATH)
					fileType = XML_TYPE_RENDERPATH;
				else if (type == XML_TYPE_TEXTURE_ATLAS)
					fileType = XML_TYPE_TEXTURE_ATLAS;
				else if (type == XML_TYPE_2D_PARTICLE_EFFECT)
					fileType = XML_TYPE_2D_PARTICLE_EFFECT;
				else if (type == XML_TYPE_TEXTURE_3D)
					fileType = XML_TYPE_TEXTURE_3D;
				else if (type == XML_TYPE_CUBEMAP)
					fileType = XML_TYPE_CUBEMAP;
				else if (type == XML_TYPE_SPRITER_DATA)
					fileType = XML_TYPE_SPRITER_DATA;
				else
					found = false;
			}
			return found;
		}

		String ResourceTypeName(int resourceType)
		{
			if (resourceType == RESOURCE_TYPE_UNUSABLE)
				return "Unusable";
			else if (resourceType == RESOURCE_TYPE_UNKNOWN)
				return "Unknown";
			else if (resourceType == RESOURCE_TYPE_NOTSET)
				return "Uninitialized";
			else if (resourceType == RESOURCE_TYPE_SCENE)
				return "Scene";
			else if (resourceType == RESOURCE_TYPE_SCRIPTFILE)
				return "Script File";
			else if (resourceType == RESOURCE_TYPE_MODEL)
				return "Model";
			else if (resourceType == RESOURCE_TYPE_MATERIAL)
				return "Material";
			else if (resourceType == RESOURCE_TYPE_ANIMATION)
				return "Animation";
			else if (resourceType == RESOURCE_TYPE_IMAGE)
				return "Image";
			else if (resourceType == RESOURCE_TYPE_SOUND)
				return "Sound";
			else if (resourceType == RESOURCE_TYPE_TEXTURE)
				return "Texture";
			else if (resourceType == RESOURCE_TYPE_FONT)
				return "Font";
			else if (resourceType == RESOURCE_TYPE_PREFAB)
				return "Prefab";
			else if (resourceType == RESOURCE_TYPE_TECHNIQUE)
				return "Render Technique";
			else if (resourceType == RESOURCE_TYPE_PARTICLEEFFECT)
				return "Particle Effect";
			else if (resourceType == RESOURCE_TYPE_PARTICLEEMITTER)
				return "Particle Emitter";
			else if (resourceType == RESOURCE_TYPE_UIELEMENT)
				return "UI Element";
			else if (resourceType == RESOURCE_TYPE_UIELEMENTS)
				return "UI Elements";
			else if (resourceType == RESOURCE_TYPE_ANIMATION_SETTINGS)
				return "Animation Settings";
			else if (resourceType == RESOURCE_TYPE_RENDERPATH)
				return "Render Path";
			else if (resourceType == RESOURCE_TYPE_TEXTURE_ATLAS)
				return "Texture Atlas";
			else if (resourceType == RESOURCE_TYPE_2D_PARTICLE_EFFECT)
				return "2D Particle Effect";
			else if (resourceType == RESOURCE_TYPE_TEXTURE_3D)
				return "Texture 3D";
			else if (resourceType == RESOURCE_TYPE_CUBEMAP)
				return "Cubemap";
			else if (resourceType == RESOURCE_TYPE_2D_ANIMATION_SET)
				return "2D Animation Set";
			else
				return "";
		}

		int GetResourceType(Context* context, String path)
		{
			StringHash fileType;
			return GetResourceType(context, path, fileType);
		}

		int GetResourceType(Context* context, String path, StringHash & fileType, ResourceCache* cache)
		{
			if (GetExtensionType(path, fileType) || GetBinaryType(context, path, fileType, cache) || GetXmlType(context, path, fileType, cache))
				return GetResourceType(fileType);

			return RESOURCE_TYPE_UNKNOWN;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	/// class : BrowserDir
	BrowserDir::BrowserDir(String path_, ResourceCache* cache, ResourceBrowser* resBrowser)
	{
		resourceKey = path_;
		String parent = GetParentPath(path_);
		name = path_;
		name.Replace(parent, "");
		id = browserDirIndex++;
		cache_ = cache;
		resBrowser_ = resBrowser;
	}

	BrowserDir::~BrowserDir()
	{
		for (unsigned i = 0; i < files.Size(); i++)
		{
			if (files[i])
			{
				delete files[i];
				files[i] = NULL;
			}
		}

		files.Clear();
	}

	int BrowserDir::opCmp(BrowserDir& b)
	{
		return name == b.name;
	}

	//////////////////////////////////////////////////////////////////////////
	/// class : BrowserFile
	BrowserFile* BrowserDir::AddFile(String name, unsigned int resourceSourceIndex, unsigned int sourceType)
	{
		String path = resourceKey + "/" + name;
		BrowserFile* file = new BrowserFile(path, resourceSourceIndex, sourceType, cache_, resBrowser_);
		files.Push(file);
		return file;
	}

	BrowserFile::BrowserFile(String path_, unsigned int resourceSourceIndex_, int sourceType_, ResourceCache* cache, ResourceBrowser* resBrowser)
	{
		sourceType = sourceType_;
		resourceSourceIndex = resourceSourceIndex_;
		resourceKey = path_;
		name = GetFileName(path_);
		extension = GetExtension(path_);
		fullname = GetFileNameAndExtension(path_);
		id = browserFileIndex++;
		cache_ = cache;
		resBrowser_ = resBrowser;
	}

	int BrowserFile::opCmp(BrowserFile& b)
	{
		//if (browserSearchSortMode == 1)
		return fullname== (b.fullname);
	}

	int BrowserFile::opCmpScore(BrowserFile& b)
	{
		return sortScore - b.sortScore;
	}

	Urho3D::String BrowserFile::GetResourceSource()
	{
		if (sourceType == BROWSER_FILE_SOURCE_RESOURCE_DIR)
			return cache_->GetResourceDirs()[resourceSourceIndex];
		else
			return "Unknown";
	}

	Urho3D::String BrowserFile::GetFullPath()
	{
		return String(cache_->GetResourceDirs()[resourceSourceIndex] + resourceKey);
	}

	Urho3D::String BrowserFile::GetPath()
	{
		return resourceKey;
	}

	void BrowserFile::DetermainResourceType()
	{
		resourceType = Res::GetResourceType(cache_->GetContext(), GetFullPath(), fileType, NULL);
		Text* browserFileListRow_ = browserFileListRow.Get();
		if (browserFileListRow_ != NULL)
		{
			resBrowser_->InitializeBrowserFileListRow(browserFileListRow_, this);
		}
	}

	Urho3D::String BrowserFile::ResourceTypeName()
	{
		return Res::ResourceTypeName(resourceType);
	}

	void BrowserFile::FileChanged()
	{
		// 		if (!fileSystem.FileExists(GetFullPath()))
		// 		{
		// 		}
		// 		else
		// 		{
		// 		}
	}

	//////////////////////////////////////////////////////////////////////////
	/// class : ResourceBrowser
	ResourceBrowser::ResourceBrowser(Context* context) : Object(context)
	{
		selectedBrowserDirectory = NULL;
		selectedBrowserFile = NULL;
		browserDragFile = NULL;
		rootDir = NULL;
		cache_ = GetSubsystem<ResourceCache>();
		ui_ = GetSubsystem<UI>();
		fileSystem_ = GetSubsystem<FileSystem>();

		browserSearchSortMode = 0;
		ignoreRefreshBrowserResults = false;
	}

	ResourceBrowser::~ResourceBrowser()
	{
		HashMap<String, BrowserDir*>::Iterator it;

		for (it = browserDirs.Begin(); it != browserDirs.End(); it++)
		{
			if (it->second_)
			{
				delete it->second_;
				it->second_ = NULL;
			}
		}
		browserDirs.Clear();
	}

	void ResourceBrowser::CreateResourceBrowser()
	{
		if (browserWindow != NULL) return;

		CreateResourceBrowserUI();
		InitResourceBrowserPreview();
		RebuildResourceDatabase();

		/// add an Window Menu Item and add it
		EditorView* editorView = GetSubsystem<EditorView>();
		Menu* sceneMenu_ = editorView->GetGetMenuBar()->CreateMenu("Window");
		editorView->GetGetMenuBar()->CreateMenuItem("Window", "Resource Browser", A_SHOWRESOURCE_VAR);
		SubscribeToEvent(editorView->GetGetMenuBar(), E_MENUBAR_ACTION, HANDLER(ResourceBrowser, HandleMenuBarAction));
	}

	void ResourceBrowser::Update()
	{
		if (browserFilesToScan.Size() == 0)
			return;

		int counter = 0;
		bool updateBrowserUI = false;
		BrowserFile* scanItem = browserFilesToScan[0];
		while (counter < BROWSER_WORKER_ITEMS_PER_TICK)
		{
			scanItem->DetermainResourceType();

			// next
			browserFilesToScan.Erase(0);
			if (browserFilesToScan.Size() > 0)
				scanItem = browserFilesToScan[0];
			else
				break;
			counter++;
		}

		if (browserFilesToScan.Size() > 0)
			browserStatusMessage->SetText("Files left to scan: " + String(browserFilesToScan.Size()));
		else
			browserStatusMessage->SetText("Scan complete");
	}

	bool ResourceBrowser::IsVisible()
	{
		return browserWindow->IsVisible();
	}

	void ResourceBrowser::HideResourceBrowserWindow(StringHash eventType, VariantMap& eventData)
	{
		browserWindow->SetVisible(false);
		EditorView* editorView = GetSubsystem<EditorView>();
		editorView->GetLeftFrame()->RemoveTab("Resources");
	}

	bool ResourceBrowser::ShowResourceBrowserWindow()
	{
		browserWindow->SetVisible(true);
		browserWindow->BringToFront();
		ui_->SetFocusElement(browserSearch);

		EditorView* editorView = GetSubsystem<EditorView>();
		unsigned index = editorView->GetLeftFrame()->AddTab("Resources", browserWindow);
		editorView->GetLeftFrame()->SetActiveTab(index);

		return true;
	}

	void ResourceBrowser::ToggleResourceFilterWindow(StringHash eventType, VariantMap& eventData)
	{
		if (browserFilterWindow->IsVisible())
			HideResourceFilterWindow();
		else
			ShowResourceFilterWindow();
	}

	void ResourceBrowser::HideResourceFilterWindow()
	{
		browserFilterWindow->SetVisible(false);
	}

	void ResourceBrowser::ShowResourceFilterWindow()
	{
		int x = browserWindow->GetPosition().x_ + browserWindow->GetWidth();
		int y = browserWindow->GetPosition().y_;
		browserFilterWindow->SetPosition(IntVector2(x, y));
		browserFilterWindow->SetVisible(true);
		browserFilterWindow->BringToFront();
	}

	void ResourceBrowser::RefreshBrowserPreview()
	{
		resourceBrowserPreview->QueueUpdate();
	}

	void ResourceBrowser::ScanResourceDirectories()
	{
		HashMap<String, BrowserDir*>::Iterator it;

		for (it = browserDirs.Begin(); it != browserDirs.End(); it++)
		{
			if (it->second_)
			{
				delete it->second_;
				it->second_ = NULL;
			}
		}
		browserDirs.Clear();
		browserFiles.Clear();
		browserFilesToScan.Clear();

		rootDir = new BrowserDir("", cache_, this);
		browserDirs[""] = rootDir;

		// collect all of the items and sort them afterwards
		for (unsigned int i = 0; i < cache_->GetResourceDirs().Size(); ++i)
		{
			if (activeResourceDirFilters.Find(i) != activeResourceDirFilters.End())
				continue;

			ScanResourceDir(i);
		}
	}

	void ResourceBrowser::ScanResourceDir(unsigned int resourceDirIndex)
	{
		String resourceDir = cache_->GetResourceDirs()[resourceDirIndex];

		ScanResourceDirFiles("", resourceDirIndex);

		Vector<String> dirs;
		fileSystem_->ScanDir(dirs, resourceDir, "*", SCAN_DIRS, true);

		for (unsigned int i = 0; i < dirs.Size(); ++i)
		{
			String path = dirs[i];
			if (path.EndsWith("."))
				continue;

			InitBrowserDir(path);
			ScanResourceDirFiles(path, resourceDirIndex);
		}
	}

	void ResourceBrowser::ScanResourceDirFiles(String path, unsigned int resourceDirIndex)
	{
		String fullPath = cache_->GetResourceDirs()[resourceDirIndex] + path;
		if (!fileSystem_->DirExists(fullPath))
			return;

		BrowserDir* dir = GetBrowserDir(path);

		if (dir == NULL)
			return;

		// get files in directory
		Vector<String> dirFiles;
		fileSystem_->ScanDir(dirFiles, fullPath, "*.*", SCAN_FILES, false);

		// add new files
		for (unsigned int x = 0; x < dirFiles.Size(); x++)
		{
			String filename = dirFiles[x];
			BrowserFile* browserFile = dir->AddFile(filename, resourceDirIndex, BROWSER_FILE_SOURCE_RESOURCE_DIR);
			browserFiles.Push(browserFile);
			browserFilesToScan.Push(browserFile);
		}
	}

	void ResourceBrowser::HandleMenuBarAction(StringHash eventType, VariantMap& eventData)
	{
		using namespace MenuBarAction;

		StringHash action = eventData[P_ACTION].GetStringHash();
		if (action == A_SHOWRESOURCE_VAR)
		{
			ui_->SetFocusElement(NULL);
			ShowResourceBrowserWindow();
		}
	}

	void ResourceBrowser::HandleRescanResourceBrowserClick(StringHash eventType, VariantMap& eventData)
	{
		RebuildResourceDatabase();
	}

	void ResourceBrowser::HandleResourceBrowserDirListSelectionChange(StringHash eventType, VariantMap& eventData)
	{
		if (browserDirList->GetSelection() == M_MAX_UNSIGNED)
			return;

		UIElement* uiElement = browserDirList->GetItems()[browserDirList->GetSelection()];
		BrowserDir* dir = GetBrowserDir(uiElement->GetVar(TEXT_VAR_DIR_ID).GetString());
		if (dir == NULL)
			return;

		PopulateResourceBrowserFilesByDirectory(dir);
	}

	void ResourceBrowser::HandleResourceBrowserFileListSelectionChange(StringHash eventType, VariantMap& eventData)
	{
		if (browserFileList->GetSelection() == M_MAX_UNSIGNED)
			return;

		UIElement* uiElement = browserFileList->GetItems()[browserFileList->GetSelection()];
		BrowserFile* file = GetBrowserFileFromUIElement(uiElement);
		if (file == NULL)
			return;

		if (resourcePreviewNode != NULL)
			resourcePreviewNode->Remove();

		resourcePreviewNode = resourcePreviewScene->CreateChild("PreviewNodeContainer");
		CreateResourcePreview(file->GetFullPath(), resourcePreviewNode);

		if (resourcePreviewNode != NULL)
		{
			Vector<BoundingBox> boxes;

			PODVector<StaticModel*> staticModels;
			resourcePreviewNode->GetComponents<StaticModel>(staticModels, true);
			PODVector<AnimatedModel*> animatedModels;
			resourcePreviewNode->GetComponents<AnimatedModel>(animatedModels, true);

			for (unsigned i = 0; i < staticModels.Size(); ++i)
				boxes.Push(staticModels[i]->GetWorldBoundingBox());

			for (unsigned i = 0; i < animatedModels.Size(); ++i)
				boxes.Push(animatedModels[i]->GetWorldBoundingBox());

			if (boxes.Size() > 0)
			{
				Vector3 camPosition = Vector3(0.0f, 0.0f, -1.2f);
				BoundingBox biggestBox = boxes[0];
				for (unsigned i = 1; i < boxes.Size(); ++i)
				{
					if (boxes[i].Size().Length() > biggestBox.Size().Length())
						biggestBox = boxes[i];
				}
				resourcePreviewCameraNode->SetPosition(biggestBox.Center() + camPosition * biggestBox.Size().Length());
			}

			resourcePreviewScene->AddChild(resourcePreviewNode);
			RefreshBrowserPreview();
		}
	}

	void ResourceBrowser::HandleResourceDirFilterToggled(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* checkbox = dynamic_cast<CheckBox*>(eventData[P_ELEMENT].GetPtr());
		if (!checkbox || !checkbox->GetVars().Contains(TEXT_VAR_RESOURCE_DIR_ID))
			return;

		int resourceDir = checkbox->GetVar(TEXT_VAR_RESOURCE_DIR_ID).GetInt();
		Vector<int>::Iterator find = activeResourceDirFilters.Find(resourceDir);

		if (checkbox->IsChecked() && find != activeResourceDirFilters.End())
			activeResourceDirFilters.Erase(find);
		else if (!checkbox->IsChecked() && find == activeResourceDirFilters.End())
			activeResourceDirFilters.Push(resourceDir);

		if (ignoreRefreshBrowserResults == false)
			RebuildResourceDatabase();
	}

	void ResourceBrowser::HandleResourceBrowserSearchTextChange(StringHash eventType, VariantMap& eventData)
	{
		RefreshBrowserResults();
	}

	void ResourceBrowser::HandleResourceTypeFilterToggled(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* checkbox = dynamic_cast<CheckBox*>(eventData[P_ELEMENT].GetPtr());

		if (!checkbox || !checkbox->GetVars().Contains(TEXT_VAR_RESOURCE_TYPE))
			return;

		int resourceType = checkbox->GetVar(TEXT_VAR_RESOURCE_TYPE).GetInt();
		Vector<int>::Iterator find = activeResourceTypeFilters.Find(resourceType);

		if (checkbox->IsChecked() && find != activeResourceTypeFilters.End())
			activeResourceTypeFilters.Erase(find);
		else if (!checkbox->IsChecked() && find == activeResourceTypeFilters.End())
			activeResourceTypeFilters.Push(resourceType);

		if (ignoreRefreshBrowserResults == false)
			RefreshBrowserResults();
	}

	void ResourceBrowser::HandleHideResourceFilterWindow(StringHash eventType, VariantMap& eventData)
	{
		HideResourceFilterWindow();
	}

	void ResourceBrowser::HandleResourceTypeFilterToggleAllTypesToggled(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* checkbox = dynamic_cast<CheckBox*>(eventData[P_ELEMENT].GetPtr());

		UIElement* filterHolder = browserFilterWindow->GetChild("TypeFilters", true);
		PODVector<UIElement*>  children;
		filterHolder->GetChildren(children, true);

		ignoreRefreshBrowserResults = true;
		for (unsigned i = 0; i < children.Size(); ++i)
		{
			CheckBox* filter = dynamic_cast<CheckBox*>(children[i]);
			if (filter != NULL)
				filter->SetChecked(checkbox->IsChecked());
		}
		ignoreRefreshBrowserResults = false;
		RefreshBrowserResults();
	}

	void ResourceBrowser::HandleResourceDirFilterToggleAllTypesToggled(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* checkbox = dynamic_cast<CheckBox*>(eventData[P_ELEMENT].GetPtr());

		UIElement* filterHolder = browserFilterWindow->GetChild("DirFilters", true);
		PODVector<UIElement*>  children;
		filterHolder->GetChildren(children, true);

		ignoreRefreshBrowserResults = true;
		for (unsigned i = 0; i < children.Size(); ++i)
		{
			CheckBox* filter = dynamic_cast<CheckBox*>(children[i]);
			if (filter != NULL)
				filter->SetChecked(checkbox->IsChecked());
		}
		ignoreRefreshBrowserResults = false;
		RebuildResourceDatabase();
	}

	void ResourceBrowser::HandleBrowserFileDragBegin(StringHash eventType, VariantMap& eventData)
	{
		using namespace DragBegin;
		UIElement* uiElement = (UIElement*)eventData[P_ELEMENT].GetPtr();
		browserDragFile = GetBrowserFileFromUIElement(uiElement);
	}

	void ResourceBrowser::HandleBrowserFileDragEnd(StringHash eventType, VariantMap& eventData)
	{
		if (browserDragFile == NULL)
			return;

		UIElement* element = ui_->GetElementAt(ui_->GetCursor()->GetScreenPosition());
		if (element != NULL)
			return;

		if (browserDragFile->resourceType == RESOURCE_TYPE_MATERIAL)
		{
			/// \todo
			// 			StaticModel* model = dynamic_cast<StaticModel*>(GetDrawableAtMousePostion());
			// 			if (model !is null)
			// 			{
			// 				AssignMaterial(model, browserDragFile.resourceKey);
			// 			}
		}

		browserDragFile = NULL;
		browserDragComponent = NULL;
		browserDragNode = NULL;
	}

	void ResourceBrowser::HandleFileChanged(StringHash eventType, VariantMap& eventData)
	{
		using namespace FileChanged;

		String filename = eventData[P_FILENAME].GetString();
		BrowserFile* file = GetBrowserFileFromPath(filename);

		if (file == NULL)
		{
			// TODO: new file logic when watchers are supported
			return;
		}
		else
		{
			file->FileChanged();
		}
	}

	void ResourceBrowser::RotateResourceBrowserPreview(StringHash eventType, VariantMap& eventData)
	{
		using namespace DragMove;

		int elemX = eventData[P_ELEMENTX].GetInt();
		int elemY = eventData[P_ELEMENTY].GetInt();

		if (resourceBrowserPreview->GetHeight() > 0 && resourceBrowserPreview->GetWidth() > 0)
		{
			float yaw = ((resourceBrowserPreview->GetHeight() / 2) - elemY) * (90.0f / resourceBrowserPreview->GetHeight());
			float pitch = ((resourceBrowserPreview->GetWidth() / 2) - elemX) * (90.0f / resourceBrowserPreview->GetWidth());

			resourcePreviewNode->SetRotation(resourcePreviewNode->GetRotation().Slerp(Quaternion(yaw, pitch, 0.0f), 0.1f));
			RefreshBrowserPreview();
		}
	}

	void ResourceBrowser::HandleBrowserFileClick(StringHash eventType, VariantMap& eventData)
	{
// 		using namespace ItemClicked;
//
// 		if (eventData[P_BUTTON].GetInt() != MOUSEB_RIGHT)
// 			return;
//
// 		UIElement* uiElement = dynamic_cast<UIElement*>(eventData[P_ITEM].GetPtr());
// 		BrowserFile* file = GetBrowserFileFromUIElement(uiElement);
//
// 		if (file == NULL)
// 			return;
//
// 		Vector<UIElement*> actions;
// 		if (file->resourceType == RESOURCE_TYPE_MATERIAL)
// 		{
// 			actions.Push(CreateBrowserFileActionMenu("Edit", "HandleBrowserEditResource", file));
// 		}
// 		else if (file.resourceType == RESOURCE_TYPE_MODEL)
// 		{
// 			actions.Push(CreateBrowserFileActionMenu("Instance Animated Model", "HandleBrowserInstantiateAnimatedModel", file));
// 			actions.Push(CreateBrowserFileActionMenu("Instance Static Model", "HandleBrowserInstantiateStaticModel", file));
// 		}
// 		else if (file.resourceType == RESOURCE_TYPE_PREFAB)
// 		{
// 			actions.Push(CreateBrowserFileActionMenu("Instance Prefab", "HandleBrowserInstantiatePrefab", file));
// 			actions.Push(CreateBrowserFileActionMenu("Instance in Spawner", "HandleBrowserInstantiateInSpawnEditor", file));
// 		}
// 		else if (file.fileType == EXTENSION_TYPE_OBJ ||
// 			file.fileType == EXTENSION_TYPE_COLLADA ||
// 			file.fileType == EXTENSION_TYPE_FBX ||
// 			file.fileType == EXTENSION_TYPE_BLEND)
// 		{
// 			actions.Push(CreateBrowserFileActionMenu("Import Model", "HandleBrowserImportModel", file));
// 			actions.Push(CreateBrowserFileActionMenu("Import Scene", "HandleBrowserImportScene", file));
// 		}
// 		else if (file.resourceType == RESOURCE_TYPE_UIELEMENT)
// 		{
// 			actions.Push(CreateBrowserFileActionMenu("Open UI Layout", "HandleBrowserOpenUILayout", file));
// 		}
// 		else if (file.resourceType == RESOURCE_TYPE_SCENE)
// 		{
// 			actions.Push(CreateBrowserFileActionMenu("Load Scene", "HandleBrowserLoadScene", file));
// 		}
// 		else if (file.resourceType == RESOURCE_TYPE_SCRIPTFILE)
// 		{
// 			actions.Push(CreateBrowserFileActionMenu("Execute Script", "HandleBrowserRunScript", file));
// 		}
// 		else if (file.resourceType == RESOURCE_TYPE_PARTICLEEFFECT)
// 		{
// 			actions.Push(CreateBrowserFileActionMenu("Edit", "HandleBrowserEditResource", file));
// 		}
//
// 		actions.Push(CreateBrowserFileActionMenu("Open", "HandleBrowserOpenResource", file));
//
// 		ActivateContextMenu(actions);
	}

	BrowserDir* ResourceBrowser::GetBrowserDir(String path)
	{
		HashMap<String, BrowserDir*>::Iterator it = browserDirs.Find(path);
		if (it == browserDirs.End())
			return NULL;
		return it->second_;
	}

	BrowserDir* ResourceBrowser::InitBrowserDir(String path)
	{
		BrowserDir* browserDir = NULL;

		HashMap<String, BrowserDir*>::Iterator it = browserDirs.Find(path);
		if (it != browserDirs.End())
			return it->second_;

		Vector<String> parts = path.Split('/');
		Vector<String> finishedParts;
		if (parts.Size() > 0)
		{
			BrowserDir* parent = rootDir;
			for (unsigned int i = 0; i < parts.Size(); ++i)
			{
				finishedParts.Push(parts[i]);
				String currentPath = Join(finishedParts, "/");
				browserDir = GetBrowserDir(currentPath);
				if (browserDir == NULL)
				{
					browserDir = new BrowserDir(currentPath, cache_, this);
					browserDirs[currentPath] = browserDir;
					parent->children.Push(browserDir);
				}
				parent = browserDir;

			}
			return browserDir;
		}
		return NULL;
	}

	BrowserFile* ResourceBrowser::GetBrowserFileFromId(unsigned id)
	{
		if (id == 0)
			return NULL;

		BrowserFile* file;
		for (unsigned i = 0; i < browserFiles.Size(); ++i)
		{
			file = browserFiles[i];
			if (file->id == id) return file;
		}
		return NULL;
	}

	BrowserFile* ResourceBrowser::GetBrowserFileFromUIElement(UIElement* element)
	{
		if (element == NULL || !element->GetVars().Contains(TEXT_VAR_FILE_ID))
			return NULL;
		return GetBrowserFileFromId(element->GetVar(TEXT_VAR_FILE_ID).GetUInt());
	}

	BrowserFile* ResourceBrowser::GetBrowserFileFromPath(String path)
	{
		for (unsigned i = 0; i < browserFiles.Size(); ++i)
		{
			BrowserFile* file = browserFiles[i];
			if (path == file->GetFullPath())
				return file;
		}
		return NULL;
	}

	void ResourceBrowser::CreateResourceBrowserUI()
	{
		browserWindow = ui_->LoadLayout(cache_->GetResource<XMLFile>("UI/IDEResourceBrowser.xml"));
		browserDirList = (ListView*)browserWindow->GetChild("DirectoryList", true);
		browserFileList = (ListView*)browserWindow->GetChild("FileList", true);
		browserSearch = (LineEdit*)browserWindow->GetChild("Search", true);
		browserStatusMessage = (Text*)browserWindow->GetChild("StatusMessage", true);
		browserResultsMessage = (Text*)browserWindow->GetChild("ResultsMessage", true);
		// browserWindow.visible = false;
		//browserWindow.opacity = uiMaxOpacity;

		browserFilterWindow = DynamicCast<Window>(ui_->LoadLayout(cache_->GetResource<XMLFile>("UI/EditorResourceFilterWindow.xml")));
		CreateResourceFilterUI();
		HideResourceFilterWindow();

		int height = Min(ui_->GetRoot()->GetHeight() / 4, 300);
		//browserWindow->SetSize(900, height);
		//browserWindow->SetPosition(35, ui_->GetRoot()->GetHeight() - height - 25);

		/// \todo
		//CloseContextMenu();
	//	ui_->GetRoot()->AddChild(browserWindow);
		ui_->GetRoot()->AddChild(browserFilterWindow);

		EditorView* editorView = GetSubsystem<EditorView>();
		if (editorView)
			editorView->GetLeftFrame()->AddTab("Resources", browserWindow);




		SubscribeToEvent(browserWindow->GetChild("CloseButton", true), E_RELEASED, HANDLER(ResourceBrowser, HideResourceBrowserWindow));
		SubscribeToEvent(browserWindow->GetChild("RescanButton", true), E_RELEASED, HANDLER(ResourceBrowser, HandleRescanResourceBrowserClick));
		SubscribeToEvent(browserWindow->GetChild("FilterButton", true), E_RELEASED, HANDLER(ResourceBrowser, ToggleResourceFilterWindow));
		SubscribeToEvent(browserDirList, E_SELECTIONCHANGED, HANDLER(ResourceBrowser, HandleResourceBrowserDirListSelectionChange));
		SubscribeToEvent(browserSearch, E_TEXTCHANGED, HANDLER(ResourceBrowser, HandleResourceBrowserSearchTextChange));
		SubscribeToEvent(browserFileList, E_ITEMCLICKED, HANDLER(ResourceBrowser, HandleBrowserFileClick));
		SubscribeToEvent(browserFileList, E_SELECTIONCHANGED, HANDLER(ResourceBrowser, HandleResourceBrowserFileListSelectionChange));
		SubscribeToEvent(cache_, E_FILECHANGED, HANDLER(ResourceBrowser, HandleFileChanged));
	}

	void ResourceBrowser::InitResourceBrowserPreview()
	{
		resourcePreviewScene = new  Scene(context_);
		resourcePreviewScene->SetName("PreviewScene");

		resourcePreviewScene->CreateComponent<Octree>();
		PhysicsWorld* physicsWorld = resourcePreviewScene->CreateComponent<PhysicsWorld>();
		physicsWorld->SetEnabled(false);
		physicsWorld->SetGravity(Vector3(0.0f, 0.0f, 0.0f));

		Node* zoneNode = resourcePreviewScene->CreateChild("Zone");
		Zone* zone = zoneNode->CreateComponent<Zone>();
		zone->SetBoundingBox(BoundingBox(-1000, 1000));
		zone->SetAmbientColor(Color(0.15f, 0.15f, 0.15f));
		zone->SetFogColor(Color(0.0f, 0.0f, 0.0f));
		zone->SetFogStart(10.0f);
		zone->SetFogEnd(100.0f);

		resourcePreviewCameraNode = resourcePreviewScene->CreateChild("PreviewCamera");
		resourcePreviewCameraNode->SetPosition(Vector3(0.0f, 0.0f, -1.5f));
		Camera* camera = resourcePreviewCameraNode->CreateComponent<Camera>();
		camera->SetNearClip(0.1f);
		camera->SetFarClip(100.0f);

		resourcePreviewLightNode = resourcePreviewScene->CreateChild("PreviewLight");
		resourcePreviewLightNode->SetDirection(Vector3(0.5f, -0.5f, 0.5f));
		resourcePreviewLight = resourcePreviewLightNode->CreateComponent<Light>();
		resourcePreviewLight->SetLightType(LIGHT_DIRECTIONAL);
		resourcePreviewLight->SetSpecularIntensity(0.5f);

		resourceBrowserPreview = (View3D*)browserWindow->GetChild("ResourceBrowserPreview", true);
		resourceBrowserPreview->SetFixedHeight(200);
		resourceBrowserPreview->SetFixedWidth(266);
		resourceBrowserPreview->SetView(resourcePreviewScene, camera);
		resourceBrowserPreview->SetAutoUpdate(false);

		resourcePreviewNode = resourcePreviewScene->CreateChild("PreviewNodeContainer");

		SubscribeToEvent(resourceBrowserPreview, E_DRAGMOVE, HANDLER(ResourceBrowser, RotateResourceBrowserPreview));

		RefreshBrowserPreview();
	}

	void ResourceBrowser::InitializeBrowserFileListRow(Text* fileText, BrowserFile* file)
	{
		fileText->RemoveAllChildren();
		VariantMap params = VariantMap();
		fileText->SetVar(TEXT_VAR_FILE_ID, file->id);
		fileText->SetVar(TEXT_VAR_RESOURCE_TYPE, file->resourceType);
		if (file->resourceType > 0)
			fileText->SetDragDropMode(DD_SOURCE);

		{
			Text* text = new Text(context_);
			fileText->AddChild(text);
			text->SetStyle("FileSelectorListText");
			text->SetText(file->fullname);
			text->SetName(String(file->resourceKey));
		}

	{
		Text* text = new Text(context_);
		fileText->AddChild(text);
		text->SetStyle("FileSelectorListText");
		text->SetText(file->ResourceTypeName());
	}

	if (file->resourceType == RESOURCE_TYPE_MATERIAL ||
		file->resourceType == RESOURCE_TYPE_MODEL ||
		file->resourceType == RESOURCE_TYPE_PARTICLEEFFECT ||
		file->resourceType == RESOURCE_TYPE_PREFAB
		)
	{
		SubscribeToEvent(fileText, E_DRAGBEGIN, HANDLER(ResourceBrowser, HandleBrowserFileDragBegin));
		SubscribeToEvent(fileText, E_DRAGEND, HANDLER(ResourceBrowser, HandleBrowserFileDragEnd));
	}
	}

	void ResourceBrowser::RebuildResourceDatabase()
	{
		if (browserWindow.Null())
			return;

		String newResourceDirsCache = Join(cache_->GetResourceDirs(), ";");
		ScanResourceDirectories();
		if (newResourceDirsCache != resourceDirsCache)
		{
			resourceDirsCache = newResourceDirsCache;
			PopulateResourceDirFilters();
		}
		PopulateBrowserDirectories();
		PopulateResourceBrowserFilesByDirectory(rootDir);
	}

	void ResourceBrowser::RefreshBrowserResults()
	{
		if (browserSearch->GetText().Empty())
		{
			browserDirList->SetVisible(true);
			PopulateResourceBrowserFilesByDirectory(selectedBrowserDirectory);
		}
		else
		{
			browserDirList->SetVisible(false);
			PopulateResourceBrowserBySearch();
		}
	}

	void ResourceBrowser::CreateResourceFilterUI()
	{
		UIElement* options = browserFilterWindow->GetChild("TypeOptions", true);
		CheckBox* toggleAllTypes = (CheckBox*)browserFilterWindow->GetChild("ToggleAllTypes", true);
		CheckBox* toggleAllResourceDirs = (CheckBox*)browserFilterWindow->GetChild("ToggleAllResourceDirs", true);

		SubscribeToEvent(toggleAllTypes, E_TOGGLED, HANDLER(ResourceBrowser, HandleResourceTypeFilterToggleAllTypesToggled));
		SubscribeToEvent(toggleAllResourceDirs, E_TOGGLED, HANDLER(ResourceBrowser, HandleResourceDirFilterToggleAllTypesToggled));
		SubscribeToEvent(browserFilterWindow->GetChild("CloseButton", true), E_RELEASED, HANDLER(ResourceBrowser, HandleHideResourceFilterWindow));

		int columns = 2;
		UIElement* col1 = browserFilterWindow->GetChild("TypeFilterColumn1", true);
		UIElement* col2 = browserFilterWindow->GetChild("TypeFilterColumn2", true);

		// use array to get sort of items
		Vector<ResourceType> sorted;
		for (int i = 1; i <= NUMBER_OF_VALID_RESOURCE_TYPES; ++i)
			sorted.Push(ResourceType(i, Res::ResourceTypeName(i)));
		/// \todo
		// 2 unknown types are reserved for the top, the rest are alphabetized
		//sorted.Sort();
		Sort(sorted.Begin(), sorted.End(), ResourceTypeopCmp);
		sorted.Insert(0, ResourceType(RESOURCE_TYPE_UNKNOWN, Res::ResourceTypeName(RESOURCE_TYPE_UNKNOWN)));
		sorted.Insert(0, ResourceType(RESOURCE_TYPE_UNUSABLE, Res::ResourceTypeName(RESOURCE_TYPE_UNUSABLE)));
		int halfColumns = (int)ceil(float(sorted.Size()) / float(columns));

		for (unsigned int i = 0; i < sorted.Size(); ++i)
		{
			ResourceType& type = sorted[i];
			UIElement* resourceTypeHolder = new UIElement(context_);
			if (i < (unsigned)halfColumns)
				col1->AddChild(resourceTypeHolder);
			else
				col2->AddChild(resourceTypeHolder);

			resourceTypeHolder->SetLayoutMode(LM_HORIZONTAL);
			resourceTypeHolder->SetLayoutSpacing(4);
			Text* label = new Text(context_);
			label->SetStyle("EditorAttributeText");
			label->SetText(type.name);
			CheckBox* checkbox = new CheckBox(context_);
			checkbox->SetName(String(type.id));
			checkbox->SetStyleAuto();
			checkbox->SetVar(TEXT_VAR_RESOURCE_TYPE, i);
			checkbox->SetChecked(true);
			SubscribeToEvent(checkbox, E_TOGGLED, HANDLER(ResourceBrowser, HandleResourceTypeFilterToggled));

			resourceTypeHolder->AddChild(checkbox);
			resourceTypeHolder->AddChild(label);
		}
	}

	void ResourceBrowser::CreateDirList(BrowserDir* dir, UIElement* parentUI /*= null*/)
	{
		Text* dirText = new Text(context_);
		browserDirList->InsertItem(browserDirList->GetNumItems(), dirText, parentUI);
		dirText->SetStyle("FileSelectorListText");
		dirText->SetText(dir->resourceKey.Empty() ? "Root" : dir->name);
		dirText->SetName(dir->resourceKey);
		dirText->SetVar(TEXT_VAR_DIR_ID, dir->resourceKey);

		// Sort directories alphetically
		browserSearchSortMode = BROWSER_SORT_MODE_ALPHA;
		/// \todo
		//dir->children.Sort();
		Sort(dir->children.Begin(), dir->children.End(), BrowserDiropCmp);

		for (unsigned i = 0; i < dir->children.Size(); ++i)
			CreateDirList(dir->children[i], dirText);
	}

	void ResourceBrowser::CreateFileList(BrowserFile* file)
	{
		Text* fileText = new Text(context_);
		fileText->SetStyle("FileSelectorListText");
		fileText->SetLayoutMode(LM_HORIZONTAL);
		browserFileList->InsertItem(browserFileList->GetNumItems(), fileText);
		file->browserFileListRow = fileText;
		InitializeBrowserFileListRow(fileText, file);
	}

	void ResourceBrowser::CreateResourcePreview(String path, Node* previewNode)
	{
		resourceBrowserPreview->SetAutoUpdate(false);
		int resourceType = Res::GetResourceType(context_,path);
		if (resourceType > 0)
		{
			File file(context_);
			file.Open(path);

			if (resourceType == RESOURCE_TYPE_MODEL)
			{
				Model* model = new Model(context_);
				if (model->Load(file))
				{
					StaticModel* staticModel = previewNode->CreateComponent<StaticModel>();
					staticModel->SetModel(model);
					return;
				}
			}
			else if (resourceType == RESOURCE_TYPE_MATERIAL)
			{
				Material* material = new Material(context_);
				/// \todo i get an error if i dont cast it to Resource ... :/
				if (((Resource*)material)->Load(file))
				{
					StaticModel* staticModel = previewNode->CreateComponent<StaticModel>();
					staticModel->SetModel(cache_->GetResource<Model>("Models/Sphere.mdl"));
					staticModel->SetMaterial(material);
					return;
				}
			}
			else if (resourceType == RESOURCE_TYPE_IMAGE)
			{
				SharedPtr<Image> image(new Image(context_));
				if (image->Load(file))
				{
					StaticModel* staticModel = previewNode->CreateComponent<StaticModel>();
					staticModel->SetModel(cache_->GetResource<Model>("Models/Editor/ImagePlane.mdl"));
					Material* material = cache_->GetResource<Material>("Materials/Editor/TexturedUnlit.xml");
					SharedPtr<Texture2D> texture(new Texture2D(context_));
					texture->SetData(image, true);
					material->SetTexture(TU_DIFFUSE, texture);
					staticModel->SetMaterial(material);
					return;
				}
			}
			else if (resourceType == RESOURCE_TYPE_PREFAB)
			{
				bool loaded = false;
				if (GetExtension(path) == ".xml")
				{
					XMLFile xmlFile(context_);
					if (xmlFile.Load(file))
						loaded = previewNode->LoadXML(xmlFile.GetRoot(), true);
				}
				else
				{
					loaded = previewNode->Load(file, true);
				}

				PODVector<StaticModel*> statDest;
				previewNode->GetComponents<StaticModel>(statDest, true);
				PODVector<AnimatedModel*> animDest;
				previewNode->GetComponents<AnimatedModel>(animDest, true);

				if (loaded && (statDest.Size() > 0 || animDest.Size() > 0))
					return;

				previewNode->RemoveAllChildren();
				previewNode->RemoveAllComponents();
			}
			else if (resourceType == RESOURCE_TYPE_PARTICLEEFFECT)
			{
				SharedPtr<ParticleEffect> particleEffect(new ParticleEffect(context_));
				/// \todo i get an error if i dont cast it to Resource ... :/
				if (((Resource*)particleEffect)->Load(file))
				{
					ParticleEmitter* particleEmitter = previewNode->CreateComponent<ParticleEmitter>();
					particleEmitter->SetEffect(particleEffect);
					particleEffect->SetActiveTime(0.0f);
					particleEmitter->Reset();
					resourceBrowserPreview->SetAutoUpdate(true);
					return;
				}
			}
		}

		StaticModel* staticModel = previewNode->CreateComponent<StaticModel>();
		staticModel->SetModel(cache_->GetResource<Model>("Models/Editor/ImagePlane.mdl"));
		Material* material = cache_->GetResource<Material>("Materials/Editor/TexturedUnlit.xml");
		SharedPtr<Texture2D> texture(new Texture2D(context_));
		SharedPtr<Image> noPreviewImage(cache_->GetResource<Image>("Textures/Editor/NoPreviewAvailable.png"));
		texture->SetData(noPreviewImage, false);
		material->SetTexture(TU_DIFFUSE, texture);
		staticModel->SetMaterial(material);

		return;
	}

	void ResourceBrowser::PopulateResourceDirFilters()
	{
		UIElement* resourceDirs = browserFilterWindow->GetChild("DirFilters", true);
		resourceDirs->RemoveAllChildren();
		activeResourceDirFilters.Clear();
		for (unsigned i = 0; i < cache_->GetResourceDirs().Size(); ++i)
		{
			UIElement* resourceDirHolder = new UIElement(context_);
			resourceDirs->AddChild(resourceDirHolder);
			resourceDirHolder->SetLayoutMode(LM_HORIZONTAL);
			resourceDirHolder->SetLayoutSpacing(4);
			resourceDirHolder->SetFixedHeight(16);

			Text* label = new Text(context_);
			label->SetStyle("EditorAttributeText");
			label->SetText(cache_->GetResourceDirs()[i].Replaced(fileSystem_->GetProgramDir(), ""));
			CheckBox* checkbox = new CheckBox(context_);
			checkbox->SetName(String(i));
			checkbox->SetStyleAuto();
			checkbox->SetVar(TEXT_VAR_RESOURCE_DIR_ID, i);
			checkbox->SetChecked(true);
			SubscribeToEvent(checkbox, E_TOGGLED, HANDLER(ResourceBrowser, HandleResourceDirFilterToggled));

			resourceDirHolder->AddChild(checkbox);
			resourceDirHolder->AddChild(label);
		}
	}

	void ResourceBrowser::PopulateBrowserDirectories()
	{
		browserDirList->RemoveAllItems();
		CreateDirList(rootDir);
		browserDirList->SetSelection(0);
	}

	void ResourceBrowser::PopulateResourceBrowserFilesByDirectory(BrowserDir* dir)
	{
		selectedBrowserDirectory = dir;
		browserFileList->RemoveAllItems();
		if (dir == NULL)
			return;

		Vector<BrowserFile*> files;
		for (unsigned x = 0; x < dir->files.Size(); x++)
		{
			BrowserFile* file = dir->files[x];

			if (activeResourceTypeFilters.Find(file->resourceType) == activeResourceTypeFilters.End())
				files.Push(file);
		}

		// Sort alphetically
		browserSearchSortMode = BROWSER_SORT_MODE_ALPHA;
		/// \todo
		Sort(files.Begin(), files.End(), BrowserFileopCmp);

		PopulateResourceBrowserResults(files);
		browserResultsMessage->SetText("Showing " + String(files.Size()) + " files");
	}

	void ResourceBrowser::PopulateResourceBrowserResults(Vector<BrowserFile*>& files)
	{
		browserFileList->RemoveAllItems();
		for (unsigned i = 0; i < files.Size(); ++i)
			CreateFileList(files[i]);
	}

	void ResourceBrowser::PopulateResourceBrowserBySearch()
	{
		String query = browserSearch->GetText();

		Vector<int> scores;
		Vector<BrowserFile*> scored;
		Vector<BrowserFile*> filtered;
		{
			BrowserFile* file = NULL;
			for (unsigned x = 0; x < browserFiles.Size(); x++)
			{
				file = browserFiles[x];
				file->sortScore = -1;
				if (activeResourceTypeFilters.Find(file->resourceType) == activeResourceTypeFilters.End())
					continue;

				if (activeResourceDirFilters.Find(file->resourceSourceIndex) == activeResourceDirFilters.End())
					continue;

				int find = file->fullname.Find(query, 0, false);
				if (find > -1)
				{
					int fudge = query.Length() - file->fullname.Length();
					int score = find * int(Abs(fudge * 2)) + int(Abs(fudge));
					file->sortScore = score;
					scored.Push(file);
					scores.Push(score);
				}
			}
		}

		// cut this down for a faster sort
		if (scored.Size() > BROWSER_SEARCH_LIMIT)
		{
			/// \todo
			//scores.Sort();
			Sort(scores.Begin(), scores.End());
			int scoreThreshold = scores[BROWSER_SEARCH_LIMIT];
			BrowserFile* file;
			for (unsigned x = 0; x < scored.Size(); x++)
			{
				file = scored[x];
				if (file->sortScore <= scoreThreshold)
					filtered.Push(file);
			}
		}
		else
			filtered = scored;

		browserSearchSortMode = BROWSER_SORT_MODE_ALPHA;
		/// \todo
		//filtered.Sort();
		if (browserSearchSortMode == 1)
			Sort(filtered.Begin(), filtered.End(), BrowserFileopCmp);
		else
			Sort(filtered.Begin(), filtered.End(), BrowserFileopCmpScore);

		PopulateResourceBrowserResults(filtered);
		browserResultsMessage->SetText("Showing top " + String(filtered.Size()) + " of " + String(scored.Size()) + " results");
	}
}
