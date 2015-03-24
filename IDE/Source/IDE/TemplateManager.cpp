#include "../Urho3D.h"
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

#include "TemplateManager.h"
#include "../Resource/XMLElement.h"
#include "../IO/Log.h"
#include "ProjectManager.h"
#include "../UI/ScrollView.h"
#include "../UI/Font.h"
#include "../UI/CheckBox.h"


namespace Urho3D
{


	TemplateManager::TemplateManager(Context* context) : Object(context),
		selectedtemplate_(NULL)
	{

	}

	TemplateManager::~TemplateManager()
	{

	}

	void TemplateManager::RegisterObject(Context* context)
	{
		context->RegisterFactory<TemplateManager>();
	}

	bool TemplateManager::LoadTemplates()
	{
		if (!templateProjects_.Empty())
			templateProjects_.Clear();
		selectedtemplate_ = NULL;

		ResourceCache* cache = GetSubsystem<ResourceCache>();

		XMLFile* templatesXML =  cache->GetResource<XMLFile>("Templates/Templates.xml");
		if (!templatesXML)
		{
			LOGERROR("Could not find Templates");
			return false;
		}

		XMLElement root = templatesXML->GetRoot();

		if (!root)
		{
			LOGERROR("Could not find Templates");
			return false;
		}

		XMLElement temp = root.GetChild("Template");
		if (!temp)
		{
			LOGERROR("No Templates are defined in Templates/Templates.xml");
			return false;
		}

		FileSystem* fileSystem = GetSubsystem<FileSystem>();

		String ResourceFileName = cache->GetResourceFileName(templatesXML->GetName());
		String path = GetPath(ResourceFileName);

		while (temp)
		{
			String name = temp.GetAttribute("name");
			String folder = temp.GetAttribute("folder");

			String projPath = path;

			if (folder.StartsWith("/"))
			{
				folder.Erase(0);	
			}
				

			if (!folder.EndsWith("/"))
				folder.Append("/");

			projPath.Append(folder );

			if (fileSystem->FileExists(projPath + "Urho3DProject.xml"))
			{		
				XMLFile* xmlFile = cache->GetResource<XMLFile>(projPath + "Urho3DProject.xml");

				SharedPtr<ProjectSettings> proj(new ProjectSettings(context_));
				proj->LoadXML(xmlFile->GetRoot());
				proj->path_ = projPath;
				templateProjects_.Push(proj);
			}

			temp = temp.GetNext("Template");
		}
		templatesPath_ = path;
		return true;
	}

	ProjectSettings* TemplateManager::GetSelectedTemplate()
	{
		return selectedtemplate_;
	}

	Vector<SharedPtr<ProjectSettings>>& TemplateManager::GetTemplateProjects()
	{
		return templateProjects_;
	}

	UIElement* TemplateManager::GetContainer()
	{
		ResourceCache* cache = GetSubsystem<ResourceCache>();
		XMLFile* styleFile = cache->GetResource<XMLFile>("UI/IDEStyle.xml");

		UIElement* tempview = new UIElement(context_);
		tempview->SetLayout(LM_VERTICAL, 2);

		ScrollView* container = new ScrollView(context_);
		container->SetDefaultStyle(styleFile);
		Font* font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");
		UIElement* templist = new UIElement(context_);
		templist->SetLayout(LM_HORIZONTAL, 4);
		container->SetContentElement(templist);
		
		container->SetStyleAuto();


		slectedText_ = tempview->CreateChild<Text>();
		slectedText_->SetFont(font, 12);
		slectedText_->SetText("Selected Template:");

		for (unsigned i = 0; i < templateProjects_.Size(); i++)
		{
			Button* panel = new Button(context_);
			panel->SetLayout(LM_VERTICAL, 4, IntRect(4, 4, 4, 4));
			panel->SetVar(PROJECTINDEX_VAR, i);

			BorderImage* imag = new BorderImage(context_);
			Texture2D* iconTexture = cache->GetResource<Texture2D>(templateProjects_[i]->path_ + templateProjects_[i]->icon_);
			if (!iconTexture)
				iconTexture = cache->GetResource<Texture2D>("Textures/Logo.png");

			imag->SetTexture(iconTexture);
			panel->AddChild(imag);
			templist->AddChild(panel);
			imag->SetFixedHeight(96);
			imag->SetFixedWidth(96);
			panel->SetStyleAuto();
			Text* nametile = panel->CreateChild<Text>();
			nametile->SetFont(font,12);
			nametile->SetText(templateProjects_[i]->name_);

			SubscribeToEvent(panel, E_RELEASED, HANDLER(TemplateManager, HandleMouseClick));
		}

		tempview->AddChild(container);
		return tempview;
	}

	void TemplateManager::HandleMouseClick(StringHash eventType, VariantMap& eventData)
	{
		using namespace UIMouseClick;


		UIElement* element = (UIElement*)eventData[P_ELEMENT].GetPtr();
		if (element)
		{
			unsigned index = element->GetVar(PROJECTINDEX_VAR).GetUInt();
			selectedtemplate_ = templateProjects_[index];
			if (slectedText_)
			{
				slectedText_->SetText("Selected Template: " + selectedtemplate_->name_);
			}
		}
	}

}