/* @@@LICENSE
*
*      Copyright (c) 2010-2012 Hewlett-Packard Development Company, L.P.
*      Copyright (c) 2012 Alexander Kerner <alexander.kerner@gmail.com>
*      Copyright (c) 2012 Måns Andersson <mail@mansandersson.se>
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

#ifndef KEYBOARD_UK_H
#define KEYBOARD_UK_H
#include "../TabletKeymap.h"
#include "common_keys.h"
static TabletKeymap::LayoutFamily sLayoutUkrainian("Ukrainian", "uk");
static TabletKeymap::constUKeyArray sUkrainian_DotCom_Extended = { cKey_DotCom, cKey_DotNet, cKey_DotOrg, cKey_DotEdu, cKey_None };

static TabletKeymap::constUKeyArray sUkQwerty1_extended = { Qt::Key_1, Qt::Key_Exclam, UKey(0x00B9) /* SUPERSCRIPT ONE ¹ */, UKey(0x00BC) /* VULGAR FRACTION ONE QUARTER ¼ */, UKey(0x00BD) /* VULGAR FRACTION ONE HALF ½ */, Qt::Key_exclamdown, cKey_None };
static TabletKeymap::constUKeyArray sUkQwerty2_extended = { Qt::Key_2, Qt::Key_At, UKey(0x00B2) /* SUPERSCRIPT TWO ² */, cKey_None };
static TabletKeymap::constUKeyArray sUkQwerty3_extended = { Qt::Key_3, Qt::Key_NumberSign, UKey(0x00B3) /* SUPERSCRIPT THREE ³ */, UKey(0x00BE) /* VULGAR FRACTION THREE QUARTERS ¾ */, cKey_None };
static TabletKeymap::constUKeyArray sUkQwerty4_extended = { Qt::Key_4, Qt::Key_Dollar, cKey_Euro, Qt::Key_sterling, Qt::Key_yen, Qt::Key_cent, UKey(0x00A4) /* CURRENCY SIGN ¤ */, cKey_None };
static TabletKeymap::constUKeyArray sUkQwerty5_extended = { Qt::Key_5, Qt::Key_Percent, UKey(0x2030) /* PER MILLE SIGN ‰ */, cKey_None };
static TabletKeymap::constUKeyArray sUkQwerty6_extended = { Qt::Key_6, Qt::Key_AsciiCircum, cKey_None };
static TabletKeymap::constUKeyArray sUkQwerty7_extended = { Qt::Key_7, Qt::Key_Ampersand, cKey_None };
static TabletKeymap::constUKeyArray sUkQwerty8_extended = { Qt::Key_8, Qt::Key_Asterisk, cKey_None };
static TabletKeymap::constUKeyArray sUkQwerty9_extended = { Qt::Key_9, Qt::Key_ParenLeft, Qt::Key_BracketLeft, Qt::Key_BraceLeft, cKey_None };
static TabletKeymap::constUKeyArray sUkQwerty0_extended = { Qt::Key_0, Qt::Key_ParenRight, Qt::Key_BracketRight, Qt::Key_BraceRight, cKey_None };
static TabletKeymap::constUKeyArray ukPeriod_extended = { Qt::Key_Period, Qt::Key_Comma, Qt::Key_Slash, Qt::Key_Backslash, cKey_None };
static TabletKeymap::constUKeyArray ukE_extended = { UKey(0x415), UKey(0x401), cKey_None};
static TabletKeymap::constUKeyArray ukb_extended = { UKey(0x42c), UKey(0x42a), cKey_None};
static TabletKeymap::constUKeyArray uk_extended_1 = { UKey(0x419), UKey(0x407), cKey_None };
static TabletKeymap::constUKeyArray uk_extended_2 = { UKey(0x413), UKey(0x490), cKey_None };
static TabletKeymap::constUKeyArray uk_extended_3 = { UKey(0x42b), UKey(0x406), cKey_None };
static TabletKeymap::constUKeyArray uk_extended_4 = { UKey(0x42d), UKey(0x404), cKey_None };

