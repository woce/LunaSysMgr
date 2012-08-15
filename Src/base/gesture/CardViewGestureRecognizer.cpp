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

    QGestureRecognizer::Result result = QGestureRecognizer::CancelGesture;
    
    switch (event->type()) {
		case QEvent::TouchBegin:
			//Reset Flick
            q->setFlick(0);
			result = QGestureRecognizer::MayBeGesture;
			break;
        case QEvent::TouchUpdate: {
			//Return if there are != 1 fingers
			if(ev->touchPoints().size() != 1)
				return result;
			
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
			
			//Ignore the gesture unless if it's outside the gesture border
			if(startPos.y() < displayBounds.y() - kGestureBorderSize)
			{
				result = QGestureRecognizer::Ignore;
				break;
			}
			
			QPoint delta = pos - startPos;
			
			QPoint diff = q->pos().toPoint() - q->lastPos().toPoint();
			
			if(abs(delta.x()) > abs(delta.y()))
				break;
			
			if(diff.y() > -5 || diff.y() < 5)
			{
				q->setFlick(0);
			}
			else if(diff.y() <= -25 && diff.y() >= -100)
			{
				q->setFlick(-1);
			}
			
			if(delta.y() <= 0 && delta.y() < delta.x())
			{
				q->setLastPos(q->pos());
				q->setPos(pos);
				q->setEdge(true);
				result = QGestureRecognizer::TriggerGesture;
			}
			break;
		}
		case QEvent::TouchEnd:
			result = QGestureRecognizer::FinishGesture;
			break;
		default:
			result = QGestureRecognizer::Ignore;
			break;
    }
    return result;
}