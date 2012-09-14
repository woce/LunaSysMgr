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

// this file was modified by "pcworld", 0188801@gmail.com

#ifndef KEYBOARD_SE_H
#define KEYBOARD_SE_H
#include "../TabletKeymap.h"
#include "common_keys.h"
static TabletKeymap::LayoutFamily sLayoutSwedish("Swedish", "se");
static TabletKeymap::constUKeyArray sSwedish_DotCom_Extended = { cKey_DotCom, cKey_DotSe, cKey_DotNet, cKey_DotOrg, cKey_DotEdu, cKey_None };

// DVORAK
static TabletKeymap::constUKeyArray sSeDvorak1_extended = { Qt::Key_1, Qt::Key_Exclam, UKey(0x00B9) /* SUPERSCRIPT ONE ¹ */, UKey(0x00BC) /* VULGAR FRACTION ONE QUARTER ¼ */, UKey(0x00BD) /* VULGAR FRACTION ONE HALF ½ */, Qt::Key_exclamdown, cKey_None };
static TabletKeymap::constUKeyArray sSeDvorak2_extended = { Qt::Key_2, Qt::Key_At, UKey(0x00B2) /* SUPERSCRIPT TWO ² */, cKey_None };
static TabletKeymap::constUKeyArray sSeDvorak3_extended = { Qt::Key_3, Qt::Key_NumberSign, UKey(0x00B3) /* SUPERSCRIPT THREE ³ */, UKey(0x00BE) /* VULGAR FRACTION THREE QUARTERS ¾ */, cKey_None };
static TabletKeymap::constUKeyArray sSeDvorak4_extended = { Qt::Key_4, Qt::Key_Dollar, cKey_Euro, Qt::Key_sterling, Qt::Key_yen, Qt::Key_cent, UKey(0x00A4) /* CURRENCY SIGN ¤ */, cKey_None };
static TabletKeymap::constUKeyArray sSeDvorak5_extended = { Qt::Key_5, Qt::Key_Percent, UKey(0x2030) /* PER MILLE SIGN ‰ */, cKey_None };
static TabletKeymap::constUKeyArray sSeDvorak6_extended = { Qt::Key_6, Qt::Key_AsciiCircum, cKey_None };
static TabletKeymap::constUKeyArray sSeDvorak7_extended = { Qt::Key_7, Qt::Key_Ampersand, cKey_None };
static TabletKeymap::constUKeyArray sSeDvorak8_extended = { Qt::Key_8, Qt::Key_Asterisk, cKey_None };
static TabletKeymap::constUKeyArray sSeDvorak9_extended = { Qt::Key_9, Qt::Key_ParenLeft, Qt::Key_BracketLeft, Qt::Key_BraceLeft, cKey_None };
static TabletKeymap::constUKeyArray sSeDvorak0_extended = { Qt::Key_0, Qt::Key_ParenRight, Qt::Key_BracketRight, Qt::Key_BraceRight, cKey_None };

#define SE_DVORAK_NUMBERS_10(w)			{ w, Qt::Key_1,			Qt::Key_Exclam,							sSeDvorak1_extended },\
										{ w, Qt::Key_2,			Qt::Key_At,								sSeDvorak2_extended },\
										{ w, Qt::Key_3,			Qt::Key_NumberSign,						sSeDvorak3_extended },\
										{ w, Qt::Key_4,			Qt::Key_Dollar,							sSeDvorak4_extended },\
										{ w, Qt::Key_5,			Qt::Key_Percent,						sSeDvorak5_extended },\
										{ w, Qt::Key_6,	 		Qt::Key_AsciiCircum,					sSeDvorak6_extended },\
										{ w, Qt::Key_7,			Qt::Key_Ampersand,						sSeDvorak7_extended },\
										{ w, Qt::Key_8,			Qt::Key_Asterisk,						sSeDvorak8_extended },\
										{ w, Qt::Key_9,			Qt::Key_ParenLeft,						sSeDvorak9_extended },\
										{ w, Qt::Key_0,			Qt::Key_ParenRight,						sSeDvorak0_extended }

