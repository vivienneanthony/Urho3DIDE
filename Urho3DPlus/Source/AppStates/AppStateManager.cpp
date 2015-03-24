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
#include "../Urho3D.h"
#include "AppStateManager.h"

#include "../Core/CoreEvents.h"
#include "../Resource/ResourceCache.h"
#include "../IO/Log.h"

namespace Urho3D
{
	AppStateManager::AppStateManager(Context* context) :Object(context)
	{
		activeState_ = NULL;
		stateHistory_ = NULL;
		SubscribeToEvent(E_UPDATE, HANDLER(AppStateManager, HandleUpdate));

	}

	AppStateManager::~AppStateManager()
	{
		// End all currently running states
		Stop();

		// Reset variables
		activeState_ = NULL;
		stateHistory_ = NULL;

		// Destroy any registered states
		HashMap< String, StateDesc >::Iterator it;
		for (it = registeredStates_.Begin(); it != registeredStates_.End(); ++it)
		{
			StateDesc & Desc = it->second_;
			if (Desc.state)
				Desc.state.Reset();
		}

		registeredStates_.Clear();
	}

	bool AppStateManager::RegisterState(AppState * appState)
	{
		StateDesc          Desc;
		HashMap< String, StateDesc >::Iterator it;

		if (!appState) return false;

		// State already exists in the state map?
		it = registeredStates_.Find(appState->GetStateId());
		if (it != registeredStates_.End())
			return false;

		// If it doesn't exist, add it to the list of available state types
		Desc.type = (appState->IsTransitionState()) ? StateType_Transition : StateType_Normal;
		Desc.state = appState;
		registeredStates_[appState->GetStateId()] = Desc;

		// Set up state properties
		appState->stateManager_ = this;

		// Initialize the state and return result
		return appState->Initialize();
	}

	void AppStateManager::UnregisterState(const String& stateId)
	{
		// Does this state exist?
		HashMap< String, StateDesc >::Iterator it = registeredStates_.Find(stateId);
		if (it == registeredStates_.End())
			return;

		// End the state if it is running.
		if (it->second_.state)
		{
			if (it->second_.state->IsBegun())
				it->second_.state->End();
			it->second_.state.Reset();
		}

		// Remove from the state list.
		registeredStates_.Erase(it);
	}

	AppState * AppStateManager::GetActiveState()
	{
		return activeState_;
	}

	bool AppStateManager::GetStateDesc(const String& stateId, StateDesc* descOut)
	{
		HashMap< String, StateDesc >::Iterator it;

		// Clear description for convenience
		if (descOut)
		{
			descOut->type = StateType_Normal;
			descOut->state = NULL;
		}

		// First retrieve the details about the state specified, if none
		// was found this is clearly an invalid state identifier.
		it = registeredStates_.Find(stateId);
		if (it == registeredStates_.End())
			return false;

		// A registered state was found, retrieve it's description
		if (descOut)
			*descOut = it->second_;

		return true;
	}

	bool AppStateManager::SetActiveState(const String & stateId)
	{
		StateDesc desc;

		// First find the requested state
		if (!GetStateDesc(stateId, &desc)) return false;

		// The state was found, end any currently executing states
		if (stateHistory_) stateHistory_->End();

		// Link this new state
		stateHistory_ = desc.state;
		activeState_ = desc.state;

		if (activeState_->IsBegun() == false)
			activeState_->Begin();

		return true;
	}

	AppState* AppStateManager::GetState(const String& stateId)
	{
		// First retrieve the details about the state specified, if none
		// was found this is clearly an invalid state identifier.
		HashMap< String, StateDesc >::Iterator it = registeredStates_.Find(stateId);
		if (it != registeredStates_.End())
			return NULL;
		return it->second_.state;
	}

	void AppStateManager::Update(float timestep)
	{
		if (stateHistory_) stateHistory_->Update(timestep);
	}

	void AppStateManager::Stop()
	{
		// Stop all active states
		if (stateHistory_) stateHistory_->End();
	}


