#include "../Urho3D.h"
#include "EPScene3D.h"
#include "../Core/Context.h"
#include "../Scene/Component.h"
#include "../Scene/Node.h"
#include "../Scene/Scene.h"
#include "../UI/Window.h"
#include "EditorData.h"
#include "../UI/Text.h"
#include "../UI/UIElement.h"
#include "../UI/View3D.h"
#include "../Scene/Node.h"
#include "../Graphics/Camera.h"
#include "../Resource/ResourceCache.h"
#include "../Graphics/CustomGeometry.h"
#include "../Graphics/Octree.h"
#include "../Scene/Scene.h"
#include "../Graphics/Material.h"
#include "../UI/UIEvents.h"
#include "../Graphics/Texture2D.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/Viewport.h"
#include "../Graphics/RenderSurface.h"
#include "../Graphics/Graphics.h"
#include "../Scene/Scene.h"
#include "EditorView.h"
#include "../Core/CoreEvents.h"
#include "../Input/InputEvents.h"
#include "../Graphics/GraphicsEvents.h"
#include "../Graphics/DebugRenderer.h"
#include "EditorSelection.h"
#include "../Graphics/Terrain.h"
#include "../UI/UI.h"
#include "../Input/Input.h"
#include "../Audio/SoundListener.h"
#include "../Graphics/Drawable.h"
#include "../Graphics/Renderer.h"
#include "../Physics/PhysicsWorld.h"
#include "../UI/Font.h"
#include "../UI/CheckBox.h"
#include "../UI/LineEdit.h"
#include "../UI/BorderImage.h"
#include "../UI/Button.h"
#include "UIUtils.h"
#include "../Resource/XMLFile.h"
#include "../Core/StringUtils.h"
#include "../Math/Quaternion.h"
#include "MenuBarUI.h"
#include "../UI/Menu.h"
#include "../UI/MessageBox.h"
#include "Editor.h"
#include "HierarchyWindow.h"
#include "../UI/FileSelector.h"
#include "../IO/FileSystem.h"
#include "../IO/Log.h"
#include "AttributeInspector.h"
#include "../IO/File.h"
#include "../IO/Deserializer.h"
#include "../UI/Menu.h"
#include "../Graphics/StaticModel.h"
#include "../Graphics/Model.h"
#include "MiniToolBarUI.h"
#include "../UI/Button.h"
#include "ToolBarUI.h"
#include "../UI/CheckBox.h"
#include "GizmoScene3D.h"
#include "../Physics/RigidBody.h"
#include "../UI/ListView.h"


namespace Urho3D
{
	const int pickModeDrawableFlags[] = {
		DRAWABLE_GEOMETRY,
		DRAWABLE_LIGHT,
		DRAWABLE_ZONE
	};

	const String editModeText[] = {
		"Move",
		"Rotate",
		"Scale",
		"Select",
		"Spawn"
	};

	const String axisModeText[] = {
		"World",
		"Local"
	};

	const String pickModeText[] = {
		"Geometries",
		"Lights",
		"Zones",
		"Rigidbodies",
		"UI-elements"
	};

	const 	String fillModeText[] = {
		"Solid",
		"Wire",
		"Point"
	};

	EPScene3D::EPScene3D(Context* context) : EditorPlugin(context),
		showGrid_(true),
		grid2DMode_(false),
		sceneModified(false)
	{
		ui_ = GetSubsystem<UI>();
		input_ = GetSubsystem<Input>();
		cache_ = GetSubsystem<ResourceCache>();
		renderer = GetSubsystem<Renderer>();
		fileSystem_ = GetSubsystem<FileSystem>();

		editorView_ = GetSubsystem<EditorView>();
		editorData_ = GetSubsystem<EditorData>();
		editorSelection_ = GetSubsystem<EditorSelection>();
		if (editorData_)
			editor_ = editorData_->GetEditor();

		gridColor = Color(0.1f, 0.1f, 0.1f);
		gridSubdivisionColor = Color(0.05f, 0.05f, 0.05f);
		gridXColor = Color(0.5f, 0.1f, 0.1f);
		gridYColor = Color(0.1f, 0.5f, 0.1f);
		gridZColor = Color(0.1f, 0.1f, 0.5f);

		toggledMouseLock_ = false;
		mouseOrbitMode = ORBIT_RELATIVE;
		instantiateMode = REPLICATED;

		/// Moved from Header
		/// mouse pick handling
		pickMode = PICK_GEOMETRIES;

		/// modes
		editMode = EDIT_MOVE;
		axisMode = AXIS_WORLD;
		fillMode = FILL_SOLID;
		snapScaleMode = SNAP_SCALE_FULL;

        moveSnap = false;
		rotateSnap = false;
		scaleSnap = false;

		/// debug handling
		renderingDebug = false;
		physicsDebug = false;
		octreeDebug = false;*/

        /// mouse pick handling
		pickMode = PICK_GEOMETRIES;
		/// modes
		editMode = EDIT_MOVE;
		axisMode = AXIS_WORLD;
		fillMode = FILL_SOLID;
		snapScaleMode = SNAP_SCALE_FULL;*/

				/// scene update handling
		runUpdate = false;
		revertOnPause = true;
		toolBarDirty = true;

	}

	EPScene3D::~EPScene3D()
	{
	}

	bool EPScene3D::HasMainScreen()
	{
		return true;
	}

	Urho3D::String EPScene3D::GetName() const
	{
		return String("3DView");
	}

	void EPScene3D::Edit(Object *object)
	{
		if (Component::GetTypeStatic() == object->GetBaseType())
		{
		}

		if (Node::GetTypeStatic() == object->GetBaseType())
		{
		}
	}

	bool EPScene3D::Handles(Object *object) const
	{
		if (Component::GetTypeStatic() == object->GetBaseType())
			return true;
		if (Node::GetTypeStatic() == object->GetBaseType())
			return true;
		// 		if (Scene::GetTypeStatic() == object->GetType())
		// 			return true;
		return false;
	}

	UIElement* EPScene3D::GetMainScreen()
	{
		if (!window_)
			Start();

		return window_;
	}

	void EPScene3D::SetVisible(bool visible)
	{
		if (visible_ == visible)
			return;

		visible_ = visible;

		if (window_)
		{
			window_->SetVisible(visible);
			sceneMenu_->SetVisible(visible);
			createMenu_->SetVisible(visible);

			for (unsigned i = 0; i < miniToolBarButtons_.Size(); i++)
				miniToolBarButtons_[i]->SetVisible(visible);

			for (unsigned i = 0; i < toolBarToggles.Size(); i++)
				toolBarToggles[i]->SetVisible(visible);

			if (visible)
			{
				SubscribeToEvent(E_POSTRENDERUPDATE, HANDLER(EPScene3D, HandlePostRenderUpdate));
				SubscribeToEvent(E_UIMOUSECLICK, HANDLER(EPScene3D, ViewMouseClick));
				SubscribeToEvent(E_MOUSEMOVE, HANDLER(EPScene3D, ViewMouseMove));
				SubscribeToEvent(E_UIMOUSECLICKEND, HANDLER(EPScene3D, ViewMouseClickEnd));
				SubscribeToEvent(E_BEGINVIEWUPDATE, HANDLER(EPScene3D, HandleBeginViewUpdate));
				SubscribeToEvent(E_ENDVIEWUPDATE, HANDLER(EPScene3D, HandleEndViewUpdate));
				SubscribeToEvent(E_BEGINVIEWRENDER, HANDLER(EPScene3D, HandleBeginViewRender));
				SubscribeToEvent(E_ENDVIEWRENDER, HANDLER(EPScene3D, HandleEndViewRender));
				gizmo_->ShowGizmo();
				activeView->SetAutoUpdate(true);
			}
			else
			{

				UnsubscribeFromEvent(E_POSTRENDERUPDATE);
				UnsubscribeFromEvent(E_UIMOUSECLICK);
				UnsubscribeFromEvent(E_MOUSEMOVE);
				UnsubscribeFromEvent(E_UIMOUSECLICKEND);
				UnsubscribeFromEvent(E_BEGINVIEWUPDATE);
				UnsubscribeFromEvent(E_ENDVIEWUPDATE);
				UnsubscribeFromEvent(E_BEGINVIEWRENDER);
				UnsubscribeFromEvent(E_ENDVIEWRENDER);
				gizmo_->HideGizmo();
				activeView->SetAutoUpdate(false);
			}
		}
	}

	void EPScene3D::Update(float timeStep)
	{
		UpdateStats(timeStep);

		if (runUpdate)
			editorData_->GetEditorScene()->Update(timeStep);

		if (toolBarDirty && editorView_->IsToolBarVisible())
			UpdateToolBar();

		gizmo_->UpdateGizmo();

		if (ui_->HasModalElement() || ui_->GetFocusElement() != NULL)
		{
			ReleaseMouseLock();
			return;
		}

		// Move camera
		if (!input_->GetKeyDown(KEY_LCTRL))
		{
			float speedMultiplier = 1.0;
			if (input_->GetKeyDown(KEY_LSHIFT))
				speedMultiplier = cameraShiftSpeedMultiplier;

			if (input_->GetKeyDown('W') || input_->GetKeyDown(KEY_UP))
			{
				cameraNode_->Translate(Vector3(0.0f, 0.0f, cameraBaseSpeed) * timeStep * speedMultiplier);
			}
			if (input_->GetKeyDown('S') || input_->GetKeyDown(KEY_DOWN))
			{
				cameraNode_->Translate(Vector3(0.0f, 0.0f, -cameraBaseSpeed) * timeStep * speedMultiplier);
			}
			if (input_->GetKeyDown('A') || input_->GetKeyDown(KEY_LEFT))
			{
				cameraNode_->Translate(Vector3(-cameraBaseSpeed, 0.0f, 0.0f) * timeStep * speedMultiplier);
			}
			if (input_->GetKeyDown('D') || input_->GetKeyDown(KEY_RIGHT))
			{
				cameraNode_->Translate(Vector3(cameraBaseSpeed, 0.0f, 0.0f) * timeStep * speedMultiplier);
			}
			if (input_->GetKeyDown('E') || input_->GetKeyDown(KEY_PAGEUP))
			{
				cameraNode_->Translate(Vector3(0.0f, cameraBaseSpeed, 0.0f) * timeStep * speedMultiplier, TS_WORLD);
			}
			if (input_->GetKeyDown('Q') || input_->GetKeyDown(KEY_PAGEDOWN))
			{
				cameraNode_->Translate(Vector3(0.0f, -cameraBaseSpeed, 0.0f) * timeStep * speedMultiplier, TS_WORLD);
			}
			if (input_->GetMouseMoveWheel() != 0 && ui_->GetElementAt(ui_->GetCursor()->GetPosition()) == NULL)
			{
				if (mouseWheelCameraPosition)
				{
					cameraNode_->Translate(Vector3(0.0f, 0.0f, -cameraBaseSpeed) * -(float)input_->GetMouseMoveWheel() * 20.0f * timeStep *
						speedMultiplier);
				}
				else
				{
					float zoom = camera_->GetZoom() + -(float)input_->GetMouseMoveWheel() *.1f * speedMultiplier;
					camera_->SetZoom(Clamp(zoom, .1f, 30.0f));
				}
			}
		}

		// Rotate/orbit/pan camera
		if (input_->GetMouseButtonDown(MOUSEB_RIGHT) || input_->GetMouseButtonDown(MOUSEB_MIDDLE))
		{
			SetMouseLock();

			IntVector2 mouseMove = input_->GetMouseMove();
			if (mouseMove.x_ != 0 || mouseMove.y_ != 0)
			{
				if (input_->GetKeyDown(KEY_LSHIFT) && input_->GetMouseButtonDown(MOUSEB_MIDDLE))
				{
					cameraNode_->Translate(Vector3(-(float)mouseMove.x_, (float)mouseMove.y_, 0.0f) * timeStep * cameraBaseSpeed * 0.5f);
				}
				else
				{
					activeView->cameraYaw_ += mouseMove.x_ * cameraBaseRotationSpeed;
					activeView->cameraPitch_ += mouseMove.y_ * cameraBaseRotationSpeed;

					if (limitRotation)
						activeView->cameraPitch_ = Clamp(activeView->cameraPitch_, -90.0, 90.0);

					Quaternion q = Quaternion(activeView->cameraPitch_, activeView->cameraYaw_, 0);
					cameraNode_->SetRotation(q);

					if (input_->GetMouseButtonDown(MOUSEB_MIDDLE) && (editorSelection_->GetNumSelectedNodes() > 0 || editorSelection_->GetNumSelectedComponents() > 0))
					{
						Vector3 centerPoint = SelectedNodesCenterPoint();
						Vector3 d = cameraNode_->GetWorldPosition() - centerPoint;
						cameraNode_->SetWorldPosition(centerPoint - q * Vector3(0.0, 0.0, d.Length()));
						orbiting = true;
					}
				}
			}
		}
		else
			ReleaseMouseLock();

		if (orbiting && !input_->GetMouseButtonDown(MOUSEB_MIDDLE))
			orbiting = false;

		// Move/rotate/scale object
		if (!editorSelection_->GetEditNodes().Empty() && editMode != EDIT_SELECT && input_->GetKeyDown(KEY_LCTRL))
		{
			Vector3 adjust(0, 0, 0);
			if (input_->GetKeyDown(KEY_UP))
				adjust.z_ = 1;
			if (input_->GetKeyDown(KEY_DOWN))
				adjust.z_ = -1;
			if (input_->GetKeyDown(KEY_LEFT))
				adjust.x_ = -1;
			if (input_->GetKeyDown(KEY_RIGHT))
				adjust.x_ = 1;
			if (input_->GetKeyDown(KEY_PAGEUP))
				adjust.y_ = 1;
			if (input_->GetKeyDown(KEY_PAGEDOWN))
				adjust.y_ = -1;
			if (editMode == EDIT_SCALE)
			{
				if (input_->GetKeyDown(KEY_KP_PLUS))
					adjust = Vector3(1, 1, 1);
				if (input_->GetKeyDown(KEY_KP_MINUS))
					adjust = Vector3(-1, -1, -1);
			}

			if (adjust == Vector3(0, 0, 0))
				return;

			bool moved = false;
			adjust *= timeStep * 10;

			switch (editMode)
			{
			case EDIT_MOVE:
				if (!moveSnap)
					moved = MoveNodes(adjust * moveStep);
				break;

			case EDIT_ROTATE:
				if (!rotateSnap)
					moved = RotateNodes(adjust * rotateStep);
				break;

			case EDIT_SCALE:
				if (!scaleSnap)
					moved = ScaleNodes(adjust * scaleStep);
				break;
			}

			// 			if (moved)
			// 				UpdateNodeAttributes();
		}
	}

