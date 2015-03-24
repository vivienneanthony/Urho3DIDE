#pragma once




#include "../Core/Object.h"
#include "../Core/Context.h"
#include "Macros.h"

namespace Urho3D
{

	EVENT(E_INPUTACTION, InputAction)
	{
		PARAM(P_ACTIONID, ActionID);        // StringHash
		PARAM(P_ISDOWN, IsDown);            // bool
	}

#define MouseButton_Mask 0x01;
#define Key_Mask 0x02;
#define MoveVert_Mask 0x04;
#define MoveHori_Mask 0x08;

	struct sInputAction
	{
		sInputAction(const String& name, int key, unsigned mouseButtons = 0, bool repeat = false);

		/// input definition
		int			key_;
		unsigned	mouseButtons_;
		StringHash	id_;
		String		name_;
		bool		repeat_;

		/// check if action is active
		unsigned	downMask_;
		unsigned	setMask_;
		bool IsActive(){ return ((downMask_ & setMask_) != 0); }

		/// Test for equality with another hash set.
		bool operator == (const sInputAction& rhs) const
		{		
			return id_ == rhs.id_;
		}
	};



	class ActionState : public Object
	{
		OBJECT(ActionState);
		BASEOBJECT(ActionState);
		friend class InputActionSystem;
	public:
		/// Construct.
		ActionState(Context* context);
		/// Destruct.
		virtual ~ActionState();

		void SetName(const String& name);

		void AddInputState(sInputAction* inAction);
		void AddKeyAction(const String& action, int key);
		void AddMouseButtonAction(const String& action, unsigned mouseButton);

		const String&		GetName() { return name_; }
		const StringHash&	GetID() { return nameHash_; }
		const String&		GetActionName(const StringHash& actionId);

	protected:

		StringHash	nameHash_;
		String		name_;

		HashMap<int, HashSet< sInputAction* > >			keyActionMapping2_;
		HashMap<unsigned, HashSet< sInputAction* > >			mouseButtonMapping2_;

		HashMap<int,HashSet<StringHash> >			keyActionMapping_;
		HashMap<unsigned, HashSet< StringHash> >	mouseButtonMapping_;

		HashMap<StringHash, String>		actionNameMapping_;
	};


	class InputActionSystem : public Object
	{
		OBJECT(InputActionSystem);
	public:
		/// Construct.
		InputActionSystem(Context* context);
		/// Destruct.
		virtual ~InputActionSystem();
		/// Register object factory.
		static void RegisterObject(Context* context);

		bool RegisterActionState(ActionState* actionState);
		/// does not push same states on top of each other
		void Push(const StringHash& actionStateid);
		/// Returns the last active actions state
		ActionState* Pop(const StringHash& actionStateid);
		/// Returns the active actions state
		ActionState* Get();

	protected:
		
		void HandleMouseButtonDown(StringHash eventType, VariantMap& eventData);
		void HandleMouseButtonUp(StringHash eventType, VariantMap& eventData);

		void HandleMouseMove(StringHash eventType, VariantMap& eventData);
		void HandleMouseWheel(StringHash eventType, VariantMap& eventData);

		void HandleKeyDown(StringHash eventType, VariantMap& eventData);
		void HandleKeyUp(StringHash eventType, VariantMap& eventData);

		void HandleJoystickButtonDown(StringHash eventType, VariantMap& eventData);
		void HandleJoystickButtonUp(StringHash eventType, VariantMap& eventData);
		void HandleJoystickAxisMove(StringHash eventType, VariantMap& eventData);
		void HandleJoystickHatMove(StringHash eventType, VariantMap& eventData);

		void HandleTouchEnd(StringHash eventType, VariantMap& eventData);
		void HandleTouchBegin(StringHash eventType, VariantMap& eventData);
		void HandleTouchMove(StringHash eventType, VariantMap& eventData);

		/// Active finger touches.
		HashMap<StringHash, ActionState*> actionStates_;
		Vector<ActionState*>	stack_;
		ActionState*			currentState_;
	};


}