
#include "../Urho3D.h"
#include "../Core/Object.h"
#include "../Core/Context.h"
#include "LoadScriptState.h"
#include "../IO/Log.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Core/Main.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/ResourceEvents.h>

#ifdef URHO3D_ANGELSCRIPT
#include <Urho3D/Script/ScriptFile.h>
#include <Urho3D/Script/Script.h>
#endif

#ifdef URHO3D_LUA
#include <Urho3D/LuaScript/LuaScript.h>
#endif

#include "Project/ProjectManager.h"
#include "../Resource/XMLFile.h"
#include "../IO/File.h"
#include "../Resource/Resource.h"
#include "../IO/Deserializer.h"

#include <Urho3D/DebugNew.h>
#include "../Input/Input.h"


LoadScriptState::LoadScriptState(Context* context) : AppState(context)
{
	stateId_ = "LoadScriptState";
}

LoadScriptState::~LoadScriptState()
{
	scriptFile_ = NULL;
	project_ = NULL;}

void LoadScriptState::SetProject(ProjectSettings* proj)
{
	project_ = proj;
}

bool LoadScriptState::Initialize()
{
	
	return true;
}

bool LoadScriptState::Begin()
{
	if (!project_)
	{
		ErrorExit("LoadScriptState::Begin(): projectSettings  file is not set. ");
		return false;
	}
	GetSubsystem<ResourceCache>()->SetAutoReloadResources(true);

	String extension = GetExtension(project_->mainScript_);
	if (extension != ".lua" && extension != ".luc")
	{
#ifdef URHO3D_ANGELSCRIPT
		// Instantiate and register the AngelScript subsystem
		context_->RegisterSubsystem(new Script(context_));

		// Hold a shared pointer to the script file to make sure it is not unloaded during runtime
		scriptFile_ = GetSubsystem<ResourceCache>()->GetResource<ScriptFile>(project_->mainScript_);

		/// \hack If we are running the editor, also instantiate Lua subsystem to enable editing Lua ScriptInstances
#ifdef URHO3D_LUA
		if (scriptFileName_.Contains("Editor.as", false))
			context_->RegisterSubsystem(new LuaScript(context_));
#endif
		// If script loading is successful, proceed to main loop
		if (scriptFile_ && scriptFile_->Execute("void Start()"))
		{
			// Subscribe to script's reload event to allow live-reload of the application
			SubscribeToEvent(scriptFile_, E_RELOADSTARTED, HANDLER(LoadScriptState, HandleScriptReloadStarted));
			SubscribeToEvent(scriptFile_, E_RELOADFINISHED, HANDLER(LoadScriptState, HandleScriptReloadFinished));
			SubscribeToEvent(scriptFile_, E_RELOADFAILED, HANDLER(LoadScriptState, HandleScriptReloadFailed));
			return AppState::Begin();
		}
#else
		ErrorExit("AngelScript is not enabled!");
		return false;
#endif
	}
	else
	{
#ifdef URHO3D_LUA
		// Instantiate and register the Lua script subsystem
		LuaScript* luaScript = new LuaScript(context_);
		context_->RegisterSubsystem(luaScript);

		// If script loading is successful, proceed to main loop
		if (luaScript->ExecuteFile(scriptFileName_))
		{
			luaScript->ExecuteFunction("Start");
			return AppState::Begin();
		}
#else
		ErrorExit("Lua is not enabled!");
		return false;
#endif
	}

	// The script was not successfully loaded. Show the last error message and do not run the main loop
	ErrorExit();
	return AppState::Begin();
}

void LoadScriptState::Update(float timestep)
{
	Input* input = GetSubsystem<Input>();
	if (input->GetKeyDown('Q'))
	{
		RaiseEvent("Exit");
	}
}

void LoadScriptState::End()
{
	
	
#ifdef URHO3D_ANGELSCRIPT
	if (scriptFile_)
	{
		// Execute the optional stop function
		if (scriptFile_->GetFunction("void Stop()"))
			scriptFile_->Execute("void Stop()");
	}
#else
	if (false)
	{
	}
#endif

#ifdef URHO3D_LUA
	else
	{
		LuaScript* luaScript = GetSubsystem<LuaScript>();
		if (luaScript && luaScript->GetFunction("Stop", true))
			luaScript->ExecuteFunction("Stop");
	}
#endif
	
	scriptFile_.Reset();
	GetSubsystem<ResourceCache>()->ReleaseResource(ScriptFile::GetTypeStatic(), project_->mainScript_);
	project_ = NULL;
	
	AppState::End();
}

void LoadScriptState::HandleScriptReloadStarted(StringHash eventType, VariantMap& eventData)
{
#ifdef URHO3D_ANGELSCRIPT
	if (scriptFile_->GetFunction("void Stop()"))
		scriptFile_->Execute("void Stop()");
#endif
}

void LoadScriptState::HandleScriptReloadFinished(StringHash eventType, VariantMap& eventData)
{
#ifdef URHO3D_ANGELSCRIPT
	// Restart the script application after reload
	if (!scriptFile_->Execute("void Start()"))
	{
		scriptFile_.Reset();
		ErrorExit();
	}
#endif
}

void LoadScriptState::HandleScriptReloadFailed(StringHash eventType, VariantMap& eventData)
{
#ifdef URHO3D_ANGELSCRIPT
	scriptFile_.Reset();
	ErrorExit();
#endif
}
