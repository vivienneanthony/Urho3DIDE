/*!
 * \file ProjectEditor.h
 *
 * \author vitali
 * \date Februar 2015
 *
 *
 */

#pragma once
#include "../Core/Object.h"

namespace Urho3D
{
	class ProjectSettings;
	class UIElement;
	class Window;
	class ProjectManager;
	class ListView;
	class ResourceCache;
	class FileSystem;


	class ProjectWindow : public Object
	{
		OBJECT(ProjectWindow);
	public:
		ProjectWindow(Context* context);
		virtual ~ProjectWindow();
		static void RegisterObject(Context* context);

		UIElement*	Create();
		void		Show(bool value);

		void UpdateWindow();
		void UnloadProject();

		void				SetProject(ProjectSettings* prj);
		ProjectSettings*	GetProject();
		UIElement*			GetUI();

	protected:
		void HandleOpenProject(StringHash eventType, VariantMap& eventData);

		void AddFolderItems(const String& path, const String& folder, UIElement* parentItem = 0);

		SharedPtr<ProjectSettings> project_;
		SharedPtr<Window> projectwindow_;
		SharedPtr<ListView> projectResList_;
		ProjectManager* prjMng_;

		ResourceCache* cache_;
		FileSystem* fileSystem_;
	private:
	};
}
