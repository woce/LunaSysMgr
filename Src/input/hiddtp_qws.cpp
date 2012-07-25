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

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <linux/input.h>
#include <QApplication>
#include <QDateTime>
#include <QWSServer>
#include <QInputEvent>
#include <QTouchEvent>
#include <QWidget>

#include <QtGui/qmouse_qws.h>
#include "hiddtp_qws.h"
#include "HidLib.h"
#include "Time.h"
#include "Preferences.h"
#include "Settings.h"
#include "HostBase.h"

// One page worth of events (4096/16)
#define MAX_HIDD_EVENTS 256

extern void qt_translateRawTouchEvent(QWidget *window, QTouchEvent::DeviceType deviceType, const QList<
									  QTouchEvent::TouchPoint> &touchPoints);

extern bool qt_sendSpontaneousEvent(QObject *receiver, QEvent *event);

QWSHiddTpHandler::QWSHiddTpHandler(const QString &driver, const QString &device) :
	QWSMouseHandler(driver, device) {
	d = new QWSHiddTpHandlerPrivate(this, device);
}

QWSHiddTpHandler::~QWSHiddTpHandler() {
	delete d;
}

void QWSHiddTpHandler::suspend() {
	d->suspend();
}

void QWSHiddTpHandler::resume() {
	d->resume();
}

void QWSHiddTpHandler::pumpHiddData(struct input_event* inputEvents, int numEvents)
{
	/*
	 *TODO: fix the unit test support
	 */
	/* d->parseHiddData(inputEvents, numEvents); */
}

void QWSHiddTpHandler::startRecording()
{
	d->startRecording();    
}

void QWSHiddTpHandler::stopRecording()
{
    d->stopRecording();
}


QWSHiddTpHandlerPrivate::QWSHiddTpHandlerPrivate(QWSHiddTpHandler *h, const QString &device)
	: handler(h)
	, m_mousePressTime(0)
	, m_halPenHandle(0)
	, m_penFd(0)
	, m_metaActiveTouchesCount(0)
	, m_penCancelPoint (0,0)
	, m_sendPenCancel (false)
	, m_isSuspended (false)
	, m_recordFile(0)
{

	flickGesture = new FlickGesture;
	m_screenEdgeFlickGesture = new ScreenEdgeFlickGesture;

	HostBase* host = HostBase::instance();
	const HostInfo& info = host->getInfo();
	m_deviceWidth = info.displayWidth;
	m_deviceHeight = info.displayHeight;

	/* initial HAL support */
	/* fine-tuning  support for HAL */
	InputControl* ic = host->getInputControlTouchpanel();
	if (ic)
	{
		m_halPenHandle = ic->getHandle();
		if (m_halPenHandle)
        	{
			hal_error_t error = HAL_ERROR_SUCCESS;
			error = hal_device_get_event_source(m_halPenHandle, &m_penFd);

			if (error != HAL_ERROR_SUCCESS)
			{
				g_critical("Unable to obtain touchpanel event_source");
				return;
			}

			GIOChannel* ioChannel = g_io_channel_unix_new(m_penFd);
			GSource* ioSource = g_io_create_watch(ioChannel, (GIOCondition) G_IO_IN);
			g_source_set_callback(ioSource, (GSourceFunc) ioCallback, this, NULL);
			g_source_attach(ioSource, g_main_context_default());
		}
	}
}


QWSHiddTpHandlerPrivate::~QWSHiddTpHandlerPrivate() {
	if (m_penFd >= 0)
		close(m_penFd);
}

void QWSHiddTpHandlerPrivate::suspend() {
	m_isSuspended = true;
	m_sendPenCancel = true;
	m_penCancelPoint = m_lastTouchDown;
	QList<QWSHiddTpHandlerPrivate::HiddTouch> noTouches;
	updateTouchEvents (noTouches);
}

void QWSHiddTpHandlerPrivate::resume() {
	m_isSuspended = false;
}

