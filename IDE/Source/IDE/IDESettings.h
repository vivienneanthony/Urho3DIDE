/*!
 * \file IDESettings.h
 *
 * \author vitali
 * \date Februar 2015
 *
 * 
 */

#pragma once



#include "../Core/Object.h"
#include "../Scene/Serializable.h"


namespace Urho3D
{
	class IDESettings : public Serializable
	{
		OBJECT(IDESettings);
	public:
		IDESettings(Context* context);
		virtual ~IDESettings();

		static void		RegisterObject(Context* context);
		virtual bool	SaveDefaultAttributes() const { return true; }
		void LoadConfigFile();
		bool Save();
		bool Load();
		String		GetPreferencesFullPath();
		VariantMap	ToVariantMap();

		/// Parameters
		int		windowWidth_;
		int		windowHeight_;
		String	windowTitle_;
		String	windowIcon_;
		bool	fullScreen_;
		bool	borderless_;
		bool	windowResizable_;
		String	resourcePaths_;
		String	resourcePackages_;
		String	autoloadPaths_;

		String projectsRootDir_;

	

	private:
	};
}