	void EPScene3D::ResetCamera()
	{
		//for (uint i = 0; i < viewports.length; ++i)
		activeView->ResetCamera();
	}

	void EPScene3D::ReacquireCameraYawPitch()
	{
		//for (uint i = 0; i < viewports.length; ++i)
		activeView->ReacquireCameraYawPitch();
	}

	void EPScene3D::UpdateViewParameters()
	{
		//for (uint i = 0; i < viewports.length; ++i)
		{
			activeView->camera_->SetNearClip(viewNearClip);
			activeView->camera_->SetFarClip(viewFarClip);
			activeView->camera_->SetFov(viewFov);
		}
	}

	void EPScene3D::CreateStatsBar()
	{
		Font* font = cache_->GetResource<Font>("Fonts/Anonymous Pro.ttf");

		editorModeText = new Text(context_);
		activeView->AddChild(editorModeText);
		renderStatsText = new Text(context_);
		activeView->AddChild(renderStatsText);

		if (window_->GetWidth() >= 1200)
		{
			SetupStatsBarText(editorModeText, font, 35, 64, HA_LEFT, VA_TOP);
			SetupStatsBarText(renderStatsText, font, -4, 64, HA_RIGHT, VA_TOP);
		}
		else
		{
			SetupStatsBarText(editorModeText, font, 1, 1, HA_LEFT, VA_TOP);
			SetupStatsBarText(renderStatsText, font, 1, 15, HA_LEFT, VA_TOP);
		}
	}

	void EPScene3D::SetupStatsBarText(Text* text, Font* font, int x, int y, HorizontalAlignment hAlign, VerticalAlignment vAlign)
	{
		text->SetPosition(IntVector2(x, y));
		text->SetHorizontalAlignment(hAlign);
		text->SetVerticalAlignment(vAlign);
		text->SetFont(font, 11);
		text->SetColor(Color(1, 1, 0));
		text->SetTextEffect(TE_SHADOW);
		text->SetPriority(-100);
	}

	void EPScene3D::UpdateStats(float timeStep)
	{
		editorModeText->SetText(String(
			"Mode: " + editModeText[editMode] +
			"  Axis: " + axisModeText[axisMode] +
			"  Pick: " + pickModeText[pickMode] +
			"  Fill: " + fillModeText[fillMode] +
			"  Updates: " + (runUpdate ? "Running" : "Paused")));

		renderStatsText->SetText(String(
			"Tris: " + String(renderer->GetNumPrimitives()) +
			"  Batches: " + String(renderer->GetNumBatches()) +
			"  Lights: " + String(renderer->GetNumLights(true)) +
			"  Shadowmaps: " + String(renderer->GetNumShadowMaps(true)) +
			"  Occluders: " + String(renderer->GetNumOccluders(true))));

		editorModeText->SetSize(editorModeText->GetMinSize());
		renderStatsText->SetSize(renderStatsText->GetMinSize());
	}

	void EPScene3D::SetFillMode(FillMode fM_)
	{
		fillMode = fM_;
// 		for (uint i = 0; i < viewports.length; ++i)
// 			viewports[i].camera.fillMode = fillMode_;
		camera_->SetFillMode(fM_);
	}

	void EPScene3D::Start()
	{
		EditorData* editorData_ = GetSubsystem<EditorData>();
		window_ = new UIElement(context_);
		window_->SetDefaultStyle(editorData_->GetDefaultStyle());

		activeView = window_->CreateChild<EPScene3DView>("Scene3DView");
		activeView->SetDefaultStyle(editorData_->GetDefaultStyle());
		activeView->SetView(editorData_->GetEditorScene());
		activeView->CreateViewportContextUI(editorData_->GetDefaultStyle(), editorData_->GetIconStyle());
		cameraNode_ = activeView->GetCameraNode();
		camera_ = activeView->GetCamera();
		activeView->SetAutoUpdate(true);

		CreateGrid();
		ShowGrid();
		CreateStatsBar();

		SubscribeToEvent(window_, E_RESIZED, HANDLER(EPScene3D, HandleResizeView));

		//////////////////////////////////////////////////////////////////////////
		/// Menu Bar entries

		sceneMenu_ = editorView_->GetGetMenuBar()->CreateMenu("Scene");
		editorView_->GetGetMenuBar()->CreateMenuItem("Scene", "New scene", A_NEWSCENE_VAR, 'N', QUAL_SHIFT | QUAL_CTRL);
		editorView_->GetGetMenuBar()->CreateMenuItem("Scene", "Open scene...", A_OPENSCENE_VAR, 'O', QUAL_CTRL);
		editorView_->GetGetMenuBar()->CreateMenuItem("Scene", "Save scene", A_SAVESCENE_VAR, 'S', QUAL_CTRL);
		editorView_->GetGetMenuBar()->CreateMenuItem("Scene", "Save scene as...", A_SAVESCENEAS_VAR, 'S', QUAL_SHIFT | QUAL_CTRL);

		editorView_->GetGetMenuBar()->CreateMenuItem("Scene", "Load node as replicated", A_LOADNODEASREP_VAR);
		editorView_->GetGetMenuBar()->CreateMenuItem("Scene", "Load node as local", A_LOADNODEASLOCAL_VAR);
		editorView_->GetGetMenuBar()->CreateMenuItem("Scene", "Save node as", A_SAVENODEAS_VAR);

		createMenu_ = editorView_->GetGetMenuBar()->CreateMenu("Create");

		editorView_->GetGetMenuBar()->CreateMenuItem("Create", "Replicated node", A_CREATEREPNODE_VAR, 0, 0, true, "Create Replicated node");
		editorView_->GetGetMenuBar()->CreateMenuItem("Create", "Local node", A_CREATELOCALNODE_VAR, 0, 0, true, "Create Local node");

		Menu* childMenu = editorView_->GetGetMenuBar()->CreateMenuItem("Create", "Component", StringHash::ZERO, SHOW_POPUP_INDICATOR);
		Window* childPopup = editorView_->GetGetMenuBar()->CreatePopupMenu(childMenu);

		const HashMap<String, Vector<StringHash> >& objectCategories = context_->GetObjectCategories();
		HashMap<String, Vector<StringHash> >::ConstIterator it;

		/// Mini Tool Bar entries
		/// \todo create scroll bar for the mini tool bar or something ... because there are to many components that can be added this way
// 		MiniToolBarUI* minitool = editorView_->GetMiniToolBar();
// 		Button* b = (Button*)minitool->CreateSmallToolBarButton("Node", "Replicated Node");
// 		miniToolBarButtons_.Push(b);
// 		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateReplNode));
// 		b = (Button*)minitool->CreateSmallToolBarButton("Node", "Local Node");
// 		miniToolBarButtons_.Push(b);
// 		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateLocalNode));


		for (it = objectCategories.Begin(); it != objectCategories.End(); it++)
		{
			// Skip the UI category for the component menus
			if (it->first_ == "UI")
				continue;

			Menu* menu = editorView_->GetGetMenuBar()->CreatePopupMenuItem(childPopup, it->first_, StringHash::ZERO, SHOW_POPUP_INDICATOR);
			Window* popup = editorView_->GetGetMenuBar()->CreatePopupMenu(menu);

			/// GetObjectsByCategory
			Vector<String> components;
			const HashMap<StringHash, SharedPtr<ObjectFactory> >& factories = context_->GetObjectFactories();
			const Vector<StringHash>& factoryHashes = it->second_;
			components.Reserve(factoryHashes.Size());
			for (unsigned j = 0; j < factoryHashes.Size(); ++j)
			{
				HashMap<StringHash, SharedPtr<ObjectFactory> >::ConstIterator k = factories.Find(factoryHashes[j]);
				if (k != factories.End())
					components.Push(k->second_->GetTypeName());
			}
			//minitool->CreateSmallToolBarSpacer(3);
			/// \todo CreateIconizedMenuItem
			for (unsigned j = 0; j < components.Size(); ++j)
			{
				editorView_->GetGetMenuBar()->CreatePopupMenuItem(popup, components[j], A_CREATECOMPONENT_VAR);
				/// Mini Tool Bar entries
// 				b = (Button*)minitool->CreateSmallToolBarButton(components[j]);
// 				miniToolBarButtons_.Push(b);
			}

		}

		childMenu = editorView_->GetGetMenuBar()->CreateMenuItem("Create", "Builtin object", StringHash::ZERO, SHOW_POPUP_INDICATOR);
		childPopup = editorView_->GetGetMenuBar()->CreatePopupMenu(childMenu);
		String objects[] = { "Box", "Cone", "Cylinder", "Plane", "Pyramid", "Sphere", "TeaPot", "Torus" };
		for (int i = 0; i < 8; i++)
		{
			editorView_->GetGetMenuBar()->CreatePopupMenuItem(childPopup, objects[i], A_CREATEBUILTINOBJ_VAR);
		}

		SubscribeToEvent(editorView_->GetGetMenuBar(), E_MENUBAR_ACTION, HANDLER(EPScene3D, HandleMenuBarAction));


		/// Mini Tool Bar entries
		CreateMiniToolBarUI();

		/// Tool Bar entries
		CreateToolBarUI();

		/// create gizmo

		gizmo_ = new GizmoScene3D(context_, this);

		gizmo_->CreateGizmo();
		gizmo_->ShowGizmo();
	}

