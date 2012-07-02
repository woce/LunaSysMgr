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

#ifndef KEYBOARD_DE_QWERTZ_H
#define KEYBOARD_DE_QWERTZ_H
#include "../TabletKeymap.h"
#include "common_keys.h"

static TabletKeymap::constUKeyArray sDeQwertz_DotCom_Extended = { cKey_DotCom, cKey_DotDe, cKey_DotNet, cKey_DotOrg, cKey_DotEdu, cKey_None };
static TabletKeymap::constUKeyArray sDeQwertz1_extended = { Qt::Key_1, Qt::Key_Exclam, UKey(0x00B9) /* SUPERSCRIPT ONE ¹ */, UKey(0x00BC) /* VULGAR FRACTION ONE QUARTER ¼ */, UKey(0x00BD) /* VULGAR FRACTION ONE HALF ½ */, Qt::Key_exclamdown, cKey_None };
static TabletKeymap::constUKeyArray sDeQwertz2_extended = { Qt::Key_2, Qt::Key_QuoteDbl, UKey(0x00B2) /* SUPERSCRIPT TWO ² */, UKey(0x201D) /* RIGHT DOUBLE QUOTATION MARK ” */, UKey(0x201E) /* DOUBLE LOW-9 QUOTATION MARK „ */, UKey(0x201C) /* LEFT DOUBLE QUOTATION MARK “ */, UKey(0x00AB) /* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK « */, UKey(0x00BB) /* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK » */, cKey_None };
static TabletKeymap::constUKeyArray sDeQwertz3_extended = { Qt::Key_3, Qt::Key_At, UKey(0x00B3) /* SUPERSCRIPT THREE ³ */, UKey(0x00BE) /* VULGAR FRACTION THREE QUARTERS ¾ */, cKey_None };
static TabletKeymap::constUKeyArray sDeQwertz4_extended = { Qt::Key_4, Qt::Key_Dollar, cKey_Euro, Qt::Key_sterling, Qt::Key_yen, Qt::Key_cent, UKey(0x00A4) /* CURRENCY SIGN ¤ */, cKey_None };
static TabletKeymap::constUKeyArray sDeQwertz5_extended = { Qt::Key_5, Qt::Key_Percent, UKey(0x2030) /* PER MILLE SIGN ‰ */, cKey_None };
static TabletKeymap::constUKeyArray sDeQwertz6_extended = { Qt::Key_6, Qt::Key_Ampersand, cKey_None };
static TabletKeymap::constUKeyArray sDeQwertz7_extended = { Qt::Key_7, Qt::Key_Slash, Qt::Key_Backslash, cKey_None };
static TabletKeymap::constUKeyArray sDeQwertz8_extended = { Qt::Key_8, Qt::Key_ParenLeft, Qt::Key_BracketLeft, Qt::Key_BraceLeft, cKey_None };
static TabletKeymap::constUKeyArray sDeQwertz9_extended = { Qt::Key_9, Qt::Key_ParenRight, Qt::Key_BracketRight, Qt::Key_BraceRight, cKey_None };
static TabletKeymap::constUKeyArray sDeQwertz0_extended = { Qt::Key_0, Qt::Key_Equal, cKey_None };

static TabletKeymap::constUKeyArray sCommaSemiColon_extended = { Qt::Key_Comma, Qt::Key_Semicolon, cKey_None };
static TabletKeymap::constUKeyArray sPeriodColon_extended = { Qt::Key_Period, Qt::Key_Colon, UKey(0x2022) /* BULLET • */, UKey(0x2026) /* HORIZONTAL ELLIPSIS … */, cKey_None };
static TabletKeymap::constUKeyArray sSSharpQuestion_extended = { Qt::Key_ssharp, Qt::Key_Question, Qt::Key_questiondown, cKey_None };
static TabletKeymap::constUKeyArray sMinusApostrophe_extended = { Qt::Key_Minus, Qt::Key_Apostrophe, UKey(0x00B1) /* PLUS-MINUS SIGN ± */, UKey(0x00AC) /* NOT SIGN ¬ */, UKey(0x0060) /* GRAVE ACCENT ` */, UKey(0x201A) /* SINGLE LOW-9 QUOTATION MARK ‚ */, UKey(0x2018) /* LEFT SINGLE QUOTATION MARK ‘ */, UKey(0x2019) /* RIGHT SINGLE QUOTATION MARK ’ */, cKey_None };

