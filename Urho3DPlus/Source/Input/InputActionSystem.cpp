#include "../Urho3D.h"
#include "InputActionSystem.h"
#include "../Input/InputEvents.h"
#include "../Core/Variant.h"
#include "SDL/SDL_stdinc.h"

namespace Urho3D
{
	InputActionSystem::InputActionSystem(Context* context) : Object(context),
		currentState_(NULL)
	{
		SubscribeToEvent(E_KEYDOWN, HANDLER(InputActionSystem, HandleKeyDown));
		SubscribeToEvent(E_KEYUP, HANDLER(InputActionSystem, HandleKeyUp));
		SubscribeToEvent(E_MOUSEBUTTONDOWN, HANDLER(InputActionSystem, HandleMouseButtonDown));
		SubscribeToEvent(E_MOUSEBUTTONUP, HANDLER(InputActionSystem, HandleMouseButtonUp));
	}

	InputActionSystem::~InputActionSystem()
	{
		HashMap<StringHash, ActionState*>::Iterator it;

		for (it = actionStates_.Begin(); it != actionStates_.End(); it++)
		{
			if (it->second_)
			{
				delete it->second_;
				it->second_ = NULL;
			}
		}
		actionStates_.Clear();
	}

	void InputActionSystem::RegisterObject(Context* context)
	{
		context->RegisterFactory<InputActionSystem>();
	}

	bool InputActionSystem::RegisterActionState(ActionState* actionState)
	{
		if (!actionState)
			return false;

		if (actionStates_.Contains(actionState->GetID()))
			return false;

		actionStates_[actionState->GetID()] = actionState;
		return true;
	}

	void InputActionSystem::Push(const StringHash& actionStateid)
	{
		HashMap<StringHash, ActionState*>::Iterator it = actionStates_.Find(actionStateid);
		if (it != actionStates_.End())
			if (stack_.Empty() || stack_.Back() != it->second_)
			{
				stack_.Push(it->second_);
				currentState_ = it->second_;
			}
	}

	ActionState* InputActionSystem::Pop(const StringHash& actionStateid)
	{
		if (stack_.Empty())
			return NULL;

		ActionState* last = stack_.Back();
		stack_.Pop();
		if (stack_.Empty())
		{
			currentState_ = NULL;
		}
		else
			currentState_ = stack_.Back();

		return last;
	}

	ActionState* InputActionSystem::Get()
	{
		return currentState_;
	}

	void InputActionSystem::HandleMouseButtonDown(StringHash eventType, VariantMap& eventData)
	{
		using namespace MouseButtonDown;

		int mousebutton = eventData[P_BUTTON].GetInt();
		unsigned mouseButtonDown = eventData[P_BUTTONS].GetUInt();

		if (currentState_)
		{
			HashMap<unsigned, HashSet<StringHash> >::Iterator it = currentState_->mouseButtonMapping_.Find(mouseButtonDown);

			if (it != currentState_->mouseButtonMapping_.End())
			{
				HashSet<StringHash>::Iterator it2;
				for (it2 = it->second_.Begin(); it2 != it->second_.End(); it2++)
				{
					using namespace InputAction;

					VariantMap& neweventData = GetEventDataMap();
					neweventData[P_ACTIONID] = (*it2);
					neweventData[P_ISDOWN] = true;

					SendEvent(E_INPUTACTION, neweventData);
				}
			}

			HashMap<unsigned, HashSet<sInputAction* > >::Iterator it1 = currentState_->mouseButtonMapping2_.Find(mousebutton);

			if (it1 != currentState_->mouseButtonMapping2_.End())
			{
				HashSet<sInputAction* >::Iterator it3;
				for (it3 = it1->second_.Begin(); it3 != it1->second_.End(); it3++)
				{
					(*it3)->downMask_ |= MouseButton_Mask;
					if ((*it3)->IsActive())
					{
						using namespace InputAction;
						VariantMap& neweventData = GetEventDataMap();
						neweventData[P_ACTIONID] = (*it3)->id_;
						neweventData[P_ISDOWN] = true;

						SendEvent(E_INPUTACTION, neweventData);
					}
				}
			}
		}
	}

