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

#ifndef KEYBOARD_FR_AZERTY_H
#define KEYBOARD_FR_AZERTY_H
#include "../TabletKeymap.h"
#include "common_keys.h"

static TabletKeymap::constUKeyArray sFrAzerty_DotCom_Extended = { cKey_DotCom, cKey_DotFr, cKey_DotNet, cKey_DotOrg, cKey_DotEdu, cKey_None };
static TabletKeymap::constUKeyArray sFrAzert1_extended = { Qt::Key_1, Qt::Key_Ampersand, UKey(0x00B9) /* SUPERSCRIPT ONE ¹ */, UKey(0x00BC) /* VULGAR FRACTION ONE QUARTER ¼ */, UKey(0x00BD) /* VULGAR FRACTION ONE HALF ½ */, cKey_None };
static TabletKeymap::constUKeyArray sFrAzert2_extended = { Qt::Key_2, Qt::Key_Eacute, UKey(0x00B2) /* SUPERSCRIPT TWO ² */, cKey_None };
static TabletKeymap::constUKeyArray sFrAzert3_extended = { Qt::Key_3, Qt::Key_QuoteDbl, UKey(0x00B3) /* SUPERSCRIPT THREE ³ */, UKey(0x00BE) /* VULGAR FRACTION THREE QUARTERS ¾ */,
												   UKey(0x201C) /* LEFT DOUBLE QUOTATION MARK “ */, UKey(0x201D) /* RIGHT DOUBLE QUOTATION MARK ” */,
												   UKey(0x00AB) /* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK « */, UKey(0x00BB) /* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK » */, cKey_None };
static TabletKeymap::constUKeyArray sFrAzert4_extended = { Qt::Key_4, Qt::Key_Apostrophe, UKey(0x2018) /* LEFT SINGLE QUOTATION MARK ‘ */, UKey(0x2019) /* RIGHT SINGLE QUOTATION MARK ’ */, cKey_None };
static TabletKeymap::constUKeyArray sFrAzert5_extended = { Qt::Key_5, Qt::Key_ParenLeft, Qt::Key_BracketLeft, Qt::Key_BraceLeft, cKey_None };
static TabletKeymap::constUKeyArray sFrAzert6_extended = { Qt::Key_6, Qt::Key_Minus, UKey(0x00B1) /* PLUS-MINUS SIGN ± */, UKey(0x00AC) /* NOT SIGN ¬ */, cKey_None };
static TabletKeymap::constUKeyArray sFrAzert7_extended = { Qt::Key_7, Qt::Key_Egrave, Qt::Key_QuoteLeft, cKey_None };
static TabletKeymap::constUKeyArray sFrAzert8_extended = { Qt::Key_8, Qt::Key_ParenRight, Qt::Key_BracketRight, Qt::Key_BraceRight, cKey_None };
static TabletKeymap::constUKeyArray sFrAzert9_extended = { Qt::Key_9, Qt::Key_Ccedilla, Qt::Key_cent, Qt::Key_Dollar, cKey_Euro, Qt::Key_sterling, Qt::Key_yen, UKey(0x00A4) /* CURRENCY SIGN ¤ */, cKey_None };
static TabletKeymap::constUKeyArray sFrAzert0_extended = { Qt::Key_0, Qt::Key_Agrave, Qt::Key_Percent, UKey(0x2030) /* PER MILLE SIGN ‰ */, cKey_None };

static TabletKeymap::constUKeyArray sCommaQuestion_extended = { Qt::Key_Comma, Qt::Key_Question, Qt::Key_questiondown, cKey_None };
static TabletKeymap::constUKeyArray sSemicolonPeriod_extended = { Qt::Key_Semicolon, Qt::Key_Period, UKey(0x2022) /* BULLET • */, UKey(0x2026) /* HORIZONTAL ELLIPSIS … */, cKey_None };
static TabletKeymap::constUKeyArray sPeriodSemicolon_extended = { Qt::Key_Period, Qt::Key_Semicolon, UKey(0x2022) /* BULLET • */, UKey(0x2026) /* HORIZONTAL ELLIPSIS … */, cKey_None };
static TabletKeymap::constUKeyArray sColonSlash_extended = { Qt::Key_Colon, Qt::Key_Slash, Qt::Key_Backslash, cKey_None };
static TabletKeymap::constUKeyArray sAtUnderscore_extended = { Qt::Key_At, Qt::Key_Underscore, cKey_None };
static TabletKeymap::constUKeyArray sExclamAsterisk_extended = { Qt::Key_Exclam, Qt::Key_Asterisk, Qt::Key_exclamdown, cKey_None };

