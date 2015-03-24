#pragma once

#include "EditorPlugin.h"
#include "../UI/BorderImage.h"
#include "../Math/Color.h"
#include "UIGlobals.h"
#include "../UI/UIElement.h"
#include "../Scene/Node.h"

namespace Urho3D
{
	enum MouseOrbitMode
	{
		ORBIT_RELATIVE = 0,
		ORBIT_WRAP
	};

	enum EditMode
	{
		EDIT_MOVE = 0,
		EDIT_ROTATE,
		EDIT_SCALE,
		EDIT_SELECT,
		EDIT_SPAWN
	};

	enum AxisMode
	{
		AXIS_WORLD = 0,
		AXIS_LOCAL
	};

	enum SnapScaleMode
	{
		SNAP_SCALE_FULL = 0,
		SNAP_SCALE_HALF,
		SNAP_SCALE_QUARTER
	};

	class Window;
	class View3D;
	class Camera;
	class Node;
	class Scene;
	class CustomGeometry;
	class Texture2D;
	class Viewport;
	class EditorData;
	class EditorView;
	class EditorSelection;
	class DebugRenderer;
	class UI;
	class Input;
	class SoundListener;
	class Renderer;
	class ResourceCache;
	class Text;
	class Font;
	class LineEdit;
	class CheckBox;
	class XMLFile;
	class Menu;
	class FileSelector;
	class FileSystem;
	class File;
	class Editor;
	class Button;

	class EPScene3D;
	class GizmoScene3D;

	class EPScene3DView : public BorderImage
	{
		OBJECT(EPScene3DView);
		friend class EPScene3D;
	public:
		/// Construct.
		EPScene3DView(Context* context);
		/// Destruct.
		virtual ~EPScene3DView();
		/// Register object factory.
		static void RegisterObject(Context* context);

		virtual void Update(float timeStep) ;
		/// React to resize.
		virtual void OnResize() ;
		/// React to mouse hover.
		virtual void OnHover(const IntVector2& position, const IntVector2& screenPosition, int buttons, int qualifiers, Cursor* cursor) ;
		/// React to mouse click begin.
		virtual void OnClickBegin(const IntVector2& position, const IntVector2& screenPosition, int button, int buttons, int qualifiers, Cursor* cursor) ;
		/// React to mouse click end.
		virtual void OnClickEnd(const IntVector2& position, const IntVector2& screenPosition, int button, int buttons, int qualifiers, Cursor* cursor, UIElement* beginElement) ;

        /* original code
		virtual void Update(float timeStep) override;
		/// React to resize.
		virtual void OnResize() override;
		/// React to mouse hover.
		virtual void OnHover(const IntVector2& position, const IntVector2& screenPosition, int buttons, int qualifiers, Cursor* cursor) override;
		/// React to mouse click begin.
		virtual void OnClickBegin(const IntVector2& position, const IntVector2& screenPosition, int button, int buttons, int qualifiers, Cursor* cursor) override;
		/// React to mouse click end.
		virtual void OnClickEnd(const IntVector2& position, const IntVector2& screenPosition, int button, int buttons, int qualifiers, Cursor* cursor, UIElement* beginElement) override;
        */

		/// Define the scene and camera to use in rendering. When ownScene is true the View3D will take ownership of them with shared pointers.
		void SetView(Scene* scene, bool ownScene = true);
		/// Set render texture pixel format. Default is RGB.
		void SetFormat(unsigned format);
		/// Set render target auto update mode. Default is true.
		void SetAutoUpdate(bool enable);
		/// Queue manual update on the render texture.
		void QueueUpdate();

		/// Return render texture pixel format.
		unsigned GetFormat() const { return rttFormat_; }
		/// Return whether render target updates automatically.
		bool GetAutoUpdate() const { return autoUpdate_; }
		/// Return scene.
		Scene* GetScene() const;
		/// Return camera scene node.
		Node* GetCameraNode() const;
		/// Return render texture.
		Texture2D* GetRenderTexture() const;
		/// Return depth stencil texture.
		Texture2D* GetDepthTexture() const;
		/// Return viewport.
		Viewport* GetViewport() const;

