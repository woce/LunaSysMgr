#ifndef BEZELGESTURERECOGNIZER_H
#define BEZELGESTURERECOGNIZER_H

#include "BezelGesture.h"

#include <QGestureRecognizer>

class BezelGestureRecognizer : public QGestureRecognizer
{
public:
    BezelGestureRecognizer();

    QGesture* create(QObject* target);
    QGestureRecognizer::Result recognize(QGesture* state, QObject* watched, QEvent* event);
    void reset(BezelGesture* gesture);
};

#endif // BEZELGESTURERECOGNIZER_H
