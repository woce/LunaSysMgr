/* @@@LICENSE
*
*      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* LICENSE@@@ */




#include "Common.h"

#include "AnimationSettings.h"
#include <QEasingCurve>

static const char* kSettingsFile = "/etc/palm/lunaAnimations.conf";
static const char* kSettingsFilePlatform = "/etc/palm/lunaAnimations-platform.conf";

AnimationSettings* AnimationSettings::s_instance = 0;

#define SETUP_INTEGER(cat, var)									\
	{															\
		int _v;													\
		GError* _error = 0;										\
		_v=g_key_file_get_integer(keyFile,cat,#var,&_error);	\
		if( !_error ) { var=_v; }								\
		else { g_error_free(_error); }							\
																\
		m_values[#var] = &var;									\
	}

AnimationSettings::AnimationSettings()
	: lunaFPS(60)
	, cardSlideDuration(300), cardSlideCurve(10)
	, cardTrackDuration(300), cardTrackCurve(0)
	, cardTrackGroupDuration(300), cardTrackGroupCurve(0)
	, cardMaximizeDuration(300), cardMaximizeCurve(10)
	, cardDeleteDuration(300), cardDeleteCurve(6)
	, cardShuffleReorderDuration(350), cardShuffleReorderCurve(9)
	, cardGroupReorderDuration(350), cardGroupReorderCurve(9)
	, cardPrepareAddDuration(150)
	, cardAddMaxDuration(750)
	, modalCardAddMaxDuration(10)
	, cardLoadingPulsePauseDuration(1000)
	, cardLoadingPulseDuration(1000), cardLoadingPulseCurve(1)
	, cardLoadingCrossFadeDuration(300), cardLoadingCrossFadeCurve(0)
	, cardLoadingTimeBeforeShowingPulsing(900)
	, cardTransitionDuration(300), cardTransitionCurve(20)
    , cardGhostDuration(300), cardGhostCurve(0)
    , cardDimmingDuration(300), cardDimmingCurve(6)
	, positiveSpaceChangeDuration(400), positiveSpaceChangeCurve(6)
	, quickLaunchDuration(350), quickLaunchCurve(6)
	, quickLaunchFadeDuration(200), quickLaunchFadeCurve(6)
	, launcherDuration(200), launcherCurve(2)
	, universalSearchCrossFadeDuration(200), universalSearchCrossFadeCurve(6)
	, reticleDuration(200), reticleCurve(0)
	, brickDuration(300), brickCurve(0)
	, progressPulseDuration(2000), progressPulseCurve(1)
	, progressFinishDuration(500), progressFinishCurve(1)
	, lockWindowFadeDuration(150), lockWindowFadeCurve(1)
	, lockPinDuration(250), lockPinCurve(15)
	, lockFadeDuration(200), lockFadeCurve(0)
	, dashboardSnapDuration(200), dashboardSnapCurve(6)
	, dashboardDeleteDuration(200), dashboardDeleteCurve(0)
	, dockFadeScreenAnimationDuration(900)
	, dockFadeDockAnimationDuration(500)	
	, dockFadeDockStartDelay(270)
	, dockFadeAnimationCurve(3)
	, statusBarFadeDuration(300), statusBarFadeCurve(0)
	, statusBarColorChangeDuration(300), statusBarColorChangeCurve(0)
	, statusBarTitleChangeDuration(300), statusBarTitleChangeCurve(0)
	, statusBarTabFadeDuration(500), statusBarTabFadeCurve(0)
	, statusBarArrowSlideDuration(500), statusBarArrowSlideCurve(0)
	, statusBarItemSlideDuration(500), statusBarItemSlideCurve(0)
	, statusBarMenuFadeDuration(200), statusBarMenuFadeCurve(0)
	, rotationAnimationDuration(300)
{
	s_instance = this;

	readSettings(kSettingsFile);
	readSettings(kSettingsFilePlatform);
}

AnimationSettings::~AnimationSettings()
{
    s_instance = 0;
}

void AnimationSettings::readSettings(const char* filePath)
{
    GKeyFile* keyFile = g_key_file_new();

	if (!g_key_file_load_from_file(keyFile, filePath,
								   G_KEY_FILE_NONE, NULL)) {
		goto done;
	}

	SETUP_INTEGER("FPS", lunaFPS);
		
	SETUP_INTEGER("Cards", cardSlideDuration);
	SETUP_INTEGER("Cards", cardSlideCurve);

	SETUP_INTEGER("Cards", cardTrackDuration);
	SETUP_INTEGER("Cards", cardTrackCurve);
	
    SETUP_INTEGER("Cards", cardTrackGroupDuration);
	SETUP_INTEGER("Cards", cardTrackGroupCurve);
	
	SETUP_INTEGER("Cards", cardMaximizeDuration);
	SETUP_INTEGER("Cards", cardMaximizeCurve);
	
	SETUP_INTEGER("Cards", cardDeleteDuration);
	SETUP_INTEGER("Cards", cardDeleteCurve);

	SETUP_INTEGER("Cards", cardShuffleReorderDuration);
	SETUP_INTEGER("Cards", cardShuffleReorderCurve);
	SETUP_INTEGER("Cards", cardGroupReorderDuration);
	SETUP_INTEGER("Cards", cardGroupReorderCurve);

	SETUP_INTEGER("Cards", cardPrepareAddDuration);
	SETUP_INTEGER("Cards", cardAddMaxDuration);
	SETUP_INTEGER("Cards", modalCardAddMaxDuration);
	
	SETUP_INTEGER("Cards", cardLoadingPulsePauseDuration);
	SETUP_INTEGER("Cards", cardLoadingPulseDuration);
	SETUP_INTEGER("Cards", cardLoadingPulseCurve);

	SETUP_INTEGER("Cards", cardLoadingCrossFadeDuration);
	SETUP_INTEGER("Cards", cardLoadingCrossFadeCurve);

	SETUP_INTEGER("Cards", cardLoadingTimeBeforeShowingPulsing);

	SETUP_INTEGER("Cards", cardTransitionDuration);
	SETUP_INTEGER("Cards", cardTransitionCurve);

    SETUP_INTEGER("Cards", cardGhostDuration);
    SETUP_INTEGER("Cards", cardGhostCurve);

    SETUP_INTEGER("Cards", cardDimmingDuration);
    SETUP_INTEGER("Cards", cardDimmingCurve);

	SETUP_INTEGER("Spaces", positiveSpaceChangeDuration);
	SETUP_INTEGER("Spaces", positiveSpaceChangeCurve);
	
	SETUP_INTEGER("Launcher", quickLaunchDuration);
	SETUP_INTEGER("Launcher", quickLaunchCurve);
	
	SETUP_INTEGER("Launcher", quickLaunchFadeDuration);
	SETUP_INTEGER("Launcher", quickLaunchFadeCurve);
	
	SETUP_INTEGER("Launcher", launcherDuration);
	SETUP_INTEGER("Launcher", launcherCurve);

	SETUP_INTEGER("Launcher", universalSearchCrossFadeDuration);
	SETUP_INTEGER("Launcher", universalSearchCrossFadeCurve);

	SETUP_INTEGER("Reticle", reticleDuration);
	SETUP_INTEGER("Reticle", reticleCurve);

	SETUP_INTEGER("MSM", brickDuration);
	SETUP_INTEGER("MSM", brickCurve);
	SETUP_INTEGER("MSM", progressPulseDuration);
	SETUP_INTEGER("MSM", progressPulseCurve);
	SETUP_INTEGER("MSM", progressFinishDuration);
	SETUP_INTEGER("MSM", progressFinishCurve);

	SETUP_INTEGER("Lock", lockWindowFadeDuration);
	SETUP_INTEGER("Lock", lockWindowFadeCurve);

	SETUP_INTEGER("Lock", lockPinDuration);
	SETUP_INTEGER("Lock", lockPinCurve);

	SETUP_INTEGER("Lock", lockFadeDuration);
	SETUP_INTEGER("Lock", lockFadeCurve);

	SETUP_INTEGER("Dashboard", dashboardSnapDuration);
	SETUP_INTEGER("Dashboard", dashboardSnapCurve);	
	SETUP_INTEGER("Dashboard", dashboardDeleteDuration);
	SETUP_INTEGER("Dashboard", dashboardDeleteCurve);
	
	SETUP_INTEGER("Dock", dockFadeScreenAnimationDuration);
	SETUP_INTEGER("Dock", dockFadeDockAnimationDuration);	
	SETUP_INTEGER("Dock", dockFadeDockStartDelay);
	SETUP_INTEGER("Dock", dockFadeAnimationCurve);

	SETUP_INTEGER("StatusBar", statusBarFadeDuration);
	SETUP_INTEGER("StatusBar", statusBarFadeCurve);

	SETUP_INTEGER("StatusBar", statusBarColorChangeDuration);
	SETUP_INTEGER("StatusBar", statusBarColorChangeCurve);

	SETUP_INTEGER("StatusBar", statusBarTitleChangeDuration);
	SETUP_INTEGER("StatusBar", statusBarTitleChangeCurve);

	SETUP_INTEGER("StatusBar", statusBarTabFadeDuration);
	SETUP_INTEGER("StatusBar", statusBarTabFadeCurve);

	SETUP_INTEGER("StatusBar", statusBarArrowSlideDuration);
	SETUP_INTEGER("StatusBar", statusBarArrowSlideCurve);

	SETUP_INTEGER("StatusBar", statusBarItemSlideDuration);
	SETUP_INTEGER("StatusBar", statusBarItemSlideCurve);

	SETUP_INTEGER("StatusBar", statusBarMenuFadeDuration);
	SETUP_INTEGER("StatusBar", statusBarMenuFadeCurve);

	SETUP_INTEGER("Rotation", rotationAnimationDuration);
done:

	if (keyFile) {
		g_key_file_free(keyFile);
	}
}

bool AnimationSettings::setValue(const std::string& key, int value)
{
	std::map<std::string, int*>::iterator it = m_values.find(key);
	if (it == m_values.end())
		return false;

	*((*it).second) = value;
	return true;
}

bool AnimationSettings::getValue(const std::string& key, int& value) const
{
	std::map<std::string, int*>::const_iterator it = m_values.find(key);
	if (it == m_values.end())
		return false;

	value = *((*it).second);
	return true;    
}

std::map<std::string, int> AnimationSettings::getAllValues() const
{
	std::map<std::string, int> allValues;
	
    for (std::map<std::string, int*>::const_iterator it = m_values.begin();
		 it != m_values.end(); ++it) {
		allValues[(*it).first] = *((*it).second);
	}

	return allValues;
}

AnimationEquation AnimationSettings::easeInEquation(int strength) const
{
	if (G_UNLIKELY(strength < 10))
		strength = 10;
	
	switch (strength) {
	case (10):
		return AnimationEquations::easeLinear;
	case (20):
		return AnimationEquations::easeInQuad;
	case (30):
		return AnimationEquations::easeInQuat;
	case (40):
		return AnimationEquations::easeInCubic;
	default:
		return AnimationEquations::easeInGeneric;
    }
}

AnimationEquation AnimationSettings::easeOutEquation(int strength) const
{
	if (G_UNLIKELY(strength < 10))
		strength = 10;
	
	switch (strength) {
	case (10):
		return AnimationEquations::easeLinear;
	case (20):
		return AnimationEquations::easeOutQuad;
	case (30):
		return AnimationEquations::easeOutCubic;
	case (40):
		return AnimationEquations::easeOutQuat;
	default:
		return AnimationEquations::easeOutGeneric;
    }    
}

