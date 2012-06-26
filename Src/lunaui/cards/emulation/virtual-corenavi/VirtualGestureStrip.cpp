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





#include <QApplication>
#include <QGestureEvent>
#include <QTimer>
#include <QtDebug>
#include <QGraphicsView>

#include "Event.h"
#include "VirtualGestureStrip.h"
#include "CustomEvents.h"
#include "FlickGesture.h"
#include "Utils.h"


#define DRAG_THRESHOLD     8
#define GESTURE_THRESHOLD  70
#define GESTURE_FADE_SPEED 9
#define TAP_THRESHOLD      5


VirtualGestureStrip::VirtualGestureStrip(int width, int height)
	: QGraphicsObject()
	, m_gestureStart(0)
	, m_gestureEnd(0)
{
	m_bounds = QRect(-width/2, -height/2, width, height);
	m_seenGesture = false;

	setFlag(QGraphicsItem::ItemIsFocusable, false);
	grabGesture((Qt::GestureType) SysMgrGestureFlick);

	m_quickLaunch = false;
	m_quickLaunchFire.setInterval(350);
	m_quickLaunchFire.setSingleShot(true);
	connect(&m_quickLaunchFire, SIGNAL(timeout()), SLOT(slotQuickLaunchGesture()));

	m_gestureDragTimer.setInterval(16); // 60 Hz
	m_gestureDragTimer.setSingleShot(false);
	connect(&m_gestureDragTimer, SIGNAL(timeout()), SLOT(slotGestureDragTimer()));
}

VirtualGestureStrip::~VirtualGestureStrip()
{    
}

void VirtualGestureStrip::relayout(const QRectF& bounds)
{
	prepareGeometryChange();
	m_bounds.setRect(-bounds.width()/2, -m_bounds.height()/2, bounds.width(), m_bounds.height());

	setPosTopLeft(this, 0, bounds.bottom());

	m_quickLaunchFire.stop();
	m_quickLaunch = false;
	m_seenGesture = false;
}

bool VirtualGestureStrip::sceneEvent(QEvent* e) 
{
	QGestureEvent* gevt = static_cast<QGestureEvent*>(e);

	if (e->type() == QEvent::GestureOverride) {
		if(gevt->gesture((Qt::GestureType) SysMgrGestureFlick)) {
			gevt->accept();
			return true;
		}
	} else if (e->type() == QEvent::Gesture) {
		if (QGesture* gesture = gevt->gesture((Qt::GestureType) SysMgrGestureFlick)) {
			if (gesture->state() == Qt::GestureFinished) {
				FlickGesture* fg = static_cast<FlickGesture*>(gesture);
				
				flickGesture(fg->velocity(), m_currentMousePos);
			}
			return true;
		}
	}

	return QGraphicsObject::sceneEvent(e);
}

void VirtualGestureStrip::flickGesture(const QPoint& velocity, const QPoint& startPos) 
{
	if ((velocity.y() * velocity.y()) > (velocity.x() * velocity.x())) {

		if (velocity.y() > 0)
			postGesture(Qt::Key_CoreNavi_SwipeDown, startPos, velocity);
		else
			postGesture(Qt::Key_CoreNavi_Launcher, startPos, velocity);
	}
}


void VirtualGestureStrip::mousePressEvent(QGraphicsSceneMouseEvent* event) 
{
	m_mouseDownPos = event->pos().toPoint();
	m_currentMousePos = event->pos().toPoint();
	m_quickLaunchFire.stop();
	m_quickLaunch = false;
	m_seenGesture = false;
		
	m_gestureStart = m_mouseDownPos.x();
	m_gestureEnd   = m_mouseDownPos.x();

	fingerDown(m_mouseDownPos.x());
}

void VirtualGestureStrip::mouseMoveEvent(QGraphicsSceneMouseEvent* event) 
{
	m_currentMousePos = event->pos().toPoint();
#ifndef MACHINE_CHILE
	if (m_currentMousePos.y() < m_bounds.y()) {
		if (!m_quickLaunch && !m_quickLaunchFire.isActive())
			m_quickLaunchFire.start();
	} else {
		m_quickLaunchFire.stop();
	}
#endif
	m_gestureEnd = m_currentMousePos.x();

	int diff = m_gestureEnd - m_gestureStart;
	if(diff < 0) diff = -diff;

	if (diff > DRAG_THRESHOLD) {
		if(!m_gestureDragTimer.isActive()) {
			m_gestureDragTimer.start();
		}	
		fingerDrag(m_gestureStart, m_gestureEnd);
	} else {
		if(m_gestureDragTimer.isActive())
			m_gestureDragTimer.stop();
		fingerMove(m_currentMousePos.x());
	}	
}