		Camera* GetCamera() const { return camera_; }
		float	GetYaw() const { return cameraYaw_; }
		float	GetPitch() const { return cameraPitch_; }

		void ResetCamera();
		void ReacquireCameraYawPitch();
		void CreateViewportContextUI(XMLFile* uiStyle, XMLFile* iconStyle_);
	protected:
		/// Reset scene.
		void ResetScene();

		void ToggleOrthographic();

		void SetOrthographic(bool orthographic);

		void HandleResize();

		void HandleSettingsLineEditTextChange(StringHash eventType, VariantMap& eventData);
		void HandleOrthographicToggled(StringHash eventType, VariantMap& eventData);
		void ToggleViewportSettingsWindow(StringHash eventType, VariantMap& eventData);
		void ResetCamera(StringHash eventType, VariantMap& eventData);
		void CloseViewportSettingsWindow(StringHash eventType, VariantMap& eventData);
		void UpdateSettingsUI(StringHash eventType, VariantMap& eventData);
		void OpenViewportSettingsWindow();

		/// Renderable texture.
		SharedPtr<Texture2D> renderTexture_;
		/// Depth stencil texture.
		SharedPtr<Texture2D> depthTexture_;
		/// Viewport.
		SharedPtr<Viewport> viewport_;
		/// Scene.
		SharedPtr<Scene> scene_;
		/// Camera scene node.
		SharedPtr<Node>		cameraNode_;
		/// Camera
		SharedPtr<Camera>	camera_;
		/// SoundListener
		SharedPtr<SoundListener> soundListener_;
		///
		float cameraYaw_;
		///
		float cameraPitch_;
		/// Own scene.
		bool ownScene_;
		/// Render texture format.
		unsigned rttFormat_;
		/// Render texture auto update mode.
		bool autoUpdate_;
		/// ui stuff
		SharedPtr<UIElement>statusBar;
		SharedPtr<Text> cameraPosText;

		SharedPtr<Window> settingsWindow;
		SharedPtr<LineEdit> cameraPosX;
		SharedPtr<LineEdit> cameraPosY;
		SharedPtr<LineEdit> cameraPosZ;
		SharedPtr<LineEdit> cameraRotX;
		SharedPtr<LineEdit> cameraRotY;
		SharedPtr<LineEdit> cameraRotZ;
		SharedPtr<LineEdit> cameraZoom;
		SharedPtr<LineEdit> cameraOrthoSize;
		SharedPtr<CheckBox> cameraOrthographic;
	};

	class EPScene3D : public EditorPlugin
	{
		OBJECT(EPScene3D);
		friend class GizmoScene3D;
	public:
		/// Construct.
		EPScene3D(Context* context);
		/// Destruct.
		virtual ~EPScene3D();

		virtual bool	HasMainScreen() ;
		virtual String	GetName() const ;
		virtual void	Edit(Object *object) ;
		virtual bool	Handles(Object *object) const ;
		/// calls Start, because EPScene3D is a main Editor plugin
		///	GetMainScreen will be called in AddEditorPlugin() once, so use it as Start().
		virtual UIElement*	GetMainScreen() ;
		virtual void		SetVisible(bool visible) ;
		virtual void		Update(float timeStep) ;


        /** Original code

		virtual bool	HasMainScreen() override;
		virtual String	GetName() const override;
		virtual void	Edit(Object *object) override;
		virtual bool	Handles(Object *object) const override;
		/// calls Start, because EPScene3D is a main Editor plugin
		///	GetMainScreen will be called in AddEditorPlugin() once, so use it as Start().
		virtual UIElement*	GetMainScreen() override;
		virtual void		SetVisible(bool visible) override;
		virtual void		Update(float timeStep) override;
        */

		// debug handling
		void ToggleRenderingDebug()	{ renderingDebug = !renderingDebug; }
		void TogglePhysicsDebug(){ physicsDebug = !physicsDebug; }
		void ToggleOctreeDebug(){ octreeDebug = !octreeDebug; }
		// camera handling
		void ResetCamera();
		void ReacquireCameraYawPitch();
		void UpdateViewParameters();
		// grid
		void HideGrid();
		void ShowGrid();
		// scene update handling
		void StartSceneUpdate();
		void StopSceneUpdate();
	protected:
		void Start();
		void CreateMiniToolBarUI();
		void CreateToolBarUI();

