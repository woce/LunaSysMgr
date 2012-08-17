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

	//Positions + DisplayBounds for mapping assistance
	QPoint startPos;
	QPoint pos;
	QPoint displayBounds;

	//Result to return
	QGestureRecognizer::Result result;
    
    switch (event->type()) {
	case QEvent::TouchBegin:
		//Reset the gesture
		q->setFlick(0);
		q->setFired(false);
	case QEvent::TouchUpdate:
	case QEvent::TouchEnd: {
		//Only trigger if it's a single-finger gesture
		if (ev->touchPoints().size() == 1) {
			//Get the display size & screen coordinates
			displayBounds = QPoint(info.displayWidth/2, info.displayHeight/2);
			startPos =
				QPoint(ev->touchPoints().at(0).startPos().x(),
				ev->touchPoints().at(0).startPos().y()) - displayBounds;
			pos =
				QPoint(ev->touchPoints().at(0).pos().x(),
				ev->touchPoints().at(0).pos().y()) - displayBounds;

			//Rotate the coordinates to our current orientation
			displayBounds = HostBase::instance()->map(displayBounds);
			startPos = HostBase::instance()->map(startPos);
			pos = HostBase::instance()->map(pos);
                
			//Make sure displayBounds is positive
			displayBounds.setX(abs(displayBounds.x()));
			displayBounds.setY(abs(displayBounds.y()));
                	
                	//Shift positions & display bounds into screen coordinates
			startPos += displayBounds;
			pos += displayBounds;
			displayBounds *= 2;
			
			//If the position is outside the gesture border, ignore
			if(startPos.x() > kGestureBorderSize && startPos.x() < displayBounds.x() - kGestureBorderSize)
			{
				result = QGestureRecognizer::Ignore;
			}
			else
			{
				//Work out the distance traveled
				QPoint delta = pos - startPos;
					
				if (event->type() == QEvent::TouchUpdate) {
					//Work out the distance traveled since the last frame
					QPoint diff = q->pos().toPoint() - q->lastPos().toPoint();
					if(abs(diff.x()) > abs(diff.y()))
					{
						//If the finger's moving fast enough, set flick to 1 (right), -1 (left) or 0 (no flick)
						if(diff.x() >= 25 && diff.x() <= 100)
							q->setFlick(1);
						else if(diff.x() < 5 && diff.x() > -5)
							q->setFlick(0);
						else if(diff.x() <= -25 && diff.x() >= -100)
							q->setFlick(-1);
					}
					
					//Left border
					if(startPos.x() <= kGestureBorderSize)
					{
						//If the finger has moved in a horizontal direction
						if(delta.x() >= 0 && delta.x() > delta.y())
						{
							//Set variables and trigger gesture
							q->setLastPos(q->pos());
							q->setPos(pos);
							q->setEdge(false);
							q->setFired(true);
							result = QGestureRecognizer::TriggerGesture;
						}
					}
					//Right border
					else if(startPos.x() >= displayBounds.x() - kGestureBorderSize)
					{
						//If the finger has moved in a horizontal direction
						if(delta.x() <= 0 && delta.x() < delta.y())
						{
							//Set variables and trigger gesture
							q->setLastPos(q->pos());
							q->setPos(pos);
							q->setEdge(true);
							q->setFired(true);
							result = QGestureRecognizer::TriggerGesture;
						}
					}
                        
				}
				else if (event->type() == QEvent::TouchEnd)
				{
					//Still counts as 'fired' since we want the event to get through
					q->setFired(true);
					result = QGestureRecognizer::FinishGesture;
				}
			}
		}
		else
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