#define DE_QWERTZ_NUMBERS_10(w)			{ w, Qt::Key_1,			Qt::Key_Exclam,							sDeQwertz1_extended },\
										{ w, Qt::Key_2,			Qt::Key_QuoteDbl,						sDeQwertz2_extended },\
										{ w, Qt::Key_3,			Qt::Key_At,								sDeQwertz3_extended },\
										{ w, Qt::Key_4,			Qt::Key_Dollar,							sDeQwertz4_extended },\
										{ w, Qt::Key_5,			Qt::Key_Percent,						sDeQwertz5_extended },\
										{ w, Qt::Key_6,	 		Qt::Key_Ampersand,						sDeQwertz6_extended },\
										{ w, Qt::Key_7,			Qt::Key_Slash,							sDeQwertz7_extended },\
										{ w, Qt::Key_8,			Qt::Key_ParenLeft,						sDeQwertz8_extended },\
										{ w, Qt::Key_9,			Qt::Key_ParenRight,						sDeQwertz9_extended },\
										{ w, Qt::Key_0,			Qt::Key_Equal,							sDeQwertz0_extended }

#define DE_QWERTZ_TOP_10(w)				{ w, Qt::Key_Q,			Qt::Key_QuoteLeft,						NULL },\
										{ w, Qt::Key_W,			Qt::Key_AsciiTilde,						NULL },\
										{ w, Qt::Key_E,			cKey_Euro,								sE_extended },\
										{ w, Qt::Key_R,			Qt::Key_AsciiCircum,					sR_extended },\
										{ w, Qt::Key_T,			Qt::Key_Backslash,						sT_extended },\
										{ w, Qt::Key_Z,	 		Qt::Key_Bar,							sZ_extended },\
										{ w, Qt::Key_U,			Qt::Key_BraceLeft,						sU_extended },\
										{ w, Qt::Key_I,			Qt::Key_BraceRight,						sI_extended },\
										{ w, Qt::Key_O,			Qt::Key_BracketLeft,					sO_extended },\
										{ w, Qt::Key_P,			Qt::Key_BracketRight,					sP_extended }

#define DE_QWERTZ_MID_9(w)				{ w, Qt::Key_A,			Qt::Key_Less,							sA_extended },\
										{ w, Qt::Key_S,			Qt::Key_Greater,						sS_extended },\
										{ w, Qt::Key_D,			Qt::Key_Underscore,						sD_extended },\
										{ w, Qt::Key_F,			Qt::Key_Plus,							NULL },\
										{ w, Qt::Key_G,			UKey(0x00D7) /* multiplication sign */,	sG_extended },\
										{ w, Qt::Key_H,			UKey(0x00F7) /* division sign */,		NULL },\
										{ w, Qt::Key_J,			Qt::Key_degree,							NULL },\
										{ w, Qt::Key_K,			Qt::Key_Asterisk,						NULL },\
										{ w, Qt::Key_L,			Qt::Key_NumberSign,						sL_extended }

#define DE_QWERTZ_LOW_9(w)				{ w, Qt::Key_Y,			cKey_Emoticon_Smile,					sY_extended },\
										{ w, Qt::Key_X,			cKey_Emoticon_Wink,						sOptions },\
										{ w, Qt::Key_C,			cKey_Emoticon_Frown,					sC_extended },\
										{ w, Qt::Key_V,			cKey_Emoticon_Cry,						NULL },\
										{ w, Qt::Key_B,			cKey_Emoticon_Yuck,						sToggleLanguage_extended },\
										{ w, Qt::Key_N,			cKey_Emoticon_Gasp,						sN_extended },\
										{ w, Qt::Key_M,			cKey_Emoticon_Heart,					sM_extended },\
										{ w, Qt::Key_Comma, 	Qt::Key_Semicolon,						sCommaSemiColon_extended },\
										{ w, Qt::Key_Period,	Qt::Key_Colon,							sPeriodColon_extended }