#define FR_AZERTY_NUMBERS_10(w)			{ w, Qt::Key_Ampersand,		Qt::Key_1,									sFrAzert1_extended },\
										{ w, Qt::Key_Eacute,		Qt::Key_2,									sFrAzert2_extended },\
										{ w, Qt::Key_QuoteDbl,		Qt::Key_3,									sFrAzert3_extended },\
										{ w, Qt::Key_Apostrophe,	Qt::Key_4,									sFrAzert4_extended },\
										{ w, Qt::Key_ParenLeft,		Qt::Key_5,									sFrAzert5_extended },\
										{ w, Qt::Key_Minus,			Qt::Key_6,									sFrAzert6_extended },\
										{ w, Qt::Key_Egrave,		Qt::Key_7,									sFrAzert7_extended },\
										{ w, Qt::Key_ParenRight,	Qt::Key_8,									sFrAzert8_extended },\
										{ w, Qt::Key_Ccedilla,		Qt::Key_9,									sFrAzert9_extended },\
										{ w, Qt::Key_Agrave,		Qt::Key_0,									sFrAzert0_extended }

#define FR_AZERTY_TOP_10(w)				{ w, Qt::Key_A,				Qt::Key_AsciiTilde,							sA_extended },\
										{ w, Qt::Key_Z,				Qt::Key_NumberSign,							sZ_extended },\
										{ w, Qt::Key_E,				cKey_Euro,									sE_extended },\
										{ w, Qt::Key_R,				Qt::Key_Dollar,								sR_extended },\
										{ w, Qt::Key_T,				Qt::Key_Backslash,							sT_extended },\
										{ w, Qt::Key_Y, 			Qt::Key_Bar,								sY_extended },\
										{ w, Qt::Key_U,				Qt::Key_BraceLeft,							sU_extended },\
										{ w, Qt::Key_I,				Qt::Key_BraceRight,							sI_extended },\
										{ w, Qt::Key_O,				Qt::Key_BracketLeft,						sO_extended },\
										{ w, Qt::Key_P,				Qt::Key_BracketRight,						sP_extended }

#define FR_AZERTY_MID_10(w)				{ w, Qt::Key_Q,				Qt::Key_Less,								NULL },\
										{ w, Qt::Key_S,				Qt::Key_Greater,							sS_extended },\
										{ w, Qt::Key_D,				Qt::Key_Equal,								sD_extended },\
										{ w, Qt::Key_F,				Qt::Key_Plus,								NULL },\
										{ w, Qt::Key_G,				UKey(0x00D7) /* multiplication sign */,		sG_extended },\
										{ w, Qt::Key_H,				UKey(0x00F7) /* division sign */,			NULL },\
										{ w, Qt::Key_J,				Qt::Key_Percent,							NULL },\
										{ w, Qt::Key_K,				Qt::Key_degree,								NULL },\
										{ w, Qt::Key_L,				Qt::Key_diaeresis,							sL_extended },\
										{ w, Qt::Key_M,				Qt::Key_AsciiCircum,						sM_extended }

#define FR_AZERTY_LOW_9(w)				{ w, Qt::Key_W,				cKey_Emoticon_Smile,						NULL  },\
										{ w, Qt::Key_X,				cKey_Emoticon_Wink,							sOptions },\
										{ w, Qt::Key_C,				cKey_Emoticon_Frown,						sC_extended },\
										{ w, Qt::Key_V,				cKey_Emoticon_Cry,							NULL },\
										{ w, Qt::Key_B,				cKey_Emoticon_Yuck,							sToggleLanguage_extended },\
										{ w, Qt::Key_N,				cKey_Emoticon_Gasp,							sN_extended },\
										{ w, Qt::Key_Comma,			Qt::Key_Question,							sCommaQuestion_extended },\
										{ w, Qt::Key_Period,		Qt::Key_Semicolon,							sPeriodSemicolon_extended },\
										{ w, Qt::Key_Colon,			Qt::Key_Slash,								sColonSlash_extended }

