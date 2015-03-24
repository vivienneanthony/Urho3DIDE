#pragma once

#include "AppStates/AppStateManager.h"
namespace Urho3D
{
	class ProjectSettings;
	class ScriptFile;

}

using namespace Urho3D;

class LoadScriptState : public AppState
{
	OBJECT(LoadScriptState);
public:
	/// Construct.
	LoadScriptState(Context* context);
	/// Destruct.
	virtual ~LoadScriptState();

	void SetProject(ProjectSettings* proj);

	virtual bool Initialize() override;

	virtual bool Begin() override;

	virtual void Update(float timestep) override;

	virtual void End() override;

protected:

	/// Handle reload start of the script file.
	void HandleScriptReloadStarted(StringHash eventType, VariantMap& eventData);
	/// Handle reload success of the script file.
	void HandleScriptReloadFinished(StringHash eventType, VariantMap& eventData);
	/// Handle reload failure of the script file.
	void HandleScriptReloadFailed(StringHash eventType, VariantMap& eventData);


	SharedPtr<ProjectSettings> project_;
#ifdef URHO3D_ANGELSCRIPT
	/// Script file.
	SharedPtr<ScriptFile> scriptFile_;
#endif
};
