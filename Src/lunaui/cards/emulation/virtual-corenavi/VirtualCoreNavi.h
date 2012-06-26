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




#ifndef VIRTUALCORENAVI_H
#define VIRTUALCORENAVI_H

#include <QGraphicsObject>
#include <QMouseEvent>
#include <QTimer>

#include "VirtualGestureStrip.h"
#include "GestureFeedbackItem.h"

class VirtualCoreNavi : public VirtualGestureStrip
{
	Q_OBJECT
	Q_PROPERTY(qreal lightBarBrightness READ lightBarBrightness WRITE setLightBarBrightness)

public:
	VirtualCoreNavi(int width, int height);
	virtual ~VirtualCoreNavi();
	
	qreal lightBarBrightness() const { return m_lightBarBrightness; }
	void setLightBarBrightness(const qreal brightness);


	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

protected:
	virtual void fingerDown(int xPos);
	virtual void fingerMove(int xPos);
	virtual void fingerDrag(int xStart, int xEnd);
	virtual void fingerUp(int xPos);
	virtual void gesturePerformed(Qt::Key key, QPoint startPos, QPoint velocity);

private:
	GestureFeedbackItem m_feedbackItem;
	
	qreal m_lightBarBrightness;
	QPropertyAnimation m_barGlowAnimation;
	
	QPixmap m_pixBarDarkLeft, m_pixBarDarkCenter, m_pixBarDarkRight;
	QPixmap m_pixBarBrightLeft, m_pixBarBrightCenter, m_pixBarBrightRight;
};


#endif /* VIRTUALCORENAVI_H */
