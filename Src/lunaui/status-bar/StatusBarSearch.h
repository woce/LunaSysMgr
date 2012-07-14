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




#ifndef STATUSBARSEARCH_H
#define STATUSBARSEARCH_H

#include "StatusBarItem.h"
#include <cjson/json.h>

#include <QGraphicsObject>

class StatusBarSearch : public StatusBarItem
{
	Q_OBJECT

public:
	StatusBarSearch();
	~StatusBarSearch();

	int width() const { return m_bounds.width(); }
	int height() const { return m_bounds.height(); }
	
	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

private:
	QPixmap m_pixmap;
};



#endif /* StatusBarSearch_H */
