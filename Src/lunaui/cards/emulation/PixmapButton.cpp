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




#include "Common.h"

#include <glib.h>
#include <string>

#include "PixmapButton.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QGraphicsObject>

PixmapButton::PixmapButton (const QPixmap& off, const QPixmap& pressed)
{
	m_isToggleButton = false;
	m_offPixmap = off;
	m_pressedPixmap = pressed;	
	m_active = false;
	m_pressed = false;
	
	prepareGeometryChange();
	m_bounds = QRectF(0, 0, pressed.width(), pressed.height());
	
	setPixmap(m_offPixmap);
	m_pExpandedHitTarget = 0;
}

// constructor for toggle button
PixmapButton::PixmapButton (QPixmap& off, QPixmap pressed, QPixmap& on)
{
	m_isToggleButton = true;
	m_onPixmap = on;
	m_offPixmap = off;
	m_pressedPixmap = pressed;
	m_active = false;
	m_pressed = false;
	
	prepareGeometryChange();
	
	m_bounds = QRectF(0, 0, pressed.width(), pressed.height());

	setPixmap(m_offPixmap);

	m_pExpandedHitTarget = 0;
}

PixmapButton::PixmapButton(const QPixmap& composite,const QRect& offCoords,const QRect& pressedCoords)
{
	m_isToggleButton = true;
	m_offPixmap = composite.copy(offCoords);
	m_pressedPixmap = composite.copy(pressedCoords);
	m_active = false;
	m_pressed = false;

	prepareGeometryChange();

	m_bounds = QRectF(0, 0, m_pressedPixmap.width(), m_pressedPixmap.height());

	setPixmap(m_offPixmap);

	m_pExpandedHitTarget = 0;
}

PixmapButton::~PixmapButton()
{
	if (m_pExpandedHitTarget)
		delete m_pExpandedHitTarget;
}

void PixmapButton::setActive(bool active) 
{ 
	if(m_active == active)
		return;
	
	toggle(); 
}

void PixmapButton::toggle()
{
	if(m_active) {
		m_active = false;
		
		if(!m_pressed)
			setPixmap(m_offPixmap);
	} else {
		m_active = true;
		
		if(!m_pressed)
			setPixmap(m_onPixmap);
	}
}

void PixmapButton::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if(!isVisible())
		return;
	m_pressed = true;
	setPixmap(m_pressedPixmap);
	update();
	Q_EMIT clickOn();
}

void PixmapButton::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	if(!isVisible())
		return;

	if(m_pressed && !isUnderMouse()) {
		m_pressed = false;
		
		if(m_active) 
			setPixmap(m_onPixmap);
		else
			setPixmap(m_offPixmap);
		
		update();
		ungrabMouse();
		Q_EMIT clickComplete();
	}
}

void PixmapButton::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	if(!isVisible())
		return;
	
	const bool emitComplete = m_pressed;		//m_pressed will be false if a moveEvent happened before

	m_pressed = false;
	if(m_active) 
		setPixmap(m_onPixmap);
	else
		setPixmap(m_offPixmap);

	update();
	if (emitComplete) {
		Q_EMIT clickComplete();
	}
}

//virtual
void PixmapButton::createLargerHitTargetRect(quint32 width,quint32 height)
{
	if (m_pExpandedHitTarget)
		return;
	m_pExpandedHitTarget = new PixmapButtonHitTarget(QRectF(-(qreal)width/2.0,-(qreal)height/2.0,(qreal)width,(qreal)height),this);
	m_pExpandedHitTarget->setParentItem(this);
	m_pExpandedHitTarget->setPos(boundingRect().center());
	m_pExpandedHitTarget->setVisible(true);
}

void PixmapButton::removeLargerHitTargetRect(quint32 width,quint32 height)
{
	if (m_pExpandedHitTarget)
	{
		delete m_pExpandedHitTarget;
		m_pExpandedHitTarget = 0;
	}
}

void PixmapButton::enableLargerHitTargetRect()
{
	if (m_pExpandedHitTarget)
	m_pExpandedHitTarget->setVisible(true);
}

void PixmapButton::disableLargerHitTargetRect()
{
	if (m_pExpandedHitTarget)
	m_pExpandedHitTarget->setVisible(false);
}

//reimplemented trivially so it can be a slot
void PixmapButton::setVisible(bool visible)
{
	QGraphicsPixmapItem::setVisible(visible);
	if (m_pExpandedHitTarget)
		m_pExpandedHitTarget->setVisible(visible);
}

void PixmapButton::show()
{
	setVisible(true);
}

void PixmapButton::hide()
{
	setVisible(false);
}

void PixmapButton::setManualMode(bool manualModeEnabled) {
	if (manualModeEnabled) {
		ungrabMouse();
		if (m_pExpandedHitTarget) {
			m_pExpandedHitTarget->ungrabMouse();
		}
	} else {
		if (m_pExpandedHitTarget) {
			m_pExpandedHitTarget->grabMouse();
		} else {
			grabMouse();
		}
	}
}

void PixmapButton::simulateClick() {
	//This class never actually uses the contents of the event.
	QGraphicsSceneMouseEvent evt;
	mousePressEvent(&evt);
	mouseReleaseEvent(&evt);
}

PixmapButtonHitTarget::PixmapButtonHitTarget(const QRectF& hitRect,PixmapButton * pButton)
: QObject()
, QGraphicsItem(pButton)
, m_boundingRect(hitRect)
, m_qpButton(pButton)
{
	setFlag(QGraphicsItem::ItemHasNoContents);
}

//virtual
void PixmapButtonHitTarget::paint(QPainter* painter, const QStyleOptionGraphicsItem* options, QWidget* widget)
{
	//DEBUG - uncomment to draw red outline on bound rect (i.e. the effective hit area)
	painter->save();
	painter->setPen(Qt::red);
	painter->drawRect(m_boundingRect);
	painter->restore();
}

#include <QDebug>

void PixmapButtonHitTarget::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (m_qpButton.isNull())
	{
		event->ignore();
		return;
	}
	PixmapButton
	ungrabMouse();
	m_qpButton->grabMouse();
	m_qpButton->mousePressEvent(event);		//careful if PixmapButton's mousePressEvent()
											//ever needs to consider event's coordinates - they'll be in the wrong
											//coordinate space

}


void PixmapButtonHitTarget::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	//should never get this
	event->ignore();
}

void PixmapButtonHitTarget::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	//should never get this
	event->ignore();
}

