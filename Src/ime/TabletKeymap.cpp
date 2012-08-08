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


#include "TabletKeymap.h"

#include "KeyLocationRecorder.h"
#include "Localization.h"
#include "Logging.h"
#include "QtUtils.h"
#include "Utils.h"
#include "VirtualKeyboardPreferences.h"

#include <QFile>
#include <qdebug.h>

#include <pbnjson.hpp>

namespace Tablet_Keyboard {

// Keyboard layouts (added in reverse order because the order is changed when added to linked list)
#include "tabletkeymaps/us_dvorak.h"
#include "tabletkeymaps/us_qwerty.h"
#include "tabletkeymaps/ru_qwerty.h"
#include "tabletkeymaps/se_dvorak.h"
#include "tabletkeymaps/se_qwerty.h"
#include "tabletkeymaps/fr_azerty.h"
#include "tabletkeymaps/de_qwertz.h"

const TabletKeymap::LayoutFamily * TabletKeymap::LayoutFamily::s_firstFamily = NULL;

TabletKeymap::LayoutFamily::LayoutFamily(const char * name, const char * defaultLanguage, uint16_t primaryID, uint16_t secondaryID,
										 const char * symbolKeyLabel, const char * noLanguageKeyLabel, int tab_x, int symbol_x, int return_x, int return_y, bool needNumLock, Layout & layout,
										 LayoutRow & defaultBottomRow, LayoutRow & urlBottomRow, LayoutRow & emailBottomRow) :
	m_name(name), m_defaultLanguage(defaultLanguage), m_primaryID(primaryID), m_secondaryID(secondaryID), m_symbolKeyLabel(symbolKeyLabel), m_noLanguageKeyLabel(noLanguageKeyLabel),
	m_tab_x(tab_x), m_symbol_x(symbol_x), m_return_x(return_x), m_return_y(return_y), m_needNumLock(needNumLock), m_cachedGlyphsCount(0), m_layout(layout),
	m_defaultBottomRow(defaultBottomRow), m_urlBottomRow(urlBottomRow), m_emailBottomRow(emailBottomRow)
{
	m_nextFamily = s_firstFamily;
	s_firstFamily = this;
}

const TabletKeymap::LayoutFamily * TabletKeymap::LayoutFamily::findLayoutFamily(const char * name, bool returnNullNotDefaultIfNotFound)
{
	const LayoutFamily * family = s_firstFamily;
	while (family)
	{
		if (strcasecmp(family->m_name, name) == 0)
			return family;
		else
			family = family->m_nextFamily;
	}
	if (returnNullNotDefaultIfNotFound)
		return NULL;
	family = &sUsQwertyFamily;
	g_warning("LayoutFamily::findLayoutFamily: '%s' not found, returning '%s' by default.", name, family->m_name);
	return family;
}

TabletKeymap::TabletKeymap() : m_shiftMode(TabletKeymap::eShiftMode_Off), m_symbolMode(eSymbolMode_Off), m_shiftDown(false), m_symbolDown(false), m_autoCap(false), m_numLock(false),
	m_layoutFamily(&sUsQwertyFamily), m_layoutPage(eLayoutPage_plain), m_limitsDirty(true), m_limitsVersion(0)
{
	for (int r = 0; r < cKeymapRows; ++r)
		m_rowHeight[r] = 1;
}

QList<const char *> TabletKeymap::getLayoutList()
{
	QList<const char *> list;
	const LayoutFamily * family = LayoutFamily::s_firstFamily;
	while (family)
	{
		list.push_back(family->m_name);
		family = family->m_nextFamily;
	}
	return list;
}

const char * TabletKeymap::getLayoutDefaultLanguage(const char * layoutName)
{
	const TabletKeymap::LayoutFamily * family = LayoutFamily::findLayoutFamily(layoutName);
	if (family)
		return family->m_defaultLanguage;
	return NULL;
}

void TabletKeymap::setRowHeight(int rowIndex, int height)
{
	if (VERIFY(rowIndex >= 0 && rowIndex < cKeymapRows))
		m_rowHeight[rowIndex] = height;
}

bool TabletKeymap::setLayoutFamily(const LayoutFamily * layoutFamily)
{
	if (m_layoutFamily != layoutFamily)
	{
		m_layoutFamily = layoutFamily;
		updateLanguageKey();
		setEditorState(m_editorState);
		m_limitsDirty = true;
		resetCachedGlyphsCount();
		return true;
	}
	return false;
}

bool TabletKeymap::setLanguageName(const std::string & name)
{
	QString displayName = getLanguageDisplayName(name, m_layoutFamily);
	if (displayName != m_languageName)
	{
		m_languageName = displayName;
		if (updateLanguageKey())
			m_limitsDirty = true;
		return true;
	}
	return false;
}

QString TabletKeymap::getLanguageDisplayName(const std::string & languageName, const LayoutFamily * layoutFamily)
{
	QString name;
	if (::strcasecmp(languageName.c_str(), "none") == 0)
		name = QString::fromUtf8(layoutFamily->m_noLanguageKeyLabel);
	else
	{
		if (languageName.length() > 0)
			name += QChar(languageName[0]).toUpper();
		if (languageName.length() > 1)
			name += QChar(languageName[1]).toLower();
		if (languageName.length() > 2 && languageName[2] != '-')
			for (size_t k = 2; k < languageName.size(); ++k)
				name += QChar(languageName[k]).toLower();
	}
	return name;
}

void TabletKeymap::keyboardCombosChanged()
{
	VirtualKeyboardPreferences & prefs = VirtualKeyboardPreferences::instance();
	int count = qMin<int>(G_N_ELEMENTS(sLanguageChoices_Extended), prefs.getKeyboardComboCount());
	for (int k = 0; k < count; k++)
		sLanguageChoices_Extended[k] = (UKey) (cKey_KeyboardComboChoice_First + k);
	sLanguageChoices_Extended[count] = cKey_None;
}

inline float fabs(float f) { return f >= 0.f ? f : -f; }

bool TabletKeymap::updateLanguageKey(LayoutRow * bottomRow)
{
	if (bottomRow == NULL)
		bottomRow = &m_layoutFamily->m_layout[cKeymapRows - 1];
	WKey & symbol = (*bottomRow)[m_layoutFamily->m_symbol_x];
	WKey & language = (*bottomRow)[m_layoutFamily->m_symbol_x + 1];
	int symbolWeightBefore = symbol.m_weight;
	int languageWeightBefore = language.m_weight;
	if (!m_languageName.isEmpty())
	{
		symbol.m_weight = 1;
		language.set(cKey_ToggleLanguage, 1, sLanguageChoices_Extended);
	}
	else
	{
		symbol.m_weight = 2;
		language.hide();
	}
	if (symbolWeightBefore != symbol.m_weight || languageWeightBefore != language.m_weight)
		return true;
	return false;
}

int TabletKeymap::updateLimits()
{
	if (m_limitsDirty)
	{
		int rectWidth = m_rect.width();
		if (rectWidth > 0)
		{
			for (int y = 0; y < cKeymapRows; ++y)
			{
				float width = 0.0001f;	// handle possible rounding errors by nudging up
				for (int x = 0; x < cKeymapColumns; ++x)
				{
					width += fabs(m_layoutFamily->weight(x, y));
					m_hlimits[y][x] = width;
				}
				for (int x = 0; x < cKeymapColumns; ++x)
				{
					m_hlimits[y][x] = float(m_hlimits[y][x] * rectWidth) / width;
				}
			}
		}
		int rectHeight = m_rect.height();
		if (rectHeight > 0)
		{
			float height = 0.0001f;	// handle possible rounding errors by nudging up
			for (int y = 0; y < cKeymapRows; ++y)
			{
				height += m_rowHeight[y];
				m_vlimits[y] = height;
			}
			for (int y = 0; y < cKeymapRows; ++y)
			{
				m_vlimits[y] = float(m_vlimits[y] * rectHeight) / height;
			}
		}
		m_limitsDirty = false;
		++m_limitsVersion;
	}
	return m_limitsVersion;
}

bool TabletKeymap::setShiftMode(TabletKeymap::EShiftMode shiftMode)
{
	if (m_shiftMode != shiftMode)
	{
		m_shiftMode = shiftMode;
		updateMapping();
		return true;
	}
	return false;
}

bool TabletKeymap::setAutoCap(bool autoCap)
{
	if (autoCap != m_autoCap)
	{
		m_autoCap = autoCap;
		return true;
	}
	return false;
}

bool TabletKeymap::setSymbolMode(ESymbolMode symbolMode)
{
	if (m_symbolMode != symbolMode)
	{
		m_symbolMode = symbolMode;
		updateMapping();
		return true;
	}
	return false;
}

bool TabletKeymap::setShiftKeyDown(bool shiftKeyDown)
{
	if (shiftKeyDown != m_shiftDown)
	{
		m_shiftDown = shiftKeyDown;
		return true;
	}
	return false;
}

bool TabletKeymap::setSymbolKeyDown(bool symbolKeyDown)
{
	m_symbolDown = symbolKeyDown;
	return updateMapping();
}

#define UPDATE_KEYS(x, y, plain, alt) { WKey & wkey = (*m_layoutFamily->m_layout)[y][x]; if (wkey.m_key != plain) { wkey.m_key = plain; layoutChanged = true; } if (wkey.m_altkey != alt) { wkey.m_altkey = alt; layoutChanged = true; } }

bool TabletKeymap::setEditorState(const PalmIME::EditorState & editorState)
{
	bool	layoutChanged = false;
	bool	weightChanged = false;
	bool	numLock = false;
	LayoutRow * newBottomRow = &m_layoutFamily->m_defaultBottomRow;

	if (!(m_editorState == editorState))
	{
		layoutChanged = true;
		m_editorState = editorState;
		m_editorState.enterKeyLabel[sizeof(m_editorState.enterKeyLabel) - 1] = 0;	// certify null-termination
		m_custom_Enter = QString::fromUtf8(m_editorState.enterKeyLabel);
	}

	switch (m_editorState.type)
	{
	case PalmIME::FieldType_Email:
		newBottomRow = &m_layoutFamily->m_emailBottomRow;
		break;
	case PalmIME::FieldType_URL:
		newBottomRow = &m_layoutFamily->m_urlBottomRow;
		break;
	default:
	case PalmIME::FieldType_Text:
	case PalmIME::FieldType_Password:
	case PalmIME::FieldType_Search:
	case PalmIME::FieldType_Range:
	case PalmIME::FieldType_Color:
		break;
	case PalmIME::FieldType_Phone:
	case PalmIME::FieldType_Number:
		if (m_layoutFamily->m_needNumLock)
			numLock = true;
		break;
	}

	updateLanguageKey(newBottomRow);
	const int lastRow = cKeymapRows - 1;
	LayoutRow & bottomRow = m_layoutFamily->m_layout[lastRow];

	for (int x = 0; x < cKeymapColumns; ++x)
	{
		WKey & wkey = bottomRow[x];
		const WKey & newWkey = (*newBottomRow)[x];
		if (wkey.m_weight != newWkey.m_weight)
			weightChanged = true;
		else if (wkey.m_key != newWkey.m_key || wkey.m_altkey != newWkey.m_altkey || wkey.m_extended != newWkey.m_extended)
			layoutChanged = true;
		else
			continue;
		wkey = newWkey;
	}

	if (weightChanged)
		m_limitsDirty = true;

	if (numLock != m_numLock)
	{
		m_numLock = numLock;
		layoutChanged = true;
	}

	m_localized__Enter		= fromStdUtf8(LOCALIZED("Enter"));
	m_localized__Tab		= fromStdUtf8(LOCALIZED("Tab"));
	m_localized__Next		= fromStdUtf8(LOCALIZED("Next"));
	m_localized__Previous	= fromStdUtf8(LOCALIZED("Prev"));

	return layoutChanged || weightChanged;
}

bool TabletKeymap::updateMapping()
{
	ELayoutPage	newPage = !isSymbolActive() ? eLayoutPage_plain : eLayoutPage_Alternate;
	if (newPage != m_layoutPage)
	{
		m_layoutPage = newPage;
		return true;
	}
	return false;
}

int TabletKeymap::keyboardToKeyZone(QPoint keyboardCoordinate, QRect & outZone)
{
	int count = 0;
	if (isValidLocation(keyboardCoordinate))
	{
		updateLimits();

		int x = keyboardCoordinate.x();
		int y = keyboardCoordinate.y();

		int left = m_rect.left() + (x > 0 ? m_hlimits[y][x - 1] : 0);
		int right = m_rect.left() + m_hlimits[y][x] - 1;

		int bottom = m_rect.top() + m_vlimits[y] - 1;
		int top = m_rect.top() + (y > 0 ? m_vlimits[y - 1] : 0);

		outZone.setCoords(left, top, right, bottom);

		if (right > left)
			count = m_layoutFamily->weight(x, y) < 0 ? -1 : 1;
	}
	return count;
}

UKey TabletKeymap::map(int x, int y)
{
	if (cResizeHandleCoord == QPoint(x, y))
		return cKey_ResizeHandle;
	if (!isValidLocation(x, y))
		return cKey_None;
	const WKey & wkey = m_layoutFamily->wkey(x, y);
	UKey key = wkey.m_key;
	if ((m_numLock || (m_layoutFamily->m_needNumLock && m_shiftMode == eShiftMode_CapsLock)) && wkey.m_altkey >= Qt::Key_0 && wkey.m_altkey <= Qt::Key_9)
	{
		if (!isShiftActive())
			key = wkey.m_altkey;
	}
	else
	{
#if 0 // experiment to get arrow keys when shift & option are down. Doesn't work because shift is held down, extending selection...
        if (m_shiftDown && m_symbolDown)
		{
			switch (key)
			{
			case Qt::Key_Y:	return Qt::Key_Up;
			case Qt::Key_B:	return Qt::Key_Down;
			case Qt::Key_G: return Qt::Key_Left;
			case Qt::Key_H: return Qt::Key_Right;
			case Qt::Key_T: return Qt::Key_Home;
			case Qt::Key_V: return Qt::Key_End;
			case Qt::Key_U: return Qt::Key_PageUp;
			case Qt::Key_N: return Qt::Key_PageDown;
			default:
				break;
			}
		}
#endif
		// for letters, use alternate layout when symbol is active, for non-letter, use alternate layout when shift is active
		if (UKeyIsCharacter(key) ? m_layoutPage == eLayoutPage_Alternate : isShiftActive())
			key = wkey.m_altkey;
	}
	return key;
}

int TabletKeymap::xCenterOfKey(int touchX, int x, int y, float weight)
{
	int leftSide = (x > 0) ? m_hlimits[y][x - 1] : 0;
	int rightSide = m_hlimits[y][x];
	int center = (leftSide + rightSide) / 2;
	if (weight > 1)
	{
		int radius = (rightSide - leftSide) / (weight * 2);
		if (touchX < center)
		{
			int leftMost = leftSide + radius;
			if (touchX < leftMost)
				center = leftMost;
			else
				center = touchX;
		}
		else
		{
			int rightMost = rightSide - radius;
			if (touchX > rightMost)
				center = rightMost;
			else
				center = touchX;
		}
	}
	//g_debug("TouchX: %d, x: %d, y: %d, left: %d, right: %d, center: %d, radius: %g -> %d", touchX, x, y, leftSide, rightSide, (leftSide + rightSide) / 2, (rightSide - leftSide) / (weight * 2), center);
	return center;
}

int TabletKeymap::yCenterOfRow(int y, UKey key)
{	// slightly reduce the effective height of the top & lowest rows, by moving their centers further away from the closest inner row
	const int cReduceFactorTopRow = 4;		// 1 = most neutral factor: top row tallest, higher factors reduce its effective height
	const int cReduceFactorBottomRow = 3;	// 1 = most neutral factor: bottom row tallest, higher factors reduce its effective height
	const int cReduceFactorBottomRowSpace = 2;	// 1 = most neutral factor: bottom row tallest, higher factors reduce its effective height
	int top_y = y > 0 ? m_vlimits[y - 1] : 0;
	int lower_y = m_vlimits[y];
	if (y == 0)
		return lower_y / cReduceFactorTopRow;
	else if (y < cKeymapRows - 1)
		return (top_y + lower_y) / 2;
	else if (key == Qt::Key_Space)
		return (top_y + (cReduceFactorBottomRowSpace - 1) * lower_y) / cReduceFactorBottomRowSpace;
	return (top_y + (cReduceFactorBottomRow - 1) * lower_y) / cReduceFactorBottomRow;
}

inline int square(int x)										{ return x * x; }
inline int close_distance(int x, int distance)					{ return (x < 0 ? -x : x) <= distance; }

inline float adjusted_weight(const TabletKeymap::WKey & wkey)
{
    if (wkey.m_weight < 1)
        return 0.3;
    switch ((int)wkey.m_key)
	{
    case cKey_Hide:				return 0.4;
    case Qt::Key_Tab:			return 0.7;
    case Qt::Key_Space:         return 0.9;
	//case cKey_ToggleLanguage:	return 0.8;
	default:
		;
	}
#if 1
	return 1.0;
#else
	if (symbolActive || wkey.m_key < Qt::Key_A || wkey.m_key > Qt::Key_Z)
		return 1.0;
    return 1.2;
#endif
}

QPoint TabletKeymap::pointToKeyboard(const QPoint & location, bool useDiamondOptimizations)
{
	updateLimits();
	int locy = location.y() - m_rect.top() + 1;
#if RESIZE_HANDLES
	if (locy * 3 < m_vlimits[0] * 2 && (location.x() <= m_hlimits[0][0] || location.x() >= m_hlimits[0][cKeymapColumns-2]))
		return cResizeHandleCoord;
#endif
	int y = 0;
	while (locy > m_vlimits[y] && ++y < cKeymapRows)
		;
	if (y < cKeymapRows)
	{
		int locx = location.x() + 1;
		int x = 0;
		while (locx > m_hlimits[y][x] && ++x < cKeymapColumns)
			;
		if (x < cKeymapColumns)
		{
			bool changed = false;
			const WKey & wkey = m_layoutFamily->wkey(x, y);
			if (useDiamondOptimizations)
			{	// try to improve accuracy by looking if the touch point is closer to some other key above or below...
				int center_y = yCenterOfRow(y, wkey.m_key);	// vertical center of found key
				int min = (m_vlimits[y] - (y > 0 ? m_vlimits[y - 1] : 0)) / 10;
				int oy = -1;
				if (y > 0 && locy < center_y - min)
					oy = y - 1;		// pressed the upper part of the key and there is a row above
				else if (y < cKeymapRows - 1 && locy > center_y + min)
					oy = y + 1;		// pressed the lower part of the key and there is a row below
				if (oy >= 0)	// there is a possible better match above or below, on the oy row
				{
					int ox = x;
					while (ox > 0 && locx < m_hlimits[oy][ox])
						--ox;
					while (locx > m_hlimits[oy][ox] && ++ox < cKeymapColumns)
						;
					if (ox < cKeymapColumns)
					{
						int center_x = xCenterOfKey(locx, x, y, wkey.m_weight);								// horizontal center of first found key
						const WKey & owkey = m_layoutFamily->wkey(ox, oy);
						int center_ox = xCenterOfKey(locx, ox, oy, owkey.m_weight);							// horizontal center of other candidate
						int center_oy = yCenterOfRow(oy, owkey.m_key);										// vertical center of other candidate
                        int first_d = square(locy - center_y);                          					// "distance" between tap location & first found key
                        int o_d = square(locy - center_oy);                         						// "distance" between tap location & other candidate
//                        g_debug("Key: %s, %d-%d, cx: %d, cy: %d", getKeyDisplayString(wkey.m_key, true).toUtf8().data(), x, y, center_x, center_y);
//                        g_debug("OKy: %s, %d-%d, cx: %d, cy: %d", getKeyDisplayString(owkey.m_key, true).toUtf8().data(), ox, oy, center_ox, center_oy);
                        if (!close_distance(center_x - center_ox, 2))
                        {
                            first_d += square(locx - center_x);
                            o_d += square(locx - center_ox);
                        }
						bool use_o = o_d * adjusted_weight(wkey) < first_d * adjusted_weight(owkey);
						if (use_o)
							x = ox, y = oy, changed = true;
					}
				}
			}
			if (!changed && wkey.m_weight < 0)
			{ // "invisible" key. Look for the visible neighbor that has the same key...
				for (int xo = (x == 0) ? 0 : x - 1; xo <= x + 1 && xo < cKeymapColumns; ++xo)
					for (int yo = (y == 0) ? 0 : y - 1; yo <= y + 1 && yo < cKeymapRows; ++yo)
						if ((x != xo || y != yo) && m_layoutFamily->wkey(xo, yo).m_key == wkey.m_key)
						{
							x = xo; y = yo; xo = cKeymapColumns; yo = cKeymapRows;	// update x & y, then exit both loops
						}
			}
			//g_debug("%dx%d -> %s", x, y, QString(m_layoutFamily->key(x, y, m_layoutPage)).toUtf8().data());
			return QPoint(x, y);
		}
	}
	return cOutside;
}

QString TabletKeymap::getXKeys(int locX, int k_x, int k_y)
{
	QString keys;
	UKey key = map(k_x, k_y);
	if (UKeyIsUnicodeQtKey(key) && key != Qt::Key_Space)
		keys += QChar(key).toLower();
	if (locX < xCenterOfKey(locX, k_x, k_y, 1))
	{
		if (k_x > 0)
		{
			UKey okey = map(k_x - 1, k_y);
			if (okey != key && UKeyIsUnicodeQtKey(okey) && okey != Qt::Key_Space)
				keys += QChar(okey).toLower();
		}
	}
	else
	{
		if (k_x < cKeymapColumns - 1)
		{
			UKey okey = map(k_x + 1, k_y);
			if (okey != key && UKeyIsUnicodeQtKey(okey) && okey != Qt::Key_Space)
				keys += QChar(okey).toLower();
		}
	}

	return keys;
}

std::string TabletKeymap::pointToKeys(const QPoint & location)
{
	QString	keys;
	updateLimits();
	int locy = location.y() + 1;
	int y = 0;
	while (locy > m_vlimits[y] && ++y < cKeymapRows)
		;
	if (y < cKeymapRows)
	{
		int locx = location.x() + 1;
		int x = 0;
		while (locx > m_hlimits[y][x] && ++x < cKeymapColumns)
			;
		if (x < cKeymapColumns)
		{
			keys += getXKeys(locx,  x, y);

			int center_y = yCenterOfRow(y, keys.size() > 0 ? UKey(keys[0].unicode()) : cKey_None);	// vertical center of found key
			int oy = -1;
			if (locy < center_y)
			{
				if (y > 0)
					oy = y - 1;		// pressed the upper part of the key and there is a row above
			}
			else if (y < cKeymapRows)
				oy = y + 1;		// pressed the lower part of the key and there is a row below
			if (oy >= 0)	// there is a possible better match above or below, on the oy row
			{
				int ox = x;
				while (ox > 0 && locx < m_hlimits[oy][ox])
					--ox;
				while (locx > m_hlimits[oy][ox] && ++ox < cKeymapColumns)
					;
				if (ox < cKeymapColumns)
					keys += getXKeys(locx, ox, oy);
			}
		}
	}
	return keys.toUtf8().data();	// convert to utf8
}

bool TabletKeymap::generateKeyboardLayout(const char * fullPath)
{
    if (rect().width() <= 0 || rect().height() <= 0)
        return false;
	QFile file(fullPath);
	//QFile keys(QString(fullPath) + ".keys");
	if (VERIFY(file.open(QIODevice::WriteOnly))/* && VERIFY(keys.open(QIODevice::WriteOnly))*/)
	{
		updateLimits();
        file.write("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\n");
        file.write(string_printf("<keyboard primaryId=\"0x%02X\" secondaryId=\"0x%02X\" defaultLayoutWidth=\"%d\" defaultLayoutHeight=\"%d\">\n\n",
                m_layoutFamily->m_primaryID, m_layoutFamily->m_secondaryID >> 8, rect().width(), rect().height()).c_str());
		file.write("<area conditionValue=\"0\">\n");
		QRect	r;
		for (int y = 0; y < cKeymapRows; ++y)
			for (int x = 0; x < cKeymapColumns; ++x)
			{
				const WKey & wkey = m_layoutFamily->wkey(x, y);
				UKey key = wkey.m_key;
				if (UKeyIsUnicodeQtKey(key) && keyboardToKeyZone(QPoint(x, y), r) > 0)
				{
					r.translate(-rect().left(), -rect().top());
#if 1
					r.adjust(6, 6, -6, -6);
#else
					const int cMaxWidth = 2;
					if (r.width() > cMaxWidth && r.height() > cMaxWidth)
					{
						QPoint center = r.center();
						r.setLeft(center.x() - cMaxWidth / 2);
						r.setRight(center.x() + cMaxWidth / 2);
						r.setTop(center.y() - cMaxWidth / 2);
						r.setBottom(center.y() + cMaxWidth / 2);
					}
#endif
					QString text(key);
					switch (key)
					{
					case Qt::Key_Ampersand:     text = "&amp;";     break;
					case Qt::Key_Less:          text = "&lt;";      break;
					case Qt::Key_Greater:       text = "&gt;";      break;
					case Qt::Key_QuoteDbl:      text = "&quot;";    break;
					//case Qt::Key_Apostrophe:    text = "&apos;";    break;
					default:                                        break;
					}
					if (key == Qt::Key_Space) {
						file.write(string_printf("<key keyLabel=\" \" keyType=\"function\" keyName=\"ET9KEY_SPACE\" keyLeft=\"%ddp\" keyTop=\"%ddp\" keyWidth=\"%ddp\" keyHeight=\"%ddp\" />\n",
												 r.left(), r.top(), r.width(), r.height()).c_str());
					} else {
						file.write(string_printf("<key keyLabel=\"%s\" keyType=\"%s\" keyLeft=\"%ddp\" keyTop=\"%ddp\" keyWidth=\"%ddp\" keyHeight=\"%ddp\" />\n",
							  text.toUtf8().data(), key < 256 && isalpha(key) ? "regional" : "nonRegional",
							  r.left(), r.top(), r.width(), r.height()).c_str());
					}
					//QPoint center = r.center();
					//keys.write(string_printf("%d %d %d\n", key, center.x(), center.y()).c_str());
				}
			}
		file.write("</area>\n\n");
        file.write("</keyboard>\n\n");
        file.close();
	}
	return true;
}

std::string TabletKeymap::getKeyboardLayoutAsJson()
{
	pbnjson::JValue layout = pbnjson::Object();
	layout.put("layout", m_layoutFamily->m_name);
	layout.put("width", rect().width());
	layout.put("height", rect().height());
	pbnjson::JValue keys = pbnjson::Array();
	QRect	r;
	updateLimits();
	for (int y = 0; y < cKeymapRows; ++y)
		for (int x = 0; x < cKeymapColumns; ++x)
		{
			if (keyboardToKeyZone(QPoint(x, y), r) > 0)
			{
				QPoint center = r.center();
				const WKey & wkey = m_layoutFamily->wkey(x, y);
				UKey key = wkey.m_key;
				pbnjson::JValue jkey = pbnjson::Object();
				jkey.put("label", (const char *) getKeyDisplayString(key, true).toUtf8().data());
				jkey.put("shift", false);
				jkey.put("symbol", false);
				jkey.put("x", center.x());
				jkey.put("y", center.y());
				keys.append(jkey);
				UKey altKey = wkey.m_altkey;
				if (altKey != cKey_None && key != altKey)
				{
					pbnjson::JValue jkey = pbnjson::Object();
					jkey.put("label", (const char *) getKeyDisplayString(altKey, true).toUtf8().data());
					if (key >= Qt::Key_A && key <= Qt::Key_Z)
					{
						jkey.put("shift", false);
						jkey.put("symbol", true);
					}
					else
					{
						jkey.put("shift", true);
						jkey.put("symbol", false);
					}
					jkey.put("x", center.x());
					jkey.put("y", center.y());
					keys.append(jkey);
				}
			}
		}
	layout.put("keys", keys);

	std::string	layoutString;
	pbnjson::JGenerator().toString(layout, pbnjson::JSchemaFragment("{}"), layoutString);

	return layoutString;
}

const UKey * TabletKeymap::getExtendedChars(QPoint keyboardCoordinate)
{
	if (isValidLocation(keyboardCoordinate))
	{
		const WKey & wkey = m_layoutFamily->wkey(keyboardCoordinate.x(), keyboardCoordinate.y());
		if (wkey.m_key < Qt::Key_A || wkey.m_key > Qt::Key_Z || !isSymbolActive())
			return wkey.m_extended;
	}
	return NULL;
}

TabletKeymap::ETabAction TabletKeymap::tabAction() const
{
	return eTabAction_Tab;	// killing any variable Tab key behavior. Tab key always says 'Tab' for now...

	int actions = m_editorState.actions & (PalmIME::FieldAction_Next | PalmIME::FieldAction_Previous);
	if (actions == 0)
		return eTabAction_Tab;
	else if (actions == PalmIME::FieldAction_Next)	// only next
		return eTabAction_Next;
	else if (actions == PalmIME::FieldAction_Previous || m_shiftDown)	// only previous or both & shift down
		return eTabAction_Previous;
	else
		return eTabAction_Next;	// only left option...
}

QString TabletKeymap::getKeyDisplayString(UKey key, bool logging)
{
	if (UKeyIsFunctionKey(key))
	{
		if (UKeyIsKeyboardComboKey(key))
		{
			int index = key - cKey_KeyboardComboChoice_First;
			VirtualKeyboardPreferences & prefs = VirtualKeyboardPreferences::instance();
			if (VERIFY(index >= 0 && index < prefs.getKeyboardComboCount()))
				return getLanguageDisplayName(prefs.getkeyboardCombo(index).language, LayoutFamily::findLayoutFamily(prefs.getkeyboardCombo(index).layout.c_str(), false));
			return NULL;
		}

		switch ((int)key)
		{
		case Qt::Key_Return:							return (m_custom_Enter.isEmpty()) ? m_localized__Enter : m_custom_Enter;
		case Qt::Key_Tab:
		{
			switch (tabAction())
			{
			case eTabAction_Next:						return m_localized__Next;
			case eTabAction_Previous:					return m_localized__Previous;
			case eTabAction_Tab:
			default:									return m_localized__Tab;
			}
		}
		case cKey_Resize_Tiny:							return logging ? "<XS>" : "XS";
		case cKey_Resize_Small:							return logging ? "<S>" : "S";
		case cKey_Resize_Default:						return logging ? "<M>" : "M";
		case cKey_Resize_Large:							return logging ? "<L>" : "L";
		case cKey_Emoticon_Frown:						return ":-(";
		case cKey_Emoticon_Cry:							return ":'(";
		case cKey_Emoticon_Smile:						return ":-)";
		case cKey_Emoticon_Wink:						return ";-)";
		case cKey_Emoticon_Yuck:						return ":-P";
		case cKey_Emoticon_Gasp:						return ":-O";
		case cKey_Emoticon_Heart:						return "<3";
		case cKey_Symbol:								return  QString::fromUtf8((symbolMode() == TabletKeymap::eSymbolMode_Lock) ? "A B C"/* Spaces are "Unicode Character 'HAIR SPACE' (U+200A) ' ' " */ : m_layoutFamily->m_symbolKeyLabel);
		case cKey_DotCom:								return ".com";
		case cKey_DotCoUK:								return ".co.uk";
		case cKey_DotOrg:								return ".org";
		case cKey_DotDe:								return ".de";
		case cKey_DotEdu:								return ".edu";
		case cKey_DotFr:								return ".fr";
		case cKey_DotGov:								return ".gov";
		case cKey_DotNet:								return ".net";
		case cKey_DotUs:								return ".us";
		case cKey_DotSe:								return ".se";
		case cKey_WWW:									return "www.";
		case cKey_HTTPColonSlashSlash:					return "http://";
		case cKey_HTTPSColonSlashSlash:					return "https://";
		case Qt::Key_Left:								return QChar(0x2190) /* ← */;
		case Qt::Key_Right:								return QChar(0x2192) /* → */;
		case Qt::Key_Up:								return QChar(0x2191) /* ↑ */;
		case Qt::Key_Down:								return QChar(0x2193) /* ↓ */;
		case Qt::Key_Home:								return QChar(0x21f1) /* ⇱ */;
		case Qt::Key_End:								return QChar(0x21f2) /* ⇲ */;
		case Qt::Key_PageUp:							return QChar(0x21de) /* ⇞ */;
		case Qt::Key_PageDown:							return QChar(0x21df) /* ⇟ */;
		case cKey_ToggleSuggestions:					return "XT9";
		case cKey_ShowXT9Regions:						return "XT9 Regs";
		case cKey_ShowKeymapRegions:					return "Regions";
		case cKey_ToggleLanguage:						return m_languageName;
		case cKey_CreateDefaultKeyboards:				return "Prefs";
		case cKey_ClearDefaultKeyboards:				return "Clear";
		case cKey_SwitchToQwerty:						return "QWERTY";
		case cKey_SwitchToAzerty:						return "AZERTY";
		case cKey_SwitchToQwertz:						return "QWERTZ";
		case cKey_StartStopRecording:					return KeyLocationRecorder::instance().isRecording() ? "Stop" : "Rec";
		case cKey_ToggleSoundFeedback:					return VirtualKeyboardPreferences::instance().getTapSounds() ? "Mute" : "Sound";
		case cKey_SymbolPicker:							return "Sym";
		case Qt::Key_Shift:								return logging ? "Shift" : QString();
		case Qt::Key_AltGr:								return logging ? "AltGr" : QString();
		case cKey_Hide:									return logging ? "Hide" : QString();
		case Qt::Key_Backspace:							return logging ? "Backspace" : QString();
		default:			return QString();
		}
	}
	return isCapOrAutoCapActive() ? QChar(key).toUpper() : QChar(key).toLower();
}

}; // namespace Tablet_Keyboard
