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

#ifndef KEYBOARD_US_QWERTY_H
#define KEYBOARD_US_QWERTY_H
#include "../TabletKeymap.h"
#include "common_keys.h"

static TabletKeymap::constUKeyArray sUsQwerty_DotCom_Extended = { cKey_DotCom, cKey_DotNet, cKey_DotOrg, cKey_DotEdu, cKey_None };
static TabletKeymap::constUKeyArray sUsQwerty1_extended = { Qt::Key_1, Qt::Key_Exclam, UKey(0x00B9) /* SUPERSCRIPT ONE ¹ */, UKey(0x00BC) /* VULGAR FRACTION ONE QUARTER ¼ */, UKey(0x00BD) /* VULGAR FRACTION ONE HALF ½ */, Qt::Key_exclamdown, cKey_None };
static TabletKeymap::constUKeyArray sUsQwerty2_extended = { Qt::Key_2, Qt::Key_At, UKey(0x00B2) /* SUPERSCRIPT TWO ² */, cKey_None };
static TabletKeymap::constUKeyArray sUsQwerty3_extended = { Qt::Key_3, Qt::Key_NumberSign, UKey(0x00B3) /* SUPERSCRIPT THREE ³ */, UKey(0x00BE) /* VULGAR FRACTION THREE QUARTERS ¾ */, cKey_None };
static TabletKeymap::constUKeyArray sUsQwerty4_extended = { Qt::Key_4, Qt::Key_Dollar, cKey_Euro, Qt::Key_sterling, Qt::Key_yen, Qt::Key_cent, UKey(0x00A4) /* CURRENCY SIGN ¤ */, cKey_None };
static TabletKeymap::constUKeyArray sUsQwerty5_extended = { Qt::Key_5, Qt::Key_Percent, UKey(0x2030) /* PER MILLE SIGN ‰ */, cKey_None };
static TabletKeymap::constUKeyArray sUsQwerty6_extended = { Qt::Key_6, Qt::Key_AsciiCircum, cKey_None };
static TabletKeymap::constUKeyArray sUsQwerty7_extended = { Qt::Key_7, Qt::Key_Ampersand, cKey_None };
static TabletKeymap::constUKeyArray sUsQwerty8_extended = { Qt::Key_8, Qt::Key_Asterisk, cKey_None };
static TabletKeymap::constUKeyArray sUsQwerty9_extended = { Qt::Key_9, Qt::Key_ParenLeft, Qt::Key_BracketLeft, Qt::Key_BraceLeft, cKey_None };
static TabletKeymap::constUKeyArray sUsQwerty0_extended = { Qt::Key_0, Qt::Key_ParenRight, Qt::Key_BracketRight, Qt::Key_BraceRight, cKey_None };

#define US_QWERTY_NUMBERS_10(w)			{ w, Qt::Key_1,			Qt::Key_Exclam,							sUsQwerty1_extended },\
										{ w, Qt::Key_2,			Qt::Key_At,								sUsQwerty2_extended },\
										{ w, Qt::Key_3,			Qt::Key_NumberSign,						sUsQwerty3_extended },\
										{ w, Qt::Key_4,			Qt::Key_Dollar,							sUsQwerty4_extended },\
										{ w, Qt::Key_5,			Qt::Key_Percent,						sUsQwerty5_extended },\
										{ w, Qt::Key_6,	 		Qt::Key_AsciiCircum,					sUsQwerty6_extended },\
										{ w, Qt::Key_7,			Qt::Key_Ampersand,						sUsQwerty7_extended },\
										{ w, Qt::Key_8,			Qt::Key_Asterisk,						sUsQwerty8_extended },\
										{ w, Qt::Key_9,			Qt::Key_ParenLeft,						sUsQwerty9_extended },\
										{ w, Qt::Key_0,			Qt::Key_ParenRight,						sUsQwerty0_extended }

#define US_QWERTY_TOP_10(w)				{ w, Qt::Key_Q,			Qt::Key_QuoteLeft,						NULL },\
										{ w, Qt::Key_W,			Qt::Key_AsciiTilde,						NULL },\
										{ w, Qt::Key_E,			cKey_Euro,								sE_extended },\
										{ w, Qt::Key_R,			Qt::Key_sterling,						sR_extended },\
										{ w, Qt::Key_T,			Qt::Key_Backslash,						sT_extended },\
										{ w, Qt::Key_Y,	 		Qt::Key_Bar,							sY_extended },\
										{ w, Qt::Key_U,			Qt::Key_BraceLeft,						sU_extended },\
										{ w, Qt::Key_I,			Qt::Key_BraceRight,						sI_extended },\
										{ w, Qt::Key_O,			Qt::Key_BracketLeft,					sO_extended },\
										{ w, Qt::Key_P,			Qt::Key_BracketRight,					sP_extended }

#define US_QWERTY_MID_9(w)				{ w, Qt::Key_A,			Qt::Key_Less,							sA_extended },\
										{ w, Qt::Key_S,			Qt::Key_Greater,						sS_extended },\
										{ w, Qt::Key_D,			Qt::Key_Equal,							sD_extended },\
										{ w, Qt::Key_F,			Qt::Key_Plus,							NULL },\
										{ w, Qt::Key_G,			UKey(0x00D7) /* multiplication sign */,	sG_extended },\
										{ w, Qt::Key_H,			UKey(0x00F7) /* division sign */,		NULL },\
										{ w, Qt::Key_J,			Qt::Key_degree,							NULL },\
										{ w, Qt::Key_K,			Qt::Key_Semicolon,						NULL },\
										{ w, Qt::Key_L,			Qt::Key_Colon,							sL_extended }