Qt::Key QWSHiddTpHandlerPrivate::lookupGesture(uint16_t value) {

#if defined(MACHINE_TOPAZ) || defined(MACHINE_OPAL)
	// to send only flick and home events
	switch (value) {
	case FLICK:
		return Qt::Key_Flick;
	case HOME:
		return Qt::Key_CoreNavi_Home;
	default:
		return Qt::Key_unknown;
	}
#else	
	switch (value) {
	case BACK:
		return Qt::Key_CoreNavi_Back;
	case MENU:
		return Qt::Key_CoreNavi_Menu;
	case QUICK_LAUNCH:
		return Qt::Key_CoreNavi_QuickLaunch;
	case LAUNCHER:
		return Qt::Key_CoreNavi_Launcher;
	case NEXT:
		return Qt::Key_CoreNavi_Next;
	case PREV:
		return Qt::Key_CoreNavi_Previous;
	case FLICK:
		return Qt::Key_Flick;
	case DOWN:
		return Qt::Key_CoreNavi_SwipeDown;
	case HOME:
		return Qt::Key_CoreNavi_Home;
	default:
		return Qt::Key_unknown;
	}
#endif	
}

void QWSHiddTpHandlerPrivate::readHiddData() {
	/* initial HAL support */
	/* fine-tuning  support for HAL */

	hal_error_t error = HAL_ERROR_SUCCESS;
	hal_event_handle_t event_handle = NULL;
	hal_touchpanel_event_item_t* touches;
	int count = 0;

	if (m_halPenHandle == NULL) return;

	while ((error = hal_device_get_event(m_halPenHandle, &event_handle)) == HAL_ERROR_SUCCESS && event_handle != NULL)
	{

		error = hal_touchpanel_event_get_touches(event_handle, &touches, &count);
		if (error != HAL_ERROR_SUCCESS)
		{
			g_critical("Unable to obtain m_halPenHandle event touches");
			return;
		}

		QList<HiddTouch> hiddTouches;
		HiddTouch* currentTouch = 0;
    
		for (int j=0; j<count; j++)
		{
			hal_touchpanel_event_item_t* touch = touches++;
    
			hiddTouches.append(HiddTouch());
			currentTouch = &hiddTouches[hiddTouches.size()-1];
    
			currentTouch->hiddId = touch->finger;
    
			timeval time;
			time.tv_sec = touch->timestamp/1000000000LL;
			time.tv_usec = (touch->timestamp%1000000000LL)/1000;
    
			currentTouch->time = time;
			currentTouch->x = touch->x;
			currentTouch->y = touch->y;
			currentTouch->gestureKey = lookupGesture(touch->gestureKey);
    
			if (currentTouch->gestureKey == Qt::Key_Flick || currentTouch->gestureKey == Qt::Key_CoreNavi_SwipeDown) 
			{
				currentTouch->xVelocity = touch->xVelocity;
				currentTouch->yVelocity = touch->yVelocity;
			}
    
			if (HAL_TOUCHPANEL_STATE_DOWN == touch->state)
			{
				currentTouch->state = FingerDown;
			}
			else if (HAL_TOUCHPANEL_STATE_UP == touch->state)
			{
				currentTouch->state = FingerUp;
			}
		}
    
		hal_device_release_event(m_halPenHandle, event_handle);
		if (error != HAL_ERROR_SUCCESS)
		{
			g_critical("Unable to release event_handle event");
			return;
		}
		event_handle = NULL;

		// this marks the end of a TP scan, generate events now
		// this function will update current state of touches and
		// generate a touch event when appropriate
		updateTouchEvents(hiddTouches);
		hiddTouches.clear();
		currentTouch = 0;
	}
}

