#ifndef CARDVIEWGESTURERECOGNIZER_H
#define CARDVIEWGESTURERECOGNIZER_H

#include "Common.h"

#include <QEvent>
#include <QGestureRecognizer>
#include <QTouchEvent>
#include <QTransform>
#include <QDebug>

#include "CardViewGesture.h"

#include "HostBase.h"
#include "Settings.h"
#include "WindowServer.h"

class CardViewGestureRecognizer : public QGestureRecognizer
{
public:
    CardViewGestureRecognizer();

    QGesture *create(QObject *target);
    QGestureRecognizer::Result recognize(QGesture *state, QObject *watched, QEvent *event);
};

#endif // CARDVIEWGESTURERECOGNIZER_H