		void CreateStatsBar();
		void SetupStatsBarText(Text* text, Font* font, int x, int y, HorizontalAlignment hAlign, VerticalAlignment vAlign);
		void UpdateStats(float timeStep);
		void SetFillMode(FillMode fM_);

		Vector3 SelectedNodesCenterPoint();
		void	DrawNodeDebug(Node* node, DebugRenderer* debug, bool drawNode = true);
		void	MakeBackup(const String& fileName);
		void	RemoveBackup(bool success, const String& fileName);

		/// edit nodes
		bool MoveNodes(Vector3 adjust);
		bool RotateNodes(Vector3 adjust);
		bool ScaleNodes(Vector3 adjust);

		/// Picking
		void ViewRaycast(bool mouseClick);
		void SelectComponent(Component* component, bool multiselect);
		void SelectNode(Node* node, bool multiselect);

		/// mouse handling
		void SetMouseMode(bool enable);
		void SetMouseLock();
		void ReleaseMouseLock();

		/// Engine Events Handling
		void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);
		void ViewMouseClick(StringHash eventType, VariantMap& eventData);
		void ViewMouseMove(StringHash eventType, VariantMap& eventData);
		void ViewMouseClickEnd(StringHash eventType, VariantMap& eventData);
		void HandleBeginViewUpdate(StringHash eventType, VariantMap& eventData);
		void HandleEndViewUpdate(StringHash eventType, VariantMap& eventData);
		void HandleBeginViewRender(StringHash eventType, VariantMap& eventData);
		void HandleEndViewRender(StringHash eventType, VariantMap& eventData);
		/// Resize the view
		void HandleResizeView(StringHash eventType, VariantMap& eventData);
		/// Handle Menu Bar Events
		void HandleMenuBarAction(StringHash eventType, VariantMap& eventData);
		/// messageBox
		void HandleMessageAcknowledgement(StringHash eventType, VariantMap& eventData);

		// Menu Bar actions
		/// create new scene, because we use only one scene reset it ...
		bool ResetScene();
		void HandleOpenSceneFile(StringHash eventType, VariantMap& eventData);
		void HandleSaveSceneFile(StringHash eventType, VariantMap& eventData);
		void HandleLoadNodeFile(StringHash eventType, VariantMap& eventData);
		void HandleSaveNodeFile(StringHash eventType, VariantMap& eventData);

		bool LoadScene(const String& fileName);
		bool SaveScene(const String& fileName);
		Node* LoadNode(const String& fileName, Node* parent = NULL);
		bool SaveNode(const String& fileName);
		Node* InstantiateNodeFromFile(File* file, const Vector3& position, const Quaternion& rotation, float scaleMod = 1.0f, Node* parent = NULL, CreateMode mode = REPLICATED);

		Node* CreateNode(CreateMode mode);
		void CreateComponent(const String& componentType);
		void CreateBuiltinObject(const String& name);
		bool CheckForExistingGlobalComponent(Node* node, const String& typeName);

		// Mini Tool Bar actions
		void MiniToolBarCreateLocalNode(StringHash eventType, VariantMap& eventData);
		void MiniToolBarCreateReplNode(StringHash eventType, VariantMap& eventData);
		void MiniToolBarCreateComponent(StringHash eventType, VariantMap& eventData);