bool QWSHiddTpHandlerPrivate::updateTouchEvents(QList<QWSHiddTpHandlerPrivate::HiddTouch>& hiddTouches) 
{
	bool triggerTouchEvent = false;

	for (QList<HiddTouch>::iterator it = m_touches.begin(); it != m_touches.end(); ++it) {
		it->seenInScan = false;
	}

	for (int i = 0; i < hiddTouches.size(); ++i) {
		
		int oldMetaActiveTouches = m_metaActiveTouchesCount;
	    HiddTouch& touch = hiddTouches[i];
	    if (touch.state == FingerDown) {
			addNewTouch(touch);
			triggerTouchEvent = true;
	    } else {
			if (updateOldTouch(touch))
				triggerTouchEvent = true;
	    }

	    if (m_metaActiveTouchesCount && m_metaActiveTouchesCount != oldMetaActiveTouches && !oldMetaActiveTouches) {
			QWSServer::processKeyEvent(-1, Qt::Key_CoreNavi_Meta, Qt::MetaModifier, true, false);
			HostBase::instance()->setMetaModifier(true);
		} else if (oldMetaActiveTouches && !m_metaActiveTouchesCount) {
			QWSServer::processKeyEvent(-1, Qt::Key_CoreNavi_Meta, Qt::NoModifier, false, false);
			HostBase::instance()->setMetaModifier(false);
		}

	    if (touch.gestureKey != Qt::Key_unknown) {
			if (touch.gestureKey != Qt::Key_Flick) {
				// printf ("Sending Gesture Event %d\n", touch.gestureKey);
				QWSServer::processKeyEvent(-1, touch.gestureKey, Qt::NoModifier, true, false);
				QWSServer::processKeyEvent(-1, touch.gestureKey, Qt::NoModifier, false, false);
			}
			if (touch.gestureKey == Qt::Key_Flick
#if !defined(TARGET_DESKTOP)
		   || touch.gestureKey == Qt::Key_CoreNavi_SwipeDown
#endif
			) {

				m_screenEdgeFlickGesture->m_edge = ScreenEdgeFlickGesture::EdgeUnknown;
				
				if (Preferences::instance()->sysUiEnableNextPrevGestures()) {

					const int INVALID_COORD = 0xFFFFFFFF;
					int xDown = INVALID_COORD;
					int yDown = INVALID_COORD;

					// finding the old touch to update
					for (QList<HiddTouch>::iterator it = m_touches.begin(); it != m_touches.end(); ++it) {
						if (it->hiddId == touch.hiddId) {
							xDown = it->xDown;
							yDown = it->yDown;						
							break;
						}
					}

					if (xDown != INVALID_COORD && yDown != INVALID_COORD) {

						const int borderWidth = 25;
						const int minimumYLength = 25;

						if ((xDown < borderWidth) &&
						    (abs(touch.x) > minimumYLength) &&
							(touch.xVelocity > 0) &&
							(abs(touch.xVelocity) > abs(touch.yVelocity))) {
							printf("flick on left border. distance: %d, yDist = %d\n", abs(touch.x), touch.x - xDown);
							if((touch.x - xDown) >= minimumYLength) {
								m_screenEdgeFlickGesture->m_edge = ScreenEdgeFlickGesture::EdgeLeft;
								m_screenEdgeFlickGesture->m_yDistance = (touch.x - xDown);
							} else {
								printf("Rejected Flick\n");
							}
						}
						else if ((xDown > (m_deviceWidth - borderWidth)) &&
     							 (abs(touch.x - m_deviceWidth) > minimumYLength) &&
								 (touch.xVelocity < 0) &&
								 (abs(touch.xVelocity) > abs(touch.yVelocity))) {
							printf("flick on right border. distance: %d, yDist = %d\n", abs(touch.x - m_deviceWidth), xDown - touch.x);
							if((xDown - touch.x) >= minimumYLength){
								m_screenEdgeFlickGesture->m_edge = ScreenEdgeFlickGesture::EdgeRight;
								m_screenEdgeFlickGesture->m_yDistance = (xDown - touch.x) ;
							} else {
								printf("Rejected Flick\n");
							}
						}
						else if ((yDown < borderWidth) &&
    							 (abs(touch.y) > minimumYLength) &&
								 (touch.yVelocity > 0) &&
								 (abs(touch.yVelocity) > abs(touch.xVelocity))) {
							printf("flick on top border. distance: %d, yDist = %d\n", abs(touch.y), touch.y - yDown);
							if((touch.y - yDown) >= minimumYLength) {
								m_screenEdgeFlickGesture->m_edge = ScreenEdgeFlickGesture::EdgeTop;
								m_screenEdgeFlickGesture->m_yDistance = (touch.y - yDown);
							} else {
								printf("Rejected Flick\n");
							}
						}
						else if ((yDown > (m_deviceHeight - borderWidth)) &&
    							 (abs(touch.y - m_deviceHeight) > minimumYLength) &&
								 (touch.yVelocity < 0) &&
								 (abs(touch.yVelocity) > abs(touch.xVelocity))) {
							printf("flick on bottom border. distance: %d, yDist = %d\n", abs(touch.y - m_deviceHeight), yDown - touch.y);
							if((yDown - touch.y) >= minimumYLength) {
								m_screenEdgeFlickGesture->m_edge = ScreenEdgeFlickGesture::EdgeBottom;
								m_screenEdgeFlickGesture->m_yDistance = (yDown - touch.y);
							} else {
								printf("Rejected Flick\n");
							}
						}								  
					}

					if (m_screenEdgeFlickGesture->m_edge != ScreenEdgeFlickGesture::EdgeUnknown) {

						QList<QGesture *> gestureStartedList;
						QList<QGesture *> gestureFinishedList;

						m_screenEdgeFlickGesture->setHotSpot(m_lastTouchDown);
					
						m_screenEdgeFlickGesture->setState(Qt::GestureStarted);
						gestureStartedList.append(m_screenEdgeFlickGesture);

						// determine which widget this event will go to
						QWidget* window = QApplication::topLevelAt(m_lastTouchDown);
						if (window) {
							QWidget* widget = window->childAt(window->mapFromGlobal(m_lastTouchDown));
							if (widget) {

								QGestureEvent gestureStartedEvent(gestureStartedList);
								QApplication::sendEvent((QObject*) widget, &gestureStartedEvent);

								m_screenEdgeFlickGesture->setState(Qt::GestureFinished);
								gestureFinishedList.append(m_screenEdgeFlickGesture);

								QGestureEvent gestureFinishedEvent(gestureFinishedList);
								QApplication::sendEvent((QObject*) widget, &gestureFinishedEvent);
							}
						}
					}
				}

				if (m_screenEdgeFlickGesture->m_edge == ScreenEdgeFlickGesture::EdgeUnknown) {

					QList<QGesture *> gestureStartedList;
					QList<QGesture *> gestureFinishedList;

					flickGesture->m_endPos = QPoint(touch.x, touch.y);
					flickGesture->m_velocity = HostBase::instance()->map(QPoint(touch.xVelocity, touch.yVelocity));
					flickGesture->setHotSpot(m_lastTouchDown);
					
					flickGesture->setState(Qt::GestureStarted);
					gestureStartedList.append(flickGesture);

					// determine which widget this event will go to
					QWidget* window = QApplication::topLevelAt(m_lastTouchDown);
					if (window) {
						QWidget* widget = window->childAt(window->mapFromGlobal(m_lastTouchDown));
						if (widget) {

							QGestureEvent gestureStartedEvent(gestureStartedList);
							QApplication::sendEvent((QObject*) widget, &gestureStartedEvent);

							flickGesture->setState(Qt::GestureFinished);
							gestureFinishedList.append(flickGesture);

							QGestureEvent gestureFinishedEvent(gestureFinishedList);
							QApplication::sendEvent((QObject*) widget, &gestureFinishedEvent);
						}
					}
				}
			}
	    }
	}

	for (QList<HiddTouch>::iterator it = m_touches.begin(); it != m_touches.end(); ++it) {
		if (!it->seenInScan) {
			g_critical("%s: Finger %lld went missing. lifting it up",
					  __PRETTY_FUNCTION__, it->hiddId);
			// FIXME: this should be FingerCancel
			it->state = FingerUp;
			if (it->isMetaTouch) {
				--m_metaActiveTouchesCount;			
				if (m_metaActiveTouchesCount == 0) {
					QWSServer::processKeyEvent(-1, Qt::Key_CoreNavi_Meta, Qt::NoModifier, false, false);
					HostBase::instance()->setMetaModifier(false);
				}
			}
			triggerTouchEvent = true;
		}
	}
	
	if (triggerTouchEvent) {
		generateTouchEvent();
		removeReleasedTouches();
		return true;
	}

	return false;
}

