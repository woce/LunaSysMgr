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




#ifndef VIRTUALGESTURESTRIP_H
#define VIRTUALGESTURESTRIP_H

#include <QGraphicsObject>
#include <QGraphicsSceneMouseEvent>
#include <QTimer>

class VirtualGestureStrip : public QGraphicsObject
{
	Q_OBJECT

public:
	VirtualGestureStrip(int width, int height);
	virtual ~VirtualGestureStrip();

	virtual QRectF boundingRect() const { return m_bounds; }

	bool sceneEvent(QEvent* e);
	void flickGesture(const QPoint& velocity, const QPoint& startPos);

	virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
	void postGesture(Qt::Key key, QPoint startPos, QPoint velocity);
	void postMouseUpdate(QPoint pos);
	virtual void setAcceptedKeys(QList<Qt::Key> keys);
	virtual void setTapGesture(Qt::Key key);
	
protected:
	virtual void fingerDown(int xPos) = 0;
	virtual void fingerMove(int xPos) = 0;
	virtual void fingerDrag(int xStart, int xEnd) = 0;
	virtual void fingerUp(int xPos) = 0;
	virtual void gesturePerformed(Qt::Key key, QPoint startPos, QPoint velocity) = 0;

private:
	QList<Qt::Key> m_acceptedKeys;
	Qt::Key m_tapGestureKey;
	virtual bool shouldPerformGesture(Qt::Key key);

private Q_SLOTS:
	void slotQuickLaunchGesture();
	void slotGestureDragTimer();

	virtual void relayout(const QRectF& bounds);

protected:
	QRect m_bounds;
	QPoint m_mouseDownPos;
	bool m_seenGesture;
	QPoint m_currentMousePos;
	QTimer m_quickLaunchFire;
	bool m_quickLaunch;
	
	QTimer m_gestureDragTimer;
	int m_gestureStart, m_gestureEnd;
};


#endif /* VIRTUALGESTURESTRIP_H */
