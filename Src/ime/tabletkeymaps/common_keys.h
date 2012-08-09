/* @@@LICENSE
*
*      Copyright (c) 2010-2012 Hewlett-Packard Development Company, L.P.
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

#ifndef KEYBOARD_COMMON_KEYS_H
#define KEYBOARD_COMMON_KEYS_H
#include "../TabletKeymap.h"

#define KEY_1(w, k) { w, k, k, NULL }

#define NOKEY_1 { 0, cKey_None, cKey_None, NULL }
#define NOKEY_2 NOKEY_1, NOKEY_1
#define NOKEY_3 NOKEY_2, NOKEY_1
#define NOKEY_4 NOKEY_3, NOKEY_1
#define NOKEY_5 NOKEY_4, NOKEY_1
#define NOKEY_6 NOKEY_5, NOKEY_1

#define KEY_2(w, k, a) { w, k, a, NULL }
#define KEY_3(w, k, a, e) { w, k, a, e }

#define SPACE_SIZE 5

/* UKey(0x2039) SINGLE LEFT-POINTING ANGLE QUOTATION MARK ‹ */
/* UKey(0x203A) SINGLE RIGHT-POINTING ANGLE QUOTATION MARK › */
/* UKey(0x00B7) MIDDLE DOT · */
/* UKey(0x201A) SINGLE LOW-9 QUOTATION MARK ‚ */
/* UKey(0x201E) DOUBLE LOW-9 QUOTATION MARK „ */
/* UKey(0x201B) SINGLE HIGH-REVERSED-9 QUOTATION MARK ‛ */
/* UKey(0x201F) DOUBLE HIGH-REVERSED-9 QUOTATION MARK ‟ */

// turn off backdoors for shipping version
#if SHIPPING_VERSION && !defined(TARGET_DESKTOP)
	#define sToggleLanguage_extended NULL
	#define sOptions NULL
#else
	static TabletKeymap::constUKeyArray sToggleLanguage_extended = { cKey_SwitchToQwerty, cKey_SwitchToAzerty, cKey_SwitchToQwertz, cKey_CreateDefaultKeyboards, cKey_ClearDefaultKeyboards, cKey_None };
	static TabletKeymap::constUKeyArray sOptions = { cKey_ToggleSuggestions, cKey_ShowXT9Regions, cKey_ShowKeymapRegions, cKey_StartStopRecording, cKey_ToggleSoundFeedback, cKey_None };
#endif

static UKey	sLanguageChoices_Extended[cKey_KeyboardComboChoice_Last - cKey_KeyboardComboChoice_First + 2] = { cKey_None };

static TabletKeymap::constUKeyArray sA_extended = { Qt::Key_A, Qt::Key_Agrave, Qt::Key_Aacute, Qt::Key_Acircumflex, Qt::Key_Atilde, Qt::Key_Adiaeresis, Qt::Key_Aring, UKey(0x00E6) /* LATIN SMALL LETTER AE æ */, UKey(0x00AA) /* FEMININE ORDINAL INDICATOR ª */, cKey_None };
static TabletKeymap::constUKeyArray sC_extended = { Qt::Key_C, Qt::Key_Ccedilla, UKey(0x0107) /* LATIN SMALL LETTER C WITH ACUTE ć */, Qt::Key_copyright, Qt::Key_cent, cKey_None };
static TabletKeymap::constUKeyArray sD_extended = { Qt::Key_D, UKey(0x00F0) /* LATIN SMALL LETTER ETH ð */, UKey(0x2020) /* DAGGER † */, UKey(0x2021) /* 	DOUBLE DAGGER ‡ */, cKey_None };
static TabletKeymap::constUKeyArray sE_extended = { Qt::Key_E, Qt::Key_Egrave, Qt::Key_Eacute, Qt::Key_Ecircumflex, Qt::Key_Ediaeresis, UKey(0x0119) /* LATIN SMALL LETTER E WITH OGONEK ę */, UKey(0x0113) /* LATIN SMALL LETTER E WITH MACRON ē */, cKey_None };
static TabletKeymap::constUKeyArray sG_extended = { Qt::Key_G, UKey(0x011F) /* LATIN SMALL LETTER G WITH BREVE ğ */, cKey_None };
static TabletKeymap::constUKeyArray sI_extended = { Qt::Key_I, Qt::Key_Igrave, Qt::Key_Iacute, Qt::Key_Icircumflex, Qt::Key_Idiaeresis, UKey(0x0130) /* LATIN CAPITAL LETTER I WITH DOT ABOVE İ  */, UKey(0x0131) /* LATIN SMALL LETTER DOTLESS I ı */, cKey_None };
static TabletKeymap::constUKeyArray sL_extended = { Qt::Key_L, UKey(0x00141) /* LATIN CAPITAL LETTER L WITH STROKE Ł */, cKey_None };
static TabletKeymap::constUKeyArray sM_extended = { Qt::Key_M, UKey(0x00B5) /* MICRO SIGN µ */, cKey_None };
static TabletKeymap::constUKeyArray sN_extended = { Qt::Key_N, UKey(0x00F1) /* LATIN SMALL LETTER N WITH TILDE ñ */, UKey(0x0144) /* LATIN SMALL LETTER N WITH ACUTE ń */, cKey_None };
static TabletKeymap::constUKeyArray sO_extended = { Qt::Key_O, Qt::Key_Ograve, Qt::Key_Oacute, Qt::Key_Ocircumflex, Qt::Key_Otilde, Qt::Key_Odiaeresis, Qt::Key_Ooblique, UKey(0x0151) /* LATIN SMALL LETTER O WITH DOUBLE ACUTE ő */,
											  UKey(0x0153) /* latin small letter œ */, UKey(0x00BA) /* MASCULINE ORDINAL INDICATOR º */, UKey(0x03C9) /* GREEK SMALL LETTER OMEGA ω */, cKey_None };