#define DE_QWERTZ_BOTTOM_ROW_DEFAULT \
									KEY_1(1, Qt::Key_Tab),\
									KEY_1(2, cKey_Symbol),\
									NOKEY_1,\
									NOKEY_1,\
									KEY_1(SPACE_SIZE, Qt::Key_Space),\
									NOKEY_1,\
									KEY_3(1, Qt::Key_ssharp, Qt::Key_Question, sSSharpQuestion_extended),\
									KEY_3(1, Qt::Key_Minus, Qt::Key_Apostrophe, sMinusApostrophe_extended),\
									KEY_3(1, cKey_Hide, cKey_Hide, sHide_extended),\
									NOKEY_3

#define DE_QWERTZ_BOTTOM_ROW_URL \
									KEY_1(1, Qt::Key_Tab),\
									KEY_1(2, cKey_Symbol),\
									NOKEY_1,\
									KEY_1(1, Qt::Key_Slash),\
									KEY_1(SPACE_SIZE - 2, Qt::Key_Space),\
									KEY_3(1, cKey_DotCom, cKey_DotCom, sDeQwertz_DotCom_Extended),\
									KEY_3(1, Qt::Key_ssharp, Qt::Key_Question, sSSharpQuestion_extended),\
									KEY_3(1, Qt::Key_Minus, Qt::Key_Apostrophe, sMinusApostrophe_extended),\
									KEY_3(1, cKey_Hide, cKey_Hide, sHide_extended),\
									NOKEY_3

#define DE_QWERTZ_BOTTOM_ROW_EMAIL \
									KEY_1(1, Qt::Key_Tab),\
									KEY_1(2, cKey_Symbol),\
									NOKEY_1,\
									KEY_1(1, Qt::Key_At),\
									KEY_1(SPACE_SIZE - 2, Qt::Key_Space),\
									KEY_3(1, cKey_DotCom, cKey_DotCom, sDeQwertz_DotCom_Extended),\
									KEY_3(1, Qt::Key_ssharp, Qt::Key_Question, sSSharpQuestion_extended),\
									KEY_3(1, Qt::Key_Minus, Qt::Key_Apostrophe, sMinusApostrophe_extended),\
									KEY_3(1, cKey_Hide, cKey_Hide, sHide_extended),\
									NOKEY_3

static TabletKeymap::Layout sDeQwertz = {
	{ KEY_2(-0.5, Qt::Key_Q, Qt::Key_BracketLeft), DE_QWERTZ_NUMBERS_10(1), KEY_1(-0.5, Qt::Key_Backspace) },
	{ DE_QWERTZ_TOP_10(1), KEY_1(1, Qt::Key_Backspace), NOKEY_1 },
	{ KEY_2(-0.5, Qt::Key_A, Qt::Key_Less), DE_QWERTZ_MID_9(1), KEY_1(1.5, Qt::Key_Return), NOKEY_1 },
	{ KEY_1(1, Qt::Key_Shift), DE_QWERTZ_LOW_9(1), KEY_1(1, Qt::Key_Shift), NOKEY_1 },
	{ DE_QWERTZ_BOTTOM_ROW_DEFAULT },
};

static TabletKeymap::LayoutRow sDeQwertzBottomRow_default = { DE_QWERTZ_BOTTOM_ROW_DEFAULT };
static TabletKeymap::LayoutRow sDeQwertzBottomRow_url = { DE_QWERTZ_BOTTOM_ROW_URL };
static TabletKeymap::LayoutRow sDeQwertzBottomRow_email = { DE_QWERTZ_BOTTOM_ROW_EMAIL };

static TabletKeymap::LayoutFamily sDeQwertzFamily("de qwertz", "de", IME_KBD_LANG_German, IME_KBD_SEC_REGQwerty,
                                                "+ ~ [  ]" /* Spaces are "Unicode Character 'HAIR SPACE' (U+200A) ' ' " */, "Q w z" /* Spaces are "Unicode Character 'HAIR SPACE' (U+200A) ' ' " */,
                                                0, 1, 10, 2, false, sDeQwertz, sDeQwertzBottomRow_default, sDeQwertzBottomRow_url, sDeQwertzBottomRow_email);
#endif // KEYBOARD_DE_QWERTZ_H
