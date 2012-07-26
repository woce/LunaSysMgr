#include "Common.h"

#include "WindowServer.h"
#include "CardSwitchGestureRecognizer.h"

#include <QEvent>
#include <QTouchEvent>
#include <QTransform>
#include <QDebug>

CardSwitchGestureRecognizer::CardSwitchGestureRecognizer()
{
}

QGesture *CardSwitchGestureRecognizer::create(QObject *target)
{
    return new CardSwitchGesture;
}

QGestureRecognizer::Result CardSwitchGestureRecognizer::recognize(QGesture *state,
                                                            QObject *,
                                                            QEvent *event)
{
    CardSwitchGesture *q = static_cast<CardSwitchGesture *>(state);
    const QTouchEvent *ev = static_cast<const QTouchEvent *>(event);
	const HostInfo& info = HostBase::instance()->getInfo();

	QPoint startPos;
	QPoint scenePos;
	QPoint displayBounds;

    QGestureRecognizer::Result result;
    
    switch (event->type()) {
		case QEvent::TouchBegin:
            q->setFlick(0);
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
				startPos.x() < displayBounds.x() - kGestureBorderSize)
				{
					result = QGestureRecognizer::Ignore;
				}
				else
				{
					QPoint delta = scenePos - startPos;
                    QPoint diff;
						
					if (event->type() == QEvent::TouchUpdate) {
						if(startPos.x() <= kGestureBorderSize)
						{
                            result = QGestureRecognizer::MayBeGesture;
                            if(delta.x() >= kGestureTriggerDistance && abs(delta.x()) >= abs(delta.y()))
                            {
                                q->setLastPos(q->pos());
                                q->setPos(ev->touchPoints()[0].pos());
                                result = QGestureRecognizer::TriggerGesture;
                            }
						}
						if(startPos.x() >= displayBounds.x() - kGestureBorderSize)
						{
                            result = QGestureRecognizer::MayBeGesture;
                            if(delta.x() <= -kGestureTriggerDistance && abs(delta.x()) >= abs(delta.y()))
                            {
                                q->setLastPos(q->pos());
                                q->setPos(ev->touchPoints()[0].pos());
                                result = QGestureRecognizer::TriggerGesture;
                            }
						}
                        diff = q->pos().toPoint() - q->lastPos().toPoint();
                        
                        if(diff.x() >= 25 && diff.x() <= 100)
                            q->setFlick(1);
                        else if(diff.x() < 5 && diff.x() > -5)
                            q->setFlick(0);
                        else if(diff.x() <= -25 && diff.x() >= -100)
                            q->setFlick(-1);
                        
					} else if (event->type() == QEvent::TouchEnd) {
						result = QGestureRecognizer::FinishGesture;
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
			break;
    }
    return result;
}