void VirtualGestureStrip::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) 
{
	m_quickLaunchFire.stop();
	m_quickLaunch = false;

	m_gestureDragTimer.stop();

	if (m_mouseDownPos.isNull() || m_seenGesture) {
		m_mouseDownPos = QPoint();
		m_currentMousePos = QPoint();
		m_seenGesture = false;
		fingerUp(event->pos().toPoint().x());
		return;
	}

	int x = event->pos().x();
	int y = event->pos().y();

	x = CLAMP(x, m_bounds.x(), m_bounds.x() + m_bounds.width());
	y = CLAMP(y, m_bounds.y(), m_bounds.y() + m_bounds.height());

	int deltaX = x - m_mouseDownPos.x();
	int deltaY = y - m_mouseDownPos.y();

	if ((deltaX * deltaX + deltaY * deltaY) > (TAP_THRESHOLD * TAP_THRESHOLD)) {

		if (deltaX * deltaX > deltaY * deltaY) {
			// Horizontal movement
			
			// check if the gesture is still within the valid time bounds
			int diff = m_gestureEnd - m_gestureStart;
			if(diff < 0) diff = -diff;
		
			if((diff > DRAG_THRESHOLD) && ((deltaX * deltaX) >= (GESTURE_THRESHOLD * GESTURE_THRESHOLD))) {
				if (deltaX > 0) {
					if (deltaX > boundingRect().width()/2) {

						//If next is a disallowed gesture, then fall back on
						//menu.  Note that if menu is still disallowed then
						//postGesture() will catch it; this simply ensures that
						//there is a fall-back on the gesture, because otherwise
						//a swipe that's a little too long will simply be lost

						if (shouldPerformGesture(Qt::Key_CoreNavi_Next)) {
							postGesture(Qt::Key_CoreNavi_Next, event->pos().toPoint(), QPoint()); // FIXME: calculate gesture velocity
						} else {
							postGesture(Qt::Key_CoreNavi_Menu, event->pos().toPoint(), QPoint()); // FIXME: calculate gesture velocity
						}
					}
					else {
						postGesture(Qt::Key_CoreNavi_Menu, event->pos().toPoint(), QPoint()); // FIXME: calculate gesture velocity
					}
				}
				else {
					if (-deltaX > boundingRect().width()/2) {

						//If previous is a disallowed gesture, then fall back
						//on the back gesture.  Note that if back is still disallowed
						//then postGesture() will catch it; this simply ensures that
						//there is a fall-back on the gesture, because otherwise a
						//swipe that's a little too long will simply be lost.

						if (shouldPerformGesture(Qt::Key_CoreNavi_Previous)) {
							postGesture(Qt::Key_CoreNavi_Previous, event->pos().toPoint(), QPoint()); // FIXME: calculate gesture velocity
						} else {
							postGesture(Qt::Key_CoreNavi_Back, event->pos().toPoint(), QPoint()); // FIXME: calculate gesture velocity
						}
					}
					else {
						postGesture(Qt::Key_CoreNavi_Back, event->pos().toPoint(), QPoint()); // FIXME: calculate gesture velocity
					}
				}
			}
		}
	} else {
		if (m_tapGestureKey) {
			postGesture(m_tapGestureKey, event->pos().toPoint(), QPoint());
		}
	}
	
	fingerUp(event->pos().toPoint().x());

	m_mouseDownPos = QPoint();
	m_currentMousePos = QPoint();
	m_seenGesture = false;
}

void VirtualGestureStrip::slotGestureDragTimer()
{
	if(m_gestureEnd >= m_gestureStart) {
		m_gestureStart += GESTURE_FADE_SPEED;
		if(m_gestureStart > m_gestureEnd)
			m_gestureStart = m_gestureEnd;
	} else {
		m_gestureStart -= GESTURE_FADE_SPEED;
		if(m_gestureStart < m_gestureEnd)
			m_gestureStart = m_gestureEnd;
	}
	
	int diff = m_gestureEnd - m_gestureStart;
	if(diff < 0) diff = -diff;
	
	if (diff > DRAG_THRESHOLD) {		
		fingerDrag(m_gestureStart, m_gestureEnd);
	} else {
		fingerMove(m_gestureEnd);
		m_gestureDragTimer.stop();
	}
	
}


void VirtualGestureStrip::postGesture(Qt::Key key, QPoint startPos, QPoint velocity) 
{
	if (shouldPerformGesture(key)) {
		QWidget* window = QApplication::focusWidget();
		if (window) {
			QApplication::postEvent(window, new QKeyEvent(QEvent::KeyPress, key,
														  Qt::NoModifier));
			QApplication::postEvent(window, new QKeyEvent(QEvent::KeyRelease, key,
														  Qt::NoModifier));
		}

		m_seenGesture = true;
		m_gestureDragTimer.stop();

		gesturePerformed(key, startPos, velocity);
	} else {
		if (m_tapGestureKey) {
			postGesture(m_tapGestureKey, startPos, velocity);
		}
	}
}

void VirtualGestureStrip::postMouseUpdate(QPoint pos) 
{

	QWidget* window = QApplication::focusWidget();
	if (window) {
		QApplication::sendEvent(window, new QMouseEvent(QEvent::MouseMove, pos, Qt::LeftButton, Qt::LeftButton,
													  Qt::NoModifier));
	}
}

void VirtualGestureStrip::slotQuickLaunchGesture() 
{
	int x = m_currentMousePos.x();
	int y = m_currentMousePos.y();

	x = CLAMP(x, m_bounds.x(), m_bounds.x() + m_bounds.width());

	int deltaX = x - m_mouseDownPos.x();
	int deltaY = y - m_mouseDownPos.y();
	
	printf("VirtualGestureStrip::slotQuickLaunchGesture: dX = %d, dY = %d\r\n", deltaX, deltaY);

	if (deltaY * deltaY > deltaX * deltaX) {
		// Vertical movement, so fire the quicklaunch gesture
		m_quickLaunch = true;

		postGesture(Qt::Key_CoreNavi_QuickLaunch, m_currentMousePos, QPoint());
		postMouseUpdate(m_currentMousePos);
	}
}

void VirtualGestureStrip::setAcceptedKeys(QList<Qt::Key> keys) {
	m_acceptedKeys = keys;
}


bool VirtualGestureStrip::shouldPerformGesture(Qt::Key key) {
	if (m_acceptedKeys.isEmpty()){
		return false;
	}
	for (int i=0; i<m_acceptedKeys.length(); i++) {
		if (m_acceptedKeys[i] == key) return true;
	}

	return false;
}

void VirtualGestureStrip::setTapGesture(Qt::Key key) {
	m_tapGestureKey = key;

	//The tap gesture should be an accepted gesture.
	if (!shouldPerformGesture(m_tapGestureKey)) {
		m_acceptedKeys.append(key);
	}
}

