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


DEFINE_APPLICATION_MAIN(Urho3DPlayer);

Urho3DPlayer::Urho3DPlayer(Context* context) :
    Application(context)
{
}

void Urho3DPlayer::Setup()
{
	ProjectSettings::RegisterObject(context_);
    FileSystem* filesystem = GetSubsystem<FileSystem>();
	project_ = new ProjectSettings(context_);
	
    String projectFileName = filesystem->GetProgramDir() + "Urho3DProject.xml";

	if (filesystem->FileExists(projectFileName))
    {
		SharedPtr<File> projectFile(new File(context_, projectFileName));
		SharedPtr<XMLFile> projectxml( new XMLFile(context_));
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
	scriptFileName_ = project_->mainScript_;

     // Use the script file name as the base name for the log file
     engineParameters_["LogName"] = filesystem->GetAppPreferencesDir("urho3d", "logs") + GetFileNameAndExtension(scriptFileName_) + ".log";
	 engineParameters_["ResourcePaths"] = project_->resFolders_;
	 engineParameters_["WindowTitle"] = project_->name_;
	 engineParameters_["FullScreen"] = false;
	 engineParameters_["WindowIcon"] = project_->icon_;
}

void Urho3DPlayer::Start()
{

    String extension = GetExtension(scriptFileName_);
    if (extension != ".lua" && extension != ".luc")
    {
#ifdef URHO3D_ANGELSCRIPT
        // Instantiate and register the AngelScript subsystem
        context_->RegisterSubsystem(new Script(context_));

        // Hold a shared pointer to the script file to make sure it is not unloaded during runtime
        scriptFile_ = GetSubsystem<ResourceCache>()->GetResource<ScriptFile>(scriptFileName_);

        /// \hack If we are running the editor, also instantiate Lua subsystem to enable editing Lua ScriptInstances
#ifdef URHO3D_LUA
        if (scriptFileName_.Contains("Editor.as", false))
            context_->RegisterSubsystem(new LuaScript(context_));
#endif
        // If script loading is successful, proceed to main loop
        if (scriptFile_ && scriptFile_->Execute("void Start()"))
        {
            // Subscribe to script's reload event to allow live-reload of the application
            SubscribeToEvent(scriptFile_, E_RELOADSTARTED, HANDLER(Urho3DPlayer, HandleScriptReloadStarted));
            SubscribeToEvent(scriptFile_, E_RELOADFINISHED, HANDLER(Urho3DPlayer, HandleScriptReloadFinished));
            SubscribeToEvent(scriptFile_, E_RELOADFAILED, HANDLER(Urho3DPlayer, HandleScriptReloadFailed));
            return;
        }
#else
        ErrorExit("AngelScript is not enabled!");
        return;
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
            return;
        }
#else
        ErrorExit("Lua is not enabled!");
        return;
#endif
    }

    // The script was not successfully loaded. Show the last error message and do not run the main loop
    ErrorExit();
}

void Urho3DPlayer::Stop()
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
}

void Urho3DPlayer::HandleScriptReloadStarted(StringHash eventType, VariantMap& eventData)
{
#ifdef URHO3D_ANGELSCRIPT
    if (scriptFile_->GetFunction("void Stop()"))
        scriptFile_->Execute("void Stop()");
#endif
}

void Urho3DPlayer::HandleScriptReloadFinished(StringHash eventType, VariantMap& eventData)
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

void Urho3DPlayer::HandleScriptReloadFailed(StringHash eventType, VariantMap& eventData)
{
#ifdef URHO3D_ANGELSCRIPT
    scriptFile_.Reset();
    ErrorExit();
#endif
}
