#include "../Urho3D.h"


#include "ProjectManager.h"

#include "../Core/Context.h"
#include "../Graphics/Texture2D.h"
#include "../Resource/ResourceCache.h"
#include "../Math/Color.h"
#include "../UI/UIElement.h"
#include "../UI/Window.h"
#include "../Graphics/Graphics.h"
#include "../UI/UI.h"
#include "../UI/UIEvents.h"
#include "../IO/FileSystem.h"
#include "../UI/ListView.h"
#include "../UI/Text.h"
#include "../UI/Button.h"
#include "../Resource/XMLFile.h"
#include "../Resource/ResourceCache.h"
#include "../Graphics/Texture2D.h"
#include "../UI/BorderImage.h"
#include "../Graphics/Texture.h"
#include "../Scene/Serializable.h"
#include "../Container/Str.h"
#include "../Core/Context.h"
#include "Utils/Helpers.h"
#include "UI/DirSelector.h"
#include "UI/ModalWindow.h"
#include "UI/AttributeContainer.h"
#include "UI/UIGlobals.h"
#include "TemplateManager.h"
#include "../UI/Font.h"
#include "../IO/Log.h"
#include "../Core/ProcessUtils.h"
#include "../IO/File.h"


namespace Urho3D
{
	void ProjectManager::RegisterObject(Context* context)
	{
		context->RegisterFactory<ProjectManager>();
	}

	void ProjectManager::SetProjectRootFolder(const String& path)
	{
		projectsRootDir_ = path;
	}

	ProjectManager::~ProjectManager()
	{
		SAFE_DELETE(dirSelector_);
	}

	ProjectManager::ProjectManager(Context* context) :Object(context)
	{
		cache_ = GetSubsystem<ResourceCache>();
		graphics_ = GetSubsystem<Graphics>();
		ui_ = GetSubsystem<UI>();

		dirSelector_ = NULL;
		selectedtemplate_ = NULL;
	}

	void ProjectManager::NewProject()
	{
		if (newProject_ && selectedtemplate_)
		{
			FileSystem* fileSystem = GetSubsystem<FileSystem>();

			String projectdir = projectsRootDir_ + newProject_->name_;
			String templatedir = selectedtemplate_->path_;
			if (templatedir.EndsWith("/") )
			{
				templatedir.Erase(templatedir.Length() - 1);
			}

			if (!fileSystem->DirExists(projectdir))
			{
				int  ret = -1;
				///  \todo
#ifdef _WINDOWS

				String command = "xcopy " + GetNativePath(templatedir) + " " + GetNativePath(projectdir) + " /e /i /h";
				//int  ret = fileSystem->SystemCommand(command, true);
				ret  = system(command.CString());
				LOGINFOF("%s", command.CString());
				if (!ret)
				{
					String datapath = templateManager_->GetTemplatesPath() +"Data";

					command = "xcopy " + GetNativePath(datapath) + " " + GetNativePath(projectdir + "/Data") + " /e /i /h";
					ret = system(command.CString());

					String datacorepath = templateManager_->GetTemplatesPath() + "CoreData";
					command = "xcopy " + GetNativePath(datacorepath) + " " + GetNativePath(projectdir + "/CoreData") + " /e /i /h";
					ret = system(command.CString());
				}
#endif
				if (ret)
					LOGERRORF("Could not create Folder %s", projectdir.CString());
				else
				{
					UpdateProjects(projectsRootDir_);
					selectedProject_ = newProject_;


					FileSystem* fileSystem = GetSubsystem<FileSystem>();

					File saveFile(context_, projectdir + "/Urho3DProject.xml", FILE_WRITE);
					XMLFile xmlFile(context_);
					XMLElement rootElem = xmlFile.CreateRoot("Urho3DProject");
					selectedProject_->SaveXML(rootElem);
					xmlFile.Save(saveFile);

					newProject_ = NULL;
					templateSlectedText_ = NULL;
					Text* b = NULL;
					b = dynamic_cast<Text*>(welcomeUI_->GetChild("OpenText", true));
					if (b)
						b->SetText(selectedProject_->name_);
				}

			}
			else
				LOGERRORF("Project %s does already exists", newProject_->name_.CString());

			selectedtemplate_ = NULL;
		}

}

	bool ProjectManager::ChooseRootDir()
	{
		CreateDirSelector("Choose Project Root Dir", "Open", "Cancel", projectsRootDir_);

		SubscribeToEvent(dirSelector_, E_FILESELECTED, HANDLER(ProjectManager, HandleRootSelected));

		return false;
	}

	bool ProjectManager::OpenProject(const String& dir)
	{
		if (dir.Empty())
			return false;
		/// \todo open project and set selected project
		// Open Project event
		//SendEvent(E_OPENPROJECT);
		return true;
	}