#define FR_AZERTY_BOTTOM_DEFAULT \
									KEY_1(1, Qt::Key_Tab),\
									KEY_1(2, cKey_Symbol),\
									NOKEY_1,\
									NOKEY_1,\
									KEY_1(SPACE_SIZE, Qt::Key_Space),\
									NOKEY_1,\
									{ 1, Qt::Key_At,			Qt::Key_Underscore,							sAtUnderscore_extended },\
									{ 1, Qt::Key_Exclam,		Qt::Key_Asterisk,							sExclamAsterisk_extended },\
									{ 1.5, cKey_Hide,			cKey_Hide,									sHide_extended },\
									NOKEY_3

#define FR_AZERTY_BOTTOM_URL \
									KEY_1(1, Qt::Key_Tab),\
									KEY_1(2, cKey_Symbol),\
									NOKEY_1,\
									KEY_1(1, Qt::Key_Slash),\
									KEY_1(SPACE_SIZE - 2, Qt::Key_Space),\
									KEY_3(1, cKey_DotCom, cKey_DotCom, sFrAzerty_DotCom_Extended),\
									{ 1, Qt::Key_At,			Qt::Key_Underscore,							sAtUnderscore_extended },\
									{ 1, Qt::Key_Exclam,		Qt::Key_Asterisk,							sExclamAsterisk_extended },\
									{ 1.5, cKey_Hide,			cKey_Hide,									sHide_extended },\
									NOKEY_3

#define FR_AZERTY_BOTTOM_EMAIL \
									KEY_1(1, Qt::Key_Tab),\
									KEY_1(2, cKey_Symbol),\
									NOKEY_1,\
									NOKEY_1,\
									KEY_1(SPACE_SIZE - 1, Qt::Key_Space),\
									KEY_3(1, cKey_DotCom, cKey_DotCom, sFrAzerty_DotCom_Extended),\
									{ 1, Qt::Key_At,			Qt::Key_Underscore,							sAtUnderscore_extended },\
									{ 1, Qt::Key_Exclam,		Qt::Key_Asterisk,							sExclamAsterisk_extended },\
									{ 1.5, cKey_Hide,			cKey_Hide,									sHide_extended },\
									NOKEY_3

static TabletKeymap::Layout sFrAzerty = {
	{ KEY_2(-0.5, Qt::Key_Ampersand, Qt::Key_1), FR_AZERTY_NUMBERS_10(1), KEY_1(-1, Qt::Key_Backspace) },
	{ FR_AZERTY_TOP_10(1), KEY_1(1.5, Qt::Key_Backspace), NOKEY_1 },
	{ KEY_2(-0.5, Qt::Key_Q, Qt::Key_Less), FR_AZERTY_MID_10(1), KEY_1(1, Qt::Key_Return) },
	{ KEY_1(1, Qt::Key_Shift), FR_AZERTY_LOW_9(1), KEY_1(1.5, Qt::Key_Shift), NOKEY_1 },
	{ FR_AZERTY_BOTTOM_DEFAULT },
};

static TabletKeymap::LayoutRow sFrAzertyBottomRow_default = { FR_AZERTY_BOTTOM_DEFAULT };
static TabletKeymap::LayoutRow sFrAzertyBottomRow_url = { FR_AZERTY_BOTTOM_URL };
static TabletKeymap::LayoutRow sFrAzertyBottomRow_email = { FR_AZERTY_BOTTOM_EMAIL };

static TabletKeymap::LayoutFamily sFrAzertyFamily("fr azerty", "fr", IME_KBD_LANG_French, IME_KBD_SEC_REGQwerty,
                                                "+ = [  ]" /* Spaces are "Unicode Character 'HAIR SPACE' (U+200A) ' ' " */, "A z y" /* Spaces are "Unicode Character 'HAIR SPACE' (U+200A) ' ' " */,
                                                0, 1, 11, 2, true, sFrAzerty, sFrAzertyBottomRow_default, sFrAzertyBottomRow_url, sFrAzertyBottomRow_email);
#endif // KEYBOARD_FR_AZERTY_H