// addNewTouch - called when there is a new finger down.
// this makes the new finger the primary and unmarks the existing primary finger
// the new touch is inserted at head since we want to keep the list in order of last finger down.
void QWSHiddTpHandlerPrivate::addNewTouch(QWSHiddTpHandlerPrivate::HiddTouch& touch) {

	touch.isPrimary = true;
	touch.state = QWSHiddTpHandlerPrivate::FingerDown;
	touch.seenInScan = true;
	if (m_touches.empty())
		touch.id = 0;
	else
		touch.id = m_touches.first().id + 1;

	touch.xDown = touch.x;
	touch.yDown = touch.y;

	QList<HiddTouch>::iterator it;
	for (it = m_touches.begin(); it != m_touches.end(); ++it) {
		if (it->isPrimary) {
			it->isPrimary = false;
			m_sendPenCancel = true;
			m_penCancelPoint = QPoint (it->x, it->y);
		}
	}

	if (touch.y > m_deviceHeight) {
		touch.isMetaTouch = true;
		m_metaActiveTouchesCount++;
	}

	m_touches.insert(m_touches.begin(), touch); // inserting latest touch at head

	// cache the last primary finger down
	m_lastTouchDown = QPoint(touch.x, touch.y);
}

// updateOldTouch - called for all hidd events that are not finger down
// this updates state and position of the touches and returns true if a new touch event has to be sent
bool QWSHiddTpHandlerPrivate::updateOldTouch(QWSHiddTpHandlerPrivate::HiddTouch& touch) {
	bool updated = false;
	QList<HiddTouch>::iterator it;

	// finding the old touch to update
	for (it = m_touches.begin(); it != m_touches.end(); ++it) {
		if (it->hiddId == touch.hiddId)
			break;
	}

	if (it == m_touches.end()) {
		g_debug ("did not find existing touch to update, ignoring the touch");
		return false;
	}	

	it->seenInScan = true;
	it->time = touch.time;

	if (touch.state == FingerUp) {
		it->state = touch.state;
		updated = true;
	}
	else if (squareDistance((*it), touch) > 0) {
		it->state = FingerMove;
		it->x = touch.x;
		it->y = touch.y;
		updated = true;
	}
	else {
		it->state = FingerNoMove;
	}

	// updates do not modify id as this is assigned in addNewTouch
	// updates do not modify isPrimary as this is marked in addNewTouch and removeReleasedTouches

	if (it->isMetaTouch && (it->state == FingerUp || it->y <= m_deviceHeight)) {
		it->isMetaTouch = false;
		m_metaActiveTouchesCount--;
	}

	return updated;
}

