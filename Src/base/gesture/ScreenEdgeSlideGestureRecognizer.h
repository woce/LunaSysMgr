#ifndef SCREENEDGESLIDEGESTURERECOGNIZER_H
#define SCREENEDGESLIDEGESTURERECOGNIZER_H

#include "Common.h"

#include "FlickGestureRecognizer.h"

#include <QEvent>
#include <QTouchEvent>
#include <QTransform>
#include <QDebug>

#include "ScreenEdgeSlideGesture.h"

#include "HostBase.h"
#include "Settings.h"
#include "WindowServer.h"

class ScreenEdgeSlideGestureRecognizer : public QGestureRecognizer
{
public:
    ScreenEdgeSlideGestureRecognizer();

    QGesture *create(QObject *target);
    QGestureRecognizer::Result recognize(QGesture *state, QObject *watched, QEvent *event);
};

#endif // SCREENEDGESLIDEGESTURERECOGNIZER_H