	bool ProjectManager::OpenProject(const ProjectSettings* proj)
	{
		if (proj == NULL)
			return false;
		// Open Project event
		SendEvent(E_OPENPROJECT);
		return true;
	}

	void ProjectManager::CreateDirSelector(const String& title, const String& ok, const String& cancel, const String& initialPath)
	{
		// Within the editor UI, the file selector is a kind of a "singleton". When the previous one is overwritten, also
		// the events subscribed from it are disconnected, so new ones are safe to subscribe.
		dirSelector_ = new  DirSelector(context_);
		dirSelector_->SetDefaultStyle(cache_->GetResource<XMLFile>("UI/DefaultStyle.xml"));
		dirSelector_->SetTitle(title);
		dirSelector_->SetPath(initialPath);
		dirSelector_->SetButtonTexts(ok, cancel);
		dirSelector_->SetDirectoryMode(true);

		IntVector2 size = dirSelector_->GetWindow()->GetSize();
		dirSelector_->GetWindow()->SetPosition((graphics_->GetWidth() - size.x_) / 2, (graphics_->GetHeight() - size.y_) / 2);
	}

	void ProjectManager::HandleRootSelected(StringHash eventType, VariantMap& eventData)
	{
		using namespace FileSelected;
		CloseDirSelector();

		// Check for OK
		if (eventData[P_OK].GetBool())
		{
			String fileName = eventData[P_FILENAME].GetString();
			UpdateProjects(fileName);
		}
	}

	void ProjectManager::HandleRescanProjects(StringHash eventType, VariantMap& eventData)
	{
		UpdateProjects(projectsRootDir_);
	}

	void ProjectManager::HandleOnTemplateClick(StringHash eventType, VariantMap& eventData)
	{
		using namespace UIMouseClick;


		UIElement* element = (UIElement*)eventData[P_ELEMENT].GetPtr();
		if (element)
		{
			unsigned index = element->GetVar(PROJECTINDEX_VAR).GetUInt();
			selectedtemplate_ = templateManager_->GetTemplateProjects()[index];
			if (templateSlectedText_)
			{
				templateSlectedText_->SetText("Selected Template: " + selectedtemplate_->name_);
			}

			if (newProject_)
			{
				newProject_->CopyAttr(selectedtemplate_);

				if (newProjectAttrCont_)
					newProjectAttrCont_->SetSerializableAttributes(newProject_);
			}

		}
	}

	void ProjectManager::CloseDirSelector(String& path)
	{
		// Save filter & path for next time
		path = dirSelector_->GetPath();
		CloseDirSelector();
	}

	void ProjectManager::CloseDirSelector()
	{
		if (dirSelector_)
		{
			delete dirSelector_;
			dirSelector_ = NULL;
		}
	}

	void ProjectManager::UpdateProjects(const String& dir)
	{
		if (dir.Empty())
			return;

		projectsRootDir_ = dir;

		Text* tt = NULL;
		tt = dynamic_cast<Text*>(welcomeUI_->GetChild("RootPath", true));
		if (tt)
			tt->SetText(dir);

		FileSystem* fileSystem = GetSubsystem<FileSystem>();

		ListView* projectList_ = dynamic_cast<ListView*>(welcomeUI_->GetChild("ProjectListView", true));
		Vector<String> result;

		fileSystem->ScanDir(result, dir, "*", SCAN_DIRS, false);
		projectList_->RemoveAllItems();

		selectedProject_ = NULL;
		projects_.Clear();

		for (unsigned i = 0; i < result.Size(); i++)
		{
			if (result[i] == ".." || result[i] == ".")
				continue;
			String path = dir + result[i];

			if (fileSystem->FileExists(path + "/Urho3DProject.xml"))
			{
				XMLFile* xmlFile = cache_->GetResource<XMLFile>(path + "/Urho3DProject.xml");

				SharedPtr<ProjectSettings> proj(new ProjectSettings(context_));
				proj->LoadXML(xmlFile->GetRoot());
				proj->path_ = path;

				UIElement* panel = new UIElement(context_);
				panel->SetLayout(LM_HORIZONTAL, 4, IntRect(4, 4, 4, 4));
				panel->SetVar(PROJECTINDEX_VAR, projects_.Size());

				BorderImage* imag = new BorderImage(context_);
				Texture2D* iconTexture = cache_->GetResource<Texture2D>(path + "/" + proj->icon_);
				if (!iconTexture)
					iconTexture = cache_->GetResource<Texture2D>("Textures/Logo.png");
				imag->SetTexture(iconTexture);
				panel->AddChild(imag);
				projectList_->AddItem(panel);
				imag->SetFixedHeight(128);
				imag->SetFixedWidth(128);

				UIElement* panelInfo = new UIElement(context_);
				panelInfo->SetLayout(LM_VERTICAL);
				panel->AddChild(panelInfo);

				Text* text = new Text(context_);
				panelInfo->AddChild(text);
				text->SetStyle("FileSelectorListText");
				text->SetText(proj->name_);

				text = new Text(context_);
				panelInfo->AddChild(text);
				text->SetStyle("FileSelectorListText");
				text->SetText(dir + result[i]);

				projects_.Push(proj);
			}
		}
	}

