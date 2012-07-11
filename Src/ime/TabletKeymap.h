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



#ifndef TABLET_KEYMAP_H
#define TABLET_KEYMAP_H

#include <palmimedefines.h>

#include <qlist.h>
#include <qrect.h>
#include <qstring.h>

#include "PalmIMEHelpers.h"

// Whether the keyboard should have resize handles in the top left & right corners
#define RESIZE_HANDLES 0

class QFile;

namespace Tablet_Keyboard {

// We're back to using Qt::Key definitions. This typedef allows us to change our mind...
typedef Qt::Key UKey;

// Constants defined to make keyboard definitions more readable (and more stable as we change the values they point to...)
// Note that except for the Euro sign, they are only meant to represent a key and are never used as unicode characters for display purposes...

const UKey cKey_None = UKey(0);						// nothing...
const UKey cKey_Euro = UKey(0x20ac);				// unicode "EURO SIGN" €
const UKey cKey_SymbolPicker = Qt::Key_Control;		// not used in virtual keyboard: that's to trigger Webkit's symbol picker
const UKey cKey_Symbol = Qt::Key_Alt;				// "new" virtual keyboard symbol key

// special keys not mapping to already existing Qt keys: use unique value used only internaly.
const UKey cKey_ToggleSuggestions = UKey(0x01200200);
const UKey cKey_ToggleLanguage = UKey(0x01200201);
const UKey cKey_SwitchToQwerty = UKey(0x01200202);
const UKey cKey_SwitchToAzerty = UKey(0x01200203);
const UKey cKey_SwitchToQwertz = UKey(0x01200204);
const UKey cKey_ShowXT9Regions = UKey(0x01200209);
const UKey cKey_CreateDefaultKeyboards = UKey(0x0120020D);
const UKey cKey_ClearDefaultKeyboards = UKey(0x0120020E);
const UKey cKey_Hide = UKey(0x0120020F);
const UKey cKey_ShowKeymapRegions = UKey(0x01200210);
const UKey cKey_StartStopRecording = UKey(0x01200211);
const UKey cKey_Resize_Tiny = UKey(0x01200212);
const UKey cKey_Resize_Small = UKey(0x01200213);
const UKey cKey_Resize_Default = UKey(0x01200214);
const UKey cKey_Resize_Large = UKey(0x01200215);
const UKey cKey_Resize_First = cKey_Resize_Tiny;
const UKey cKey_Resize_Last = cKey_Resize_Large;

const UKey cKey_ToggleSoundFeedback = UKey(0x01200216);
const UKey cKey_ResizeHandle = UKey(0x01200217);

// Keys that correspond to some text entry (label & entered text are equal)
const UKey cKey_DotCom = UKey(0x01200300);
const UKey cKey_DotOrg = UKey(0x01200301);
const UKey cKey_DotNet = UKey(0x01200302);
const UKey cKey_DotEdu = UKey(0x01200303);
const UKey cKey_DotGov = UKey(0x01200304);
const UKey cKey_DotCoUK = UKey(0x01200305);
const UKey cKey_DotDe = UKey(0x01200306);
const UKey cKey_DotFr = UKey(0x01200307);
const UKey cKey_DotUs = UKey(0x01200308);
const UKey cKey_WWW = UKey(0x01200309);
const UKey cKey_HTTPColonSlashSlash = UKey(0x0120030A);
const UKey cKey_HTTPSColonSlashSlash = UKey(0x0120030B);
const UKey cKey_Emoticon_Frown = UKey(0x0120030C);
const UKey cKey_Emoticon_Cry = UKey(0x0120030D);
const UKey cKey_Emoticon_Smile = UKey(0x0120030E);
const UKey cKey_Emoticon_Wink = UKey(0x0120030F);
const UKey cKey_Emoticon_Yuck = UKey(0x01200310);
const UKey cKey_Emoticon_Gasp = UKey(0x01200311);
const UKey cKey_Emoticon_Heart = UKey(0x01200312);
const UKey cKey_DotSe = UKey(0x01200313);

// Keys used for keyboard/language selections
const UKey cKey_KeyboardComboChoice_First = UKey(0x01200400);
const UKey cKey_KeyboardComboChoice_Last = UKey(0x012004ff);

// helper: can a UKey code be used as a Qt::Key and/or as an acceptable unicode character?
inline bool UKeyIsUnicodeQtKey(UKey ukey)		{ return ukey >= ' ' && ukey < Qt::Key_Escape; }
inline bool UKeyIsFunctionKey(UKey ukey)		{ return ukey >= Qt::Key_Escape; }
inline bool UKeyIsTextShortcutKey(UKey ukey)	{ return ukey >= 0x01200300 && ukey <= 0x012003FF; }
inline bool UKeyIsKeyboardComboKey(UKey ukey)	{ return ukey >= cKey_KeyboardComboChoice_First && ukey <= cKey_KeyboardComboChoice_Last;}
inline bool UKeyIsKeyboardSizeKey(UKey ukey)	{ return ukey >= cKey_Resize_First && ukey <= cKey_Resize_Last;}
inline bool UKeyIsEmoticonKey(UKey ukey)		{ return ukey >= cKey_Emoticon_Frown && ukey <= cKey_Emoticon_Heart; }
inline bool UKeyIsCharacter(UKey ukey)          { return (ukey >= Qt::Key_A && ukey <= Qt::Key_Z) || ukey == Qt::Key_Odiaeresis || ukey == Qt::Key_Adiaeresis || ukey == Qt::Key_Aring; }

const QPoint cOutside(-1, -1);		// special value meaning representing "outside of keyboard", or "no key".

class TabletKeymap : public Mapper_IF
{
public:
	enum {
		cKeymapRows = 5,
		cKeymapColumns = 12,
	};

