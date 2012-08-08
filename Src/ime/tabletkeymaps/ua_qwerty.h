/* @@@LICENSE
*
*      Copyright (c) 2010-2012 Hewlett-Packard Development Company, L.P.
*      Copyright (c) 2012 Alexander Kerner <alexander.kerner@gmail.com>
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

#ifndef KEYBOARD_UA_QWERTY_H
#define KEYBOARD_UA_QWERTY_H
#include "../TabletKeymap.h"
#include "common_keys.h"

static TabletKeymap::constUKeyArray sUaQwerty_DotCom_Extended = { cKey_DotCom, cKey_DotNet, cKey_DotOrg, cKey_DotEdu, cKey_None };
static TabletKeymap::constUKeyArray sUaQwerty1_extended = { Qt::Key_1, Qt::Key_Exclam, UKey(0x00B9) /* SUPERSCRIPT ONE ¹ */, UKey(0x00BC) /* VULGAR FRACTION ONE QUARTER ¼ */, UKey(0x00BD) /* VULGAR FRACTION ONE HALF ½ */, Qt::Key_exclamdown, cKey_None };
static TabletKeymap::constUKeyArray sUaQwerty2_extended = { Qt::Key_2, Qt::Key_At, UKey(0x00B2) /* SUPERSCRIPT TWO ² */, cKey_None };
static TabletKeymap::constUKeyArray sUaQwerty3_extended = { Qt::Key_3, Qt::Key_NumberSign, UKey(0x00B3) /* SUPERSCRIPT THREE ³ */, UKey(0x00BE) /* VULGAR FRACTION THREE QUARTERS ¾ */, cKey_None };
static TabletKeymap::constUKeyArray sUaQwerty4_extended = { Qt::Key_4, Qt::Key_Dollar, cKey_Euro, Qt::Key_sterling, Qt::Key_yen, Qt::Key_cent, UKey(0x00A4) /* CURRENCY SIGN ¤ */, cKey_None };
static TabletKeymap::constUKeyArray sUaQwerty5_extended = { Qt::Key_5, Qt::Key_Percent, UKey(0x2030) /* PER MILLE SIGN ‰ */, cKey_None };
static TabletKeymap::constUKeyArray sUaQwerty6_extended = { Qt::Key_6, Qt::Key_AsciiCircum, cKey_None };
static TabletKeymap::constUKeyArray sUaQwerty7_extended = { Qt::Key_7, Qt::Key_Ampersand, cKey_None };
static TabletKeymap::constUKeyArray sUaQwerty8_extended = { Qt::Key_8, Qt::Key_Asterisk, cKey_None };
static TabletKeymap::constUKeyArray sUaQwerty9_extended = { Qt::Key_9, Qt::Key_ParenLeft, Qt::Key_BracketLeft, Qt::Key_BraceLeft, cKey_None };
static TabletKeymap::constUKeyArray sUaQwerty0_extended = { Qt::Key_0, Qt::Key_ParenRight, Qt::Key_BracketRight, Qt::Key_BraceRight, cKey_None };
static TabletKeymap::constUKeyArray uaPeriod_extended = { Qt::Key_Period, Qt::Key_Comma, Qt::Key_Slash, Qt::Key_Backslash, cKey_None };
static TabletKeymap::constUKeyArray uaE_extended = { UKey(0x415), UKey(0x401), cKey_None};
static TabletKeymap::constUKeyArray uab_extended = { UKey(0x42c), UKey(0x42a), cKey_None};
static TabletKeymap::constUKeyArray ua_extended_1 = { UKey(0x419), UKey(0x407), cKey_None };
static TabletKeymap::constUKeyArray ua_extended_2 = { UKey(0x413), UKey(0x490), cKey_None };
static TabletKeymap::constUKeyArray ua_extended_3 = { UKey(0x42b), UKey(0x406), cKey_None };
static TabletKeymap::constUKeyArray ua_extended_4 = { UKey(0x42d), UKey(0x404), cKey_None };