	bool AppStateManager::TransitionState(AppState* fromState, const String& toStateId, const String& viaStateId)
	{
		StateDesc   ToDesc, ViaDesc;
		AppState *pParentState = NULL, *pSelectedToState = NULL, *pState = NULL;
		AppTransitionState  *pSelectedViaState = NULL;

		if (!fromState) return false;

		// States can only be transitioned if they are in the current state history
		for (pState = stateHistory_; pState != NULL; pState = pState->childState_)
		{
			if (pState == fromState) break;
		}

		// Not in list?
		if (pState == NULL)
		{
			LOGERRORF("AppStateManager::TransitionState: You can only transition states that currently exist in the state history,%s. ",
				fromState->GetStateId().CString());
			return false;
		}

		// Select the state we're transitioning to
		// Note: We allow there to be no outgoing state (application will transition to exit?)
		ToDesc.state = NULL;
		if (GetStateDesc(toStateId, &ToDesc))
		{
			// Is this a valid state type?
			if (ToDesc.type != StateType_Normal)
			{
				LOGERROR("AppStateManager::TransitionState:Unable to transition to any state type other than 'normal' using complex transitioning. ");
				return false;
			}
		}

		// Select the state we're transitioning via
		if (!GetStateDesc(viaStateId, &ViaDesc))
		{
			LOGERRORF("AppStateManager::TransitionState:The specified app state %s that should be used to perform a transition was not found.", viaStateId.CString());
			return false;
		}

		// Is this a valid state type?
		if (ViaDesc.type != StateType_Transition)
		{
			LOGERROR("AppStateManager::TransitionState:Unable to perform a transition between two states using any state type other than 'transition'.");
			return false;
		}

		// Store the parent of the state we're ending, we may have to attach to it
		pParentState = fromState->parentState_;

		// Attach the new transition state to the necessary items
		pSelectedViaState = dynamic_cast<AppTransitionState*>(ViaDesc.state.Get());
		pSelectedViaState->parentState_ = pParentState;

		// Update information in all states for pointer navigation
		pSelectedToState = ToDesc.state;
		pSelectedViaState->fromState_ = fromState;
		pSelectedViaState->toState_ = pSelectedToState;
		fromState->outgoingTransition_ = pSelectedViaState;
		if (pSelectedToState != NULL)
			pSelectedToState->incomingTransition_ = pSelectedViaState;

		// If any parent is available, it will be attached as a child
		// otherwise it will become the root of our state history
		if (pParentState)
			pParentState->childState_ = pSelectedViaState;
		else
			stateHistory_ = pSelectedViaState;

		// The transition will become our newly active state
		activeState_ = pSelectedViaState;

		// Begin the transition state
		if (activeState_->IsBegun() == false)
			activeState_->Begin();

		// Success
		return true;
	}

	bool AppStateManager::TransitionState(AppState* fromState, const String& toStateId)
	{
		StateDesc     Desc;
		AppState * pParentState = NULL, *pSelectedState = NULL, *pState = NULL;

		// Validate Requirements
		if (!fromState) return false;

		// States can only be transitioned if they are in the current state history
		for (pState = stateHistory_; pState != NULL; pState = pState->childState_)
		{
			if (pState == fromState) break;
		}

		// Not in list?
		if (pState == NULL)
		{
			LOGERRORF("AppStateManager::TransitionState: You can only transition states that currently exist in the state history,%s. ",
				fromState->GetStateId().CString());
			return false;
		}

		// Select the state we're transitioning to
		if (!GetStateDesc(toStateId, &Desc))
		{
			LOGERRORF("AppStateManager::TransitionState:The specified app state %s to which we should transition was not found.",
				toStateId.CString());
			return false;
		}

		// Is this a valid state type?
		if (Desc.type != StateType_Normal)
		{
			LOGERROR("AppStateManager::TransitionState:Unable to transition to any state type other than 'normal' using simple transitioning. ");
			return false;
		}

		// Store the parent of the state we're ending, we may have to attach to it
		pParentState = fromState->parentState_;

		// End the specified state
		fromState->End();

		// Update necessary navigation pointers
		pSelectedState = Desc.state;
		pSelectedState->parentState_ = pParentState;
		pSelectedState->incomingTransition_ = NULL;

		// If any parent is available, it will be attached as a child
		// otherwise it will become the root of our state history
		if (pParentState)
			pParentState->childState_ = pSelectedState;
		else
			stateHistory_ = pSelectedState;

		// This must become our newly active state since any below the previous
		// state have been ended
		activeState_ = pSelectedState->GetTerminalState();

		// Begin the new state
		if (activeState_->IsBegun() == false)
			activeState_->Begin();

		return true;
	}

	AppState* AppStateManager::SpawnChildState(AppState * parentState, const String & newStateId, bool suspendParent /*= false*/)
	{
		StateDesc   desc;
		AppState*	selectedState = NULL;
		AppState*	state = NULL;

		if (!activeState_)
			return NULL;

		if (!GetStateDesc(newStateId, &desc))
			return NULL;
		selectedState = desc.state;

		if (selectedState->IsBegun())
		{
			LOGERRORF("AppStateManager::SpawnChildState: AppState %s is already running!", newStateId.CString());
			return NULL;
		}

		selectedState->parentState_ = parentState;
		parentState->childState_ = selectedState;

		// This should now be the state that is currently active
		// if the parent was previously active.
		if (activeState_ == parentState)
			activeState_ = selectedState;

		if (!selectedState->IsBegun())
			selectedState->Begin();

		if (suspendParent)
			parentState->Suspend();

		return selectedState;
	}