	void EPScene3D::CreateMiniToolBarUI()
	{

		MiniToolBarUI* minitool = editorView_->GetMiniToolBar();

		Button* b = (Button*)minitool->CreateSmallToolBarButton("Node", "Replicated Node");
		miniToolBarButtons_.Push(b);
		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateReplNode));
		b = (Button*)minitool->CreateSmallToolBarButton("Node", "Local Node");
		miniToolBarButtons_.Push(b);
		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateLocalNode));

		minitool->CreateSmallToolBarSpacer(3);
		b = (Button*)minitool->CreateSmallToolBarButton("Light");
		miniToolBarButtons_.Push(b);
		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateComponent));
		b = (Button*)minitool->CreateSmallToolBarButton("Camera");
		miniToolBarButtons_.Push(b);
		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateComponent));
		b = (Button*)minitool->CreateSmallToolBarButton("Zone");
		miniToolBarButtons_.Push(b);
		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateComponent));
		b = (Button*)minitool->CreateSmallToolBarButton("StaticModel");
		miniToolBarButtons_.Push(b);
		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateComponent));
		b = (Button*)minitool->CreateSmallToolBarButton("AnimatedModel"); miniToolBarButtons_.Push(b);
		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateComponent));
		b = (Button*)minitool->CreateSmallToolBarButton("BillboardSet"); miniToolBarButtons_.Push(b);
		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateComponent));
		b = (Button*)minitool->CreateSmallToolBarButton("ParticleEmitter"); miniToolBarButtons_.Push(b);
		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateComponent));
		b = (Button*)minitool->CreateSmallToolBarButton("Skybox"); miniToolBarButtons_.Push(b);
		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateComponent));
		b = (Button*)minitool->CreateSmallToolBarButton("Terrain"); miniToolBarButtons_.Push(b);
		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateComponent));
		b = (Button*)minitool->CreateSmallToolBarButton("Text3D"); miniToolBarButtons_.Push(b);
		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateComponent));

		minitool->CreateSmallToolBarSpacer(3);
		b = (Button*)minitool->CreateSmallToolBarButton("SoundListener"); miniToolBarButtons_.Push(b);
		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateComponent));
		b = (Button*)minitool->CreateSmallToolBarButton("SoundSource3D"); miniToolBarButtons_.Push(b);
		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateComponent));
		b = (Button*)minitool->CreateSmallToolBarButton("SoundSource"); miniToolBarButtons_.Push(b);
		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateComponent));

		minitool->CreateSmallToolBarSpacer(3);
		b = (Button*)minitool->CreateSmallToolBarButton("RigidBody"); miniToolBarButtons_.Push(b);
		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateComponent));
		b = (Button*)minitool->CreateSmallToolBarButton("CollisionShape"); miniToolBarButtons_.Push(b);
		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateComponent));
		b = (Button*)minitool->CreateSmallToolBarButton("Constraint"); miniToolBarButtons_.Push(b);
		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateComponent));

		minitool->CreateSmallToolBarSpacer(3);
		b = (Button*)minitool->CreateSmallToolBarButton("AnimationController"); miniToolBarButtons_.Push(b);
		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateComponent));
		b = (Button*)minitool->CreateSmallToolBarButton("ScriptInstance"); miniToolBarButtons_.Push(b);
		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateComponent));

		minitool->CreateSmallToolBarSpacer(3);
		b = (Button*)minitool->CreateSmallToolBarButton("Navigable"); miniToolBarButtons_.Push(b);
		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateComponent));
		b = (Button*)minitool->CreateSmallToolBarButton("NavigationMesh"); miniToolBarButtons_.Push(b);
		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateComponent));
		b = (Button*)minitool->CreateSmallToolBarButton("OffMeshConnection"); miniToolBarButtons_.Push(b);
		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateComponent));
		minitool->CreateSmallToolBarSpacer(3);
		b = (Button*)minitool->CreateSmallToolBarButton("NetworkPriority"); miniToolBarButtons_.Push(b);
		SubscribeToEvent(b, E_RELEASED, HANDLER(EPScene3D, MiniToolBarCreateComponent));
	}

	void EPScene3D::CreateToolBarUI()
	{

		ToolBarUI* minitool = editorView_->GetToolBar();


		UIElement* e = minitool->CreateGroup("RunUpdateGroup", LM_HORIZONTAL);
		toolBarToggles.Push(e);
		CheckBox* checkbox = minitool->CreateToolBarToggle("RunUpdateGroup", "RunUpdatePlay");
		if (checkbox->IsChecked() != runUpdate)
			checkbox->SetChecked(runUpdate);
		SubscribeToEvent(checkbox, E_TOGGLED, HANDLER(EPScene3D,ToolBarRunUpdatePlay));
		checkbox = minitool->CreateToolBarToggle("RunUpdateGroup", "RunUpdatePause");
		if (checkbox->IsChecked() != (runUpdate == false))
			checkbox->SetChecked(runUpdate == false);
		SubscribeToEvent(checkbox, E_TOGGLED, HANDLER(EPScene3D, ToolBarRunUpdatePause));
		checkbox = minitool->CreateToolBarToggle("RunUpdateGroup", "RevertOnPause");
		if (checkbox->IsChecked() != revertOnPause)
			checkbox->SetChecked(revertOnPause);
		SubscribeToEvent(checkbox, E_TOGGLED, HANDLER(EPScene3D, ToolBarRevertOnPause));
		minitool->CreateToolBarSpacer(4);



		e = minitool->CreateGroup("EditModeGroup", LM_HORIZONTAL);
		toolBarToggles.Push(e);
		checkbox = minitool->CreateToolBarToggle("EditModeGroup", "EditMove");
		if (checkbox->IsChecked() != (editMode == EDIT_MOVE))
			checkbox->SetChecked(editMode == EDIT_MOVE);
		SubscribeToEvent(checkbox, E_TOGGLED, HANDLER(EPScene3D, ToolBarEditModeMove));
		checkbox = minitool->CreateToolBarToggle("EditModeGroup", "EditRotate");
		if (checkbox->IsChecked() != (editMode == EDIT_ROTATE))
			checkbox->SetChecked(editMode == EDIT_ROTATE);
		SubscribeToEvent(checkbox, E_TOGGLED, HANDLER(EPScene3D, ToolBarEditModeRotate));
		checkbox = minitool->CreateToolBarToggle("EditModeGroup", "EditScale");
		if (checkbox->IsChecked() != (editMode == EDIT_SCALE))
			checkbox->SetChecked(editMode == EDIT_SELECT);
		SubscribeToEvent(checkbox, E_TOGGLED, HANDLER(EPScene3D, ToolBarEditModeScale));
		checkbox = minitool->CreateToolBarToggle("EditModeGroup", "EditSelect");
		if (checkbox->IsChecked() != (editMode == EDIT_SELECT))
			checkbox->SetChecked(editMode == EDIT_SELECT);
		SubscribeToEvent(checkbox, E_TOGGLED, HANDLER(EPScene3D, ToolBarEditModeSelect));

		e = minitool->CreateGroup("AxisModeGroup", LM_HORIZONTAL);
		toolBarToggles.Push(e);
		checkbox = minitool->CreateToolBarToggle("AxisModeGroup", "AxisWorld");
		if (checkbox->IsChecked() != (axisMode == AXIS_WORLD))
			checkbox->SetChecked(axisMode == AXIS_WORLD);
		SubscribeToEvent(checkbox, E_TOGGLED, HANDLER(EPScene3D, ToolBarAxisModeWorld));
		checkbox = minitool->CreateToolBarToggle("AxisModeGroup", "AxisLocal");
		if (checkbox->IsChecked() != (axisMode == AXIS_LOCAL))
			checkbox->SetChecked((axisMode == AXIS_LOCAL));
		SubscribeToEvent(checkbox, E_TOGGLED, HANDLER(EPScene3D, ToolBarAxisModeLocal));

		minitool->CreateToolBarSpacer(4);
		checkbox = minitool->CreateToolBarToggle("MoveSnap");
		if (checkbox->IsChecked() != moveSnap)
			checkbox->SetChecked(moveSnap);
		SubscribeToEvent(checkbox, E_TOGGLED, HANDLER(EPScene3D, ToolBarMoveSnap));
		toolBarToggles.Push(checkbox);
		checkbox = minitool->CreateToolBarToggle("RotateSnap");
		if (checkbox->IsChecked() != rotateSnap)
			checkbox->SetChecked(rotateSnap);
		SubscribeToEvent(checkbox, E_TOGGLED, HANDLER(EPScene3D, ToolBarRotateSnap));
		toolBarToggles.Push(checkbox);
		checkbox = minitool->CreateToolBarToggle("ScaleSnap");
		if (checkbox->IsChecked() != scaleSnap)
			checkbox->SetChecked(scaleSnap);
		SubscribeToEvent(checkbox, E_TOGGLED, HANDLER(EPScene3D, ToolBarScaleSnap));
		toolBarToggles.Push(checkbox);

		e = minitool->CreateGroup("SnapScaleModeGroup", LM_HORIZONTAL);
		toolBarToggles.Push(e);
		checkbox = minitool->CreateToolBarToggle("SnapScaleModeGroup", "SnapScaleHalf");
		if (checkbox->IsChecked() != (snapScaleMode == SNAP_SCALE_HALF))
			checkbox->SetChecked((snapScaleMode == SNAP_SCALE_HALF));
		SubscribeToEvent(checkbox, E_TOGGLED, HANDLER(EPScene3D, ToolBarSnapScaleModeHalf));
		checkbox = minitool->CreateToolBarToggle("SnapScaleModeGroup", "SnapScaleQuarter");
		if (checkbox->IsChecked() != (snapScaleMode == SNAP_SCALE_QUARTER))
			checkbox->SetChecked((snapScaleMode == SNAP_SCALE_QUARTER));
		SubscribeToEvent(checkbox, E_TOGGLED, HANDLER(EPScene3D, ToolBarSnapScaleModeQuarter));

		minitool->CreateToolBarSpacer(4);
		e = minitool->CreateGroup("PickModeGroup", LM_HORIZONTAL);
		toolBarToggles.Push(e);
		checkbox = minitool->CreateToolBarToggle("PickModeGroup", "PickGeometries");
		if (checkbox->IsChecked() != (pickMode == PICK_GEOMETRIES))
			checkbox->SetChecked((pickMode == PICK_GEOMETRIES));
		SubscribeToEvent(checkbox, E_TOGGLED, HANDLER(EPScene3D, ToolBarPickModeGeometries));
		checkbox = minitool->CreateToolBarToggle("PickModeGroup", "PickLights");
		if (checkbox->IsChecked() != (pickMode == PICK_LIGHTS))
			checkbox->SetChecked((pickMode == PICK_LIGHTS));
		SubscribeToEvent(checkbox, E_TOGGLED, HANDLER(EPScene3D, ToolBarPickModeLights));
		checkbox = minitool->CreateToolBarToggle("PickModeGroup", "PickZones");
		if (checkbox->IsChecked() != (pickMode == PICK_ZONES))
			checkbox->SetChecked((pickMode == PICK_ZONES));
		SubscribeToEvent(checkbox, E_TOGGLED, HANDLER(EPScene3D, ToolBarPickModeZones));
		checkbox = minitool->CreateToolBarToggle("PickModeGroup", "PickRigidBodies");
		if (checkbox->IsChecked() != (pickMode == PICK_RIGIDBODIES))
			checkbox->SetChecked((pickMode == PICK_RIGIDBODIES));
		SubscribeToEvent(checkbox, E_TOGGLED, HANDLER(EPScene3D, ToolBarPickModeRigidBodies));
		checkbox = minitool->CreateToolBarToggle("PickModeGroup", "PickUIElements");
		if (checkbox->IsChecked() != (pickMode == PICK_UI_ELEMENTS))
			checkbox->SetChecked((pickMode == PICK_UI_ELEMENTS));
		SubscribeToEvent(checkbox, E_TOGGLED, HANDLER(EPScene3D, ToolBarPickModeUIElements));

		minitool->CreateToolBarSpacer(4);
		e = minitool->CreateGroup("FillModeGroup", LM_HORIZONTAL);
		toolBarToggles.Push(e);
		checkbox = minitool->CreateToolBarToggle("FillModeGroup", "FillPoint");
		if (checkbox->IsChecked() != (fillMode == FILL_POINT))
			checkbox->SetChecked((fillMode == FILL_POINT));
		SubscribeToEvent(checkbox, E_TOGGLED, HANDLER(EPScene3D, ToolBarFillModePoint));
		checkbox = minitool->CreateToolBarToggle("FillModeGroup", "FillWireFrame");
		if (checkbox->IsChecked() != (fillMode == FILL_WIREFRAME))
			checkbox->SetChecked((fillMode == FILL_WIREFRAME));
		SubscribeToEvent(checkbox, E_TOGGLED, HANDLER(EPScene3D, ToolBarFillModeWireFrame));
		checkbox = minitool->CreateToolBarToggle("FillModeGroup", "FillSolid");
		if (checkbox->IsChecked() != (fillMode == FILL_SOLID))
			checkbox->SetChecked((fillMode == FILL_SOLID));
		SubscribeToEvent(checkbox, E_TOGGLED, HANDLER(EPScene3D, ToolBarFillModeSolid));


	}

	Urho3D::Vector3 EPScene3D::SelectedNodesCenterPoint()
	{
		Vector3 centerPoint;
		unsigned int count = editorSelection_->GetNumSelectedNodes();
		for (unsigned int i = 0; i < editorSelection_->GetNumSelectedNodes(); ++i)
			centerPoint += editorSelection_->GetSelectedNodes()[i]->GetWorldPosition();

		for (unsigned int i = 0; i < editorSelection_->GetNumSelectedComponents(); ++i)
		{
			Drawable* drawable = dynamic_cast<Drawable*>(editorSelection_->GetSelectedComponents()[i]);
			count++;
			if (drawable != NULL)
				centerPoint += drawable->GetNode()->LocalToWorld(drawable->GetBoundingBox().Center());
			else
				centerPoint += editorSelection_->GetSelectedComponents()[i]->GetNode()->GetWorldPosition();
		}

		if (count > 0)
			return centerPoint / float(count);
		else
			return centerPoint;
	}

	void EPScene3D::DrawNodeDebug(Node* node, DebugRenderer* debug, bool drawNode /*= true*/)
	{
		if (drawNode)
			debug->AddNode(node, 1.0f, false);

		// Exception for the scene to avoid bringing the editor to its knees: drawing either the whole hierarchy or the subsystem-
		// components can have a large performance hit. Also do not draw terrain child nodes due to their large amount
		// (TerrainPatch component itself draws nothing as debug geometry)
		if (node != editorData_->GetEditorScene() && node->GetComponent<Terrain>() == NULL)
		{
			for (unsigned int j = 0; j < node->GetNumComponents(); ++j)
				node->GetComponents()[j]->DrawDebugGeometry(debug, false);

			// To avoid cluttering the view, do not draw the node axes for child nodes
			for (unsigned int k = 0; k < node->GetNumChildren(); ++k)
				DrawNodeDebug(node->GetChildren()[k], debug, false);
		}
	}

	bool EPScene3D::MoveNodes(Vector3 adjust)
	{
		bool moved = false;

		if (adjust.Length() > M_EPSILON)
		{
			for (unsigned int i = 0; i < editorSelection_->GetNumEditNodes(); ++i)
			{
				if (moveSnap)
				{
					float moveStepScaled = moveStep * snapScale;
					adjust.x_ = floor(adjust.x_ / moveStepScaled + 0.5f) * moveStepScaled;
					adjust.y_ = floor(adjust.y_ / moveStepScaled + 0.5f) * moveStepScaled;
					adjust.z_ = floor(adjust.z_ / moveStepScaled + 0.5f) * moveStepScaled;
				}

				Node* node = editorSelection_->GetEditNodes()[i];
				Vector3 nodeAdjust = adjust;
				if (axisMode == AXIS_LOCAL && editorSelection_->GetNumEditNodes() == 1)
					nodeAdjust = node->GetWorldRotation() * nodeAdjust;

				Vector3 worldPos = node->GetWorldPosition();
				Vector3 oldPos = node->GetPosition();

				worldPos += nodeAdjust;

				if (node->GetParent() == NULL)
					node->SetPosition(worldPos);
				else
					node->SetPosition(node->GetParent()->WorldToLocal(worldPos));

				if (node->GetPosition() != oldPos)
					moved = true;
			}
		}

		return moved;
	}

	bool EPScene3D::RotateNodes(Vector3 adjust)
	{
		bool moved = false;

		if (rotateSnap)
		{
			float rotateStepScaled = rotateStep * snapScale;
			adjust.x_ = floor(adjust.x_ / rotateStepScaled + 0.5f) * rotateStepScaled;
			adjust.y_ = floor(adjust.y_ / rotateStepScaled + 0.5f) * rotateStepScaled;
			adjust.z_ = floor(adjust.z_ / rotateStepScaled + 0.5f) * rotateStepScaled;
		}

		if (adjust.Length() > M_EPSILON)
		{
			moved = true;

			for (unsigned int i = 0; i < editorSelection_->GetNumEditNodes(); ++i)
			{
				Node* node = editorSelection_->GetEditNodes()[i];
				Quaternion rotQuat(adjust.x_, adjust.y_, adjust.z_);
				if (axisMode == AXIS_LOCAL && editorSelection_->GetNumEditNodes() == 1)
					node->SetRotation(node->GetRotation() * rotQuat);
				else
				{
					Vector3 offset = node->GetWorldPosition();/// \todo -gizmoAxisX.axisRay.origin;

					if (node->GetParent() != NULL && node->GetParent()->GetWorldRotation() != Quaternion(1, 0, 0, 0))
						rotQuat = node->GetParent()->GetWorldRotation().Inverse() * rotQuat * node->GetParent()->GetWorldRotation();

					node->SetRotation(rotQuat * node->GetRotation());
					Vector3 newPosition = rotQuat * offset; /// \todo gizmoAxisX.axisRay.origin +

					if (node->GetParent() != NULL)
						newPosition = node->GetParent()->WorldToLocal(newPosition);

					node->SetPosition(newPosition);
				}
			}
		}

		return moved;
	}

	bool EPScene3D::ScaleNodes(Vector3 adjust)
	{
		bool moved = false;

		if (adjust.Length() > M_EPSILON)
		{
			for (unsigned int i = 0; i < editorSelection_->GetNumEditNodes(); ++i)
			{
				Node* node = editorSelection_->GetEditNodes()[i];

				Vector3 scale = node->GetScale();
				Vector3 oldScale = scale;

				if (!scaleSnap)
					scale += adjust;
				else
				{
					float scaleStepScaled = scaleStep * snapScale;
					if (adjust.x_ != 0)
					{
						scale.x_ += adjust.x_ * scaleStepScaled;
						scale.x_ = floor(scale.x_ / scaleStepScaled + 0.5f) * scaleStepScaled;
					}
					if (adjust.y_ != 0)
					{
						scale.y_ += adjust.y_ * scaleStepScaled;
						scale.y_ = floor(scale.y_ / scaleStepScaled + 0.5f) * scaleStepScaled;
					}
					if (adjust.z_ != 0)
					{
						scale.z_ += adjust.z_ * scaleStepScaled;
						scale.z_ = floor(scale.z_ / scaleStepScaled + 0.5f) * scaleStepScaled;
					}
				}

				if (scale != oldScale)
					moved = true;

				node->SetScale(scale);
			}
		}

		return moved;
	}

	void EPScene3D::ViewRaycast(bool mouseClick)
	{
		// Ignore if UI has modal element
		if (ui_->HasModalElement())
			return;

		// Ignore if mouse is grabbed by other operation
		if (input_->IsMouseGrabbed())
			return;
		Scene* editorScene = editorData_->GetEditorScene();

		Input* input = GetSubsystem<Input>();

		IntVector2 pos = ui_->GetCursorPosition();
		UIElement* elementAtPos = ui_->GetElementAt(pos, pickMode != PICK_UI_ELEMENTS);
		if (editMode == EDIT_SPAWN)
		{
// 			if (mouseClick && input_->GetMouseButtonPress(MOUSEB_LEFT) && elementAtPos == NULL)
// 				SpawnObject();
			return;
		}

		// Do not raycast / change selection if hovering over the gizmo
		if (gizmo_->IsGizmoSelected())
			return;

		DebugRenderer* debug = editorScene->GetComponent<DebugRenderer>();

		if (pickMode == PICK_UI_ELEMENTS)
		{
// 			bool leftClick = mouseClick && input->GetMouseButtonPress(MOUSEB_LEFT);
// 			bool multiselect = input->GetQualifierDown(QUAL_CTRL);
//
// 			// Only interested in user-created UI elements
// 			if (elementAtPos != NULL && elementAtPos != editorUIElement && elementAtPos.GetElementEventSender() == editorUIElement)
// 			{
// 				ui.DebugDraw(elementAtPos);
//
// 				if (leftClick)
// 					SelectUIElement(elementAtPos, multiselect);
// 			}
// 			// If clicked on emptiness in non-multiselect mode, clear the selection
// 			else if (leftClick && !multiselect && ui.GetElementAt(pos) is null)
// 				hierarchyList.ClearSelection();

			return;
		}

		// Do not raycast / change selection if hovering over a UI element when not in PICK_UI_ELEMENTS Mode
		if (elementAtPos != activeView)
			return;
		const IntVector2& screenpos = activeView->GetScreenPosition();
		float	posx = float(pos.x_ - screenpos.x_) / float(activeView->GetWidth());
		float	posy = float(pos.y_ - screenpos.y_) / float(activeView->GetHeight());
		Ray cameraRay = camera_->GetScreenRay(posx, posy);

		Component* selectedComponent = NULL;
		if (pickMode < PICK_RIGIDBODIES)
		{
			if (editorScene->GetComponent<Octree>() == NULL)
				return;

			PODVector<RayQueryResult> result_;
			editorScene->GetComponent<Octree>()->RaycastSingle(RayOctreeQuery(result_,cameraRay, RAY_TRIANGLE, camera_->GetFarClip(),
				pickModeDrawableFlags[pickMode], 0x7fffffff));

			if (result_.Size() != 0 && result_[0].drawable_ != NULL)
			{
				Drawable* drawable = result_[0].drawable_;
				// If selecting a terrain patch, select the parent terrain instead
				if (drawable->GetTypeName() != "TerrainPatch")
				{
					selectedComponent = drawable;
					if (debug != NULL)
					{
						debug->AddNode(drawable->GetNode(), 1.0, false);
						drawable->DrawDebugGeometry(debug, false);
					}
				}
				else if (drawable->GetNode()->GetParent() != NULL)
					selectedComponent = drawable->GetNode()->GetParent()->GetComponent<Terrain>();
			}
		}
		else
		{
			if (editorScene->GetComponent<PhysicsWorld>() == NULL)
				return;

			// If we are not running the actual physics update, refresh collisions before raycasting
			if (!runUpdate)
				editorScene->GetComponent<PhysicsWorld>()->UpdateCollisions();

			PhysicsRaycastResult result;
			editorScene->GetComponent<PhysicsWorld>()->RaycastSingle(result,cameraRay, camera_->GetFarClip());

			if (result.body_ != NULL)
			{
				RigidBody* body = result.body_;
				if (debug != NULL)
				{
					debug->AddNode(body->GetNode(), 1.0, false);
					body->DrawDebugGeometry(debug, false);
				}
				selectedComponent = body;
			}
		}

		if (mouseClick && input->GetMouseButtonPress(MOUSEB_LEFT))
		{
			bool multiselect = input->GetQualifierDown(QUAL_CTRL);
			if (selectedComponent != NULL)
			{
				if (input->GetQualifierDown(QUAL_SHIFT))
				{
					// If we are selecting components, but have nodes in existing selection, do not multiselect to prevent confusion
					if (!editorSelection_->GetSelectedNodes().Empty())
						multiselect = false;
					SelectComponent(selectedComponent, multiselect);
				}
				else
				{
					// If we are selecting nodes, but have components in existing selection, do not multiselect to prevent confusion
					if (!editorSelection_->GetSelectedComponents().Empty())
						multiselect = false;
					SelectNode(selectedComponent->GetNode(), multiselect);
				}
			}
			else
			{
				// If clicked on emptiness in non-multiselect mode, clear the selection
				if (!multiselect)
					SelectComponent(NULL, false);
			}
		}

	}

	void EPScene3D::SelectComponent(Component* component, bool multiselect)
	{
		if (component == NULL && !multiselect)
		{
			editor_->GetHierarchyWindow()->GetHierarchyList()->ClearSelection();
			return;
		}

		Node* node = component->GetNode();
		if (node == NULL && !multiselect)
		{
			editor_->GetHierarchyWindow()->GetHierarchyList()->ClearSelection();
			return;
		}
		ListView* hierarchyList = editor_->GetHierarchyWindow()->GetHierarchyList();

		unsigned int nodeIndex = editor_->GetHierarchyWindow()->GetListIndex(node);
		unsigned int componentIndex = editor_->GetHierarchyWindow()->GetComponentListIndex(component);
		unsigned int numItems = hierarchyList->GetNumItems();

		if (nodeIndex < numItems && componentIndex < numItems)
		{
			// Expand the node chain now
			if (!multiselect || !hierarchyList->IsSelected(componentIndex))
			{
				// Go in the parent chain up to make sure the chain is expanded
				Node* current = node;
				do
				{
					hierarchyList->Expand(editor_->GetHierarchyWindow()->GetListIndex(current), true);
					current = current->GetParent();
				} while (current != NULL);
			}

			// This causes an event to be sent, in response we set the node/component selections, and refresh editors
			if (!multiselect)
				hierarchyList->SetSelection( componentIndex);
			else
				hierarchyList->ToggleSelection(componentIndex);
		}
		else if (!multiselect)
			hierarchyList->ClearSelection();
	}

	void EPScene3D::SelectNode(Node* node, bool multiselect)
	{
		if (node == NULL && !multiselect)
		{
			editor_->GetHierarchyWindow()->GetHierarchyList()->ClearSelection();
			return;
		}
		ListView* hierarchyList = editor_->GetHierarchyWindow()->GetHierarchyList();
		unsigned int index = editor_->GetHierarchyWindow()->GetListIndex(node);
		unsigned int numItems = hierarchyList->GetNumItems();

		if (index < numItems)
		{
			// Expand the node chain now
			if (!multiselect || !hierarchyList->IsSelected(index))
			{
				// Go in the parent chain up to make sure the chain is expanded
				Node* current = node;
				do
				{
					hierarchyList->Expand(editor_->GetHierarchyWindow()->GetListIndex(current), true);
					current = current->GetParent();
				} while (current != NULL);
			}

			// This causes an event to be sent, in response we set the node/component selections, and refresh editors
			if (!multiselect)
				hierarchyList->SetSelection( index);
			else
				hierarchyList->ToggleSelection(index);
		}
		else if (!multiselect)
			hierarchyList->ClearSelection();
	}

	void EPScene3D::SetMouseMode(bool enable)
	{
		if (enable)
		{
			if (mouseOrbitMode == ORBIT_RELATIVE)
			{
				input_->SetMouseMode(MM_RELATIVE);
				ui_->GetCursor()->SetVisible(false);
			}
			else if (mouseOrbitMode == ORBIT_WRAP)
				input_->SetMouseMode(MM_WRAP);
		}
		else
		{
			input_->SetMouseMode(MM_ABSOLUTE);
			ui_->GetCursor()->SetVisible(true);
		}
	}

	void EPScene3D::SetMouseLock()
	{
		toggledMouseLock_ = true;
		SetMouseMode(true);
	}

	void EPScene3D::ReleaseMouseLock()
	{
		if (toggledMouseLock_)
		{
			toggledMouseLock_ = false;
			SetMouseMode(false);
		}
	}

	void EPScene3D::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
	{
		using namespace PostRenderUpdate;

		Scene* scene = editorData_->GetEditorScene();

		DebugRenderer* debug = scene->GetComponent<DebugRenderer>();
		if (debug == NULL)
			return;

		// Visualize the currently selected nodes
		for (unsigned int i = 0; i < editorSelection_->GetNumSelectedNodes(); ++i)
			DrawNodeDebug(editorSelection_->GetSelectedNodes()[i], debug);

		// Visualize the currently selected components
		for (unsigned int i = 0; i < editorSelection_->GetNumSelectedComponents(); ++i)
			editorSelection_->GetSelectedComponents()[i]->DrawDebugGeometry(debug, false);

		// Visualize the currently selected UI-elements
		for (unsigned int i = 0; i < editorSelection_->GetNumSelectedUIElements(); ++i)
			ui_->DebugDraw(editorSelection_->GetSelectedUIElements()[i]);

		if (renderingDebug)
			renderer->DrawDebugGeometry(false);

		PhysicsWorld* physics = scene->GetComponent<PhysicsWorld>();
		if (physicsDebug && physics != NULL)
			physics->DrawDebugGeometry(true);

		Octree* octree = scene->GetComponent<Octree>();
		if (octreeDebug && octree != NULL)
			octree->DrawDebugGeometry(true);

		ViewRaycast(false);
	}

	void EPScene3D::ViewMouseClick(StringHash eventType, VariantMap& eventData)
	{
		using namespace UIMouseClick;

		ViewRaycast(true);
	}

	void EPScene3D::ViewMouseMove(StringHash eventType, VariantMap& eventData)
	{
		using namespace MouseMove;
	}

	void EPScene3D::ViewMouseClickEnd(StringHash eventType, VariantMap& eventData)
	{
		using namespace UIMouseClickEnd;
	}

	void EPScene3D::HandleBeginViewUpdate(StringHash eventType, VariantMap& eventData)
	{
		using namespace BeginViewUpdate;
	}

	void EPScene3D::HandleEndViewUpdate(StringHash eventType, VariantMap& eventData)
	{
		using namespace EndViewUpdate;
	}

	void EPScene3D::HandleBeginViewRender(StringHash eventType, VariantMap& eventData)
	{
		using namespace BeginViewRender;
	}

	void EPScene3D::HandleEndViewRender(StringHash eventType, VariantMap& eventData)
	{
		using namespace EndViewUpdate;
	}

	void EPScene3D::HandleResizeView(StringHash eventType, VariantMap& eventData)
	{
		if (activeView)
			activeView->SetSize(window_->GetSize());
	}

	void EPScene3D::HandleMenuBarAction(StringHash eventType, VariantMap& eventData)
	{
		using namespace MenuBarAction;

		StringHash action = eventData[P_ACTION].GetStringHash();
		if (action == A_NEWSCENE_VAR)
		{
			ResetScene();
		}
		else if (action == A_OPENSCENE_VAR)
		{
			editor_->CreateFileSelector("Open scene", "Open", "Cancel", editorData_->uiScenePath, editorData_->uiSceneFilters, editorData_->uiSceneFilter);
			SubscribeToEvent(editor_->GetUIFileSelector(), E_FILESELECTED, HANDLER(EPScene3D, HandleOpenSceneFile));
		}
		else if (action == A_SAVESCENE_VAR || action == A_SAVESCENEAS_VAR)
		{
			editor_->CreateFileSelector("Save scene as", "Save", "Cancel", editorData_->uiScenePath, editorData_->uiSceneFilters, editorData_->uiSceneFilter);
			editor_->GetUIFileSelector()->SetFileName(GetFileNameAndExtension(editorData_->GetEditorScene()->GetFileName()));
			SubscribeToEvent(editor_->GetUIFileSelector(), E_FILESELECTED, HANDLER(EPScene3D, HandleSaveSceneFile));
		}
		else if (action == A_LOADNODEASREP_VAR)
		{
			instantiateMode = REPLICATED;
			editor_->CreateFileSelector("Load node", "Load", "Cancel", editorData_->uiNodePath, editorData_->uiSceneFilters, editorData_->uiNodeFilter);
			SubscribeToEvent(editor_->GetUIFileSelector(), E_FILESELECTED, HANDLER(EPScene3D, HandleLoadNodeFile));
		}
		else if (action == A_LOADNODEASLOCAL_VAR)
		{
			instantiateMode = LOCAL;
			editor_->CreateFileSelector("Load node", "Load", "Cancel", editorData_->uiNodePath, editorData_->uiSceneFilters, editorData_->uiNodeFilter);
			SubscribeToEvent(editor_->GetUIFileSelector(), E_FILESELECTED, HANDLER(EPScene3D, HandleLoadNodeFile));
		}
		else if (action == A_SAVENODEAS_VAR)
		{
			if (editorSelection_->GetEditNode() != NULL && editorSelection_->GetEditNode() != editorData_->GetEditorScene())
			{
				editor_->CreateFileSelector("Save node", "Save", "Cancel", editorData_->uiNodePath, editorData_->uiSceneFilters, editorData_->uiNodeFilter);
				editor_->GetUIFileSelector()->SetFileName(GetFileNameAndExtension(instantiateFileName));
				SubscribeToEvent(editor_->GetUIFileSelector(), E_FILESELECTED, HANDLER(EPScene3D, HandleSaveNodeFile));
			}
		}
		else if (action == A_CREATEREPNODE_VAR)
		{
			CreateNode(REPLICATED);
		}
		else if (action == A_CREATELOCALNODE_VAR)
		{
			CreateNode(LOCAL);
		}
		else if (action == A_CREATECOMPONENT_VAR)
		{
			String uiname = eventData[P_UINAME].GetString();
			CreateComponent(uiname);
		}
		else if (action == A_CREATEBUILTINOBJ_VAR)
		{
			String uiname = eventData[P_UINAME].GetString();
			CreateBuiltinObject(uiname);
		}
	}

	void EPScene3D::HandleMessageAcknowledgement(StringHash eventType, VariantMap& eventData)
	{
		using namespace MessageACK;

		if (eventData[P_OK].GetBool())
		{
			sceneModified = false;
			ResetScene();
		}
	}

	bool EPScene3D::ResetScene()
	{
		ui_->GetCursor()->SetShape(CS_BUSY);

		if (sceneModified)
		{
			SharedPtr<MessageBox> messageBox(new MessageBox(context_, "Scene has been modified.\nContinue to reset?", "Warning"));
			messageBox->AddRef();
			if (messageBox->GetWindow() != NULL)
			{
				Button* cancelButton = (Button*)messageBox->GetWindow()->GetChild("CancelButton", true);
				cancelButton->SetVisible(true);
				cancelButton->SetFocus(true);
				SubscribeToEvent(messageBox, E_MESSAGEACK, HANDLER(EPScene3D, HandleMessageAcknowledgement));

				return false;
			}
		}

		// Clear stored script attributes
		//scriptAttributes.Clear();

		Editor* editor = editorData_->GetEditor();
		editor->GetHierarchyWindow()->SetSuppressSceneChanges(true);

		// Create a scene with default values, these will be overridden when loading scenes
		editorData_->GetEditorScene()->Clear();
		editorData_->GetEditorScene()->CreateComponent<Octree>();
		editorData_->GetEditorScene()->CreateComponent<DebugRenderer>();

		// Release resources that became unused after the scene clear
		//cache.ReleaseAllResources(false);

		sceneModified = false;
		revertData = NULL;
		StopSceneUpdate();

		//		UpdateWindowTitle();
		//		DisableInspectorLock();
		editor->GetHierarchyWindow()->UpdateHierarchyItem(editorData_->GetEditorScene(), true);
		//		ClearEditActions();

		editor->GetHierarchyWindow()->SetSuppressSceneChanges(false);

		ResetCamera();
		//	CreateGizmo();
		CreateGrid();
		//	SetActiveViewport(viewports[0]);

		return true;
	}

	void EPScene3D::HandleOpenSceneFile(StringHash eventType, VariantMap& eventData)
	{
		editor_->CloseFileSelector(editorData_->uiSceneFilter, editorData_->uiScenePath);
		LoadScene(UIUtils::ExtractFileName(eventData));
	}

	void EPScene3D::HandleSaveSceneFile(StringHash eventType, VariantMap& eventData)
	{
		editor_->CloseFileSelector(editorData_->uiSceneFilter, editorData_->uiScenePath);
		SaveScene(UIUtils::ExtractFileName(eventData, true));
	}

	void EPScene3D::HandleLoadNodeFile(StringHash eventType, VariantMap& eventData)
	{
		editor_->CloseFileSelector(editorData_->uiSceneFilter, editorData_->uiScenePath);
		LoadNode(UIUtils::ExtractFileName(eventData));
	}

	void EPScene3D::HandleSaveNodeFile(StringHash eventType, VariantMap& eventData)
	{
		editor_->CloseFileSelector(editorData_->uiSceneFilter, editorData_->uiScenePath);
		SaveNode(UIUtils::ExtractFileName(eventData, true));
	}

	bool EPScene3D::LoadScene(const String& fileName)
	{
		if (fileName.Empty())
			return false;

		ui_->GetCursor()->SetShape(CS_BUSY);

		// Always load the scene from the filesystem, not from resource paths
		if (!fileSystem_->FileExists(fileName))
		{
			LOGERRORF("No such scene %s", fileName.CString());

			MessageBox(context_, "No such scene.\n" + fileName);
			return false;
		}

		File file(context_);
		if (!file.Open(fileName, FILE_READ))
		{
			LOGERRORF("Could not open file %s", fileName.CString());

			MessageBox(context_, "Could not open file.\n" + fileName);
			return false;
		}

		// Reset stored script attributes.
		// 	scriptAttributes.Clear();
		//
		// 	// Add the scene's resource path in case it's necessary
		// 	String newScenePath = GetPath(fileName);
		// 	if (!rememberResourcePath || !sceneResourcePath.StartsWith(newScenePath, false))
		// 		SetResourcePath(newScenePath);

		editor_->GetHierarchyWindow()->SetSuppressSceneChanges(true);
		sceneModified = false;
		revertData = NULL;
		StopSceneUpdate();

		String extension = GetExtension(fileName);
		bool loaded;
		if (extension != ".xml")
			loaded = editorData_->GetEditorScene()->Load(file);
		else
			loaded = editorData_->GetEditorScene()->LoadXML(file);

		// Release resources which are not used by the new scene
		/// \todo this creates an bug in the attribute inspector because the loaded xml files are released
		cache_->ReleaseAllResources(false);

		// Always pause the scene, and do updates manually
		editorData_->GetEditorScene()->SetUpdateEnabled(false);

		// 	UpdateWindowTitle();
		// 	DisableInspectorLock();
		editor_->GetHierarchyWindow()->UpdateHierarchyItem(editorData_->GetEditorScene(), true);
		// 	ClearEditActions();
		//

		editor_->GetHierarchyWindow()->SetSuppressSceneChanges(false);
		/// \todo
		editorSelection_->ClearSelection();
		editor_->GetAttributeWindow()->GetEditNodes() = editorSelection_->GetEditNodes();
		editor_->GetAttributeWindow()->GetEditComponents() = editorSelection_->GetEditComponents();
		editor_->GetAttributeWindow()->GetEditUIElements() = editorSelection_->GetEditUIElements();
		editor_->GetAttributeWindow()->Update();
		//
		// 	// global variable to mostly bypass adding mru upon importing tempscene
		// 	if (!skipMruScene)
		// 		UpdateSceneMru(fileName);
		//
		// 	skipMruScene = false;
		//
		ResetCamera();
		// 	CreateGizmo();
		CreateGrid();
		// 	SetActiveViewport(viewports[0]);
		//
		// 	// Store all ScriptInstance and LuaScriptInstance attributes
		// 	UpdateScriptInstances();

		return loaded;
	}

	bool EPScene3D::SaveScene(const String& fileName)
	{
		if (fileName.Empty())
			return false;

		ui_->GetCursor()->SetShape(CS_BUSY);

		// Unpause when saving so that the scene will work properly when loaded outside the editor
		editorData_->GetEditorScene()->SetUpdateEnabled(true);

		MakeBackup(fileName);
		File file(context_, fileName, FILE_WRITE);
		String extension = GetExtension(fileName);
		bool success = (extension != ".xml" ? editorData_->GetEditorScene()->Save(file) : editorData_->GetEditorScene()->SaveXML(file));
		RemoveBackup(success, fileName);

		editorData_->GetEditorScene()->SetUpdateEnabled(false);

		if (success)
		{
			//	UpdateSceneMru(fileName);
			sceneModified = false;
			//	UpdateWindowTitle();
		}
		else
			MessageBox(context_, "Could not save scene successfully!\nSee Urho3D.log for more detail.");

		return success;
	}

	Node* EPScene3D::LoadNode(const String& fileName, Node* parent /*= NULL*/)
	{
		if (fileName.Empty())
			return NULL;

		if (!fileSystem_->FileExists(fileName))
		{
			MessageBox(context_, "No such node file.\n" + fileName);
			return NULL;
		}

		File file(context_);
		if (!file.Open(fileName, FILE_READ))
		{
			MessageBox(context_, "Could not open file.\n" + fileName);
			return NULL;
		}

		ui_->GetCursor()->SetShape(CS_BUSY);

		// Before instantiating, add object's resource path if necessary
		//SetResourcePath(GetPath(fileName), true, true);

		Ray cameraRay = camera_->GetScreenRay(0.5, 0.5); // Get ray at view center
		Vector3 position, normal;
		//	GetSpawnPosition(cameraRay, newNodeDistance, position, normal, 0, true);

		Node* newNode = InstantiateNodeFromFile(&file, position, Quaternion(), 1, parent, instantiateMode);
		if (newNode != NULL)
		{
			//FocusNode(newNode);
			instantiateFileName = fileName;
		}
		return newNode;
	}

	bool EPScene3D::SaveNode(const String& fileName)
	{
		if (fileName.Empty())
			return false;

		ui_->GetCursor()->SetShape(CS_BUSY);

		MakeBackup(fileName);

		File file(context_);
		if (!file.Open(fileName, FILE_WRITE))
		{
			MessageBox(context_, "Could not open file.\n" + fileName);
			return NULL;
		}

		String extension = GetExtension(fileName);
		bool success = (extension != ".xml" ? editorSelection_->GetEditNode()->Save(file) : editorSelection_->GetEditNode()->SaveXML(file));
		RemoveBackup(success, fileName);

		if (success)
			instantiateFileName = fileName;
		else
			MessageBox(context_, "Could not save node successfully!\nSee Urho3D.log for more detail.");

		return success;
	}

	Node* EPScene3D::InstantiateNodeFromFile(File* file, const Vector3& position, const Quaternion& rotation, float scaleMod /*= 1.0f*/, Node* parent /*= NULL*/, CreateMode mode /*= REPLICATED*/)
	{
		if (file == NULL)
			return NULL;

		Node* newNode = NULL;
		unsigned int numSceneComponent = editorData_->GetEditorScene()->GetNumComponents();

		editor_->GetHierarchyWindow()->SetSuppressSceneChanges(true);

		String extension = GetExtension(file->GetName());
		if (extension != ".xml")
			newNode = editorData_->GetEditorScene()->Instantiate(*file, position, rotation, mode);
		else
			newNode = editorData_->GetEditorScene()->InstantiateXML(*file, position, rotation, mode);

		editor_->GetHierarchyWindow()->SetSuppressSceneChanges(false);

		if (parent != NULL)
			newNode->SetParent(parent);

		if (newNode != NULL)
		{
			newNode->SetScale(newNode->GetScale() * scaleMod);
			// 			if (alignToAABBBottom)
			// 			{
			// 				Drawable@ drawable = GetFirstDrawable(newNode);
			// 				if (drawable !is null)
			// 				{
			// 					BoundingBox aabb = drawable.worldBoundingBox;
			// 					Vector3 aabbBottomCenter(aabb.center.x, aabb.min.y, aabb.center.z);
			// 					Vector3 offset = aabbBottomCenter - newNode.worldPosition;
			// 					newNode.worldPosition = newNode.worldPosition - offset;
			// 				}
			// 			}

			// Create an undo action for the load
			// 			CreateNodeAction action;
			// 			action.Define(newNode);
			// 			SaveEditAction(action);
			// 			SetSceneModified();
			sceneModified = true;

			if (numSceneComponent != editorData_->GetEditorScene()->GetNumComponents())
				editor_->GetHierarchyWindow()->UpdateHierarchyItem(editorData_->GetEditorScene());
			else
				editor_->GetHierarchyWindow()->UpdateHierarchyItem(newNode);
		}

		return newNode;
	}

	Node* EPScene3D::CreateNode(CreateMode mode)
	{
		Node* newNode = NULL;
		if (editorSelection_->GetEditNode() != NULL)
			newNode = editorSelection_->GetEditNode()->CreateChild("", mode);
		else
			newNode = editorData_->GetEditorScene()->CreateChild("", mode);
		// Set the new node a certain distance from the camera
		//	newNode.position = GetNewNodePosition();

		// Create an undo action for the create
		// 		CreateNodeAction action;
		// 		action.Define(newNode);
		// 		SaveEditAction(action);
		// 		SetSceneModified();
		sceneModified = true;

		//		FocusNode(newNode);

		return newNode;
	}

	void EPScene3D::CreateComponent(const String& componentType)
	{
		// If this is the root node, do not allow to create duplicate scene-global components
		if (editorSelection_->GetEditNode() == editorData_->GetEditorScene() && CheckForExistingGlobalComponent(editorSelection_->GetEditNode(), componentType))
			return;

		// Group for storing undo actions
		//EditActionGroup group;

		// For now, make a local node's all components local
		/// \todo Allow to specify the createmode
		for (unsigned int i = 0; i < editorSelection_->GetNumEditNodes(); ++i)
		{
			Component* newComponent = editorSelection_->GetEditNodes()[i]->CreateComponent(componentType, editorSelection_->GetEditNodes()[i]->GetID() < FIRST_LOCAL_ID ? REPLICATED : LOCAL);
			if (newComponent != NULL)
			{
				// Some components such as CollisionShape do not create their internal object before the first call to ApplyAttributes()
				// to prevent unnecessary initialization with default values. Call now
				newComponent->ApplyAttributes();

				// 				CreateComponentAction action;
				// 				action.Define(newComponent);
				// 				group.actions.Push(action);
			}
		}

		// 		SaveEditActionGroup(group);
		// 		SetSceneModified();
		sceneModified = true;

		// Although the edit nodes selection are not changed, call to ensure attribute inspector notices new components of the edit nodes
		//	HandleHierarchyListSelectionChange();

		editor_->GetAttributeWindow()->Update();
	}

	void EPScene3D::CreateBuiltinObject(const String& name)
	{
		Node* newNode = editorData_->GetEditorScene()->CreateChild(name, REPLICATED);
		// Set the new node a certain distance from the camera
		//	newNode.position = GetNewNodePosition();

		StaticModel* object = newNode->CreateComponent<StaticModel>();

		object->SetModel(cache_->GetResource<Model>("Models/" + name + ".mdl"));

		// Create an undo action for the create
		// 		CreateNodeAction action;
		// 		action.Define(newNode);
		// 		SaveEditAction(action);
		// 		SetSceneModified();

		sceneModified = true;

		//		FocusNode(newNode);
	}

	bool EPScene3D::CheckForExistingGlobalComponent(Node* node, const String& typeName)
	{
		if (typeName != "Octree" && typeName != "PhysicsWorld" && typeName != "DebugRenderer")
			return false;
		else
			return node->HasComponent(typeName);
	}

	void EPScene3D::MiniToolBarCreateLocalNode(StringHash eventType, VariantMap& eventData)
	{
		CreateNode(LOCAL);
	}

	void EPScene3D::MiniToolBarCreateReplNode(StringHash eventType, VariantMap& eventData)
	{
		CreateNode(REPLICATED);
	}

	void EPScene3D::MiniToolBarCreateComponent(StringHash eventType, VariantMap& eventData)
	{
		Button* b = dynamic_cast<Button*>(GetEventSender());
		if (b)
			CreateComponent(b->GetName());
	}

	void EPScene3D::UpdateToolBar()
	{
		/// \todo

		ToolBarUI* toolBar = editorView_->GetToolBar();

		CheckBox* checkbox = (CheckBox*)toolBar->GetChild("RunUpdatePlay", true);
		if (checkbox->IsChecked() != runUpdate)
			checkbox->SetChecked( runUpdate);

		checkbox = (CheckBox*)toolBar->GetChild("RunUpdatePause", true);
		if (checkbox->IsChecked() != (runUpdate == false))
			checkbox->SetChecked(runUpdate == false);

		checkbox = (CheckBox*)toolBar->GetChild("RevertOnPause", true);
		if (checkbox->IsChecked() != revertOnPause)
			checkbox->SetChecked(revertOnPause);

		checkbox = (CheckBox*)toolBar->GetChild("EditMove", true);
		if (checkbox->IsChecked() != (editMode == EDIT_MOVE))
			checkbox->SetChecked(editMode == EDIT_MOVE);

		checkbox = (CheckBox*)toolBar->GetChild("EditRotate", true);
		if (checkbox->IsChecked() != (editMode == EDIT_ROTATE))
			checkbox->SetChecked(editMode == EDIT_ROTATE);

		checkbox = (CheckBox*)toolBar->GetChild("EditScale", true);
		if (checkbox->IsChecked() != (editMode == EDIT_SCALE))
			checkbox->SetChecked(editMode == EDIT_SCALE);

		checkbox = (CheckBox*)toolBar->GetChild("EditSelect", true);
		if (checkbox->IsChecked() != (editMode == EDIT_SELECT))
			checkbox->SetChecked(editMode == EDIT_SELECT);

		checkbox = (CheckBox*)toolBar->GetChild("AxisWorld", true);
		if (checkbox->IsChecked() != (axisMode == AXIS_WORLD))
			checkbox->SetChecked(axisMode == AXIS_WORLD);

		checkbox = (CheckBox*)toolBar->GetChild("AxisLocal", true);
		if (checkbox->IsChecked() != (axisMode == AXIS_LOCAL))
			checkbox->SetChecked(axisMode == AXIS_LOCAL);

		checkbox = (CheckBox*)toolBar->GetChild("MoveSnap", true);
		if (checkbox->IsChecked() != moveSnap)
			checkbox->SetChecked(moveSnap);

		checkbox = (CheckBox*)toolBar->GetChild("RotateSnap", true);
		if (checkbox->IsChecked() != rotateSnap)
			checkbox->SetChecked(rotateSnap);

		checkbox = (CheckBox*)toolBar->GetChild("ScaleSnap", true);
		if (checkbox->IsChecked() != scaleSnap)
			checkbox->SetChecked(scaleSnap);

		checkbox = (CheckBox*)toolBar->GetChild("SnapScaleHalf", true);
		if (checkbox->IsChecked() != (snapScaleMode == SNAP_SCALE_HALF))
			checkbox->SetChecked(snapScaleMode == SNAP_SCALE_HALF);

		checkbox = (CheckBox*)toolBar->GetChild("SnapScaleQuarter", true);
		if (checkbox->IsChecked() != (snapScaleMode == SNAP_SCALE_QUARTER))
			checkbox->SetChecked(snapScaleMode == SNAP_SCALE_QUARTER);

		checkbox = (CheckBox*)toolBar->GetChild("PickGeometries", true);
		if (checkbox->IsChecked() != (pickMode == PICK_GEOMETRIES))
			checkbox->SetChecked(pickMode == PICK_GEOMETRIES);

		checkbox = (CheckBox*)toolBar->GetChild("PickLights", true);
		if (checkbox->IsChecked() != (pickMode == PICK_LIGHTS))
			checkbox->SetChecked(pickMode == PICK_LIGHTS);

		checkbox = (CheckBox*)toolBar->GetChild("PickZones", true);
		if (checkbox->IsChecked() != (pickMode == PICK_ZONES))
			checkbox->SetChecked(pickMode == PICK_ZONES);

		checkbox = (CheckBox*)toolBar->GetChild("PickRigidBodies", true);
		if (checkbox->IsChecked() != (pickMode == PICK_RIGIDBODIES))
			checkbox->SetChecked(pickMode == PICK_RIGIDBODIES);

		checkbox = (CheckBox*)toolBar->GetChild("PickUIElements", true);
		if (checkbox->IsChecked() != (pickMode == PICK_UI_ELEMENTS))
			checkbox->SetChecked(pickMode == PICK_UI_ELEMENTS);

		checkbox = (CheckBox*)toolBar->GetChild("FillPoint", true);
		if (checkbox->IsChecked() != (fillMode == FILL_POINT))
			checkbox->SetChecked(fillMode == FILL_POINT);

		checkbox = (CheckBox*)toolBar->GetChild("FillWireFrame", true);
		if (checkbox->IsChecked() != (fillMode == FILL_WIREFRAME))
			checkbox->SetChecked(fillMode == FILL_WIREFRAME);

		checkbox = (CheckBox*)toolBar->GetChild("FillSolid", true);
		if (checkbox->IsChecked() != (fillMode == FILL_SOLID))
			checkbox->SetChecked(fillMode == FILL_SOLID);

		toolBarDirty = false;
	}

	void EPScene3D::ToolBarRunUpdatePlay(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;

		CheckBox* edit = (CheckBox*)eventData[P_ELEMENT].GetPtr();
		if (edit && edit->IsChecked())
			StartSceneUpdate();
		toolBarDirty = true;
	}

	void EPScene3D::ToolBarRunUpdatePause(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* edit = (CheckBox*)eventData[P_ELEMENT].GetPtr();
		if (edit && edit->IsChecked())
			StopSceneUpdate();
		toolBarDirty = true;
	}

	void EPScene3D::ToolBarRevertOnPause(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* edit = (CheckBox*)eventData[P_ELEMENT].GetPtr();
		revertOnPause = edit->IsChecked();
		toolBarDirty = true;
	}

	void EPScene3D::ToolBarEditModeMove(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* edit = (CheckBox*)eventData[P_ELEMENT].GetPtr();
		if (edit && edit->IsChecked())
			editMode = EDIT_MOVE;
		toolBarDirty = true;
	}

	void EPScene3D::ToolBarEditModeRotate(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* edit = (CheckBox*)eventData[P_ELEMENT].GetPtr();
		if (edit && edit->IsChecked())
			editMode = EDIT_ROTATE;
		toolBarDirty = true;
	}

	void EPScene3D::ToolBarEditModeScale(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* edit = (CheckBox*)eventData[P_ELEMENT].GetPtr();
		if (edit && edit->IsChecked())
			editMode = EDIT_SCALE;
		toolBarDirty = true;
	}

	void EPScene3D::ToolBarEditModeSelect(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* edit = (CheckBox*)eventData[P_ELEMENT].GetPtr();
		if (edit && edit->IsChecked())
			editMode = EDIT_SELECT;
		toolBarDirty = true;
	}

	void EPScene3D::ToolBarAxisModeWorld(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* edit = (CheckBox*)eventData[P_ELEMENT].GetPtr();
		if (edit && edit->IsChecked())
			axisMode = AXIS_WORLD;
		toolBarDirty = true;
	}

	void EPScene3D::ToolBarAxisModeLocal(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* edit = (CheckBox*)eventData[P_ELEMENT].GetPtr();
		if (edit && edit->IsChecked())
			axisMode = AXIS_LOCAL;
		toolBarDirty = true;
	}

	void EPScene3D::ToolBarMoveSnap(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* edit = (CheckBox*)eventData[P_ELEMENT].GetPtr();
		moveSnap = edit->IsChecked();
		toolBarDirty = true;
	}

	void EPScene3D::ToolBarRotateSnap(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* edit = (CheckBox*)eventData[P_ELEMENT].GetPtr();
		rotateSnap = edit->IsChecked();
		toolBarDirty = true;
	}

	void EPScene3D::ToolBarScaleSnap(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* edit = (CheckBox*)eventData[P_ELEMENT].GetPtr();
		scaleSnap = edit->IsChecked();
		toolBarDirty = true;
	}

	void EPScene3D::ToolBarSnapScaleModeHalf(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* edit = (CheckBox*)eventData[P_ELEMENT].GetPtr();
		if (edit && edit->IsChecked())
		{
			snapScaleMode = SNAP_SCALE_HALF;
			snapScale = 0.5;
		}
		else if (snapScaleMode == SNAP_SCALE_HALF)
		{
			snapScaleMode = SNAP_SCALE_FULL;
			snapScale = 1.0;
		}
		toolBarDirty = true;
	}

	void EPScene3D::ToolBarSnapScaleModeQuarter(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* edit = (CheckBox*)eventData[P_ELEMENT].GetPtr();
		if (edit && edit->IsChecked())
		{
			snapScaleMode = SNAP_SCALE_QUARTER;
			snapScale = 0.25;
		}
		else if (snapScaleMode == SNAP_SCALE_QUARTER)
		{
			snapScaleMode = SNAP_SCALE_FULL;
			snapScale = 1.0;
		}
		toolBarDirty = true;
	}

	void EPScene3D::ToolBarPickModeGeometries(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* edit = (CheckBox*)eventData[P_ELEMENT].GetPtr();
		if (edit && edit->IsChecked())
			pickMode = PICK_GEOMETRIES;
		toolBarDirty = true;
	}

	void EPScene3D::ToolBarPickModeLights(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* edit = (CheckBox*)eventData[P_ELEMENT].GetPtr();
		if (edit && edit->IsChecked())
			pickMode = PICK_LIGHTS;
		toolBarDirty = true;
	}

	void EPScene3D::ToolBarPickModeZones(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* edit = (CheckBox*)eventData[P_ELEMENT].GetPtr();
		if (edit && edit->IsChecked())
			pickMode = PICK_ZONES;
		toolBarDirty = true;
	}

	void EPScene3D::ToolBarPickModeRigidBodies(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* edit = (CheckBox*)eventData[P_ELEMENT].GetPtr();
		if (edit && edit->IsChecked())
			pickMode = PICK_RIGIDBODIES;
		toolBarDirty = true;
	}

	void EPScene3D::ToolBarPickModeUIElements(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* edit = (CheckBox*)eventData[P_ELEMENT].GetPtr();
		if (edit && edit->IsChecked())
			pickMode = PICK_UI_ELEMENTS;
		toolBarDirty = true;
	}

	void EPScene3D::ToolBarFillModePoint(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* edit = (CheckBox*)eventData[P_ELEMENT].GetPtr();
		if (edit && edit->IsChecked())
		{
			fillMode = FILL_POINT;
			SetFillMode(fillMode);
		}
		toolBarDirty = true;
	}

	void EPScene3D::ToolBarFillModeWireFrame(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* edit = (CheckBox*)eventData[P_ELEMENT].GetPtr();
		if (edit && edit->IsChecked())
		{
			fillMode = FILL_WIREFRAME;
			SetFillMode(fillMode);
		}
		toolBarDirty = true;
	}

	void EPScene3D::ToolBarFillModeSolid(StringHash eventType, VariantMap& eventData)
	{
		using namespace Toggled;
		CheckBox* edit = (CheckBox*)eventData[P_ELEMENT].GetPtr();
		if (edit && edit->IsChecked())
		{
			fillMode = FILL_SOLID;
			SetFillMode(fillMode);
		}
		toolBarDirty = true;
	}

	void EPScene3D::MakeBackup(const String& fileName)
	{
		fileSystem_->Rename(fileName, fileName + ".old");
	}

	void EPScene3D::RemoveBackup(bool success, const String& fileName)
	{
		if (success)
			fileSystem_->Delete(fileName + ".old");
	}

	void EPScene3D::HideGrid()
	{
		if (grid_ != NULL)
			grid_->SetEnabled(false);
	}

	void EPScene3D::ShowGrid()
	{
		if (grid_ != NULL)
		{
			grid_->SetEnabled(true);

			EditorData* editorData_ = GetSubsystem<EditorData>();
			if (editorData_->GetEditorScene()->GetComponent<Octree>() != NULL)
				editorData_->GetEditorScene()->GetComponent<Octree>()->AddManualDrawable(grid_);
		}
	}

	void EPScene3D::StartSceneUpdate()
	{
		runUpdate = true;
		// Run audio playback only when scene is updating, so that audio components' time-dependent attributes stay constant when
		// paused (similar to physics)
		//audio.Play();
		toolBarDirty = true;

		// Save scene data for reverting if enabled
		if (revertOnPause)
		{
			revertData = new XMLFile(context_);
			XMLElement root = revertData->CreateRoot("scene");
			editorData_->GetEditorScene()->SaveXML(root);
		}
		else
			revertData = NULL;
	}

	void EPScene3D::StopSceneUpdate()
	{
		runUpdate = false;
		//audio.Stop();
		toolBarDirty = true;

		// If scene should revert on update stop, load saved data now
		if (revertOnPause && revertData.NotNull())
		{
			editor_->GetHierarchyWindow()->SetSuppressSceneChanges(true);

			editorData_->GetEditorScene()->Clear();
			editorData_->GetEditorScene()->LoadXML(revertData->GetRoot());
			CreateGrid();
			editor_->GetHierarchyWindow()->UpdateHierarchyItem(editorData_->GetEditorScene(), true);
			//ClearEditActions();
			editor_->GetHierarchyWindow()->SetSuppressSceneChanges(false);
		}

		revertData = NULL;
	}

	void EPScene3D::CreateGrid()
	{
		ResourceCache* cache = GetSubsystem<ResourceCache>();
		if (!gridNode_)
		{
			gridNode_ = new Node(context_);
			grid_ = gridNode_->CreateComponent<CustomGeometry>();
			grid_->SetNumGeometries(1);
			grid_->SetMaterial(cache->GetResource<Material>("Materials/VColUnlit.xml"));
			grid_->SetViewMask(0x80000000); // Editor raycasts use viewmask 0x7fffffff
			grid_->SetOccludee(false);
		}
		UpdateGrid();
	}

	void EPScene3D::UpdateGrid(bool updateGridGeometry /*= true*/)
	{
		showGrid_ ? ShowGrid() : HideGrid();
		gridNode_->SetScale(Vector3(8.0f, 8.0f, 8.0f));

		if (!updateGridGeometry)
			return;

		unsigned int size = unsigned int(floor(8.0f / 2.0f) * 2.0f);
		float halfSizeScaled = size / 2.0f;
		float scale = 1.0f;
		unsigned int subdivisionSize = unsigned int(pow(2.0f, 3.0f));

		if (subdivisionSize > 0)
		{
			size *= subdivisionSize;
			scale /= subdivisionSize;
		}

		unsigned int halfSize = size / 2;

		grid_->BeginGeometry(0, LINE_LIST);
		float lineOffset = -halfSizeScaled;
		for (unsigned int i = 0; i <= size; ++i)
		{
			bool lineCenter = i == halfSize;
			bool lineSubdiv = !Equals(float(i% subdivisionSize), 0.0f);

			if (!grid2DMode_)
			{
				grid_->DefineVertex(Vector3(lineOffset, 0.0, halfSizeScaled));
				grid_->DefineColor(lineCenter ? gridZColor : (lineSubdiv ? gridSubdivisionColor : gridColor));
				grid_->DefineVertex(Vector3(lineOffset, 0.0, -halfSizeScaled));
				grid_->DefineColor(lineCenter ? gridZColor : (lineSubdiv ? gridSubdivisionColor : gridColor));

				grid_->DefineVertex(Vector3(-halfSizeScaled, 0.0, lineOffset));
				grid_->DefineColor(lineCenter ? gridXColor : (lineSubdiv ? gridSubdivisionColor : gridColor));
				grid_->DefineVertex(Vector3(halfSizeScaled, 0.0, lineOffset));
				grid_->DefineColor(lineCenter ? gridXColor : (lineSubdiv ? gridSubdivisionColor : gridColor));
			}
			else
			{
				grid_->DefineVertex(Vector3(lineOffset, halfSizeScaled, 0.0));
				grid_->DefineColor(lineCenter ? gridYColor : (lineSubdiv ? gridSubdivisionColor : gridColor));
				grid_->DefineVertex(Vector3(lineOffset, -halfSizeScaled, 0.0));
				grid_->DefineColor(lineCenter ? gridYColor : (lineSubdiv ? gridSubdivisionColor : gridColor));

				grid_->DefineVertex(Vector3(-halfSizeScaled, lineOffset, 0.0));
				grid_->DefineColor(lineCenter ? gridXColor : (lineSubdiv ? gridSubdivisionColor : gridColor));
				grid_->DefineVertex(Vector3(halfSizeScaled, lineOffset, 0.0));
				grid_->DefineColor(lineCenter ? gridXColor : (lineSubdiv ? gridSubdivisionColor : gridColor));
			}

			lineOffset += scale;
		}
		grid_->Commit();
	}

	EPScene3DView::EPScene3DView(Context* context) : BorderImage(context),
		ownScene_(true),
		rttFormat_(Graphics::GetRGBFormat()),
		autoUpdate_(true),
		cameraYaw_(0.0f),
		cameraPitch_(0.0f)
	{
		SetEnabled(true);
		bringToFront_ = true;

		renderTexture_ = new Texture2D(context_);
		depthTexture_ = new Texture2D(context_);
		viewport_ = new Viewport(context_);

		cameraNode_ = new Node(context_);
		camera_ = cameraNode_->CreateComponent<Camera>();
		//		camera.fillMode = fillMode;
		soundListener_ = cameraNode_->CreateComponent<SoundListener>();
		camera_->SetViewMask(0xffffffff); // It's easier to only have 1 gizmo active this viewport is shared with the gizmo
	}

	EPScene3DView::~EPScene3DView()
	{
		ResetScene();
	}

	void EPScene3DView::RegisterObject(Context* context)
	{
		context->RegisterFactory<EPScene3DView>();
	}

	void EPScene3DView::OnResize()
	{
		int width = GetWidth();
		int height = GetHeight();

		if (width > 0 && height > 0)
		{
			renderTexture_->SetSize(width, height, rttFormat_, TEXTURE_RENDERTARGET);
			depthTexture_->SetSize(width, height, Graphics::GetDepthStencilFormat(), TEXTURE_DEPTHSTENCIL);
			RenderSurface* surface = renderTexture_->GetRenderSurface();
			surface->SetViewport(0, viewport_);
			surface->SetUpdateMode(autoUpdate_ ? SURFACE_UPDATEALWAYS : SURFACE_MANUALUPDATE);
			surface->SetLinkedDepthStencil(depthTexture_->GetRenderSurface());

			SetTexture(renderTexture_);
			SetImageRect(IntRect(0, 0, width, height));

			if (!autoUpdate_)
				surface->QueueUpdate();
		}
		HandleResize();
	}

	void EPScene3DView::OnHover(const IntVector2& position, const IntVector2& screenPosition, int buttons, int qualifiers, Cursor* cursor)
	{
		UIElement::OnHover(position, screenPosition, buttons, qualifiers, cursor);
	}

	void EPScene3DView::OnClickBegin(const IntVector2& position, const IntVector2& screenPosition, int button, int buttons, int qualifiers, Cursor* cursor)
	{
		UIElement::OnClickBegin(position, screenPosition, button, buttons, qualifiers, cursor);
		UI* ui = GetSubsystem<UI>();
		ui->SetFocusElement(NULL);
	}

	void EPScene3DView::OnClickEnd(const IntVector2& position, const IntVector2& screenPosition, int button, int buttons, int qualifiers, Cursor* cursor, UIElement* beginElement)
	{
		UIElement::OnClickEnd(position, screenPosition, button, buttons, qualifiers, cursor, beginElement);
	}

	void EPScene3DView::SetView(Scene* scene, bool ownScene /*= true*/)
	{
		ResetScene();
		ResetCamera();

		scene_ = scene;
		ownScene_ = ownScene;

		viewport_->SetScene(scene_);
		viewport_->SetCamera(camera_);
		QueueUpdate();
	}

	void EPScene3DView::SetFormat(unsigned format)
	{
		if (format != rttFormat_)
		{
			rttFormat_ = format;
			OnResize();
		}
	}

	void EPScene3DView::SetAutoUpdate(bool enable)
	{
		if (enable != autoUpdate_)
		{
			autoUpdate_ = enable;
			RenderSurface* surface = renderTexture_->GetRenderSurface();
			if (surface)
				surface->SetUpdateMode(autoUpdate_ ? SURFACE_UPDATEALWAYS : SURFACE_MANUALUPDATE);
		}
	}

	void EPScene3DView::QueueUpdate()
	{
		if (!autoUpdate_)
		{
			RenderSurface* surface = renderTexture_->GetRenderSurface();
			if (surface)
				surface->QueueUpdate();
		}
	}

	Scene* EPScene3DView::GetScene() const
	{
		return scene_;
	}

	Node* EPScene3DView::GetCameraNode() const
	{
		return cameraNode_;
	}

	void EPScene3DView::ResetCamera()
	{
		cameraNode_->SetPosition(Vector3(0.0f, 5.0f, -10.0f));
		// Look at the origin so user can see the scene.
		cameraNode_->SetRotation(Quaternion(Vector3(0.0f, 0.0f, 1.0f), -cameraNode_->GetPosition()));
		ReacquireCameraYawPitch();
		//	UpdateSettingsUI();
	}

	void EPScene3DView::ResetCamera(StringHash eventType, VariantMap& eventData)
	{
		ResetCamera();
	}

	void EPScene3DView::CloseViewportSettingsWindow(StringHash eventType, VariantMap& eventData)
	{
		settingsWindow->SetVisible(false);
	}

	void EPScene3DView::UpdateSettingsUI(StringHash eventType, VariantMap& eventData)
	{
		cameraPosX->SetText(String(cameraNode_->GetPosition().x_));
		cameraPosY->SetText(String(cameraNode_->GetPosition().y_));
		cameraPosZ->SetText(String(cameraNode_->GetPosition().z_));
		cameraRotX->SetText(String(cameraNode_->GetRotation().PitchAngle()));
		cameraRotY->SetText(String(cameraNode_->GetRotation().YawAngle()));
		cameraRotZ->SetText(String(cameraNode_->GetRotation().RollAngle()));
		cameraZoom->SetText(String(camera_->GetZoom()));
		cameraOrthoSize->SetText(String(camera_->GetOrthoSize()));
		cameraOrthographic->SetChecked(camera_->IsOrthographic());
	}

	void EPScene3DView::OpenViewportSettingsWindow()
	{
		UpdateSettingsUI(StringHash::ZERO, VariantMap());

		settingsWindow->SetVisible(true);
		settingsWindow->BringToFront();
	}

	void EPScene3DView::ReacquireCameraYawPitch()
	{
		cameraYaw_ = cameraNode_->GetRotation().YawAngle();
		cameraPitch_ = cameraNode_->GetRotation().PitchAngle();
	}

	void EPScene3DView::CreateViewportContextUI(XMLFile* uiStyle, XMLFile* iconStyle_)
	{
		ResourceCache* cache = GetSubsystem<ResourceCache>();
		Font* font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");
		UI* ui = GetSubsystem<UI>();

		statusBar = CreateChild<BorderImage>("ToolBar");
		AddChild(statusBar);

		statusBar->SetStyle("EditorToolBar");

		statusBar->SetLayout(LM_HORIZONTAL);
		statusBar->SetAlignment(HA_LEFT, VA_BOTTOM);
		statusBar->SetLayoutSpacing(4);
		//statusBar->SetOpacity(editor_->GetuiMaxOpacity());

		Button* settingsButton = UIUtils::CreateSmallToolBarButton(context_, uiStyle, iconStyle_, "Settings");
		statusBar->AddChild(settingsButton);

		cameraPosText = new  Text(context_);
		statusBar->AddChild(cameraPosText);

		cameraPosText->SetFont(font, 11);
		cameraPosText->SetColor(Color(1.0f, 1.0f, 0.0f));
		cameraPosText->SetTextEffect(TE_SHADOW);
		cameraPosText->SetPriority(-100);

		settingsWindow = DynamicCast<Window>(ui->LoadLayout(cache->GetResource<XMLFile>("UI/EditorViewport.xml")));
		//settingsWindow->SetOpacity(editor_->GetuiMaxOpacity());
		settingsWindow->SetVisible(false);
		AddChild(settingsWindow);

		cameraPosX = (LineEdit*)settingsWindow->GetChild("PositionX", true);
		cameraPosY = (LineEdit*)settingsWindow->GetChild("PositionY", true);
		cameraPosZ = (LineEdit*)settingsWindow->GetChild("PositionZ", true);
		cameraRotX = (LineEdit*)settingsWindow->GetChild("RotationX", true);
		cameraRotY = (LineEdit*)settingsWindow->GetChild("RotationY", true);
		cameraRotZ = (LineEdit*)settingsWindow->GetChild("RotationZ", true);
		cameraOrthographic = (CheckBox*)settingsWindow->GetChild("Orthographic", true);
		cameraZoom = (LineEdit*)settingsWindow->GetChild("Zoom", true);
		cameraOrthoSize = (LineEdit*)settingsWindow->GetChild("OrthoSize", true);

		SubscribeToEvent(cameraPosX, E_TEXTCHANGED, HANDLER(EPScene3DView, HandleSettingsLineEditTextChange));
		SubscribeToEvent(cameraPosY, E_TEXTCHANGED, HANDLER(EPScene3DView, HandleSettingsLineEditTextChange));
		SubscribeToEvent(cameraPosZ, E_TEXTCHANGED, HANDLER(EPScene3DView, HandleSettingsLineEditTextChange));
		SubscribeToEvent(cameraRotX, E_TEXTCHANGED, HANDLER(EPScene3DView, HandleSettingsLineEditTextChange));
		SubscribeToEvent(cameraRotY, E_TEXTCHANGED, HANDLER(EPScene3DView, HandleSettingsLineEditTextChange));
		SubscribeToEvent(cameraRotZ, E_TEXTCHANGED, HANDLER(EPScene3DView, HandleSettingsLineEditTextChange));
		SubscribeToEvent(cameraZoom, E_TEXTCHANGED, HANDLER(EPScene3DView, HandleSettingsLineEditTextChange));
		SubscribeToEvent(cameraOrthoSize, E_TEXTCHANGED, HANDLER(EPScene3DView, HandleSettingsLineEditTextChange));
		SubscribeToEvent(cameraOrthographic, E_TOGGLED, HANDLER(EPScene3DView, HandleOrthographicToggled));

		SubscribeToEvent(settingsButton, E_RELEASED, HANDLER(EPScene3DView, ToggleViewportSettingsWindow));
		SubscribeToEvent(settingsWindow->GetChild("ResetCamera", true), E_RELEASED, HANDLER(EPScene3DView, ResetCamera));
		SubscribeToEvent(settingsWindow->GetChild("CloseButton", true), E_RELEASED, HANDLER(EPScene3DView, CloseViewportSettingsWindow));
		SubscribeToEvent(settingsWindow->GetChild("Refresh", true), E_RELEASED, HANDLER(EPScene3DView, UpdateSettingsUI));
		HandleResize();
	}

	Texture2D* EPScene3DView::GetRenderTexture() const
	{
		return renderTexture_;
	}

	Texture2D* EPScene3DView::GetDepthTexture() const
	{
		return depthTexture_;
	}

	Viewport* EPScene3DView::GetViewport() const
	{
		return viewport_;
	}

	void EPScene3DView::ResetScene()
	{
		if (!scene_)
			return;

		if (!ownScene_)
		{
			RefCount* refCount = scene_->RefCountPtr();
			++refCount->refs_;
			scene_ = 0;
			--refCount->refs_;
		}
		else
			scene_ = 0;
	}

	void EPScene3DView::ToggleOrthographic()
	{
		SetOrthographic(!camera_->IsOrthographic());
	}

	void EPScene3DView::SetOrthographic(bool orthographic)
	{
		camera_->SetOrthographic(orthographic);
		UpdateSettingsUI(StringHash::ZERO, VariantMap());
	}

	void EPScene3DView::HandleResize()
	{
		statusBar->SetLayoutBorder(IntRect(8, 4, 4, 8));
		IntVector2 pos = settingsWindow->GetPosition();
		pos.x_ = 5;
		settingsWindow->SetPosition(pos);

		statusBar->SetFixedSize(GetWidth(), 22);
	}

	void EPScene3DView::HandleSettingsLineEditTextChange(StringHash eventType, VariantMap& eventData)
	{
		using namespace TextChanged;

		LineEdit* element = (LineEdit*)eventData[P_ELEMENT].GetPtr();
		if (!element || element->GetText().Empty())
			return;

		if (element == cameraRotX || element == cameraRotY || element == cameraRotZ)
		{
			Vector3 euler = cameraNode_->GetRotation().EulerAngles();
			if (element == cameraRotX)
				euler.x_ = ToFloat(element->GetText());
			else if (element == cameraRotY)
				euler.y_ = ToFloat(element->GetText());
			else if (element == cameraRotZ)
				euler.z_ = ToFloat(element->GetText());

			cameraNode_->SetRotation(Quaternion(euler.x_, euler.y_, euler.z_));
		}
		else if (element == cameraPosX || element == cameraPosY || element == cameraPosZ)
		{
			Vector3 pos = cameraNode_->GetPosition();
			if (element == cameraPosX)
				pos.x_ = ToFloat(element->GetText());
			else if (element == cameraPosY)
				pos.y_ = ToFloat(element->GetText());
			else if (element == cameraPosZ)
				pos.z_ = ToFloat(element->GetText());

			cameraNode_->SetPosition(pos);
		}
		else if (element == cameraZoom)
			camera_->SetZoom(ToFloat(element->GetText()));
		else if (element == cameraOrthoSize)
			camera_->SetOrthoSize(ToFloat(element->GetText()));
	}

	void EPScene3DView::HandleOrthographicToggled(StringHash eventType, VariantMap& eventData)
	{
		SetOrthographic(cameraOrthographic->IsChecked());
	}

	void EPScene3DView::ToggleViewportSettingsWindow(StringHash eventType, VariantMap& eventData)
	{
		if (settingsWindow->IsVisible())
			CloseViewportSettingsWindow(StringHash::ZERO, VariantMap());
		else
			OpenViewportSettingsWindow();
	}

	void EPScene3DView::Update(float timeStep)
	{
		Vector3 cameraPos = cameraNode_->GetPosition();
		String xText(cameraPos.x_);
		String yText(cameraPos.y_);
		String zText(cameraPos.z_);
		xText.Resize(8);
		yText.Resize(8);
		zText.Resize(8);

		cameraPosText->SetText(String(
			"Pos: " + xText + " " + yText + " " + zText +
			" Zoom: " + String(camera_->GetZoom())));

		cameraPosText->SetSize(cameraPosText->GetMinSize());
	}
}
