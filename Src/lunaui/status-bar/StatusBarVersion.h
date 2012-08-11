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




#ifndef STATUSBARVERSION_H
#define STATUSBARVERSION_H

#include "StatusBarItem.h"
#include <QGraphicsObject>
#include <QTextLayout>

class StatusBarVersion : public StatusBarItem
{
	Q_OBJECT

public:
	StatusBarVersion(unsigned int padding = 0);
	virtual ~StatusBarVersion();

	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

	void setPadding( unsigned int padding);

private:
	QRectF m_textRect;
	QFont* m_font;
	
	char* m_curVersionStr;
	unsigned int m_textPadding;
};



#endif /* STATUSBARCLOCK_H */