	void AppStateManager::StateEnded(AppState* state)
	{
		// Paranoia check, do i need that ?! :/
		if (state == NULL) return;

		// If this is the currently active state, and there is another restorable
		// state above it, switch to its parent
		if (state == activeState_)
			activeState_ = state->parentState_;

		// If this was at the root of the state history, we're all out of states
		if (state == stateHistory_)
			stateHistory_ = NULL;

		// If currently active state is suspended, resume it.
		if (activeState_ && activeState_->IsSuspended())
			activeState_->Resume();
		if (!activeState_)
		{
			using namespace NoActiveAppState;

			VariantMap& eventData_ = GetEventDataMap();
			SendEvent(E_NOACTIVEAPPSTATE, eventData_);
		}
	}

	void AppStateManager::HandleUpdate(StringHash eventType, VariantMap& eventData)
	{
		using namespace Update;
		float timestep = eventData[P_TIMESTEP].GetFloat();
		Update(timestep);
	}

	void AppStateManager::RegisterObject(Context* context)
	{
		context->RegisterFactory<AppStateManager>();
	}

	///////////////////////////////////////////////////////////////////////////////
	/// AppState Member Functions
	///////////////////////////////////////////////////////////////////////////////
	AppState::AppState(Context* context) : Serializable(context)
	{
		stateId_ = GetTypeName();
		stateManager_ = NULL;
		stateSuspended_ = false;
		begun_ = false;
		childState_ = NULL;
		parentState_ = NULL;
		outgoingTransition_ = NULL;
		incomingTransition_ = NULL;
	}

	AppState::~AppState()
	{
		// End the state if it is begun.
		if (begun_ == true)
			End();

		// Clear variables
		stateManager_ = NULL;
		stateSuspended_ = false;
		begun_ = false;
		childState_ = NULL;
		parentState_ = NULL;
		outgoingTransition_ = NULL;
		incomingTransition_ = NULL;

		eventActions_.Clear();
	}


	void AppState::ProcessEvent(const String & eventName)
	{
		HashMap< String, EventActionDesc>::Iterator it;

		// Any event action registered with this name?
		it = eventActions_.Find(eventName);
		if (it == eventActions_.End()) return;

		// An action was found, let's retrieve it
		EventActionDesc& Desc = it->second_;

		// Select the state we're operating on in the stack based on the action stack offset.
		AppState* selectedState = this;
		for (int i = 0; i < Desc.stackOffset && selectedState->GetParentState(); ++i)
			selectedState = selectedState->GetParentState();

		// Now process based on the type of the event action
		switch (Desc.actionType)
		{
		case ActionType_Transition:

			// This type will transition the state on which the event was raised, to a new state.
			if (Desc.flags & ActionFlag_UseTransitionState)
			{
				// The application requested that we should use a transition
				// state to perform a more comprehensive transition process.
				stateManager_->TransitionState(selectedState, Desc.toStateId, Desc.transitionStateId);
			}
			else
			{
				// Just transition immediately
				stateManager_->TransitionState(selectedState, Desc.toStateId);
			}

			break;

		case ActionType_TransitionRoot:

			// This type will transition the root state in the relationship list, to a new state
			// irrespective of which state object the event was raised on.
			if (Desc.flags & ActionFlag_UseTransitionState)
			{
				// The application requested that we should use a transition
				// state to perform a more comprehensive transition process.
				stateManager_->TransitionState(GetRootState(), Desc.toStateId, Desc.transitionStateId);
			} // End if use transition state
			else
			{
				// Just transition immediately
				stateManager_->TransitionState(GetRootState(), Desc.toStateId);
			} // End if immediate / simple transition

			break;

		case ActionType_SpawnChild:
			selectedState->SpawnChildState(Desc.toStateId, (Desc.flags & ActionFlag_SuspendParent) != 0);
			break;

		case ActionType_EndState:

			// The state should simply end
			selectedState->End();
			break;

		case ActionType_EndRoot:

			// The who system should simply end.
			stateManager_->Stop();
			break;

		case ActionType_PassUp:

			// This action should simply pass the message on to a parent (if any)
			if (parentState_)
				parentState_->RaiseEvent(eventName, true);
			break;

		case ActionType_PassDown:

			// This action should simply pass the message on to a child (if any)
			if (childState_)
				childState_->RaiseEvent(eventName, true);
			break;
		} // End Switch actionType
	}