void QWSHiddTpHandlerPrivate::removeReleasedTouches() {
	bool markNewPrimary = false;

	QList<HiddTouch>::iterator it = m_touches.begin();
	while (it != m_touches.end()) {
		if (it->state == FingerUp) {
			if (it->isPrimary)
				markNewPrimary = true;
			it = m_touches.erase(it); // dont increment as erase will return the next item
		} else {
			++it;
		}
	}

	// if the primary touch is released, the last finger down is the new primary
	if (markNewPrimary && m_touches.size() != 0)
		m_touches.first().isPrimary = true;
}

// generateTouchEvent - iterates through existing touches and creates a QTouchEvent.
// For the primary finger, it also creates a mouse event if the location has move
void QWSHiddTpHandlerPrivate::generateTouchEvent()
{
	if (m_touches.empty())
		return;
	
	QList<QTouchEvent::TouchPoint> touchPoints;
	QList<HiddTouch>::const_iterator it;

	QWidget* widget = QWidget::mouseGrabber();
	if (!widget) {
		QWidget* window = QApplication::topLevelAt(m_lastTouchDown);
		if (window)
			widget = window->childAt(window->mapFromGlobal(m_lastTouchDown));
	}

	if (!widget) {
		QPoint dummyPt(10, 10);
		QWidget* window = QApplication::topLevelAt(dummyPt);
		if (window)
			widget = window->childAt(window->mapFromGlobal(dummyPt));
	}

	Qt::KeyboardModifiers keyboardModifiers = QApplication::keyboardModifiers();

	if (widget && m_sendPenCancel) {
		//printf("Mouse Up for Pen Cancel: %d, %d\n", m_penCancelPoint.x(), m_penCancelPoint.y());
		QMouseEvent ev(QEvent::MouseButtonRelease, m_penCancelPoint, m_penCancelPoint, true,
				Qt::LeftButton, Qt::NoButton, keyboardModifiers);
		qt_sendSpontaneousEvent((QObject*) widget, &ev);							
		m_sendPenCancel = false;
		m_penCancelPoint = QPoint (0,0);
	}
	
	for (it = m_touches.begin(); it != m_touches.end(); ++it) {
		QTouchEvent::TouchPoint touchPoint;
		touchPoint.setId(it->id);
		touchPoint.setPos(QPoint(it->x, it->y));

		touchPoint.setScreenPos(touchPoint.pos());
		switch (it->state) {
		case QWSHiddTpHandlerPrivate::FingerDown:
			touchPoint.setState(Qt::TouchPointPressed);
			break;
		case QWSHiddTpHandlerPrivate::FingerUp:
			touchPoint.setState(Qt::TouchPointReleased);
			break;
		case QWSHiddTpHandlerPrivate::FingerMove:
			touchPoint.setState(Qt::TouchPointMoved);
			break;
		default:
			touchPoint.setState(Qt::TouchPointStationary);
			break;
		}

		if (it->isPrimary) {
			touchPoint.setState(touchPoint.state() | Qt::TouchPointPrimary);
		}

		touchPoints.append(touchPoint);
//		printf ("%s: adding touch point id %d (hiddId %llu) for pos (%d, %d) primary %d\n", 
//			__func__, it->id, it->hiddId, it->x, it->y, it->isPrimary);

		if (it->isPrimary) {

			QPoint mousePos = QPoint(it->x, it->y);
			if (widget) {
				
				if (it->state == QWSHiddTpHandlerPrivate::FingerDown) {
					uint32_t currTime = Time::curTimeMs();
					int dx = mousePos.x() - m_mousePress.x();
					int dy = mousePos.y() - m_mousePress.y();
					
					if (((currTime - m_mousePressTime) < (uint32_t) QApplication::doubleClickInterval()) &&
						((dx * dx + dy * dy) <= Settings::LunaSettings()->tapRadiusSquared)) {

						//printf("Mouse Double Click: %d, %d\n", mousePos.x(), mousePos.y());
						QMouseEvent ev(QEvent::MouseButtonDblClick, mousePos, mousePos,
									   Qt::LeftButton, Qt::LeftButton, keyboardModifiers);
						qt_sendSpontaneousEvent((QObject*) widget, &ev);

						m_mousePressTime = 0;
					}
					else {

						//printf("Mouse Down: %d, %d\n", mousePos.x(), mousePos.y());
						QMouseEvent ev(QEvent::MouseButtonPress, mousePos, mousePos,
									   Qt::LeftButton, Qt::LeftButton, keyboardModifiers);
						qt_sendSpontaneousEvent((QObject*) widget, &ev);

						m_mousePress = mousePos;
						m_mousePressTime = currTime;
					}
				} else if (it->state == QWSHiddTpHandlerPrivate::FingerMove) {
					//printf("Mouse Move: %d, %d\n", mousePos.x(), mousePos.y());
					QMouseEvent ev(QEvent::MouseMove, mousePos, mousePos,
								   Qt::LeftButton, Qt::LeftButton, keyboardModifiers);
					qt_sendSpontaneousEvent((QObject*) widget, &ev);							
				}
			}
		}
	}
	
	//printf ("sending touch event\n");
	qt_translateRawTouchEvent(QApplication::activeWindow(), QTouchEvent::TouchScreen, touchPoints);

	for (it = m_touches.begin(); it != m_touches.end(); ++it) {

		if (it->isPrimary) {

			QPoint mousePos = QPoint(it->x, it->y);
			if (widget) {
			
				if (it->state == QWSHiddTpHandlerPrivate::FingerUp) {
					//printf("Mouse Up: %d, %d\n", mousePos.x(), mousePos.y());
					QMouseEvent ev(QEvent::MouseButtonRelease, mousePos, mousePos,
								   Qt::LeftButton, Qt::NoButton, keyboardModifiers);
					qt_sendSpontaneousEvent((QObject*) widget, &ev);							
				}
			}
		}
	}
}

