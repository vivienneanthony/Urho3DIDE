//
// Copyright (c) 2008-2013 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "../Core/Context.h"
#include "../Core/Object.h"
#include "../Container/List.h"
#include "../Container/HashMap.h"
#include "../Scene/Serializable.h"

// All Urho3D classes reside in namespace Urho3D
namespace Urho3D
{

	EVENT(E_NOACTIVEAPPSTATE, NoActiveAppState)
	{
	}


	class AppState;
	class AppTransitionState;

	class AppStateManager : public Object
	{
		OBJECT(AppStateManager);

		friend class AppState;
		friend class AppTransitionState;
	public:
		enum StateType
		{
			StateType_Normal = 0,
			StateType_Transition = 1
		};

		struct StateDesc
		{
			StateDesc() : type(StateType_Normal){}
			StateType   type;
			SharedPtr<AppState> state;
		};

		AppStateManager(Context* context);
		virtual ~AppStateManager();
		static void RegisterObject(Context* context);


		bool		RegisterState	(AppState* appState);
		void		UnregisterState	(const String& stateId);
		AppState*	GetState		(const String& stateId);
		bool		SetActiveState	(const String& stateId);
		AppState*	GetActiveState	();
		bool		GetStateDesc(const String& stateId, StateDesc* descOut);
		void		Update(float timestep);
		void		Stop();

	private:
		/// State transition should occurring using a more comprehensive transition
		/// process. This is achieved through a special type of game state
		/// called a transition state (AppTransitionState). This transition
		/// state will become the active state and decides when to ultimately
		/// perform the transition.
		bool        TransitionState(AppState* fromState, const String& toStateId, const String& viaStateId);
		/// Simple state transition. This will simply end one state, and move on to another.
		bool        TransitionState(AppState* fromState, const String& toStateId);
		AppState*	SpawnChildState(AppState * parentState, const String & newStateId, bool suspendParent = false);
		void		StateEnded(AppState * state);
		void		HandleUpdate(StringHash eventType, VariantMap& eventData);
		HashMap< String, StateDesc >	registeredStates_;
		AppState*						stateHistory_;
		AppState*						activeState_;
	};


	class AppState : public Serializable
	{

		OBJECT(AppState);
		BASEOBJECT(AppState);

		friend class AppStateManager;
		friend class AppTransitionState;
	public:
		enum EventActionType
		{
			ActionType_Transition = 0,
			ActionType_TransitionRoot = 1,
			ActionType_SpawnChild = 2,
			ActionType_EndState = 3,
			ActionType_EndRoot = 4,
			ActionType_PassUp = 5,
			ActionType_PassDown = 6

		};

		enum EventActionFlags
		{
			ActionFlag_None = 0,
			ActionFlag_SuspendParent = 1,
			ActionFlag_UseTransitionState = 2,

		};

		struct EventActionDesc
		{
			EventActionType actionType;
			unsigned int    flags;
			String			toStateId;
			String			transitionStateId;
			int				stackOffset;

			EventActionDesc() : flags(0), stackOffset(0) {}

		};

		AppState(Context* context);
		virtual ~AppState();

		void				SetStateId(const String& stateId);
		/// Spawn a new child state and, if requested, also suspend this state.
		AppState*			SpawnChildState(const String& stateId, bool suspendParent);
		bool				IsActive() const;
		bool				IsSuspended() const;
		const String&		GetStateId() const;
		/// Get the root parent state.
		AppState*			GetRootState();
		/// Get the last child state.
		AppState*			GetTerminalState();
		AppState*			GetParentState();
		AppTransitionState* GetOutgoingTransition();
		AppTransitionState* GetIncomingTransition();
		AppStateManager*	GetManager();

		/// Is called directly after state is registered.
		virtual bool	Initialize();
		virtual bool	Begin();
		/// Is always called if active state, you have to do your own suspension handling.
		virtual void	Update(float timestep);
		virtual void	End();

		virtual void	Suspend();
		virtual void	Resume();
		/// States which are not currently active may not raise events.
		virtual void    RaiseEvent(const String& eventName, bool bForce = true);
		virtual bool    RegisterEventAction(const String & eventName, const EventActionDesc & description);
		virtual void    UnregisterEventAction(const String & eventName);

		virtual bool    IsTransitionState() const { return false; }
		bool			IsBegun() const { return begun_; }



	private:
		void            ProcessEvent(const String & eventName);
	protected:

		HashMap< String, EventActionDesc> eventActions_; // The actions that should be taken when an event is raised
		AppStateManager*	stateManager_;          // Manager for this game state object
		String				stateId_;               // The unique identifier of this state as it has been registered with the manager.
		bool				stateSuspended_;        // Is the state currently suspended?
		bool				begun_;                 // Has the state already been begun?
		AppState*			childState_;            // Any currently active child state that may have been spawned.
		AppState*			parentState_;           // The state above us in the relationship list.
 		AppTransitionState* outgoingTransition_;    // The transition state (if any) that is being used to link us to a new state.
 		AppTransitionState* incomingTransition_;    // The transition state (if any) that is being used to link us to a previous state.
	};


	class AppTransitionState : public AppState
	{
		OBJECT(AppState);
		friend class AppStateManager;
	public:


		AppTransitionState(Context* context);
		virtual ~AppTransitionState();

		AppState*	GetFromState();
		AppState*	GetToState();
		void		TransitionComplete();

		virtual void	End() override;
		virtual bool	IsTransitionState() const { return true; }


	protected:
		AppState* fromState_;
		AppState* toState_;
	};
}