#define UA_QWERTY_NUMBERS(w)			{ w, Qt::Key_1,			Qt::Key_Exclam,							sUaQwerty1_extended },\
										{ w, Qt::Key_2,			Qt::Key_At,								sUaQwerty2_extended },\
										{ w, Qt::Key_3,			Qt::Key_NumberSign,						sUaQwerty3_extended },\
										{ w, Qt::Key_4,			Qt::Key_Dollar,							sUaQwerty4_extended },\
										{ w, Qt::Key_5,			Qt::Key_Percent,						sUaQwerty5_extended },\
										{ w, Qt::Key_6,	 		Qt::Key_AsciiCircum,					sUaQwerty6_extended },\
										{ w, Qt::Key_7,			Qt::Key_Ampersand,						sUaQwerty7_extended },\
										{ w, Qt::Key_8,			Qt::Key_Asterisk,						sUaQwerty8_extended },\
										{ w, Qt::Key_9,			Qt::Key_ParenLeft,						sUaQwerty9_extended },\
										{ w, Qt::Key_0,			Qt::Key_ParenRight,						sUaQwerty0_extended },\
										{ w, Qt::Key_Left,		Qt::Key_Up,							    NULL },\
										{ w, Qt::Key_Right,		Qt::Key_Down,							NULL }

#define UA_QWERTY_TOP(w)				{ w, UKey(0x419),       Qt::Key_QuoteLeft,						ua_extended_1},\
										{ w, UKey(0x426),       Qt::Key_AsciiTilde,						NULL },\
										{ w, UKey(0x423),       cKey_Euro,								NULL },\
										{ w, UKey(0x41a),		Qt::Key_sterling,						NULL },\
										{ w, UKey(0x415),		Qt::Key_Backslash,						uaE_extended },\
										{ w, UKey(0x41d), 		Qt::Key_Bar,							NULL },\
										{ w, UKey(0x413),		Qt::Key_BraceLeft,						ua_extended_2 },\
										{ w, UKey(0x428),		Qt::Key_BraceRight,						NULL },\
										{ w, UKey(0x429),		Qt::Key_BracketLeft,					NULL },\
										{ w, UKey(0x417),		Qt::Key_BracketRight,					NULL },\
										{ w, UKey(0x425),		Qt::Key_BracketRight,					NULL }

#define UA_QWERTY_MID(w)				{ w, UKey(0x424),		Qt::Key_Less,							NULL },\
										{ w, UKey(0x42b),		Qt::Key_Greater,						ua_extended_3 },\
										{ w, UKey(0x412),		Qt::Key_Equal,							NULL },\
										{ w, UKey(0x410),		Qt::Key_Plus,							NULL },\
										{ w, UKey(0x41f),		UKey(0x00D7) /* multiplication sign */,	NULL },\
										{ w, UKey(0x420),		UKey(0x00F7) /* division sign */,		NULL },\
										{ w, UKey(0x41e),		Qt::Key_degree,							NULL },\
										{ w, UKey(0x41b),		Qt::Key_Semicolon,						NULL },\
										{ w, UKey(0x414),		Qt::Key_Colon,							NULL },\
										{ w, UKey(0x416),		Qt::Key_Colon,							NULL },\
										{ w, UKey(0x42d),		Qt::Key_Colon,							ua_extended_4 }

#define UA_QWERTY_LOW(w)				{ w, UKey(0x42f),		cKey_Emoticon_Smile,					NULL },\
										{ w, UKey(0x427),		cKey_Emoticon_Wink,						NULL },\
										{ w, UKey(0x421),		cKey_Emoticon_Frown,					NULL },\
										{ w, UKey(0x41c),		cKey_Emoticon_Cry,						NULL },\
										{ w, UKey(0x418),		cKey_Emoticon_Yuck,						NULL },\
										{ w, UKey(0x422),		cKey_Emoticon_Gasp,						NULL },\
										{ w, UKey(0x42c),		cKey_Emoticon_Heart,					uab_extended },\
										{ w, UKey(0x411),		Qt::Key_Slash,      					NULL },\
										{ w, UKey(0x42e),		Qt::Key_Question,   					NULL },\
										{ w, Qt::Key_Period,	Qt::Key_Comma,	    					uaPeriod_extended }

