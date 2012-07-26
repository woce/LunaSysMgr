#ifndef CARDSWITCHGESTURERECOGNIZER_H
#define CARDSWITCHGESTURERECOGNIZER_H

#include "Common.h"

#include <QEvent>
#include <QGestureRecognizer>
#include <QTouchEvent>
#include <QTransform>
#include <QDebug>

#include "CardSwitchGesture.h"

#include "HostBase.h"
#include "Settings.h"
#include "WindowServer.h"

class CardSwitchGestureRecognizer : public QGestureRecognizer
{
public:
    CardSwitchGestureRecognizer();

    QGesture *create(QObject *target);
    QGestureRecognizer::Result recognize(QGesture *state, QObject *watched, QEvent *event);
};

#endif // CARDSWITCHGESTURERECOGNIZER_H
