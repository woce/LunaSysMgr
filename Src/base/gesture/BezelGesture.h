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




#ifndef BEZELGESTURE_H
#define BEZELGESTURE_H

#include "Common.h"

#include <QGesture>
#include <QPoint>

#include <SysMgrDefs.h>

//Enum for referencing the gesture as a (Qt::GestureType)
enum Type { BezelGestureType };

enum Edge { None, Left, Right, Bottom };

class BezelGesture : public QGesture
{
public:
	BezelGesture(QObject* parent = 0) : QGesture(parent, (Qt::GestureType) BezelGestureType) {}

	QPoint pos() const { return m_pos; }
	void setPos(QPoint pos) { m_pos = pos; }
	QPoint lastPos() const { return m_lastPos; }
	void setLastPos(QPoint lastPos) { m_lastPos = lastPos; }
	QPoint delta() const { return m_delta; }
	void setDelta(QPoint delta) { m_delta = delta; }
	QPoint diff() const { return m_diff; }
	void setDiff(QPoint diff) { m_diff = diff; }
	
	int flick() const { return m_flick; }
	void setFlick(int flick) { m_flick = flick; }
	Edge edge() const { return m_edge; }
	void setEdge(Edge edge) { m_edge = edge; }

private:
	QPoint m_pos;
	QPoint m_lastPos;
	QPoint m_delta;
	QPoint m_diff;
	
	int m_flick;
	Edge m_edge;

private:

	friend class BezelGestureRecognizer;
};

#endif /* BEZELGESTURE_H */
