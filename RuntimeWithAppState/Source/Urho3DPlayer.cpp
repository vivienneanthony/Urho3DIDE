//
// Copyright (c) 2008-2015 the Urho3D project.
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

#include <Urho3D/Urho3D.h>

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

#include "Urho3DPlayer.h"

#include <Urho3D/DebugNew.h>
#include "Project/ProjectManager.h"
#include "../Resource/XMLFile.h"
#include "../IO/File.h"
#include "../Container/Ptr.h"
#include "../Resource/Resource.h"
#include "../IO/Deserializer.h"
#include "AppStates/AppStateManager.h"
#include "LoadScriptState.h"

DEFINE_APPLICATION_MAIN(Urho3DPlayer);

Urho3DPlayer::Urho3DPlayer(Context* context) :
Application(context)
{
}

void Urho3DPlayer::Setup()
{
	AppStateManager::RegisterObject(context_);
	context_->RegisterSubsystem(new AppStateManager(context_));

	ProjectSettings::RegisterObject(context_);
	FileSystem* filesystem = GetSubsystem<FileSystem>();
	project_ = new ProjectSettings(context_);

	String projectFileName = filesystem->GetProgramDir() + "Urho3DProject.xml";

	if (filesystem->FileExists(projectFileName))
	{
		SharedPtr<File> projectFile(new File(context_, projectFileName));
		SharedPtr<XMLFile> projectxml(new XMLFile(context_));
		if (!projectxml->Load(*projectFile.Get()))
		{
			ErrorExit("Could not load Project File  Urho3DProject.xml");
		}
		project_->LoadXML(projectxml->GetRoot());
	}
	if (project_->mainScript_.Empty())
	{
		ErrorExit("No Script defined in Urho3DProject.xml");
	}

	// Use the script file name as the base name for the log file
	engineParameters_["LogName"] = "Urho3DPlayer.log";///filesystem->GetAppPreferencesDir("urho3d", "logs") + GetFileNameAndExtension(scriptFileName_) + ".log";
	engineParameters_["ResourcePaths"] = project_->resFolders_;
	engineParameters_["AutoloadPaths"] = project_->resFolders_;
	
	engineParameters_["WindowTitle"] = project_->name_;
	engineParameters_["FullScreen"] = false;
	engineParameters_["WindowIcon"] = project_->icon_;
}

void Urho3DPlayer::Start()
{
	AppStateManager* appStateMng = GetSubsystem<AppStateManager>();
	SubscribeToEvent(appStateMng, E_NOACTIVEAPPSTATE, HANDLER(Urho3DPlayer, HandleNoActiveAppState));
	LoadScriptState* loadScriptState = new LoadScriptState(context_);
	appStateMng->RegisterState(loadScriptState);

	// Register exit action
	AppState::EventActionDesc actionDescription;
	actionDescription.actionType = AppState::ActionType_EndRoot;
	loadScriptState->RegisterEventAction("Exit", actionDescription);
	loadScriptState->SetProject(project_);

	// Set the initial state
	appStateMng->SetActiveState(loadScriptState->GetStateId());

//	appStateMng->UnregisterState(loadScriptState->GetStateId());
}

void Urho3DPlayer::Stop()
{
	GetSubsystem<AppStateManager>()->Stop();

}

void Urho3DPlayer::HandleNoActiveAppState(StringHash eventType, VariantMap& eventData)
{
	GetSubsystem<Engine>()->Exit();
}
