/* @@@LICENSE
*
*      Copyright (c) 2010-2012 Hewlett-Packard Development Company, L.P.
*                    2012 Måns Andersson <mail@mansandersson.se>
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




#include "VirtualKeyboardPreferences.h"
#include "Preferences.h"
#include "Logging.h"
#include "SystemUiController.h"
#include "JSONUtils.h"

VirtualKeyboardPreferences & VirtualKeyboardPreferences::instance()
{
	static VirtualKeyboardPreferences sInstance;
	return sInstance;
}

pbnjson::JValue jsonPair(const char * key1, const char * value1, const char * key2, const char * value2)
{
	pbnjson::JValue value = pbnjson::Object();
	value.put(key1, value1);
	value.put(key2, value2);
	return value;
}

VirtualKeyboardPreferences::VirtualKeyboardPreferences() : mTapSounds(true), mSpaces2period(true), mKeyboardSize(0), mSettingsReceived(false), mPrefsReceived(false), mVirtualKeyboard(NULL)
{	// will be created in every sysmgr based process. Don't do anything expensive here!
}

void VirtualKeyboardPreferences::applyInitSettings(VirtualKeyboard * keyboard)
{	// called by the VirtualKeyboard derived class constructor. We are in the "real" LunaSysMgr, not in WebAppMgr on the webkit side!
	mVirtualKeyboard = keyboard;

	pbnjson::JValue values = pbnjson::Array();
	QList<const char *> list = mVirtualKeyboard->getLayoutNameList();
	Q_FOREACH(const char * layout, list)
	{
		values.append(layout);
	}
	Preferences * prefs = Preferences::instance();
	if (VERIFY(prefs))
		prefs->setStringPreference(PALM_VIRTUAL_KEYBOARD_LAYOUTS, jsonToString(values).c_str());
	if (VERIFY(mVirtualKeyboard))	// at this point, there should always be an existing keyboard!
	{
		keyboard->requestSize(mKeyboardSize);
		activateCombo();
	}
	connect(SystemUiController::instance(), SIGNAL(signalBootFinished()), SLOT(bootFinished()));
}

void VirtualKeyboardPreferences::bootFinished()
{
	if (mVirtualKeyboard)
	{
		if (!mSettingsReceived)
		{
			g_warning("VirtualKeyboardPreferences::bootFinished: no keyboard settings received. Forcing next keyboard combo...");
			selectNextKeyboardCombo();
		}
		if (!mPrefsReceived)
		{
			g_warning("VirtualKeyboardPreferences::bootFinished: no keyboard preferences received. Saving prefs...");
			savePreferences(mCombos);
		}
	}
}

void VirtualKeyboardPreferences::applyFirstUseSettings()
{
	if (mVirtualKeyboard)
	{
		clearDefaultDeyboards();
		selectNextKeyboardCombo();
	}
}

void VirtualKeyboardPreferences::localeChanged()
{
	if (mVirtualKeyboard && mCombos.empty())
    {
		mActiveCombo = getDefault();
        activateCombo();
        saveSettings();
		g_message("VirtualKeyboardPreferences::localeChanged: resetting to locale default: %s/%s", mActiveCombo.keyboardLanguage.c_str(), mActiveCombo.autoCorrectLanguage.c_str());
	}
}

void VirtualKeyboardPreferences::selectKeymap(const std::string keymap)
{
	if (mVirtualKeyboard && !keymap.empty())
	{
		mActiveCombo.setKeymap(keymap);
		activateCombo();
		saveSettings();
	}
}

void VirtualKeyboardPreferences::selectNextKeyboardCombo()
{
	if (mCombos.empty())
		mActiveCombo.clear();
	else if (mCombos.size() == 1)
		mActiveCombo = mCombos[0];
	else
	{
		size_t i = 0;
		while (i < mCombos.size())
		{
			if (mCombos[i++] == mActiveCombo)
				break;
		}
		mActiveCombo = mCombos[i < mCombos.size() ? i : 0];	// if we haven't found the current combo, or if it was last, wrap around to 0
	}
	activateCombo();
	saveSettings();
}

void VirtualKeyboardPreferences::selectKeyboardCombo(int index)
{
	if (index >= 0 && index < (int) mCombos.size())
	{
		mActiveCombo = mCombos[index];
		activateCombo();
		saveSettings();
	}
}

void VirtualKeyboardPreferences::selectLayoutCombo(const char * layoutName)
{
	// first, find a combo using that layout...
	for (std::vector<SKeyboardCombo>::iterator iter = mCombos.begin(); iter != mCombos.end(); ++iter)
	{
		if (strcasecmp(iter->keyboardLanguage.c_str(), layoutName) == 0)
		{
			mActiveCombo = *iter;
			activateCombo();
			saveSettings();
			return;
		}
	}
	if (mVirtualKeyboard)
	{
		// if no combo, make one!
		const char * language = mVirtualKeyboard->getLayoutDefaultLanguage(layoutName);
		if (language)
		{
			mActiveCombo.keyboardLanguage = layoutName;
			mActiveCombo.autoCorrectLanguage = language;
			activateCombo();
			saveSettings();
		}
	}
}

void VirtualKeyboardPreferences::activateCombo()
{
	if (mVirtualKeyboard)
	{
        if (mCombos.size() == 0 || (mCombos.size() == 1 && mActiveCombo == mCombos[0]))
		{
			if (mActiveCombo.keyboardLanguage.empty())
				mActiveCombo = getDefault(); // @@@ Default?
			mVirtualKeyboard->setKeyboardCombo(mActiveCombo.keyboardLanguage, mActiveCombo.keymap, mActiveCombo.autoCorrectLanguage, false);
		}
		else
		{
			mVirtualKeyboard->setKeyboardCombo(mActiveCombo.keyboardLanguage, mActiveCombo.keymap, mActiveCombo.autoCorrectLanguage, true);
		}
	}
}

void VirtualKeyboardPreferences::selectKeyboardSize(int size)
{
	if (size != mKeyboardSize)
	{
		mKeyboardSize = size;
		saveSettings();
		if (mVirtualKeyboard)
			mVirtualKeyboard->requestSize(mKeyboardSize);
	}
}

void VirtualKeyboardPreferences::saveSettings()
{
	Preferences * prefs = Preferences::instance();
	if (VERIFY(prefs))
	{
		pbnjson::JValue keymapsPref = pbnjson::Object();
		size_t i = 0;
		
		pbnjson::JValue settings = jsonPair("layout", mActiveCombo.keyboardLanguage.c_str(), "language", mActiveCombo.autoCorrectLanguage.c_str());
		settings.put("keyboard size", mKeyboardSize);
		prefs->setStringPreference(PALM_VIRTUAL_KEYBOARD_SETTINGS, jsonToString(settings).c_str());

		// Save keymap settings
		i = 0;
		while (i < mCombos.size())
		{
			if (mCombos[i] == mActiveCombo)
			{
				mCombos[i].setKeymap(mActiveCombo.keymap);
			}
			if (!mCombos[i].keymap.empty())
			{
				keymapsPref.put(mCombos[i].keyboardLanguage.c_str(), mCombos[i].keymap);
			}
			i++;
		}
		prefs->setStringPreference(PALM_VIRTUAL_KEYBOARD_SELECTED_KEYMAPS, jsonToString(keymapsPref).c_str());
	}
}

void VirtualKeyboardPreferences::virtualKeyboardPreferencesChanged(const char * prefs)
{
	if (mVirtualKeyboard)
	{
		JsonMessageParser	parser(prefs, SCHEMA_ANY);
		if (parser.parse(__FUNCTION__))
		{
			mPrefsReceived = true;
            bool currentComboKnown = false;
			//g_debug("%s: Parsed '%s'", __FUNCTION__, prefs);
			pbnjson::JValue	array(parser.get("keyboards"));
			if (array.isArray())
			{
				mCombos.clear();
				for (ssize_t index = 0; index < array.arraySize(); ++index)
				{
					JsonValue value(array[index]);
					SKeyboardCombo combo;
					if (value.get("layout", combo.keyboardLanguage) && value.get("language", combo.autoCorrectLanguage) && !combo.empty())
                    {
						mCombos.push_back(combo);
                        if (mActiveCombo == combo)
                            currentComboKnown = true;
                    }
				}
			}
			else
				mPrefsReceived = false;
            if (currentComboKnown)
				activateCombo();
            else
                selectNextKeyboardCombo();
			bool value;
			if (parser.get("TapSounds", value))
				mTapSounds = value;
			else
				mPrefsReceived = false;
			if (parser.get("spaces2period", value))
				mSpaces2period = value;
			else
				mPrefsReceived = false;
			mVirtualKeyboard->keyboardCombosChanged();
		}
	}
}

void VirtualKeyboardPreferences::virtualKeyboardSettingsChanged(const char * settings)
{
	if (mVirtualKeyboard)
	{
		JsonMessageParser	parser(settings, SCHEMA_ANY);
		if (parser.parse(__FUNCTION__))
		{
			//g_debug("%s: Parsed '%s'", __FUNCTION__, settings);
			SKeyboardCombo combo;
			if (parser.get("layout", combo.keyboardLanguage) && parser.get("language", combo.autoCorrectLanguage) && !combo.empty())
			{
				mSettingsReceived = true;	// we got some valid settings: don't force them again (required by the smartkey service to work properly)
				if (combo != mActiveCombo)
				{
					mActiveCombo = combo;
					if (mActiveCombo.autoCorrectLanguage.empty())
						mActiveCombo.autoCorrectLanguage = getDefault().autoCorrectLanguage;
					activateCombo();
				}
			}
			int	size;
			if (parser.get("keyboard size", size) && mKeyboardSize != size)
			{
				mKeyboardSize = size;
				mVirtualKeyboard->requestSize(size);
			}
		}
	}
}

void VirtualKeyboardPreferences::virtualKeyboardSelectedKeymapsChanged(const char * selectedKeymaps)
{
	if (mVirtualKeyboard)
	{
		JsonMessageParser	parser(selectedKeymaps, SCHEMA_ANY);
		if (parser.parse(__FUNCTION__))
		{
			mSelectedKeymapsReceived = true;
			//g_debug("%s: Parsed '%s'", __FUNCTION__, prefs);
			unsigned int i=0;
			while (i<mCombos.size())
			{
				std::string keymap;
				if (parser.get(mCombos[i].keyboardLanguage.c_str(), keymap))
				{
					mCombos[i].setKeymap(keymap);
					if (mCombos[i] == mActiveCombo)
					{
						mActiveCombo.setKeymap(keymap);
					}
				}
				i++;
			}
			mVirtualKeyboard->keyboardCombosChanged();
			activateCombo();
		}
	}
}

VirtualKeyboardPreferences::SKeyboardCombo	VirtualKeyboardPreferences::getDefault()
{
	std::string	locale;
	SKeyboardCombo	combo;
	Preferences * prefs = Preferences::instance();
	if (prefs)
		locale = prefs->locale();
	const char * language = NULL;
	if (mVirtualKeyboard && strncasecmp(locale.c_str(), "fr_fr", 5) == 0)
	{
        combo.autoCorrectLanguage = "fr";
        combo.keyboardLanguage = "French";
	}
	else if (mVirtualKeyboard && strncasecmp(locale.c_str(), "de_de", 5) == 0)
	{
        combo.autoCorrectLanguage = "de";
        combo.keyboardLanguage = "German";
	}
	else
	{
        combo.keyboardLanguage = "English";
		combo.autoCorrectLanguage = locale;
	}
	return combo;
}

void VirtualKeyboardPreferences::setTapSounds(bool on)
{
	mTapSounds = on;
	savePreferences(mCombos);
}

// for test purposes
void VirtualKeyboardPreferences::createDefaultKeyboards()
{
	Preferences * prefs = Preferences::instance();
	if (prefs && mVirtualKeyboard)
	{
		pbnjson::JValue pref = pbnjson::Object();
		pbnjson::JValue array = pbnjson::Array();
		QList<const char *> list = mVirtualKeyboard->getLayoutNameList();
		Q_FOREACH(const char * layout, list)
		{
			const char * language = mVirtualKeyboard->getLayoutDefaultLanguage(layout);
			if (VERIFY(language))
				array.append(jsonPair("layout", layout, "language", language));
		}
		array.append(jsonPair("layout", "English", "language", "none"));
		pref.put("keyboards", array);
		pref.put("TapSounds", mTapSounds);
		pref.put("spaces2period", mSpaces2period);
		prefs->setStringPreference(PALM_VIRTUAL_KEYBOARD_PREFS, jsonToString(pref).c_str());
	}
}

// for test purposes
void VirtualKeyboardPreferences::clearDefaultDeyboards()
{
	Preferences * prefs = Preferences::instance();
	if (prefs)
	{
		std::vector<SKeyboardCombo> combos;
		savePreferences(combos);
	}
}

// for test purposes
void VirtualKeyboardPreferences::savePreferences(const std::vector<SKeyboardCombo> & combos)
{
	Preferences * prefs = Preferences::instance();
	if (prefs)
	{
		pbnjson::JValue pref = pbnjson::Object();
		pbnjson::JValue array = pbnjson::Array();
		for (std::vector<SKeyboardCombo>::const_iterator iter = combos.begin(); iter != combos.end(); ++iter)
		{
			array.append(jsonPair("layout", iter->keyboardLanguage.c_str(), "language", iter->autoCorrectLanguage.c_str()));
		}
		pref.put("keyboards", array);
		pref.put("TapSounds", mTapSounds);
		pref.put("spaces2period", mSpaces2period);
		prefs->setStringPreference(PALM_VIRTUAL_KEYBOARD_PREFS, jsonToString(pref).c_str());
	}
}
