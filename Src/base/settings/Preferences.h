/* @@@LICENSE
*
*      Copyright (c) 2008-2012 Hewlett-Packard Development Company, L.P.
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




#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "Common.h"

#include <string>
#include <lunaservice.h>

#include "Mutex.h"
#include "CustomEvents.h"

#include <QObject>

class Preferences : public QObject
{
	Q_OBJECT

public:

	static Preferences* instance();

	/*
	 * MT safe
	 */
	std::string locale() const;
	std::string localeRegion() const;
	std::string phoneRegion() const;
	std::string timeFormat() const;
	std::string deviceName() const;
	uint32_t lockTimeout() const;
	void setLockTimeout(uint32_t timeout);
	
	std::string getCurrentRingtone() { return m_currentRingtoneFile;}
	std::string getCurrentAlerttone() { return m_currentAlerttoneFile;}
	std::string getCurrentNotificationtone() { return m_currentNotificationtoneFile;}
	
	bool showAlertsWhenLocked() const { return m_showAlertsWhenLocked; }
	bool ledThrobberEnabled() const { return m_ledThrobberEnabled; }
	bool playFeedbackSounds() const { return m_playFeedbackSounds; }

	bool sysUiNoHomeButtonMode() const { return m_sysUiNoHomeButtonMode; }
	bool sysUiEnableNextPrevGestures() const { return m_sysUiEnableNextPrevGestures; }
	int sysUiGestureDetection() const { return m_sysUiGestureDetection; }
	bool sysUiEnableAppSwitchGestures() const { return m_sysUiEnableAppSwitchGestures; }
	bool sysUiStatusBarSlide() const { return m_sysUiStatusBarSlide; }
	bool sysUiEnableStatusBarSearch() const { return m_sysUiEnableStatusBarSearch; }
	bool sysUiEnableMaximizeEdges() const { return m_sysUiEnableMaximizeEdges; }
	bool sysUiShowDeviceNameAsCarrierText() const { return m_sysUiShowDeviceNameAsCarrierText; }
	bool sysUiUseCustomCarrierString() const { return m_sysUiUseCustomCarrierString; }
	std::string sysUiCarrierString() { return m_sysUiCarrierString; }
	bool sysUiEnableSpreadGesture() const { return m_sysUiEnableSpreadGesture; }
	bool sysUiEnableMiniCards() const { return m_sysUiEnableMiniCards; }
	bool sysUiEnableZoomGesture() const { return m_sysUiEnableZoomGesture; }
	bool sysUiEnableWaveLauncher() const { return m_sysUiEnableWaveLauncher; }

	bool imeEnabled() const { return m_imeEnabled; }
	bool pinyinEnabled() const { return m_pinyinEnabled; }
	bool hwrEnabled() const { return m_hwrEnabled; }

	bool getPinyinPassthrough() const { return m_pinyinPassthrough; }
	void setPinyinPassthrough(bool enabled) { m_pinyinPassthrough = enabled; }

	bool getVoiceDialEnabled() const { return m_enableVoiceDial; }

	static uint32_t roundLockTimeout(uint32_t unrounded);

	bool airplaneMode() const;
	bool setAirplaneMode(bool on);
	bool wifiState() const;
	bool saveWifiState (bool on);
	bool bluetoothState() const;
	bool saveBluetoothState (bool on);

	bool show3GForEvdo() const { return m_show3GForEvdo; }
	bool useDualRSSI() const { return m_dualRSSI; }
	bool hideWANAlert() const { return m_hideWANAlert; }
	std::string roamingIndicator() const { return m_roamingIndicator; }
    OrientationEvent::Orientation rotationLock() const;
    bool setRotationLockPref(OrientationEvent::Orientation lockedOrientation);
	bool isMuteOn() const;
	bool setMuteSoundPref(bool mute);
	bool isAlsEnabled() const;

	bool setStringPreference(const char * keyName, const char * value);
  bool getTabbedCardsPreference() const { return m_tabbedCardsEnabled; }

  bool getInfiniteCardCyclingPreference() const { return m_infiniteCardCyclingEnabled; }

	bool getShowReticleAnimationPreference() const { return m_showReticleAnimation; }

Q_SIGNALS:

	// Signals
	void signalWallPaperChanged(const char* filePath);
	void signalDockModeWallPaperChanged(const char* filePath);
	void signalSetLockTimeout(uint32_t timeout);
	void signalAirplaneModeChanged(bool enabled);
	void signalRoamingIndicatorChanged();
	void signalDualRssiEnabled();
	void signalTimeFormatChanged(const char* format);
	void signalVoiceDialSettingChanged(bool v);
    void signalRotationLockChanged(OrientationEvent::Orientation rotationLock);
	void signalMuteSoundChanged(bool muteOn);
	void signalAlsEnabled(bool enable);
	void signalDeviceNameChanged(std::string deviceName);
	void signalGetPrefsComplete();
	
private:

	Preferences();
	~Preferences();

	void registerService();
	void init();

	static bool serverConnectCallback(LSHandle *sh, LSMessage *message, void *ctx);
	static bool getPreferencesCallback(LSHandle *sh, LSMessage *message, void *ctx);

	std::string m_locale;
	std::string m_localeRegion;
	std::string m_phoneRegion;
	std::string m_currentRingtoneFile;		//path and filename of ringtone
	std::string m_currentAlerttoneFile;		//path and filename of alert tone
	std::string m_currentNotificationtoneFile;	//path and filename of alert tone
	std::string m_currentTimeFormat;
	std::string m_deviceName; // device name
	bool m_showAlertsWhenLocked;
	bool m_ledThrobberEnabled;
	bool m_playFeedbackSounds;

	bool m_sysUiNoHomeButtonMode;
	bool m_sysUiEnableNextPrevGestures;
	int m_sysUiGestureDetection;
	bool m_sysUiEnableAppSwitchGestures;
	bool m_sysUiStatusBarSlide;
	bool m_sysUiEnableStatusBarSearch;
	bool m_sysUiEnableMaximizeEdges;
	bool m_sysUiShowDeviceNameAsCarrierText;
	bool m_sysUiUseCustomCarrierString;
	std::string m_sysUiCarrierString;
	bool m_sysUiEnableSpreadGesture;
	bool m_sysUiEnableMiniCards;
	bool m_sysUiEnableZoomGesture;
	bool m_sysUiEnableWaveLauncher;

	bool m_imeEnabled;
	bool m_pinyinEnabled;
	bool m_hwrEnabled;
    bool m_pinyinPassthrough;

	std::string m_roamingIndicator;
	bool m_hideWANAlert;
	bool m_dualRSSI;
	bool m_airplaneMode;
	bool m_wifiOn;
	bool m_bluetoothOn;
	bool m_show3GForEvdo;
    bool m_enableVoiceDial;
	uint32_t m_lockTimeout;
    OrientationEvent::Orientation m_rotationLock;
	bool m_muteOn;
	bool m_enableALS;
	
	mutable Mutex m_mutex;
	LSHandle* m_lsHandle;
	LSMessageToken m_serverStatusToken;

	bool m_infiniteCardCyclingEnabled;
	bool m_tabbedCardsEnabled;

	bool m_showReticleAnimation;

};
	
#endif /* PREFERENCES_H */