gboolean QWSHiddTpHandlerPrivate::ioCallback(GIOChannel* channel, GIOCondition condition, gpointer arg)
{
    QWSHiddTpHandlerPrivate* d = (QWSHiddTpHandlerPrivate*) arg;
	d->readHiddData();

	return TRUE;
}

void QWSHiddTpHandlerPrivate::startRecording()
{
	if (m_recordFile) {
		fclose(m_recordFile);
		m_recordFile = 0;
	}

	QDateTime now = QDateTime::currentDateTime();
	QString nowString = now.toString(Qt::ISODate);

	char fileName[128];
	sprintf(fileName, "/var/log/sysmgr-tp.%s", nowString.toUtf8().data());

	m_recordFile = fopen(fileName, "w");
	fprintf(m_recordFile, "Start Recording....\n");
	fprintf(m_recordFile, "Initial State: \n");
	fprintf(m_recordFile, "Num touches: %d, Num Meta touches: %d\n", m_touches.size(), m_metaActiveTouchesCount);
			
	QList<HiddTouch>::iterator it;
	int index = 0;
	for (it = m_touches.begin(); it != m_touches.end(); ++it) {
		fprintf(m_recordFile, "Touch %d: hiddId: %lld, id: %d, x: %d, y: %d, state: %d, isPrimary: %d, isMetaTouch: %d\n",
				index++, it->hiddId, it->id, it->x, it->y, it->state, it->isPrimary, it->isMetaTouch);
    }
	fprintf(m_recordFile, "\n");
}

void QWSHiddTpHandlerPrivate::stopRecording()
{
	if (!m_recordFile)
		return;

	fprintf(m_recordFile, "Final State: \n");
	fprintf(m_recordFile, "Num touches: %d, Num Meta touches: %d\n", m_touches.size(), m_metaActiveTouchesCount);
			
	QList<HiddTouch>::iterator it;
	int index = 0;
	for (it = m_touches.begin(); it != m_touches.end(); ++it) {
		fprintf(m_recordFile, "Touch %d: hiddId: %lld, id: %d, x: %d, y: %d, state: %d, isPrimary: %d, isMetaTouch: %d\n",
				index++, it->hiddId, it->id, it->x, it->y, it->state, it->isPrimary, it->isMetaTouch);
    }
	fprintf(m_recordFile, "Stopped Recording....\n");
	
	fclose(m_recordFile);
	m_recordFile = 0;
}