#define UK_QWERTY_NUMBERS(w)			{ w, Qt::Key_1,			Qt::Key_Exclam,							sUkQwerty1_extended },\
										{ w, Qt::Key_2,			Qt::Key_At,								sUkQwerty2_extended },\
										{ w, Qt::Key_3,			Qt::Key_NumberSign,						sUkQwerty3_extended },\
										{ w, Qt::Key_4,			Qt::Key_Dollar,							sUkQwerty4_extended },\
										{ w, Qt::Key_5,			Qt::Key_Percent,						sUkQwerty5_extended },\
										{ w, Qt::Key_6,	 		Qt::Key_AsciiCircum,					sUkQwerty6_extended },\
										{ w, Qt::Key_7,			Qt::Key_Ampersand,						sUkQwerty7_extended },\
										{ w, Qt::Key_8,			Qt::Key_Asterisk,						sUkQwerty8_extended },\
										{ w, Qt::Key_9,			Qt::Key_ParenLeft,						sUkQwerty9_extended },\
										{ w, Qt::Key_0,			Qt::Key_ParenRight,						sUkQwerty0_extended }

#define UK_QWERTY_TOP(w)				{ w, UKey(0x419),       Qt::Key_QuoteLeft,						uk_extended_1},\
										{ w, UKey(0x426),       Qt::Key_AsciiTilde,						NULL },\
										{ w, UKey(0x423),       cKey_Euro,								NULL },\
										{ w, UKey(0x41a),		Qt::Key_sterling,						NULL },\
										{ w, UKey(0x415),		Qt::Key_Backslash,						ukE_extended },\
										{ w, UKey(0x41d), 		Qt::Key_Bar,							NULL },\
										{ w, UKey(0x413),		Qt::Key_BraceLeft,						uk_extended_2 },\
										{ w, UKey(0x428),		Qt::Key_BraceRight,						NULL },\
										{ w, UKey(0x429),		Qt::Key_BracketLeft,					NULL },\
										{ w, UKey(0x417),		Qt::Key_BracketRight,					NULL },\
										{ w, UKey(0x425),		Qt::Key_BracketRight,					NULL }

#define UK_QWERTY_MID(w)				{ w, UKey(0x424),		Qt::Key_Less,							NULL },\
										{ w, UKey(0x42b),		Qt::Key_Greater,						uk_extended_3 },\
										{ w, UKey(0x412),		Qt::Key_Equal,							NULL },\
										{ w, UKey(0x410),		Qt::Key_Plus,							NULL },\
										{ w, UKey(0x41f),		UKey(0x00D7) /* multiplication sign */,	NULL },\
										{ w, UKey(0x420),		UKey(0x00F7) /* division sign */,		NULL },\
										{ w, UKey(0x41e),		Qt::Key_degree,							NULL },\
										{ w, UKey(0x41b),		Qt::Key_Semicolon,						NULL },\
										{ w, UKey(0x414),		Qt::Key_Colon,							NULL },\
										{ w, UKey(0x416),		Qt::Key_Colon,							NULL },\
										{ w, UKey(0x42d),		Qt::Key_Colon,							uk_extended_4 }

#define UK_QWERTY_LOW(w)				{ w, UKey(0x42f),		cKey_Emoticon_Smile,					NULL },\
										{ w, UKey(0x427),		cKey_Emoticon_Wink,						NULL },\
										{ w, UKey(0x421),		cKey_Emoticon_Frown,					NULL },\
										{ w, UKey(0x41c),		cKey_Emoticon_Cry,						NULL },\
										{ w, UKey(0x418),		cKey_Emoticon_Yuck,						NULL },\
										{ w, UKey(0x422),		cKey_Emoticon_Gasp,						NULL },\
										{ w, UKey(0x42c),		cKey_Emoticon_Heart,					ukb_extended },\
										{ w, UKey(0x411),		Qt::Key_Slash,      					NULL },\
										{ w, UKey(0x42e),		Qt::Key_Question,   					NULL },\
										{ w, Qt::Key_Period,	Qt::Key_Comma,	    					ukPeriod_extended }

