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




#include "StatusBarVersion.h"
#include "StatusBar.h"
#include "Settings.h"
#include "Preferences.h"
#include "Localization.h"

#include <QPainter>
#include <glib.h>

#define TEXT_BASELINE_OFFSET            (-1)

static const QString currentVersion = "WebOS Ports LunaCE 4.9.10";

StatusBarVersion::StatusBarVersion(unsigned int padding)
	: m_font(0)
	, m_textPadding(padding)
{

	// Set up text
	const char* fontName = Settings::LunaSettings()->fontStatusBar.c_str();
	m_font = new QFont(fontName, 14);
	m_font->setPixelSize(14);
	
	m_font->setLetterSpacing(QFont::PercentageSpacing, kStatusBarQtLetterSpacing);

	if (m_font) {
		m_font->setBold(true);
	}

	//Figure out & set bounds
	QFontMetrics fontMetrics(*m_font);
	m_textRect = fontMetrics.boundingRect(currentVersion);
	
	m_bounds = QRect(-m_textRect.width()/2 - m_textPadding, -m_textRect.height()/2,
			         m_textRect.width() + m_textPadding * 2, m_textRect.height());
}

StatusBarVersion::~StatusBarVersion()
{
	delete m_font;
}

QRectF StatusBarVersion::boundingRect() const
{
	return m_bounds;
}

void StatusBarVersion::setPadding( unsigned int padding)
{
	m_textPadding = padding;
}

void StatusBarVersion::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QPen oldPen = painter->pen();

	QFont origFont = painter->font();
	painter->setFont(*m_font);

	QFontMetrics fontMetrics(*m_font);

	int baseLine = m_textRect.height()/2 - fontMetrics.descent() + TEXT_BASELINE_OFFSET;

	// paint the text
	painter->setPen(QColor(0xFF, 0xFF, 0xFF, 0xFF));
	
	// Draw text
	painter->drawText(QPointF(-m_textRect.width()/2, baseLine), currentVersion);

	painter->setPen(oldPen);
	painter->setFont(origFont);

}
