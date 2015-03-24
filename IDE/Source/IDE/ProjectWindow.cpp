#include "../Urho3D.h"
#include "../Core/Context.h"
#include "ProjectWindow.h"
#include "../UI/Window.h"
#include "../Resource/XMLFile.h"
#include "../Resource/ResourceCache.h"
#include "ProjectManager.h"
#include "../UI/ListView.h"
#include "../UI/Text.h"
#include "../IO/FileSystem.h"



namespace Urho3D
{


	void ProjectWindow::RegisterObject(Context* context)
	{
		context->RegisterFactory<ProjectWindow>();
	}

	ProjectWindow::~ProjectWindow()
	{

	}

	ProjectWindow::ProjectWindow(Context* context) :Object(context),
		prjMng_(NULL)
	{

	}

	void ProjectWindow::SetProject(ProjectSettings* prj)
	{
		if (project_ != prj)
		{
			UnloadProject();
			project_ = prj;
			if (project_)
				UpdateWindow();
		}

	}

	ProjectSettings* ProjectWindow::GetProject()
	{
		return project_;
	}

	UIElement* ProjectWindow::Create()
	{
		prjMng_ = GetSubsystem<ProjectManager>();
		cache_ = GetSubsystem<ResourceCache>();
		fileSystem_ = GetSubsystem<FileSystem>();


		XMLFile* styleFile = cache_->GetResource<XMLFile>("UI/IDEStyle.xml");

		projectwindow_ = new Window(context_);
		projectwindow_->SetName("projectwindow");
		projectwindow_->LoadXML(cache_->GetResource<XMLFile>("UI/IDEProjectWindow.xml")->GetRoot(), styleFile);

		projectResList_ = dynamic_cast<ListView*>(projectwindow_->GetChild("ProjectResList", true));

		SubscribeToEvent(E_OPENPROJECT, HANDLER(ProjectWindow, HandleOpenProject));

		return projectwindow_;
	}

	void ProjectWindow::Show(bool value)
	{
		projectwindow_->SetVisible(value);
		projectwindow_->SetEnabled(value);
	}

	void ProjectWindow::HandleOpenProject(StringHash eventType, VariantMap& eventData)
	{
		SetProject(prjMng_->GetProject());
	}

	void ProjectWindow::UpdateWindow()
	{
		FileSystem* fileSystem = GetSubsystem<FileSystem>();
		Text* b = NULL;
		b = dynamic_cast<Text*>(projectwindow_->GetChild("ProjectName", true));
		if (b)
			b->SetText(project_->name_);

		ResourceCache* cache = GetSubsystem<ResourceCache>();
		Vector<String> folders = project_->resFolders_.Split(';');

		// Whenever we're updating, disable layout update to optimize speed
		projectResList_->GetContentElement()->DisableLayoutUpdate();

		for (unsigned i = 0; i < folders.Size(); i++)
		{
			String p = project_->path_ + "/" + folders[i];
			p.Replace('//', '/');
			if (cache->AddResourceDir(p))
			{
				AddFolderItems(p, folders[i]);
			}		
		}

		// Re-enable layout update (and do manual layout) now
		projectResList_->GetContentElement()->EnableLayoutUpdate();
		projectResList_->GetContentElement()->UpdateLayout();
	}

	void ProjectWindow::UnloadProject()
	{
		if (project_)
		{
			ResourceCache* cache = GetSubsystem<ResourceCache>();
			Vector<String> folders = project_->resFolders_.Split(';');

			for (unsigned i = 0; i < folders.Size(); i++)
			{
				String p = project_->path_ + "/" + folders[i];
				p.Replace('//', '/');
				cache->RemoveResourceDir(p);			
			}
			projectResList_->RemoveAllItems();

			Text* b = NULL;
			b = dynamic_cast<Text*>(projectwindow_->GetChild("ProjectName", true));
			if (b)
				b->SetText("NoProject");
			
		}
	}

	void ProjectWindow::AddFolderItems(const String& path, const String& folder, UIElement* parentItem )
	{
		if (!parentItem)
		{	
			Text* ptext = new Text(context_);
			ptext->SetStyle("FileSelectorListText");
			ptext->SetText("[DIR]" + folder);
			projectResList_->AddItem(ptext);
			parentItem = ptext;
		}

		Vector<String> dirResult;
		fileSystem_->ScanDir(dirResult, path, "*", SCAN_DIRS, false);

		for (unsigned i = 0; i < dirResult.Size(); i++)
		{
			if (dirResult[i] == ".." || dirResult[i] == ".")
				continue;

			Text* text = new Text(context_);
			text->SetStyle("FileSelectorListText");
			text->SetText("[DIR]" + dirResult[i]);
			projectResList_->InsertItem(M_MAX_UNSIGNED, text, parentItem);

			AddFolderItems(path + "/" + dirResult[i], dirResult[i], text);
			
		}


		Vector<String> fileResult;
		fileSystem_->ScanDir(fileResult, path, "*", SCAN_FILES, false);

		for (unsigned i = 0; i < fileResult.Size(); i++)
		{
			if (fileResult[i] == ".." || fileResult[i] == ".")
				continue;

			Text* text = new Text(context_);
			text->SetStyle("FileSelectorListText");
			text->SetText(fileResult[i]);
			projectResList_->InsertItem(M_MAX_UNSIGNED, text, parentItem);
			
		}
	}

	UIElement* ProjectWindow::GetUI()
	{
		return projectwindow_;
	}

}
