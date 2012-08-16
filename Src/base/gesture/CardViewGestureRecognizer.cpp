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
	QPoint delta;
	QPoint diff;

    QGestureRecognizer::Result result = QGestureRecognizer::Ignore;
    
    switch (event->type()) {
		case QEvent::TouchBegin:
        case QEvent::TouchUpdate:
		case QEvent::TouchEnd: {
			if(event->type() == QEvent::TouchBegin)
			{
				//Reset
				q->setFlick(0);
				q->setFired(false);
			}
			
			//Break if there are more than 1 fingers
			if(ev->touchPoints().size() > 1)
			{
				result = QGestureRecognizer::FinishGesture;
				break;
			}
			
			//Get the various coordinates
			displayBounds = QPoint(info.displayWidth/2, info.displayHeight/2);
			startPos =
				QPoint(ev->touchPoints().at(0).startPos().x(),
				ev->touchPoints().at(0).startPos().y()) - displayBounds;
			pos =
				QPoint(ev->touchPoints().at(0).pos().x(),
				ev->touchPoints().at(0).pos().y()) - displayBounds;
			
			//Map them to the screen orientation
			displayBounds = HostBase::instance()->map(displayBounds);
			startPos = HostBase::instance()->map(startPos);
			pos = HostBase::instance()->map(pos);
			
			//Make sure displayBounds is positive
			displayBounds.setX(abs(displayBounds.x()));
			displayBounds.setY(abs(displayBounds.y()));
			
			//Add displayBounds to startPos and pos to put them in normal coord space
			startPos += displayBounds;
			pos += displayBounds;
			displayBounds *= 2;
			
			q->setLastPos(q->pos());
			q->setPos(pos);
			
			delta = pos - startPos;
			diff = pos - q->lastPos().toPoint();
			
			//Ignore the gesture unless if it's outside the gesture border
			if(startPos.y() < displayBounds.y() - kGestureBorderSize)
				break;
			
			//Ignore the gesture if it's not vertical
			if(abs(delta.x()) > abs(delta.y()))
				break;
			
			//Is the user's finger moving fast enough for a flick?
			if(diff.y() > -5 && diff.y() < 5)
			{
				q->setFlick(0);
			}
			else if(diff.y() >= 25)
			{
				q->setFlick(1);
			}
			else if(diff.y() <= -25)
			{
				q->setFlick(-1);
			}
			
			//Do we?
			if(delta.y() <= 0 && event->type() == QEvent::TouchUpdate)
			{
				q->setFired(true);
				result = QGestureRecognizer::TriggerGesture;
			}
			
			if(event->type() == QEvent::TouchEnd)
				result = QGestureRecognizer::FinishGesture;
			break;
		}
		default:
			result = QGestureRecognizer::Ignore;
			break;
    }
    return result;
}