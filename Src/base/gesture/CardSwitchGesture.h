/* @@@LICENSE
*
*      Copyright (c) 2010-2012 Hewlett-Packard Development Company, L.P.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* LICENSE@@@ */




#ifndef CARDSWITCHGESTURE_H
#define CARDSWITCHGESTURE_H

#include "Common.h"

#include <QGesture>
#include <QPoint>

#include <SysMgrDefs.h>

enum Gesture {
	GestureCardSwitch  = 0x0100 + 5
};

enum Edge {
	Left = 0,
	Right,
	Bottom
};

class CardSwitchGesture : public QGesture
{
public:

	CardSwitchGesture(QObject* parent = 0) : QGesture(parent, (Qt::GestureType) GestureCardSwitch) {}
	int getEdge() { return edge; }
	void setEdge(int inEdge) { edge = inEdge; }

private:
	int edge;

private:

	friend class CardSwitchGestureRecognizer;
};

#endif /* CARDSWITCHGESTURE_H */