		// Mini Tool Bar actions
		void UpdateToolBar();
		void ToolBarRunUpdatePlay(StringHash eventType, VariantMap& eventData);
		void ToolBarRunUpdatePause(StringHash eventType, VariantMap& eventData);
		void ToolBarRevertOnPause(StringHash eventType, VariantMap& eventData);
		void ToolBarEditModeMove(StringHash eventType, VariantMap& eventData);
		void ToolBarEditModeRotate(StringHash eventType, VariantMap& eventData);
		void ToolBarEditModeScale(StringHash eventType, VariantMap& eventData);
		void ToolBarEditModeSelect(StringHash eventType, VariantMap& eventData);
		void ToolBarAxisModeWorld(StringHash eventType, VariantMap& eventData);
		void ToolBarAxisModeLocal(StringHash eventType, VariantMap& eventData);
		void ToolBarMoveSnap(StringHash eventType, VariantMap& eventData);
		void ToolBarRotateSnap(StringHash eventType, VariantMap& eventData);
		void ToolBarScaleSnap(StringHash eventType, VariantMap& eventData);
		void ToolBarSnapScaleModeHalf(StringHash eventType, VariantMap& eventData);
		void ToolBarSnapScaleModeQuarter(StringHash eventType, VariantMap& eventData);
		void ToolBarPickModeGeometries(StringHash eventType, VariantMap& eventData);
		void ToolBarPickModeLights(StringHash eventType, VariantMap& eventData);
		void ToolBarPickModeZones(StringHash eventType, VariantMap& eventData);
		void ToolBarPickModeRigidBodies(StringHash eventType, VariantMap& eventData);
		void ToolBarPickModeUIElements(StringHash eventType, VariantMap& eventData);
		void ToolBarFillModePoint(StringHash eventType, VariantMap& eventData);
		void ToolBarFillModeWireFrame(StringHash eventType, VariantMap& eventData);
		void ToolBarFillModeSolid(StringHash eventType, VariantMap& eventData);

		SharedPtr<UIElement>		window_;
		SharedPtr<EPScene3DView>	activeView;
		SharedPtr<Node>				cameraNode_;
		SharedPtr<Camera>			camera_;

		/// cache editor subsystems
		EditorData*			editorData_;
		EditorView*			editorView_;
		EditorSelection*	editorSelection_;
		Editor*				editor_;
		/// cache subsystems
		UI*				ui_;
		Input*			input_;
		Renderer*		renderer;
		ResourceCache*	cache_;
		FileSystem*		fileSystem_;

		///  properties \todo make an input edit state system or ...
		/// mouse handling
		bool	toggledMouseLock_;
		int		mouseOrbitMode;
		/// scene update handling
		/*bool	runUpdate = false;
		bool    revertOnPause = true;*/

		bool	runUpdate;
		bool    revertOnPause;

		SharedPtr<XMLFile> revertData;

		///camera handling
		float	cameraBaseSpeed;
		float	cameraBaseRotationSpeed ;
		float	cameraShiftSpeedMultiplier;
		bool	mouseWheelCameraPosition;
		bool	orbiting;
		bool	limitRotation;
		float	viewNearClip;
		float	viewFarClip;
		float	viewFov;
		/// create node
		float	newNodeDistance;
		/// edit input states
		float	moveStep;
		float	rotateStep;
		float	scaleStep;
		float	snapScale;

		bool	moveSnap;
		bool	rotateSnap;
		bool	scaleSnap;
		/// debug handling
		bool	renderingDebug;
		bool	physicsDebug;
		bool	octreeDebug;

		/// mouse pick handling
		int		pickMode;
		/// modes
		EditMode editMode;
		AxisMode axisMode;
		FillMode fillMode;
		SnapScaleMode snapScaleMode;

		/// scene handling
		bool sceneModified;
		String instantiateFileName;
		CreateMode instantiateMode;
		/// ui stuff
		SharedPtr<Text> editorModeText;
		SharedPtr<Text> renderStatsText;
		SharedPtr<Menu>	sceneMenu_;
		SharedPtr<Menu>	createMenu_;
		/// cached mini tool bar buttons, to set visibility
		Vector<Button*> miniToolBarButtons_;
		/// cached tool bar toggles, to set visibility
		Vector<UIElement*> toolBarToggles;
		/*rewrite*bool toolBarDirty = true;*/

        bool toolBarDirty;
		/// gizmo
		SharedPtr<GizmoScene3D> gizmo_;

		//////////////////////////////////////////////////////////////////////////
		/// Grid handling \todo put it into a component or object ...

		void CreateGrid();
		void UpdateGrid(bool updateGridGeometry = true);

		SharedPtr<Node>				gridNode_;
		SharedPtr<CustomGeometry>	grid_;
		bool	showGrid_;
		bool	grid2DMode_;
		Color gridColor;
		Color gridSubdivisionColor;
		Color gridXColor;
		Color gridYColor;
		Color gridZColor;
	};
}
