#pragma once


#include "../Core/Object.h"
#include "../Core/Context.h"
#include "Utils/Macros.h"

namespace Urho3D
{
	class InGameEditor;
	class UIElement;
	class EditorView;

	class EditorPlugin : public Object
	{
		OBJECT(EditorPlugin);
		BASEOBJECT(EditorPlugin);
	public:
		/// Construct.
		EditorPlugin(Context* context);
		/// Destruct.
		virtual ~EditorPlugin();



		/// return false if this plugin is a subeditor, does not manipulate the middle frame tabs.
		virtual bool HasMainScreen() { return false; }
		/// called from the main editor, when this editor plugin is a subeditor (overlayed editor).
		virtual bool ForwardInputEvent(){ return false; }
		/// called from the main editor, when this editor plugin is a subeditor (overlayed editor).
		virtual bool ForwardSceneInputEvent(){ return false; }
		/// returns the name of this plugin, default return the type name.
		virtual String GetName() const { return GetTypeName(); }
		/// called if the an object is selected to be edited. 
		virtual void Edit(Object *object) {}
		/// check if the object can be edited by this plugin
		virtual bool Handles(Object *object) const { return false; }
 		/// add UI elements to the Editor View. \todo  ....
// 		virtual void AddUIToView(EditorView* view) { }
		/// returns ui element content for this plugin, that will be displayed in the middle frame tabs.
		virtual UIElement* GetMainScreen() { return NULL; }
		/// make the editor plugin visible or not °°. 
		virtual void SetVisible(bool visible) { visible_ = visible; }
		/// is this plugin active/visible.
		bool IsVisible() { return visible_; }
		/// update is called only for main plugins. 
		virtual void Update(float timeStep) {  }

		/// \todo handle changes that a pending (marked as *), external data, save/load editor state ??

	protected:
		bool visible_;
	};
}

