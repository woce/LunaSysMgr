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
	QPoint pos;
	QPoint displayBounds;

    QGestureRecognizer::Result result;
    
    switch (event->type()) {
		case QEvent::TouchBegin:
            q->setFlick(0);
        case QEvent::TouchUpdate:
		case QEvent::TouchEnd: {
			if (ev->touchPoints().size() == 1) {
				//Map the touchpoints to screen coordinates
				displayBounds = QPoint(info.displayWidth/2, info.displayHeight/2);
				startPos =
					QPoint(ev->touchPoints().at(0).startPos().x(),
					ev->touchPoints().at(0).startPos().y()) - displayBounds;
				pos =
					QPoint(ev->touchPoints().at(0).pos().x(),
					ev->touchPoints().at(0).pos().y()) - displayBounds;
                
				displayBounds = HostBase::instance()->map(displayBounds);
				startPos = HostBase::instance()->map(startPos);
				pos = HostBase::instance()->map(pos);
                
                //Make sure displayBounds is positive
                displayBounds.setX(abs(displayBounds.x()));
                displayBounds.setY(abs(displayBounds.y()));
                
                startPos += displayBounds;
                pos += displayBounds;
                displayBounds *= 2;
				
				if(startPos.x() > kGestureBorderSize &&
				startPos.x() < displayBounds.x() - kGestureBorderSize)
				{
					result = QGestureRecognizer::Ignore;				}
				else
				{
					QPoint delta = pos - startPos;
						
					if (event->type() == QEvent::TouchUpdate) {
						if(startPos.x() <= kGestureBorderSize)
						{
                            result = QGestureRecognizer::MayBeGesture;
                            if(delta.x() >= kGestureTriggerDistance)
                            {
                                q->setLastPos(q->pos());
                                q->setPos(pos);
                                q->setEdge(false);
                                result = QGestureRecognizer::TriggerGesture;

                            }
						}
						else if(startPos.x() >= displayBounds.x() - kGestureBorderSize)
						{
                            result = QGestureRecognizer::MayBeGesture;
                            if(delta.x() <= -kGestureTriggerDistance)
                            {
                                q->setLastPos(q->pos());
                                q->setPos(pos);
                                q->setEdge(true);
                                result = QGestureRecognizer::TriggerGesture;
                            }
						}
                        
                        QPoint diff = q->pos().toPoint() - q->lastPos().toPoint();
                        
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