#define US_QWERTY_LOW_9(w)				{ w, Qt::Key_Z,			cKey_Emoticon_Smile,					sZ_extended },\
										{ w, Qt::Key_X,			cKey_Emoticon_Wink,						sOptions },\
										{ w, Qt::Key_C,			cKey_Emoticon_Frown,					sC_extended },\
										{ w, Qt::Key_V,			cKey_Emoticon_Cry,						NULL },\
										{ w, Qt::Key_B,			cKey_Emoticon_Yuck,						sToggleLanguage_extended },\
										{ w, Qt::Key_N,			cKey_Emoticon_Gasp,						sN_extended },\
										{ w, Qt::Key_M,			cKey_Emoticon_Heart,					sM_extended },\
										{ w, Qt::Key_Comma,		Qt::Key_Slash,							sCommaSlash_extended },\
										{ w, Qt::Key_Period,	Qt::Key_Question,						sPeriodQuestion_extended }

#define US_QWERTY_BOTTOM_ROW_DEFAULT \
									KEY_1(1, Qt::Key_Tab),\
									KEY_1(2, cKey_Symbol),\
									NOKEY_1,\
									NOKEY_1,\
									KEY_1(SPACE_SIZE, Qt::Key_Space),\
									NOKEY_1,\
									KEY_3(1, Qt::Key_Apostrophe, Qt::Key_QuoteDbl, sSingleAndDoubleQuote_extended),\
									KEY_3(1, Qt::Key_Minus, Qt::Key_Underscore, sMinusUnderscore_extended),\
									KEY_3(1, cKey_Hide, cKey_Hide, sHide_extended),\
									NOKEY_3

#define US_QWERTY_BOTTOM_ROW_URL \
									KEY_1(1, Qt::Key_Tab),\
									KEY_1(2, cKey_Symbol),\
									NOKEY_1,\
									KEY_3(1, Qt::Key_Slash, Qt::Key_Slash, sURL_extended),\
									KEY_1(SPACE_SIZE - 2, Qt::Key_Space),\
									KEY_3(1, cKey_DotCom, cKey_DotCom, sUsQwerty_DotCom_Extended),\
									KEY_3(1, Qt::Key_Apostrophe, Qt::Key_QuoteDbl, sSingleAndDoubleQuote_extended),\
									KEY_3(1, Qt::Key_Minus, Qt::Key_Underscore, sMinusUnderscore_extended),\
									KEY_3(1, cKey_Hide, cKey_Hide, sHide_extended),\
									NOKEY_3

#define US_QWERTY_BOTTOM_ROW_EMAIL \
									KEY_1(1, Qt::Key_Tab),\
									KEY_1(2, cKey_Symbol),\
									NOKEY_1,\
									KEY_1(1, Qt::Key_At),\
									KEY_1(SPACE_SIZE - 2, Qt::Key_Space),\
									KEY_3(1, cKey_DotCom, cKey_DotCom, sUsQwerty_DotCom_Extended),\
									KEY_3(1, Qt::Key_Apostrophe, Qt::Key_QuoteDbl, sSingleAndDoubleQuote_extended),\
									KEY_3(1, Qt::Key_Minus, Qt::Key_Underscore, sMinusUnderscore_extended),\
									KEY_3(1, cKey_Hide, cKey_Hide, sHide_extended),\
									NOKEY_3

static TabletKeymap::Layout sUsQwerty = {
	{ KEY_2(-0.5, Qt::Key_Q, Qt::Key_BracketLeft), US_QWERTY_NUMBERS_10(1), KEY_1(-0.5, Qt::Key_Backspace) },
	{ US_QWERTY_TOP_10(1), KEY_1(1, Qt::Key_Backspace), NOKEY_1 },
	{ KEY_2(-0.5, Qt::Key_A, Qt::Key_Less), US_QWERTY_MID_9(1), KEY_1(1.5, Qt::Key_Return), NOKEY_1 },
	{ KEY_1(1, Qt::Key_Shift), US_QWERTY_LOW_9(1), KEY_1(1, Qt::Key_Shift), NOKEY_1 },
	{ US_QWERTY_BOTTOM_ROW_DEFAULT },
};

static TabletKeymap::LayoutRow sUsQwertyBottomRow_default = { US_QWERTY_BOTTOM_ROW_DEFAULT };
static TabletKeymap::LayoutRow sUsQwertyBottomRow_url = { US_QWERTY_BOTTOM_ROW_URL };
static TabletKeymap::LayoutRow sUsQwertyBottomRow_email = { US_QWERTY_BOTTOM_ROW_EMAIL };

static TabletKeymap::LayoutFamily sUsQwertyFamily("us qwerty", "en", IME_KBD_LANG_English, IME_KBD_SEC_REGQwerty,
                                                "+ = [  ]" /* Spaces are "Unicode Character 'HAIR SPACE' (U+200A) ' ' " */, "Q w y" /* Spaces are "Unicode Character 'HAIR SPACE' (U+200A) ' ' " */,
                                                0, 1, 10, 2, false, sUsQwerty, sUsQwertyBottomRow_default, sUsQwertyBottomRow_url, sUsQwertyBottomRow_email);
#endif // KEYBOARD_US_QWERTY_H
