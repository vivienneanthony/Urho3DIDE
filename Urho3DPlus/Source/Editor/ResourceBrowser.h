#pragma once




#include "../Core/Object.h"



namespace Urho3D
{
	class Text;
	class BrowserFile;
	class ResourceCache;
	class FileSystem;
	class UIElement;
	class Window;
	class LineEdit;
	class ListView;
	class Node;
	class Scene;
	class Light;
	class Component;
	class View3D;
	class UI;
	class ResourceBrowser;

	class BrowserDir
	{
	public:
		BrowserDir(String path_, ResourceCache* cache, ResourceBrowser* resBrowser);
		virtual ~BrowserDir();
		int opCmp(BrowserDir& b);

		BrowserFile* AddFile(String name, unsigned int resourceSourceIndex, unsigned int sourceType);

		unsigned int id;
		String resourceKey;
		String name;
		Vector<BrowserDir*> children;
		Vector<BrowserFile*> files;
		static unsigned int browserDirIndex;
		ResourceCache* cache_;
		ResourceBrowser* resBrowser_;
	};

	class BrowserFile
	{
	public:
		BrowserFile(String path_, unsigned int resourceSourceIndex_, int sourceType_, ResourceCache* cache, ResourceBrowser* resBrowser_);
		virtual ~BrowserFile(){}

		int opCmp(BrowserFile& b);
		int opCmpScore(BrowserFile& b);

		String GetResourceSource();

		String GetFullPath();

		String GetPath();

		void DetermainResourceType();

		String ResourceTypeName();

		void FileChanged();


		unsigned int id;
		unsigned int resourceSourceIndex;
		String resourceKey;
		String name;
		String fullname;
		String extension;
		StringHash fileType;
		///int resourceType = 0;
		///int sourceType = 0;
		///int sortScore = 0;
		int resourceType;
		int sourceType;
		int sortScore;
		WeakPtr<Text > browserFileListRow;
		ResourceCache* cache_;
		static unsigned int browserFileIndex;
		ResourceBrowser* resBrowser_;
	};


	class ResourceType
	{
	public:
		int id;
		String name;
		ResourceType(int id_ = 0, String name_= String::EMPTY)
		{
			id = id_;
			name = name_;
		}
		int opCmp(ResourceType& b)
		{
			return name==(b.name);
		}
	};

	class ResourceBrowser : public Object
	{
		OBJECT(ResourceBrowser);
		friend class BrowserDir;
		friend class BrowserFile;
	public:
		/// Construct.
		ResourceBrowser(Context* context);
		/// Destruct.
		virtual ~ResourceBrowser();

		void CreateResourceBrowser();

		/// used to stop ui from blocking while determining file types. imp. of DoResourceBrowserWork
		void Update();
		bool IsVisible();

		void HideResourceBrowserWindow(StringHash eventType, VariantMap& eventData);
		bool ShowResourceBrowserWindow();
		void ToggleResourceFilterWindow(StringHash eventType, VariantMap& eventData);
		void HideResourceFilterWindow();
		void ShowResourceFilterWindow();

		void RefreshBrowserPreview();
		void RebuildResourceDatabase();
		void RefreshBrowserResults();
	protected:
		void InitResourceBrowserPreview();
		void InitializeBrowserFileListRow(Text* fileText, BrowserFile* file);

		void CreateResourceBrowserUI();
		void CreateResourceFilterUI();
		void CreateDirList(BrowserDir* dir, UIElement* parentUI = NULL);
		void CreateFileList(BrowserFile* file);
		void CreateResourcePreview(String path, Node* previewNode);

		void PopulateResourceDirFilters();
		void PopulateBrowserDirectories();
		void PopulateResourceBrowserFilesByDirectory(BrowserDir* dir);
		void PopulateResourceBrowserResults(Vector<BrowserFile*>& files);
		void PopulateResourceBrowserBySearch();

		void ScanResourceDirectories();
		void ScanResourceDir( unsigned int resourceDirIndex);
		void ScanResourceDirFiles(String path, unsigned int resourceDirIndex);


		void HandleMenuBarAction(StringHash eventType, VariantMap& eventData);
		void HandleRescanResourceBrowserClick(StringHash eventType, VariantMap& eventData);
		void HandleResourceBrowserDirListSelectionChange(StringHash eventType, VariantMap& eventData);
		void HandleResourceBrowserFileListSelectionChange(StringHash eventType, VariantMap& eventData);
		void HandleResourceDirFilterToggled(StringHash eventType, VariantMap& eventData);
		void HandleResourceBrowserSearchTextChange(StringHash eventType, VariantMap& eventData);
		void HandleResourceTypeFilterToggled(StringHash eventType, VariantMap& eventData);
		void HandleHideResourceFilterWindow(StringHash eventType, VariantMap& eventData);
		void HandleResourceTypeFilterToggleAllTypesToggled(StringHash eventType, VariantMap& eventData);
		void HandleResourceDirFilterToggleAllTypesToggled(StringHash eventType, VariantMap& eventData);
		void HandleBrowserFileDragBegin(StringHash eventType, VariantMap& eventData);
		void HandleBrowserFileDragEnd(StringHash eventType, VariantMap& eventData);
		void HandleFileChanged(StringHash eventType, VariantMap& eventData);
		// Opens a contextual menu based on what resource item was actioned
		void HandleBrowserFileClick(StringHash eventType, VariantMap& eventData);

		void RotateResourceBrowserPreview(StringHash eventType, VariantMap& eventData);

		BrowserDir* GetBrowserDir(String path);
		/// Makes sure the entire directory tree exists and new dir is linked to parent
		BrowserDir* InitBrowserDir(String path);
		BrowserFile* GetBrowserFileFromId(unsigned id);
		BrowserFile* GetBrowserFileFromUIElement(UIElement* element);
		BrowserFile* GetBrowserFileFromPath(String path);

		ResourceCache* cache_;
		UI* ui_;
		FileSystem* fileSystem_;

		BrowserDir* selectedBrowserDirectory;
		BrowserFile* selectedBrowserFile;
		SharedPtr<Text> browserStatusMessage;
		SharedPtr<Text> browserResultsMessage;
		/*rewrite*/
		bool ignoreRefreshBrowserResults;
		String resourceDirsCache;

		SharedPtr<UIElement> browserWindow;
		SharedPtr<Window> browserFilterWindow;
		SharedPtr<ListView>  browserDirList;
		SharedPtr<ListView> browserFileList;
		SharedPtr<LineEdit> browserSearch;
		BrowserFile* browserDragFile;
		SharedPtr<Node> browserDragNode;
		SharedPtr<Component> browserDragComponent;
		SharedPtr<View3D> resourceBrowserPreview;
		SharedPtr<Scene> resourcePreviewScene;
		SharedPtr<Node> resourcePreviewNode;
		SharedPtr<Node> resourcePreviewCameraNode;
		SharedPtr<Node> resourcePreviewLightNode;
		SharedPtr<Light> resourcePreviewLight;
		/*rewrite*/
		int browserSearchSortMode;

		BrowserDir* rootDir;
		Vector<BrowserFile*> browserFiles;
		HashMap<String, BrowserDir*> browserDirs;
		Vector<int> activeResourceTypeFilters;
		Vector<int> activeResourceDirFilters;

		Vector<BrowserFile*> browserFilesToScan;


	};
}