	void ProjectManager::ShowWelcomeScreen(bool value)
	{
		if (welcomeUI_.NotNull())
		{
			if (value)
			{
				welcomeUI_->SetVisible(true);
				welcomeUI_->SetEnabled(true);
				UpdateProjects(projectsRootDir_);
				templateManager_->LoadTemplates();
			}
			else
			{
				welcomeUI_->SetVisible(false);
				welcomeUI_->SetEnabled(false);
			}
			return;
		}
	}

	UIElement* ProjectManager::CreateWelcomeScreen()
	{
		if (welcomeUI_.NotNull())
			return welcomeUI_;

		templateManager_ = new TemplateManager(context_);
		templateManager_->LoadTemplates();
		cache_		= GetSubsystem<ResourceCache>();
		graphics_	= GetSubsystem<Graphics>();
		ui_			= GetSubsystem<UI>();

		XMLFile* styleFile = cache_->GetResource<XMLFile>("UI/IDEStyle.xml");

		welcomeUI_ = new UIElement(context_);
		welcomeUI_->SetDefaultStyle(styleFile);
		welcomeUI_->SetName("WelcomeScreen");
		welcomeUI_->LoadXML(cache_->GetResource<XMLFile>("UI/WelcomeScreen.xml")->GetRoot());

		Button* b = NULL;

		b = dynamic_cast<Button*>(welcomeUI_->GetChild("NewProject", true));
		SubscribeToEvent(b, E_RELEASED, HANDLER(ProjectManager, HandleNewProject));
		b = dynamic_cast<Button*>(welcomeUI_->GetChild("ChangeRoot", true));
		SubscribeToEvent(b, E_RELEASED, HANDLER(ProjectManager, HandleChangeRootDir));
		b = dynamic_cast<Button*>(welcomeUI_->GetChild("OpenProject", true));
		SubscribeToEvent(b, E_RELEASED, HANDLER(ProjectManager, HandleOpenProject));
		b = dynamic_cast<Button*>(welcomeUI_->GetChild("RescanButton", true));
		SubscribeToEvent(b, E_RELEASED, HANDLER(ProjectManager, HandleRescanProjects));

		UpdateProjects(projectsRootDir_);

		ListView*	hierarchyList = dynamic_cast<ListView*>(welcomeUI_->GetChild("ProjectListView", true));

		SubscribeToEvent(hierarchyList, E_ITEMCLICKED, HANDLER(ProjectManager, HandleProjectListClick));

		Text* tt = NULL;
		tt = dynamic_cast<Text*>(welcomeUI_->GetChild("RootPath", true));
		if (tt)
			tt->SetText(projectsRootDir_);

		return welcomeUI_;
	}

