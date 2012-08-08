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




#ifndef SCREENEDGESLIDEGESTURE_H
#define SCREENEDGESLIDEGESTURE_H

#include "Common.h"

#include <QGesture>
#include <QPoint>

#include <SysMgrDefs.h>

enum Gesture {
	GestureScreenEdgeSlide  = 0x0100 + 5
};

enum Edge {
	Left = 0,
	Right,
	Bottom
};

class ScreenEdgeSlideGesture : public QGesture
{
public:

	ScreenEdgeSlideGesture(QObject* parent = 0) : QGesture(parent, (Qt::GestureType) GestureScreenEdgeSlide) {}
	int getEdge() { return edge; }
	bool getFired() { return fired; }
	void setEdge(int inEdge) { edge = inEdge; }
	void setFired(bool inFired) { fired = inFired; }

private:
	int edge;
	bool fired;

private:

	friend class ScreenEdgeSlideGestureRecognizer;
};

#endif /* SCREENEDGESLIDEGESTURE_H */