	void InputActionSystem::HandleMouseButtonUp(StringHash eventType, VariantMap& eventData)
	{
		using namespace MouseButtonUp;
		int mousebutton = eventData[P_BUTTON].GetInt();
		unsigned mouseButtonDown = eventData[P_BUTTONS].GetUInt();

		if (currentState_)
		{
			HashMap<unsigned, HashSet<StringHash> >::Iterator it = currentState_->mouseButtonMapping_.Find(mousebutton);

			if (it != currentState_->mouseButtonMapping_.End())
			{
				HashSet<StringHash>::Iterator it2;
				for (it2 = it->second_.Begin(); it2 != it->second_.End(); it2++)
				{
					using namespace InputAction;

					VariantMap& neweventData = GetEventDataMap();
					neweventData[P_ACTIONID] = (*it2);
					neweventData[P_ISDOWN] = false;

					SendEvent(E_INPUTACTION, neweventData);
				}
			}

			HashMap<unsigned, HashSet< sInputAction* > >::Iterator it1 = currentState_->mouseButtonMapping2_.Find(mousebutton);

			if (it1 != currentState_->mouseButtonMapping2_.End())
			{
				HashSet< sInputAction* >::Iterator it3;
				for (it3 = it1->second_.Begin(); it3 != it1->second_.End(); it3++)
				{
					(*it3)->downMask_ &= ~MouseButton_Mask;
					if (!(*it3)->IsActive())
					{
						using namespace InputAction;
						VariantMap& neweventData = GetEventDataMap();
						neweventData[P_ACTIONID] = (*it3)->id_;
						neweventData[P_ISDOWN] = false;

						SendEvent(E_INPUTACTION, neweventData);
					}
				}
			}
		}
	}

	void InputActionSystem::HandleMouseMove(StringHash eventType, VariantMap& eventData)
	{
		using namespace MouseMove;
	}

	void InputActionSystem::HandleMouseWheel(StringHash eventType, VariantMap& eventData)
	{
		using namespace MouseWheel;
	}

	void InputActionSystem::HandleKeyDown(StringHash eventType, VariantMap& eventData)
	{
		using namespace KeyDown;
		int key = eventData[P_KEY].GetInt();
		bool repeat = eventData[P_REPEAT].GetBool();

		if (currentState_ && !repeat)
		{
			HashMap<int, HashSet<StringHash> >::Iterator it = currentState_->keyActionMapping_.Find(key);

			if (it != currentState_->keyActionMapping_.End())
			{
				HashSet<StringHash>::Iterator it2;
				for (it2 = it->second_.Begin(); it2 != it->second_.End(); it2++)
				{
					using namespace InputAction;

					VariantMap& neweventData = GetEventDataMap();
					neweventData[P_ACTIONID] = (*it2);
					neweventData[P_ISDOWN] = true;

					SendEvent(E_INPUTACTION, neweventData);
				}
			}

			HashMap<int, HashSet< sInputAction* > >::Iterator it1 = currentState_->keyActionMapping2_.Find(key);

			if (it1 != currentState_->keyActionMapping2_.End())
			{
				HashSet< sInputAction* >::Iterator it3;
				for (it3 = it1->second_.Begin(); it3 != it1->second_.End(); it3++)
				{
					(*it3)->downMask_ |= Key_Mask;
					if ((*it3)->IsActive())
					{
						using namespace InputAction;

						VariantMap& neweventData = GetEventDataMap();
						neweventData[P_ACTIONID] = (*it3)->id_;
						neweventData[P_ISDOWN] = true;

						SendEvent(E_INPUTACTION, neweventData);
					}
				}
			}
		}
	}

