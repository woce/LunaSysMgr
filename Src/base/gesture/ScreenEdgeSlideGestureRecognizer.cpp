#include "Common.h"

#include "WindowServer.h"
#include "ScreenEdgeFlickGesture.h"
#include "ScreenEdgeSlideGestureRecognizer.h"

#include <QEvent>
#include <QTouchEvent>
#include <QTransform>
#include <QDebug>

ScreenEdgeSlideGestureRecognizer::ScreenEdgeSlideGestureRecognizer()
{
}

QGesture *ScreenEdgeSlideGestureRecognizer::create(QObject *target)
{
    return new ScreenEdgeSlideGesture;
}

QGestureRecognizer::Result ScreenEdgeSlideGestureRecognizer::recognize(QGesture *state,
                                                            QObject *,
                                                            QEvent *event)
{
    ScreenEdgeSlideGesture *q = static_cast<ScreenEdgeSlideGesture *>(state);
    const QTouchEvent *ev = static_cast<const QTouchEvent *>(event);
	const HostInfo& info = HostBase::instance()->getInfo();

	QPoint startPos;
	QPoint scenePos;
	QPoint displayBounds;

    QGestureRecognizer::Result result = QGestureRecognizer::CancelGesture;
    
    switch (event->type()) {
    case QEvent::TouchBegin:
		q->setFired(false);
		return result = QGestureRecognizer::MayBeGesture;
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd: {
        if (ev->touchPoints().size() == 1) {
			//Map the touchpoints to screen coordinates
			startPos =
				QPoint(ev->touchPoints().at(0).startScreenPos().x(),
				ev->touchPoints().at(0).startScreenPos().y());
			scenePos =
				QPoint(ev->touchPoints().at(0).screenPos().x(),
				ev->touchPoints().at(0).screenPos().y());
			displayBounds = QPoint(info.displayWidth, info.displayHeight);
				
			startPos = HostBase::instance()->map(startPos);
			scenePos = HostBase::instance()->map(scenePos);
			displayBounds = HostBase::instance()->map(displayBounds);
			
			QPoint offset;
			
			//I hate coordinate mapping.
			switch (WindowServer::instance()->getUiOrientation())
			{
				case OrientationEvent::Orientation_Up:
					break;
				case OrientationEvent::Orientation_Down:
					offset = QPoint(displayBounds.x(), displayBounds.y());
					startPos -= offset;
					scenePos -= offset;
					displayBounds = -displayBounds;
					break;
				case OrientationEvent::Orientation_Right:
					offset = QPoint(displayBounds.x(), 0);
					startPos -= offset;
					scenePos -= offset;
					displayBounds.setX(-displayBounds.x());
					break;
				case OrientationEvent::Orientation_Left:
					offset = QPoint(0, displayBounds.y());
					startPos -= offset;
					scenePos -= offset;
					displayBounds.setY(-displayBounds.y());
					break;
				default:
					break;
			}
			
			if(startPos.x() > kGestureBorderSize &&
			startPos.x() < displayBounds.x() - kGestureBorderSize &&
			startPos.y() < displayBounds.y() - kGestureBorderSize)
			{
				return result = QGestureRecognizer::Ignore;
			}
			else
			{
				QPoint delta = scenePos - startPos;
				
				if (q->getFired() == true)
					return result = QGestureRecognizer::Ignore;
					
				if (event->type() == QEvent::TouchUpdate) {
					if(startPos.x() <= kGestureBorderSize && delta.x() >= kGestureTriggerDistance && abs(delta.x()) >= abs(delta.y()))
					{
						qCritical() << abs(delta.x()) << abs(delta.y());
						q->setEdge(Left);
						q->setFired(true);
						result = QGestureRecognizer::FinishGesture;
					}
					if(startPos.x() >= displayBounds.x() - kGestureBorderSize && delta.x() <= -kGestureTriggerDistance && abs(delta.x()) >= abs(delta.y()))
					{
						qCritical() << abs(delta.x()) << abs(delta.y());
						q->setEdge(Right);
						q->setFired(true);
						result = QGestureRecognizer::FinishGesture;
					}
					if(startPos.y() >= displayBounds.y() - kGestureBorderSize && delta.y() <= -kGestureTriggerDistance && abs(delta.y()) >= abs(delta.x()))
					{
						qCritical() << abs(delta.x()) << abs(delta.y());
						q->setEdge(Bottom);
						q->setFired(true);
						result = QGestureRecognizer::FinishGesture;
					}
				}
			}
        }
        else if (ev->touchPoints().size() > 1)
        {
            result = QGestureRecognizer::CancelGesture;
        }
        break;
    }
    default:
        result = QGestureRecognizer::Ignore;
        break;
    }
    return result;
}