	enum EShiftMode
	{
		eShiftMode_Undefined = -1,

		eShiftMode_Off = 0,
		eShiftMode_Once,
		eShiftMode_CapsLock
	};

	enum ESymbolMode
	{
		eSymbolMode_Undefined = -1,

		eSymbolMode_Off = 0,
		eSymbolMode_Lock
	};

	enum ELayoutPage {
		eLayoutPage_plain = 0,
		eLayoutPage_Alternate,

		eLayoutPageCount = 2
	};

	enum ETabAction {
		eTabAction_Tab = 0,
		eTabAction_Next,
		eTabAction_Previous
	};

	typedef const UKey constUKeyArray[];

	struct WKey {
		void set(UKey key, float weight = 1, const UKey * extended = NULL)
		{
			m_weight = weight, m_key = key, m_altkey = key, m_extended = extended;
		}
		void set(UKey key, UKey altkey, float weight = 1, const UKey * extended = NULL)
		{
			m_weight = weight, m_key = key, m_altkey = altkey, m_extended = extended;
		}
		void hide()
		{
			m_weight = 0, m_key = cKey_None, m_altkey = cKey_None, m_extended = NULL;
		}
		bool operator !=(const WKey & rhs)
		{
			return m_weight != rhs.m_weight || m_key != rhs.m_weight || m_altkey != rhs.m_altkey || m_extended != rhs.m_extended;
		}

		float			m_weight;
		UKey			m_key;
		UKey			m_altkey;
		const UKey *	m_extended;
	};

	typedef WKey		LayoutRow[cKeymapColumns];
	typedef LayoutRow	Layout[cKeymapRows];
	typedef float		HLimits[cKeymapRows][cKeymapColumns];
	typedef float		VLimits[cKeymapRows];

	struct LayoutFamily {

		LayoutFamily(const char * name, const char * defaultLanguage, uint16_t primaryID, uint16_t secondaryID,
						const char * symbolKeyLabel, const char * noLanguageKeyLabel,
						int tabX, int symbolX, int returnX, int returnY, bool needNumbLock, Layout & layout,
						LayoutRow & defaultBottomRow, LayoutRow & urlBottomRow, LayoutRow & emailBottomRow);

		const WKey &	wkey(int x, int y) const						{ return m_layout[y][x]; }
		UKey			key(int x, int y, ELayoutPage page) const		{ return (page == eLayoutPage_plain) ? wkey(x, y).m_key : wkey(x, y).m_altkey; }
		float			weight(int x, int y) const						{ return wkey(x, y).m_weight; }

		const char *	m_name;
		const char *	m_defaultLanguage;

		uint16_t		m_primaryID;
		uint16_t		m_secondaryID;

        const char *    m_symbolKeyLabel;
        const char *    m_noLanguageKeyLabel;

		int				m_tab_x;			// x of tab key on last row
		int				m_symbol_x;			// x of option key on last row

		int				m_return_x;			// x of return key
		int				m_return_y;			// y of return key

		bool			m_needNumLock;
		int				m_cachedGlyphsCount;

		Layout &		m_layout;
		LayoutRow &		m_defaultBottomRow;
		LayoutRow &		m_urlBottomRow;
		LayoutRow &		m_emailBottomRow;

		// self registration of layout families. Start with first, iterate until nextFamily is null.
		const LayoutFamily *		m_nextFamily;

		static const LayoutFamily * findLayoutFamily(const char * name, bool returnNullNotDefaultIfNotFound = true);

		static const LayoutFamily * s_firstFamily;

	};

	TabletKeymap();

	void				setRect(int x, int y, int w, int h)		{ m_rect.setRect(x, y, w, h); m_limitsDirty = true; }
	const QRect &		rect() const							{ return m_rect; }
	void				setRowHeight(int rowIndex, int height);

	QPoint				pointToKeyboard(const QPoint & location, bool useDiamondOptimizations = true);		// convert screen coordinate in keyboard coordinate
	int					keyboardToKeyZone(QPoint keyboardCoordinate, QRect & outZone);						// convert keyboard coordinate to rect of the key

	// The following functions that return a bool return true when the layout effectively changed (and you probably need to update your display)
	bool				setLayoutFamily(const LayoutFamily * layoutFamily);
	const LayoutFamily*	layoutFamily() const					{ return m_layoutFamily; }
	bool				setLanguageName(const std::string & name);		// if empty string, hide language key, otherwise show it.
	void				keyboardCombosChanged();						// called when available keyboard combos change
	QList<const char *>	getLayoutList();
	const char *		getLayoutDefaultLanguage(const char * layoutName);

