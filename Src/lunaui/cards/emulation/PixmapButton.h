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




#ifndef PIXMAPBUTTON_H
#define PIXMAPBUTTON_H

#include "Common.h"
#include <qglobal.h>
#include <QGraphicsPixmapItem>
#include <QObject>
#include <QPointer>

class PixmapButton;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

class PixmapButtonHitTarget : public QObject , public QGraphicsItem
{
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)

public:
	PixmapButtonHitTarget(const QRectF& hitRect,PixmapButton * pButton);

	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);
	QRectF boundingRect() const { return m_boundingRect; }

	virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

private:
	QRectF m_boundingRect;
	QPointer<PixmapButton>	m_qpButton;
};

class PixmapButton : public QObject, public QGraphicsPixmapItem {
	Q_OBJECT
public:
	// constructor for press button
	PixmapButton (const QPixmap& off, const QPixmap& pressed);
	
	//alternate constructor for press button
	// (where the QPixmap 'composite' contains both states. the qrect's
	// indicate where in the pixmap they are stored)
	PixmapButton (const QPixmap& composite,const QRect& offCoords,const QRect& pressedCoords);

	// constructor for toggle button
	PixmapButton (QPixmap& off, QPixmap pressed, QPixmap& on);
	
	virtual ~PixmapButton();

	bool isToggleButton() { return m_isToggleButton; }
	void setActive(bool active);
	bool isActive() { return m_active; }
	void toggle();
	
	virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

	virtual void createLargerHitTargetRect(quint32 width,quint32 height);
	virtual void removeLargerHitTargetRect(quint32 width,quint32 height);
	virtual void enableLargerHitTargetRect();
	virtual void disableLargerHitTargetRect();
	virtual void setManualMode(bool manualModeEnabled);
	virtual void simulateClick();

Q_SIGNALS:

	void clickOn();			//emitted when a click is activated on the button (mousePressEvent) but not yet released.
							//If a move happens (mouseMoveEvent) off of the item w/o a release, then this will be the only signal
	void clickComplete();	//emitted after a click is released (mouseReleaseEvent)

public Q_SLOTS:
	void setVisible(bool visible);
	void show();
	void hide();
private:
	QPixmap m_onPixmap, m_offPixmap, m_pressedPixmap;
	bool m_pressed;
	bool m_isToggleButton;
	bool m_active;
	QRectF m_bounds;
	PixmapButtonHitTarget * m_pExpandedHitTarget;
};


#endif /* PIXMAPBUTTON_H */