	void ProjectManager::HandleNewProject(StringHash eventType, VariantMap& eventData)
	{
		ResourceCache* cache = GetSubsystem<ResourceCache>();
		XMLFile* styleFile = cache_->GetResource<XMLFile>("UI/IDEStyle.xml");
		XMLFile* iconstyle = cache_->GetResource<XMLFile>("UI/IDEIcons.xml");

		UIElement* tempview = new UIElement(context_);
		tempview->SetLayout(LM_VERTICAL, 2);

		ScrollView* TemplateScrollView = new ScrollView(context_);
		TemplateScrollView->SetDefaultStyle(styleFile);
		Font* font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");
		UIElement* templist = new UIElement(context_);
		templist->SetLayout(LM_HORIZONTAL, 4);
		TemplateScrollView->SetContentElement(templist);

		TemplateScrollView->SetStyleAuto();

		templateSlectedText_ = tempview->CreateChild<Text>();
		templateSlectedText_->SetFont(font, 12);
		templateSlectedText_->SetText("Selected Template:");

		for (unsigned i = 0; i < templateManager_->GetTemplateProjects().Size(); i++)
		{
			Button* panel = new Button(context_);
			panel->SetLayout(LM_VERTICAL, 4, IntRect(4, 4, 4, 4));
			panel->SetVar(PROJECTINDEX_VAR, i);

			BorderImage* imag = new BorderImage(context_);
			Texture2D* iconTexture = cache->GetResource<Texture2D>(templateManager_->GetTemplateProjects()[i]->path_ + templateManager_->GetTemplateProjects()[i]->icon_);
			if (!iconTexture)
				iconTexture = cache->GetResource<Texture2D>("Textures/Logo.png");

			imag->SetTexture(iconTexture);
			panel->AddChild(imag);
			templist->AddChild(panel);
			imag->SetFixedHeight(96);
			imag->SetFixedWidth(96);
			panel->SetStyleAuto();
			Text* nametile = panel->CreateChild<Text>();
			nametile->SetFont(font, 12);
			nametile->SetText(templateManager_->GetTemplateProjects()[i]->name_);

			SubscribeToEvent(panel, E_RELEASED, HANDLER(ProjectManager, HandleOnTemplateClick));
		}

		tempview->AddChild(TemplateScrollView);

		newProject_ = new ProjectSettings(context_);

		newProjectAttrCont_ = new AttributeContainer(context_);
		newProjectAttrCont_->SetDefaultStyle(styleFile);
		newProjectAttrCont_->SetTitle("ProjectSettings");
		newProjectAttrCont_->SetSerializableAttributes(newProject_);
		newProjectAttrCont_->GetResetToDefault()->SetVisible(false);
		// Resize the node editor according to the number of variables, up to a certain maximum
		unsigned int maxAttrs = newProjectAttrCont_->GetAttributeList()->GetContentElement()->GetNumChildren();
		newProjectAttrCont_->GetAttributeList()->SetHeight(maxAttrs * ATTR_HEIGHT + 4);
		newProjectAttrCont_->SetFixedWidth(ATTRNAME_WIDTH * 2);
		newProjectAttrCont_->SetStyleAuto(styleFile);

		tempview->AddChild(newProjectAttrCont_);

		SharedPtr<Urho3D::ModalWindow> newProjectWindow(new Urho3D::ModalWindow(context_, tempview, "Create New Project"));

		if (newProjectWindow->GetWindow() != NULL)
		{
			Button* cancelButton = (Button*)newProjectWindow->GetWindow()->GetChild("CancelButton", true);
			cancelButton->SetVisible(true);
			cancelButton->SetFocus(true);
			SubscribeToEvent(newProjectWindow, E_MESSAGEACK, HANDLER(ProjectManager, HandleNewProjectAck));
		}

		newProjectWindow->AddRef();
	}

	void ProjectManager::HandleNewProjectAck(StringHash eventType, VariantMap& eventData)
	{
		using namespace MessageACK;

		bool ok_ = eventData[P_OK].GetBool();

		if (ok_)
			NewProject();

	}

	void ProjectManager::HandleOpenProject(StringHash eventType, VariantMap& eventData)
	{
		if (selectedProject_)
		{
			OpenProject(selectedProject_.Get());
		}

	}

	void ProjectManager::HandleChangeRootDir(StringHash eventType, VariantMap& eventData)
	{
		ChooseRootDir();
	}

	const String& ProjectManager::GetRootDir()
	{
		return projectsRootDir_;
	}

	void ProjectManager::HandleProjectListClick(StringHash eventType, VariantMap& eventData)
	{
		using namespace ItemClicked;
		UIElement* e = (UIElement*)eventData[P_ITEM].GetPtr();

		if (e)
		{
			const Variant& v = e->GetVar(PROJECTINDEX_VAR);
			if (v.IsEmpty())
				return;
			selectedProject_ = projects_[v.GetUInt()];

			Text* b = NULL;
			b = dynamic_cast<Text*>(welcomeUI_->GetChild("OpenText", true));
			if (b)
				b->SetText(selectedProject_->name_);
		}
	}

	ProjectSettings* ProjectManager::GetProject()
	{
		return selectedProject_;
	}

	ProjectSettings::ProjectSettings(Context* context) : Serializable(context),
		icon_("Logo.png"),
		name_("noName"),
		resFolders_(""),
		mainScript_(""),
		mainScene_("")
	{

	}

	ProjectSettings::~ProjectSettings()
	{

	}

	void ProjectSettings::RegisterObject(Context* context)
	{
		context->RegisterFactory<ProjectSettings>();

		ATTRIBUTE("Name", String, name_, String("noName"), AM_FILE);
		ATTRIBUTE("Icon", String, icon_, String::EMPTY, AM_FILE);
		ATTRIBUTE("Resource Folders", String, resFolders_, String::EMPTY, AM_FILE);
		ATTRIBUTE("Main Script", String, mainScript_, String::EMPTY, AM_FILE);
		ATTRIBUTE("Main Scene", String, mainScene_, String::EMPTY, AM_FILE);
	}

	void ProjectSettings::CopyAttr(ProjectSettings* proj)
	{
		if (!proj)
			return;
		name_ = proj->name_;
		icon_ = proj->icon_;
		resFolders_ = proj->resFolders_;
		mainScript_ = proj->mainScript_;
		mainScene_ = proj->mainScene_;
	}

}