#define SE_DVORAK_TOP_10(w)				{ w, Qt::Key_Aring,		Qt::Key_section,						NULL },\
										{ w, Qt::Key_Comma,		Qt::Key_Slash,							sCommaSlash_extended },\
										{ w, Qt::Key_Period,	Qt::Key_Question,						sPeriodQuestion_extended },\
										{ w, Qt::Key_P,			Qt::Key_QuoteLeft,						sP_extended },\
										{ w, Qt::Key_Y,	 		Qt::Key_AsciiTilde,						sY_extended },\
										{ w, Qt::Key_F,			Qt::Key_Backslash,						NULL },\
										{ w, Qt::Key_G,			Qt::Key_Bar,							sG_extended },\
										{ w, Qt::Key_C,			Qt::Key_degree,							sC_extended },\
										{ w, Qt::Key_R,			Qt::Key_Colon,							sR_extended },\
										{ w, Qt::Key_L,			Qt::Key_Semicolon,						sL_extended }
										

#define SE_DVORAK_MID_10(w)				{ w, Qt::Key_A,			Qt::Key_Less,							sA_extended },\
										{ w, Qt::Key_O,			Qt::Key_Greater,						sO_extended },\
										{ w, Qt::Key_E,			cKey_Euro,								sE_extended },\
										{ w, Qt::Key_U,			Qt::Key_sterling,						sU_extended },\
										{ w, Qt::Key_I,			Qt::Key_BraceLeft,						sI_extended },\
										{ w, Qt::Key_D,			Qt::Key_BraceRight,						sD_extended },\
										{ w, Qt::Key_H,			Qt::Key_BracketLeft,					NULL },\
										{ w, Qt::Key_T,			Qt::Key_BracketRight,					sT_extended },\
										{ w, Qt::Key_N,			UKey(0x00D7) /* multiplication sign */,	sN_extended },\
										{ w, Qt::Key_S,			UKey(0x00F7) /* division sign */,		sS_extended }
										

#define SE_DVORAK_LOW_10(w)				{ w, Qt::Key_Odiaeresis,cKey_Emoticon_Smile,					NULL },\
										{ w, Qt::Key_Adiaeresis,cKey_Emoticon_Wink,						NULL },\
										{ w, Qt::Key_Q,			cKey_Emoticon_Frown,					NULL },\
										{ w, Qt::Key_J,			cKey_Emoticon_Cry,						NULL },\
										{ w, Qt::Key_K,			cKey_Emoticon_Yuck,						NULL },\
										{ w, Qt::Key_X,			cKey_Emoticon_Gasp,						sOptions },\
										{ w, Qt::Key_B,			cKey_Emoticon_Heart,					sToggleLanguage_extended },\
										{ w, Qt::Key_M,			Qt::Key_Plus,							sM_extended },\
										{ w, Qt::Key_W,			Qt::Key_Equal,							NULL },\
										{ w, Qt::Key_V,			Qt::Key_notsign,						NULL }

#define SE_DVORAK_BOTTOM_ROW_DEFAULT \
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

#define SE_DVORAK_BOTTOM_ROW_URL \
									KEY_1(1, Qt::Key_Tab),\
									KEY_1(2, cKey_Symbol),\
									NOKEY_1,\
									KEY_3(1, Qt::Key_Slash, Qt::Key_Slash, sURL_extended),\
									KEY_1(SPACE_SIZE - 2, Qt::Key_Space),\
									KEY_3(1, cKey_DotCom, cKey_DotCom, sSwedish_DotCom_Extended),\
									KEY_3(1, Qt::Key_Apostrophe, Qt::Key_QuoteDbl, sSingleAndDoubleQuote_extended),\
									KEY_3(1, Qt::Key_Minus, Qt::Key_Underscore, sMinusUnderscore_extended),\
									KEY_3(1, cKey_Hide, cKey_Hide, sHide_extended),\
									NOKEY_3

