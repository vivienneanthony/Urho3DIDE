#pragma once



#include "../Urho3D.h"


#include "EditorPlugin.h"
#include "Macros.h"
namespace Urho3D
{
	class Camera;
	class Node;

	class PluginScene3DEditor : public EditorPlugin
	{
		OBJECT(PluginScene3DEditor);
	public:
		/// Construct.
		PluginScene3DEditor(Context* context);
		/// Destruct.
		virtual ~PluginScene3DEditor();
		/// Register object factory.
		static void RegisterObject(Context* context);

		/// called when the plugin is activated. 
		virtual void Enter();
		/// called when the plugin is closed or switched to other main plugin.
		virtual void Leave();
		/// return false if the input was not consumed otherwise true.
		virtual bool OnKeyInput(int key, bool down);
		/// return false if the input was not consumed otherwise true.
		virtual bool OnMouseButtonInput(int button, bool down);
		/// return false if this plugin is a subeditor and does not manipulate the main frame window
		virtual bool HasMainFrame();

		virtual void Update(float delta);
	protected:
		void HandleInputAction(StringHash eventType, VariantMap& eventData);

		bool initialized_;

		SharedPtr<Camera> camera_;

		SharedPtr<Node> cameraNode_;

		/// Camera yaw angle.
		float yaw_;
		/// Camera pitch angle.
		float pitch_;
		/// Input Vector
		Vector3 moveVector;

	};
}