	bool				setShiftMode(EShiftMode shiftMode);
	EShiftMode			shiftMode() const						{ return m_shiftMode; }
	bool				setSymbolMode(ESymbolMode symbolMode);
	ESymbolMode			symbolMode() const						{ return m_symbolMode; }
	bool				setShiftKeyDown(bool shiftKeyDown);
	bool				setSymbolKeyDown(bool symbolKeyDown);
	bool				setEditorState(const PalmIME::EditorState & editorState);
	const PalmIME::EditorState &	editorState() const			{ return m_editorState; }
	bool				setAutoCap(bool autoCap);

	bool				isSymbolActive() const					{ return ((m_symbolMode == eSymbolMode_Lock) ? 1 : 0) + (m_symbolDown ? 1 : 0) == 1; }
	bool				isShiftActive() const					{ return ((m_shiftMode == eShiftMode_Once) ? 1 : 0) + (m_shiftDown ? 1 : 0) == 1; }
	bool				isShiftDown() const						{ return m_shiftDown; }
	bool				isSymbolDown() const					{ return m_symbolDown; }
	bool				isCapsLocked() const					{ return m_shiftMode == eShiftMode_CapsLock; }
	bool				isCapActive() const						{ return (m_shiftDown && m_shiftMode == eShiftMode_Off) || (!m_shiftDown && m_shiftMode != eShiftMode_Off); }
	bool				isCapOrAutoCapActive() const			{ return m_autoCap || isCapActive(); }
	bool				isAutoCapActive() const					{ return m_autoCap; }

	UKey				map(QPoint p)							{ return map(p.x(), p.y()); }
	UKey				map(int x, int y);
	UKey				map(QPoint p, ELayoutPage page)			{ return map(p.x(), p.y(), page); }
	UKey				map(int x, int y, ELayoutPage page)		{ return isValidLocation(x, y) ? m_layoutFamily->key(x, y, page) : cKey_None; }
	quint32				getPage() const							{ return m_layoutPage; }

	ETabAction			tabAction() const;

	const char *		layoutName()							{ return m_layoutFamily->m_name; }

	uint16_t			primaryKeyboardID()						{ return m_layoutFamily->m_primaryID; }
	uint16_t			secondaryKeyboardID()					{ return m_layoutFamily->m_secondaryID; }

	bool				generateKeyboardLayout(const char * fullPath);
	std::string			getKeyboardLayoutAsJson();

	int					getCachedGlyphsCount() const			{ return m_layoutFamily->m_cachedGlyphsCount; }
	void				incCachedGlyphs()						{ const_cast<LayoutFamily *>(m_layoutFamily)->m_cachedGlyphsCount++; }
	void				resetCachedGlyphsCount()				{ if (m_layoutFamily->m_cachedGlyphsCount > 1) const_cast<LayoutFamily *>(m_layoutFamily)->m_cachedGlyphsCount = 1; }

	static inline bool	isValidLocation(int x, int y)			{ return x >= 0 && x < cKeymapColumns && y >= 0 && y < cKeymapRows; }
	static inline bool	isValidLocation(QPoint location)		{ return isValidLocation(location.x(), location.y()); }

	const UKey *		getExtendedChars(QPoint keyboardCoordinate);			// MAY RETURN NULL!
	QString				getKeyDisplayString(UKey key, bool logging = false);	// for display purposes. NOT ALL KEYS can be handled that way! "logging" gives you more...
	bool				showEmoticonsAsGraphics()				{ return m_editorState.flags & PalmIME::FieldFlags_Emoticons; }

	int					updateLimits();

	// Mapper_IF implementation
	std::string			pointToKeys(const QPoint & point);

private:
	EShiftMode			m_shiftMode;
	ESymbolMode			m_symbolMode;
	bool				m_shiftDown;
	bool				m_symbolDown;
	bool				m_autoCap;
	bool				m_numLock;							// in number & phone fields, when numbers need shift.
	PalmIME::EditorState m_editorState;
	const LayoutFamily * m_layoutFamily;
	ELayoutPage			m_layoutPage;
	QRect				m_rect;
	int					m_rowHeight[cKeymapRows];
	HLimits				m_hlimits;
	VLimits				m_vlimits;
	bool				m_limitsDirty;
	int					m_limitsVersion;

	QString				m_languageName;

	QString				m_custom_Enter;
	QString				m_localized__Enter;
	QString				m_localized__Tab;
	QString				m_localized__Next;
	QString				m_localized__Previous;

	bool				updateMapping();					// true if layout changed
	bool				updateLanguageKey(LayoutRow * bottomRow = NULL);
	QString				getLanguageDisplayName(const std::string & languageName, const LayoutFamily * layoutFamily);

	int					xCenterOfKey(int touchX, int x, int y, float weight);
	int					yCenterOfRow(int y, UKey key);

	QString				getXKeys(int locX, int k_x, int k_y);
};

const QPoint cResizeHandleCoord(TabletKeymap::cKeymapColumns, 0);

}; // namespace TabletKeyboard

#endif // TABLET_KEYMAP_H