#define SE_DVORAK_BOTTOM_ROW_EMAIL \
									KEY_1(1, Qt::Key_Tab),\
									KEY_1(2, cKey_Symbol),\
									NOKEY_1,\
									KEY_1(1, Qt::Key_At),\
									KEY_1(SPACE_SIZE - 2, Qt::Key_Space),\
									KEY_3(1, cKey_DotCom, cKey_DotCom, sSwedish_DotCom_Extended),\
									KEY_3(1, Qt::Key_Apostrophe, Qt::Key_QuoteDbl, sSingleAndDoubleQuote_extended),\
									KEY_3(1, Qt::Key_Minus, Qt::Key_Underscore, sMinusUnderscore_extended),\
									KEY_3(1, cKey_Hide, cKey_Hide, sHide_extended),\
									NOKEY_3

static TabletKeymap::Layout sSeDvorakLayout = {
	{ SE_DVORAK_NUMBERS_10(1), KEY_1(2, cKey_Trackball) },
	{ SE_DVORAK_TOP_10(1), KEY_1(1, Qt::Key_Backspace) },
	{ KEY_2(-0.5, Qt::Key_A, Qt::Key_Less), SE_DVORAK_MID_10(1), KEY_1(1.5, Qt::Key_Return) },
	{ KEY_1(1, Qt::Key_Shift), SE_DVORAK_LOW_10(1), KEY_1(2.3, Qt::Key_Shift) },
	{ SE_DVORAK_BOTTOM_ROW_DEFAULT },
};

static TabletKeymap::LayoutRow sSeDvorakBottomRow_default = { SE_DVORAK_BOTTOM_ROW_DEFAULT };
static TabletKeymap::LayoutRow sSeDvorakBottomRow_url = { SE_DVORAK_BOTTOM_ROW_URL };
static TabletKeymap::LayoutRow sSeDvorakBottomRow_email = { SE_DVORAK_BOTTOM_ROW_EMAIL };

static TabletKeymap::Keymap sSeDvorak(&sLayoutSwedish, "Dvorak", IME_KBD_LANG_English, IME_KBD_SEC_REGQwerty,
                                                "+ = [  ]" /* Spaces are "Unicode Character 'HAIR SPACE' (U+200A) ' ' " */, "S E D v k" /* Spaces are "Unicode Character 'HAIR SPACE' (U+200A) ' ' " */,
                                                0, 1, 12, 2, false, sSeDvorakLayout, sSeDvorakBottomRow_default, sSeDvorakBottomRow_url, sSeDvorakBottomRow_email);

// QWERTY
static TabletKeymap::constUKeyArray sSeQwerty1_extended = { Qt::Key_1, Qt::Key_Exclam, UKey(0x00B9) /* SUPERSCRIPT ONE ¹ */, UKey(0x00BC) /* VULGAR FRACTION ONE QUARTER ¼ */, UKey(0x00BD) /* VULGAR FRACTION ONE HALF ½ */, Qt::Key_exclamdown, cKey_None };
static TabletKeymap::constUKeyArray sSeQwerty2_extended = { Qt::Key_2, Qt::Key_At, UKey(0x00B2) /* SUPERSCRIPT TWO ² */, cKey_None };
static TabletKeymap::constUKeyArray sSeQwerty3_extended = { Qt::Key_3, Qt::Key_NumberSign, UKey(0x00B3) /* SUPERSCRIPT THREE ³ */, UKey(0x00BE) /* VULGAR FRACTION THREE QUARTERS ¾ */, cKey_None };
static TabletKeymap::constUKeyArray sSeQwerty4_extended = { Qt::Key_4, UKey(0x00A4) /* CURRENCY SIGN ¤ */, cKey_Euro, Qt::Key_sterling, Qt::Key_yen, Qt::Key_cent, Qt::Key_Dollar, cKey_None };
static TabletKeymap::constUKeyArray sSeQwerty5_extended = { Qt::Key_5, Qt::Key_Percent, UKey(0x2030) /* PER MILLE SIGN ‰ */, cKey_None };
static TabletKeymap::constUKeyArray sSeQwerty6_extended = { Qt::Key_6, Qt::Key_Ampersand, cKey_None };
static TabletKeymap::constUKeyArray sSeQwerty7_extended = { Qt::Key_7, Qt::Key_Slash, cKey_None };
static TabletKeymap::constUKeyArray sSeQwerty8_extended = { Qt::Key_8, Qt::Key_ParenLeft, cKey_None };
static TabletKeymap::constUKeyArray sSeQwerty9_extended = { Qt::Key_9, Qt::Key_ParenRight, Qt::Key_BracketLeft, Qt::Key_BraceLeft, cKey_None };
static TabletKeymap::constUKeyArray sSeQwerty0_extended = { Qt::Key_0, Qt::Key_Question, Qt::Key_BracketRight, Qt::Key_BraceRight, cKey_None };