static TabletKeymap::constUKeyArray sP_extended = { Qt::Key_P /*, UKey(0x00B6) / * PILCROW SIGN ¶ */, UKey(0x00A7) /* SECTION SIGN § */, UKey(0x03C0) /* GREEK SMALL LETTER PI π */, cKey_None };
static TabletKeymap::constUKeyArray sR_extended = { Qt::Key_R, Qt::Key_registered, cKey_None };
static TabletKeymap::constUKeyArray sS_extended = { Qt::Key_S, UKey(0x0161) /* LATIN SMALL LETTER S WITH CARON š */, UKey(0x015E) /* LATIN CAPITAL LETTER S WITH CEDILLA ş */, Qt::Key_ssharp, UKey(0x03C3) /* GREEK SMALL LETTER SIGMA σ */, cKey_None };
static TabletKeymap::constUKeyArray sT_extended = { Qt::Key_T, UKey(0x2122) /* TRADE MARK SIGN ™ */, Qt::Key_THORN, cKey_None };
static TabletKeymap::constUKeyArray sU_extended = { Qt::Key_U, Qt::Key_Ugrave, Qt::Key_Uacute, Qt::Key_Ucircumflex, Qt::Key_Udiaeresis, UKey(0x0171) /* LATIN SMALL LETTER U WITH DOUBLE ACUTE ű */, cKey_None };
static TabletKeymap::constUKeyArray sY_extended = { Qt::Key_Y, Qt::Key_Yacute, Qt::Key_ydiaeresis, cKey_None };
static TabletKeymap::constUKeyArray sZ_extended = { Qt::Key_Z, UKey(0x017E) /* LATIN SMALL LETTER Z WITH CARON ž */, UKey(0x017A) /* LATIN SMALL LETTER Z WITH ACUTE ź */, UKey(0x017C) /* LATIN SMALL LETTER Z WITH DOT ABOVE ż */, cKey_None };

static TabletKeymap::constUKeyArray sHide_extended = { cKey_Resize_Tiny, cKey_Resize_Small, cKey_Resize_Default, cKey_Resize_Large, cKey_None };

static TabletKeymap::constUKeyArray sSingleAndDoubleQuote_extended = { Qt::Key_Apostrophe, Qt::Key_QuoteDbl, UKey(0x0060) /* GRAVE ACCENT ` */,
																 UKey(0x2018) /* LEFT SINGLE QUOTATION MARK ‘ */, UKey(0x2019) /* RIGHT SINGLE QUOTATION MARK ’ */,
																 UKey(0x201C) /* LEFT DOUBLE QUOTATION MARK “ */, UKey(0x201D) /* RIGHT DOUBLE QUOTATION MARK ” */,
																 UKey(0x00AB) /* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK « */, UKey(0x00BB) /* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK » */, cKey_None };
static TabletKeymap::constUKeyArray sPeriodQuestion_extended = { Qt::Key_Period, Qt::Key_Question, UKey(0x2022) /* BULLET • */, UKey(0x2026) /* HORIZONTAL ELLIPSIS … */, Qt::Key_questiondown, cKey_None };
static TabletKeymap::constUKeyArray sMinusUnderscore_extended = { Qt::Key_Minus, Qt::Key_Underscore, UKey(0x00B1) /* PLUS-MINUS SIGN ± */, UKey(0x00AC) /* NOT SIGN ¬ */, cKey_None };
static TabletKeymap::constUKeyArray sCommaSlash_extended = { Qt::Key_Comma, Qt::Key_Slash, Qt::Key_Backslash, cKey_None };

static TabletKeymap::constUKeyArray sURL_extended = { cKey_HTTPColonSlashSlash, cKey_HTTPSColonSlashSlash, cKey_WWW, cKey_None };

#endif // KEYBOARD_COMMON_KEYS_H
