#include "Common.h"

#include "DragGestureRecognizer.h"

#include <QEvent>
#include <QTouchEvent>
#include <QTransform>
#include <QDebug>

#include "DragGesture.h"

#ifndef QT_NO_GESTURES

QT_BEGIN_NAMESPACE

DragGestureRecognizer::DragGestureRecognizer()
{
}

QGesture *DragGestureRecognizer::create(QObject *target)
{
    return new DragGesture;
}

QGestureRecognizer::Result DragGestureRecognizer::recognize(QGesture *state,
                                                            QObject *,
                                                            QEvent *event)
{
    DragGesture *q = static_cast<DragGesture *>(state);

    const QTouchEvent *ev = static_cast<const QTouchEvent *>(event);

    QGestureRecognizer::Result result = QGestureRecognizer::CancelGesture;

    switch (event->type()) {
    case QEvent::TouchBegin: {
        q->setHotSpot(ev->touchPoints().at(0).screenPos());
        result = QGestureRecognizer::MayBeGesture;
        break;
    }
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd: {
        if (ev->touchPoints().size() == 1) {
            QTouchEvent::TouchPoint p = ev->touchPoints().at(0);
        	q->setHotSpot(ev->touchPoints().at(0).screenPos());
            QPoint delta = p.pos().toPoint() - p.startPos().toPoint();
            enum { TapRadius = 40 };
            if (delta.manhattanLength() > TapRadius) {
                if (event->type() == QEvent::TouchEnd)
                    result = QGestureRecognizer::FinishGesture;
                else
                    result = QGestureRecognizer::TriggerGesture;
            }
        }
        break;
    }
    case QEvent::MouseButtonPress:
    case QEvent::MouseMove:
    case QEvent::MouseButtonRelease:
        result = QGestureRecognizer::Ignore;
        break;
#ifdef QT_WEBOS
    case QEvent::Gesture:
    {
/* Commented out for now, looks like deference to other events
    	QGesture* g = static_cast<QGestureEvent*>(event)->gesture(Qt::SysMgrGestureFlick);
		if (g && g->state() == Qt::GestureFinished && q->state() != Qt::NoGesture ) {
			result = QGestureRecognizer::CancelGesture;
			break;
		}
        g = static_cast<QGestureEvent*>(event)->gesture(Qt::SysMgrGestureScreenEdgeFlick);
        if (g && g->state() == Qt::GestureFinished && q->state() != Qt::NoGesture ) {
            result = QGestureRecognizer::CancelGesture;
            break;
        }
*/
    }
    // fall through
#endif // QT_WEBOS
    default:
        result = QGestureRecognizer::Ignore;
        break;
    }
    return result;
}

void DragGestureRecognizer::reset(QGesture *state)
{
    DragGesture *q = static_cast<DragGesture *>(state);

    QGestureRecognizer::reset(state);
}

QT_END_NAMESPACE

#endif // QT_NO_GESTURES
