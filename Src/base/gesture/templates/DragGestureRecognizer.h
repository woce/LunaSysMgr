#ifndef DRAGGESTURERECOGNIZER_H
#define DRAGGESTURERECOGNIZER_H

#include "Common.h"

#include "FlickGestureRecognizer.h"

#include <QEvent>
#include <QTouchEvent>
#include <QTransform>
#include <QDebug>

#include "FlickGesture.h"

#include "Time.h"
#include "HostBase.h"

#ifndef QT_NO_GESTURES

QT_BEGIN_NAMESPACE

class DragGestureRecognizer : public QGestureRecognizer
{
public:
    DragGestureRecognizer();

    QGesture *create(QObject *target);
    QGestureRecognizer::Result recognize(QGesture *state, QObject *watched, QEvent *event);
    void reset(QGesture *state);
};

QT_END_NAMESPACE

#endif // QT_NO_GESTURES

#endif // DRAGGESTURERECOGNIZER_H