	void InputActionSystem::HandleKeyUp(StringHash eventType, VariantMap& eventData)
	{
		using namespace KeyUp;
		int key = eventData[P_KEY].GetInt();

		if (currentState_)
		{
			HashMap<int, HashSet<StringHash> >::Iterator it = currentState_->keyActionMapping_.Find(key);

			if (it != currentState_->keyActionMapping_.End())
			{
				HashSet<StringHash>::Iterator it2;
				for (it2 = it->second_.Begin(); it2 != it->second_.End(); it2++)
				{
					using namespace InputAction;

					VariantMap& neweventData = GetEventDataMap();
					neweventData[P_ACTIONID] = (*it2);
					neweventData[P_ISDOWN] = false;

					SendEvent(E_INPUTACTION, neweventData);
				}
			}

			HashMap<int, HashSet< sInputAction* > >::Iterator it1 = currentState_->keyActionMapping2_.Find(key);

			if (it1 != currentState_->keyActionMapping2_.End())
			{
				HashSet< sInputAction* >::Iterator it3;
				for (it3 = it1->second_.Begin(); it3 != it1->second_.End(); it3++)
				{
					(*it3)->downMask_ &= ~Key_Mask;

					if (!(*it3)->IsActive())
					{
						using namespace InputAction;

						VariantMap& neweventData = GetEventDataMap();
						neweventData[P_ACTIONID] = (*it3)->id_;
						neweventData[P_ISDOWN] = false;

						SendEvent(E_INPUTACTION, neweventData);
					}
				}
			}
		}
	}

	void InputActionSystem::HandleJoystickButtonDown(StringHash eventType, VariantMap& eventData)
	{
		using namespace JoystickButtonDown;
	}

	void InputActionSystem::HandleJoystickButtonUp(StringHash eventType, VariantMap& eventData)
	{
		using namespace JoystickButtonUp;
	}

	void InputActionSystem::HandleJoystickAxisMove(StringHash eventType, VariantMap& eventData)
	{
		using namespace JoystickAxisMove;
	}

	void InputActionSystem::HandleJoystickHatMove(StringHash eventType, VariantMap& eventData)
	{
		using namespace JoystickHatMove;
	}

	void InputActionSystem::HandleTouchEnd(StringHash eventType, VariantMap& eventData)
	{
		using namespace TouchEnd;
	}

	void InputActionSystem::HandleTouchBegin(StringHash eventType, VariantMap& eventData)
	{
		using namespace TouchBegin;
	}

	void InputActionSystem::HandleTouchMove(StringHash eventType, VariantMap& eventData)
	{
		using namespace TouchMove;
	}

	ActionState::ActionState(Context* context) : Object(context)
	{
	}

	ActionState::~ActionState()
	{
	}

	void ActionState::AddKeyAction(const String& action, int key)
	{
		int key_ = SDL_toupper(key);
		StringHash actionHash(action);
		actionNameMapping_[actionHash] = action;
		keyActionMapping_[key_].Insert(actionHash);
	}

	void ActionState::AddMouseButtonAction(const String& action, unsigned mouseButton)
	{
		StringHash actionHash(action);
		actionNameMapping_[actionHash] = action;
		mouseButtonMapping_[mouseButton].Insert(actionHash);
	}

	const String& ActionState::GetActionName(const StringHash& actionId)
	{
		HashMap<StringHash, String>::Iterator it = actionNameMapping_.Find(actionId);

		if (it != actionNameMapping_.End())
			return it->second_;

		return String::EMPTY;
	}

	void ActionState::SetName(const String& name)
	{
		name_ = name;
		nameHash_ = StringHash(name);
	}

	void ActionState::AddInputState(sInputAction* inAction)
	{
		if (inAction)
		{
			if (inAction->key_ != 0)
			{
				keyActionMapping2_[inAction->key_].Insert(inAction);
				//AddKeyAction(inAction->name_, inAction->key_);
			}

			if (inAction->mouseButtons_ != 0)
			{
				mouseButtonMapping2_[inAction->mouseButtons_].Insert(inAction);
				//AddMouseButtonAction(inAction->name_, inAction->mouseButtons_);
			}
		}
	}

	sInputAction::sInputAction(const String& name, int key, unsigned mouseButtons, bool repeat) : mouseButtons_(0), name_(name)
	{
		mouseButtons_ = mouseButtons;
		key_ = SDL_toupper(key);
		id_ = StringHash(name);
		repeat_ = repeat;
		downMask_ = 0;
		setMask_ = 0;
		if (key_ != 0)
			setMask_ |= Key_Mask;
		if (mouseButtons_ != 0)
			setMask_ |= MouseButton_Mask;
	}
}