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




#ifndef GESTUREFEEDBACKITEM_H
#define GESTUREFEEDBACKITEM_H

#include <QGraphicsObject>
#include <QMouseEvent>
#include <QTimer>
#include <QPropertyAnimation>

#include "VirtualGestureStrip.h"


class GestureFeedbackItem : public QGraphicsObject
{
	Q_OBJECT
	Q_PROPERTY(qreal tapFeedbackProgress READ tapFeedbackProgress WRITE setTapFeedbackProgress)

public:
	enum FeedbackState {
		StaticFeedback = 0,
		DraggedFeedback,
		FlickFeedback,
		TapFeedback
	}; 

public:
	GestureFeedbackItem(int parentWidth, int parentHeight);
	~GestureFeedbackItem();

	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
	
	void setFeedbackState(FeedbackState state);
	FeedbackState feedbackState() { return m_state; }
	
	qreal tapFeedbackProgress() const { return m_feedbackProg; }
	void setTapFeedbackProgress(const qreal progress);

	void fingerDown(int x);
	void fingerMove(int x);
	void fingerUp(int x);
	
	void fingerDrag(int start, int current);
	
	void gesture(Qt::Key gestureKey, QPoint startPos, QPoint velocity);

private Q_SLOTS:
	void slotGlowAnimationFinished();
	void slotPosAnimationFinished();

private:
	
	enum FlickAnimationDirection {
		FlickRight = 0,
		FlickUp,
		FlickLeft,
		FlickDown
	};
	
	QRect m_bounds, m_parentBounds;

	FeedbackState m_state;
	
	int m_dragLength;
	qreal m_feedbackProg;
	
	FlickAnimationDirection m_direction;
	
	QPropertyAnimation m_glowAnimation;
	QPropertyAnimation m_posAnimation;
	QPropertyAnimation m_tapAnimation;
	
private:
	// static images
	static QPixmap *s_glowPixmap;
	//static QPixmap* s_flickPixmap;
	
	static QPixmap* glowPixmap();
	//static QPixmap* flickPixmap();

};


#endif /* GESTUREFEEDBACKITEM_H */