#define UK_QWERTY_BOTTOM_ROW_DEFAULT \
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

#define UK_QWERTY_BOTTOM_ROW_URL \
									KEY_1(1, Qt::Key_Tab),\
									KEY_1(2, cKey_Symbol),\
									NOKEY_1,\
									KEY_3(1, Qt::Key_Slash, Qt::Key_Slash, sURL_extended),\
									KEY_1(SPACE_SIZE - 2, Qt::Key_Space),\
									KEY_3(1, cKey_DotCom, cKey_DotCom, sUkrainian_DotCom_Extended),\
									KEY_3(1, Qt::Key_Apostrophe, Qt::Key_QuoteDbl, sSingleAndDoubleQuote_extended),\
									KEY_3(1, Qt::Key_Minus, Qt::Key_Underscore, sMinusUnderscore_extended),\
									KEY_3(1, cKey_Hide, cKey_Hide, sHide_extended),\
									NOKEY_3

#define UK_QWERTY_BOTTOM_ROW_EMAIL \
									KEY_1(1, Qt::Key_Tab),\
									KEY_1(2, cKey_Symbol),\
									NOKEY_1,\
									KEY_1(1, Qt::Key_At),\
									KEY_1(SPACE_SIZE - 2, Qt::Key_Space),\
									KEY_3(1, cKey_DotCom, cKey_DotCom, sUkrainian_DotCom_Extended),\
									KEY_3(1, Qt::Key_Apostrophe, Qt::Key_QuoteDbl, sSingleAndDoubleQuote_extended),\
									KEY_3(1, Qt::Key_Minus, Qt::Key_Underscore, sMinusUnderscore_extended),\
									KEY_3(1, cKey_Hide, cKey_Hide, sHide_extended),\
									NOKEY_3

static TabletKeymap::Layout sUkQwertyLayout = {
	{ UK_QWERTY_NUMBERS(1), KEY_1(2, cKey_Trackball) },
	{ UK_QWERTY_TOP(1), KEY_1(1.5, Qt::Key_Backspace), NOKEY_1 },
	{ UK_QWERTY_MID(1), KEY_1(1.5, Qt::Key_Return), NOKEY_1 },
	{ KEY_1(1, Qt::Key_Shift), UK_QWERTY_LOW(1), KEY_1(1.5, Qt::Key_Shift), NOKEY_1 },
	{ UK_QWERTY_BOTTOM_ROW_DEFAULT },
};

static TabletKeymap::LayoutRow sUkQwertyBottomRow_default = { UK_QWERTY_BOTTOM_ROW_DEFAULT };
static TabletKeymap::LayoutRow sUkQwertyBottomRow_url = { UK_QWERTY_BOTTOM_ROW_URL };
static TabletKeymap::LayoutRow sUkQwertyBottomRow_email = { UK_QWERTY_BOTTOM_ROW_EMAIL };

static TabletKeymap::Keymap sUkQwerty(&sLayoutUkrainian, "йцукен", IME_KBD_LANG_English, IME_KBD_SEC_REGQwerty,
                                                "+ = [  ]" /* Spaces are "Unicode Character 'HAIR SPACE' (U+200A) ' ' " */, "йцукен" /* Spaces are "Unicode Character 'HAIR SPACE' (U+200A) ' ' " */,
                                                0, 1, 10, 2, false, sUkQwertyLayout, sUkQwertyBottomRow_default, sUkQwertyBottomRow_url, sUkQwertyBottomRow_email);
#endif // KEYBOARD_UK_H