#define SE_QWERTY_NUMBERS_10(w)			{ w, Qt::Key_1,			Qt::Key_Exclam,							sSeQwerty1_extended },\
										{ w, Qt::Key_2,			Qt::Key_At,								sSeQwerty2_extended },\
										{ w, Qt::Key_3,			Qt::Key_NumberSign,						sSeQwerty3_extended },\
										{ w, Qt::Key_4,			UKey(0x00A4) /* CURRENCY SIGN ¤ */,		sSeQwerty4_extended },\
										{ w, Qt::Key_5,			Qt::Key_Percent,						sSeQwerty5_extended },\
										{ w, Qt::Key_6,	 		Qt::Key_Ampersand,						sSeQwerty6_extended },\
										{ w, Qt::Key_7,			Qt::Key_Slash,							sSeQwerty7_extended },\
										{ w, Qt::Key_8,			Qt::Key_ParenLeft,						sSeQwerty8_extended },\
										{ w, Qt::Key_9,			Qt::Key_ParenRight,						sSeQwerty9_extended },\
										{ w, Qt::Key_0,			Qt::Key_Question,						sSeQwerty0_extended },\
										{ w, Qt::Key_Left,		Qt::Key_Up,							    NULL },\
										{ w, Qt::Key_Right,		Qt::Key_Down,							NULL }

#define SE_QWERTY_TOP_11(w)				{ w, Qt::Key_Q,			Qt::Key_QuoteLeft,						NULL },\
										{ w, Qt::Key_W,			Qt::Key_AsciiTilde,						NULL },\
										{ w, Qt::Key_E,			cKey_Euro,								sE_extended },\
										{ w, Qt::Key_R,			Qt::Key_sterling,						sR_extended },\
										{ w, Qt::Key_T,			Qt::Key_Dollar,							sT_extended },\
										{ w, Qt::Key_Y,	 		Qt::Key_yen,							sY_extended },\
										{ w, Qt::Key_U,			Qt::Key_Bar,							sU_extended },\
										{ w, Qt::Key_I,			Qt::Key_BraceLeft,						sI_extended },\
										{ w, Qt::Key_O,			Qt::Key_BraceRight,						sO_extended },\
										{ w, Qt::Key_P,			Qt::Key_BracketLeft,					sP_extended },\
										{ w, Qt::Key_Aring,		Qt::Key_BracketRight,					NULL }

#define SE_QWERTY_MID_11(w)				{ w, Qt::Key_A,			Qt::Key_Less,							sA_extended },\
										{ w, Qt::Key_S,			Qt::Key_Greater,						sS_extended },\
										{ w, Qt::Key_D,			Qt::Key_Equal,							sD_extended },\
										{ w, Qt::Key_F,			Qt::Key_Plus,							NULL },\
										{ w, Qt::Key_G,			UKey(0x00D7) /* multiplication sign */,	sG_extended },\
										{ w, Qt::Key_H,			UKey(0x00F7) /* division sign */,		NULL },\
										{ w, Qt::Key_J,			Qt::Key_degree,							NULL },\
										{ w, Qt::Key_K,			Qt::Key_Asterisk,						NULL },\
										{ w, Qt::Key_L,			Qt::Key_AsciiCircum,					sL_extended },\
										{ w, Qt::Key_Odiaeresis,Qt::Key_section,						NULL },\
										{ w, Qt::Key_Adiaeresis,Qt::Key_Backslash,						NULL }

