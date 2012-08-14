#include "Common.h"

#include "WindowServer.h"
#include "CardViewGestureRecognizer.h"

#include <QEvent>
#include <QTouchEvent>
#include <QTransform>
#include <QDebug>

CardViewGestureRecognizer::CardViewGestureRecognizer()
{
}

QGesture *CardViewGestureRecognizer::create(QObject *target)
{
    return new CardViewGesture;
}

QGestureRecognizer::Result CardViewGestureRecognizer::recognize(QGesture *state,
                                                            QObject *,
                                                            QEvent *event)
{
    CardViewGesture *q = static_cast<CardViewGesture *>(state);
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
				
				if(startPos.y() < displayBounds.y() - kGestureBorderSize)
				{
					result = QGestureRecognizer::Ignore;
				}
				else
				{
					QPoint delta = pos - startPos;
						
					if (event->type() == QEvent::TouchUpdate) {
						if(startPos.y() >= displayBounds.y() - kGestureBorderSize)
						{
                            result = QGestureRecognizer::MayBeGesture;
                            if(delta.y() <= -kGestureTriggerDistance)
                            {
                                q->setLastPos(q->pos());
                                q->setPos(pos);
                                q->setEdge(true);
                                result = QGestureRecognizer::TriggerGesture;
                            }
						}
                        
                        QPoint diff = q->pos().toPoint() - q->lastPos().toPoint();
                        
                        if(diff.y() > -5 || diff.y() < 5)
                            q->setFlick(0);
                        else if(diff.y() >= 25 && diff.y() <= 100)
                            q->setFlick(1);
                        else if(diff.y() <= -25 && diff.y() >= -100)
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