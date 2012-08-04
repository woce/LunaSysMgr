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
	QPoint screenPos;
	QPoint displayBounds;

    QGestureRecognizer::Result result = QGestureRecognizer::CancelGesture;
    
    switch (event->type()) {
    case QEvent::TouchBegin:
		q->setFired(false);
		return result = QGestureRecognizer::MayBeGesture;
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd: {
		//Only trigger for single touches
        if (ev->touchPoints().size() == 1) {
			//Get the start & current positions of our touch
			startPos =
				QPoint(ev->touchPoints().at(0).startScreenPos().x(),
				ev->touchPoints().at(0).startScreenPos().y());
			screenPos =
				QPoint(ev->touchPoints().at(0).screenPos().x(),
				ev->touchPoints().at(0).screenPos().y());
			displayBounds = QPoint(info.displayWidth, info.displayHeight);
			
			//Map to the current orientation
			startPos = HostBase::instance()->map(startPos);
			screenPos = HostBase::instance()->map(screenPos);
			displayBounds = HostBase::instance()->map(displayBounds);
			
			//Fix up the output of map() into positive coordinates
			//(I hate coordinate mapping)
			QPoint offset;
			switch (WindowServer::instance()->getUiOrientation())
			{
				case OrientationEvent::Orientation_Up:
					break;
				case OrientationEvent::Orientation_Down:
					offset = QPoint(displayBounds.x(), displayBounds.y());
					startPos -= offset;
					screenPos -= offset;
					displayBounds = -displayBounds;
					break;
				case OrientationEvent::Orientation_Right:
					offset = QPoint(displayBounds.x(), 0);
					startPos -= offset;
					screenPos -= offset;
					displayBounds.setX(-displayBounds.x());
					break;
				case OrientationEvent::Orientation_Left:
					offset = QPoint(0, displayBounds.y());
					startPos -= offset;
					screenPos -= offset;
					displayBounds.setY(-displayBounds.y());
					break;
				default:
					break;
			}
			
			//If the start position is outsize the gesture border, ignore
			if(startPos.x() > kGestureBorderSize &&
			startPos.x() < displayBounds.x() - kGestureBorderSize &&
			startPos.y() < displayBounds.y() - kGestureBorderSize)
			{
				return result = QGestureRecognizer::Ignore;
			}
			else
			{
				QPoint delta = screenPos - startPos;
				
				//Only fire the gesture once
				if (q->getFired() == true)
					return result = QGestureRecognizer::Ignore;
				
				//Figure out what edge we're on and output some values
				if (event->type() == QEvent::TouchUpdate) {
					if(delta.x() >= kGestureTriggerDistance && abs(delta.x()) >= abs(delta.y()))
					{
						qCritical() << abs(delta.x()) << abs(delta.y());
						q->setEdge(Left);
						q->setFired(true);
						result = QGestureRecognizer::FinishGesture;
					}
					if(delta.x() <= -kGestureTriggerDistance && abs(delta.x()) >= abs(delta.y()))
					{
						qCritical() << abs(delta.x()) << abs(delta.y());
						q->setEdge(Right);
						q->setFired(true);
						result = QGestureRecognizer::FinishGesture;
					}
					if(delta.y() <= -kGestureTriggerDistance && abs(delta.y()) >= abs(delta.x()))
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