#define SE_QWERTY_LOW_9(w)				{ w, Qt::Key_Z,			cKey_Emoticon_Smile,					sZ_extended },\
										{ w, Qt::Key_X,			cKey_Emoticon_Wink,						sOptions },\
										{ w, Qt::Key_C,			cKey_Emoticon_Frown,					sC_extended },\
										{ w, Qt::Key_V,			cKey_Emoticon_Cry,						NULL },\
										{ w, Qt::Key_B,			cKey_Emoticon_Yuck,						sToggleLanguage_extended },\
										{ w, Qt::Key_N,			cKey_Emoticon_Gasp,						sN_extended },\
										{ w, Qt::Key_M,			cKey_Emoticon_Heart,					sM_extended },\
										{ w, Qt::Key_Comma,		Qt::Key_Semicolon,						sCommaSlash_extended },\
										{ w, Qt::Key_Period,	Qt::Key_Colon,							sPeriodQuestion_extended }

#define SE_QWERTY_BOTTOM_ROW_DEFAULT \
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

#define SE_QWERTY_BOTTOM_ROW_URL \
									KEY_1(1, Qt::Key_Tab),\
									KEY_1(2, cKey_Symbol),\
									NOKEY_1,\
									KEY_3(1, Qt::Key_Slash, Qt::Key_Slash, sURL_extended),\
									KEY_1(SPACE_SIZE - 2, Qt::Key_Space),\
									KEY_3(1, cKey_DotCom, cKey_DotCom, sSwedish_DotCom_Extended),\
									KEY_3(1, Qt::Key_Apostrophe, Qt::Key_QuoteDbl, sSingleAndDoubleQuote_extended),\
									KEY_3(1, Qt::Key_Minus, Qt::Key_Underscore, sMinusUnderscore_extended),\
									KEY_3(1, cKey_Hide, cKey_Hide, sHide_extended),\
									NOKEY_3

#define SE_QWERTY_BOTTOM_ROW_EMAIL \
									KEY_1(1, Qt::Key_Tab),\
									KEY_1(2, cKey_Symbol),\
									NOKEY_1,\
									KEY_1(1, Qt::Key_At),\
									KEY_1(SPACE_SIZE - 2, Qt::Key_Space),\
									KEY_3(1, cKey_DotCom, cKey_DotCom, sSwedish_DotCom_Extended),\
									KEY_3(1, Qt::Key_Apostrophe, Qt::Key_QuoteDbl, sSingleAndDoubleQuote_extended),\
									KEY_3(1, Qt::Key_Minus, Qt::Key_Underscore, sMinusUnderscore_extended),\
									KEY_3(1, cKey_Hide, cKey_Hide, sHide_extended),\

static TabletKeymap::Layout sSeQwertyLayout = {
	{ SE_QWERTY_NUMBERS_10(1) },
	{ SE_QWERTY_TOP_11(1), KEY_1(2, Qt::Key_Backspace), NOKEY_1 },
	{ KEY_2(-0.2, Qt::Key_A, Qt::Key_Less), SE_QWERTY_MID_11(1), KEY_1(1, Qt::Key_Return), NOKEY_1 },
	{ KEY_1(0.8, Qt::Key_Shift), SE_QWERTY_LOW_9(1), KEY_1(2, Qt::Key_Shift), NOKEY_1 },
	{ SE_QWERTY_BOTTOM_ROW_DEFAULT },
};

static TabletKeymap::LayoutRow sSeQwertyBottomRow_default = { SE_QWERTY_BOTTOM_ROW_DEFAULT };
static TabletKeymap::LayoutRow sSeQwertyBottomRow_url = { SE_QWERTY_BOTTOM_ROW_URL };
static TabletKeymap::LayoutRow sSeQwertyBottomRow_email = { SE_QWERTY_BOTTOM_ROW_EMAIL };

static TabletKeymap::Keymap sSeQwerty(&sLayoutSwedish, "Qwerty", IME_KBD_LANG_English, IME_KBD_SEC_REGQwerty,
                                                "+ = [  ]" /* Spaces are "Unicode Character 'HAIR SPACE' (U+200A) ' ' " */, "S E Q w y" /* Spaces are "Unicode Character 'HAIR SPACE' (U+200A) ' ' " */,
                                                0, 1, 11, 2, false, sSeQwertyLayout, sSeQwertyBottomRow_default, sSeQwertyBottomRow_url, sSeQwertyBottomRow_email);
#endif // KEYBOARD_SE_H