	bool AppState::IsActive() const
	{
		if (!stateManager_) return false;
		return (this == stateManager_->GetActiveState());
	}

	bool AppState::IsSuspended() const
	{
		return stateSuspended_;
	}

	const String & AppState::GetStateId() const
	{
		return stateId_;
	}

	AppState* AppState::GetRootState()
	{
		AppState* currentState = this;

		// Loop until there is no longer a parent
		for (; currentState->parentState_ != NULL;)
			currentState = currentState->parentState_;

		// Return the state item
		return currentState;
	}

	AppState* AppState::GetTerminalState()
	{
		AppState* currentState = this;

		// Loop until there is no longer a child
		for (; currentState->childState_ != NULL;)
			currentState = currentState->childState_;

		// Return the state item.
		return currentState;
	}

	AppState* AppState::GetParentState()
	{
		return parentState_;
	}

	AppTransitionState* AppState::GetOutgoingTransition()
	{
		return outgoingTransition_;
	}

	AppTransitionState* AppState::GetIncomingTransition()
	{
		return incomingTransition_;
	}

	AppStateManager * AppState::GetManager()
	{
		return stateManager_;
	}

	bool AppState::Initialize()
	{
		return true;
	}

	bool AppState::Begin()
	{
		begun_ = true;
		return begun_;
	}

	void AppState::Update(float timestep)
	{
		if (childState_)
			childState_->Update(timestep);
	}

	void AppState::End()
	{
		// First recurse into any child state
		if (childState_)
			childState_->End();
		childState_ = NULL;

		// Notify manager that the state ended
		stateManager_->StateEnded(this);

		// Now detach this from parent
		if (parentState_)
			parentState_->childState_ = NULL;

		// We are no longer attached to anything (simply being stored in
		// the registered state list maintained by the manager).
		stateSuspended_ = false;
		begun_ = false;
		childState_ = NULL;
		parentState_ = NULL;
		outgoingTransition_ = NULL;
		incomingTransition_ = NULL;
	}

	void AppState::Suspend()
	{
		stateSuspended_ = true;
	}

	void AppState::Resume()
	{
		stateSuspended_ = false;
	}

	void AppState::RaiseEvent(const String& eventName, bool bForce /*= true*/)
	{
		// States that are not active are blocked from raising events by default.
		// This helps to automatically prevent inactive states from messing
		// up the application flow.
		if (bForce == false && this->IsActive() == false)
			return;

		ProcessEvent(eventName);
	}

	bool AppState::RegisterEventAction(const String & eventName, const EventActionDesc & description)
	{
		HashMap< String, EventActionDesc>::Iterator it;

		// Does an action with this matching event name already exist?
		it = eventActions_.Find(eventName);
		if (it != eventActions_.End()) return false;

		eventActions_[eventName] = description;

		return true;
	}

	void AppState::UnregisterEventAction(const String & eventName)
	{
		HashMap< String, EventActionDesc>::Iterator it;

		// Does an action with this matching event name already exist?
		it = eventActions_.Find(eventName);
		if (it == eventActions_.End()) return;

		// If we found an action, remove it from the list
		eventActions_.Erase(it);
	}

	void AppState::SetStateId(const String & stateId)
	{
		stateId_ = stateId;
	}

	AppState* AppState::SpawnChildState(const String& stateId, bool suspendParent)
	{
		return stateManager_->SpawnChildState(this, stateId, suspendParent);
	}

	///////////////////////////////////////////////////////////////////////////////
	/// AppTransitionState Member Functions
	///////////////////////////////////////////////////////////////////////////////
	AppTransitionState::AppTransitionState(Context* context) : AppState(context)
	{
		fromState_ = NULL;
		toState_ = NULL;
	}

	AppTransitionState::~AppTransitionState()
	{
		// Clean up
		fromState_ = NULL;
		toState_ = NULL;
	}

	AppState* AppTransitionState::GetFromState()
	{
		return fromState_;
	}

	AppState* AppTransitionState::GetToState()
	{
		return toState_;
	}

	void AppTransitionState::TransitionComplete()
	{
		if (toState_ != NULL)
			stateManager_->TransitionState(this, toState_->GetStateId());
		else
			End();
	}

	void AppTransitionState::End()
	{
		// If we are being ended (for whatever reason) we must also
		// end the state that we are transitioning from.
		if (fromState_) fromState_->End();

		// Clear required members, we've ended
		fromState_ = NULL;
		fromState_ = NULL;

		// Call base class implementation
		AppState::End();
	}


}