#define UA_QWERTY_BOTTOM_ROW_DEFAULT \
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

#define UA_QWERTY_BOTTOM_ROW_URL \
									KEY_1(1, Qt::Key_Tab),\
									KEY_1(2, cKey_Symbol),\
									NOKEY_1,\
									KEY_3(1, Qt::Key_Slash, Qt::Key_Slash, sURL_extended),\
									KEY_1(SPACE_SIZE - 2, Qt::Key_Space),\
									KEY_3(1, cKey_DotCom, cKey_DotCom, sUaQwerty_DotCom_Extended),\
									KEY_3(1, Qt::Key_Apostrophe, Qt::Key_QuoteDbl, sSingleAndDoubleQuote_extended),\
									KEY_3(1, Qt::Key_Minus, Qt::Key_Underscore, sMinusUnderscore_extended),\
									KEY_3(1, cKey_Hide, cKey_Hide, sHide_extended),\
									NOKEY_3

#define UA_QWERTY_BOTTOM_ROW_EMAIL \
									KEY_1(1, Qt::Key_Tab),\
									KEY_1(2, cKey_Symbol),\
									NOKEY_1,\
									KEY_1(1, Qt::Key_At),\
									KEY_1(SPACE_SIZE - 2, Qt::Key_Space),\
									KEY_3(1, cKey_DotCom, cKey_DotCom, sUaQwerty_DotCom_Extended),\
									KEY_3(1, Qt::Key_Apostrophe, Qt::Key_QuoteDbl, sSingleAndDoubleQuote_extended),\
									KEY_3(1, Qt::Key_Minus, Qt::Key_Underscore, sMinusUnderscore_extended),\
									KEY_3(1, cKey_Hide, cKey_Hide, sHide_extended),\
									NOKEY_3

static TabletKeymap::Layout sUaQwerty = {
	{ UA_QWERTY_NUMBERS(1) },
	{ UA_QWERTY_TOP(1), KEY_1(1.5, Qt::Key_Backspace), NOKEY_1 },
	{ UA_QWERTY_MID(1), KEY_1(1.5, Qt::Key_Return), NOKEY_1 },
	{ KEY_1(1, Qt::Key_Shift), UA_QWERTY_LOW(1), KEY_1(1.5, Qt::Key_Shift), NOKEY_1 },
	{ UA_QWERTY_BOTTOM_ROW_DEFAULT },
};

static TabletKeymap::LayoutRow sUaQwertyBottomRow_default = { UA_QWERTY_BOTTOM_ROW_DEFAULT };
static TabletKeymap::LayoutRow sUaQwertyBottomRow_url = { UA_QWERTY_BOTTOM_ROW_URL };
static TabletKeymap::LayoutRow sUaQwertyBottomRow_email = { UA_QWERTY_BOTTOM_ROW_EMAIL };

static TabletKeymap::LayoutFamily sUaQwertyFamily("ua йцукен", "ua", IME_KBD_LANG_English, IME_KBD_SEC_REGQwerty,
                                                "+ = [  ]" /* Spaces are "Unicode Character 'HAIR SPACE' (U+200A) ' ' " */, "U a" /* Spaces are "Unicode Character 'HAIR SPACE' (U+200A) ' ' " */,
                                                0, 1, 10, 2, false, sUaQwerty, sUaQwertyBottomRow_default, sUaQwertyBottomRow_url, sUaQwertyBottomRow_email);
#endif // KEYBOARD_UA_QWERTY_H
