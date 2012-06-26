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






#include "Event.h"

#include <QPainter>

#include "GestureFeedbackItem.h"
#include "Settings.h"
#include "SoundPlayerPool.h"


//static const char* kBackGestureSound = "/usr/palm/sounds/BackGesture.wav";
//static const char* kUpGestureSound = "/usr/palm/sounds/UpGesture.wav";


#define GLOW_ANIM_DURATION  200
#define UP_GESTURE_LENGTH   250
#define TAP_GESTURE_SCALE   0.75

// static pixmap members
QPixmap* GestureFeedbackItem::s_glowPixmap = 0;


QPixmap* GestureFeedbackItem::glowPixmap() 
{
	if(!s_glowPixmap) {
		std::string path = Settings::LunaSettings()->lunaSystemResourcesPath + "/corenavi/glow.png";
		s_glowPixmap = new QPixmap(path.c_str());
	}
	
	return s_glowPixmap;
}


GestureFeedbackItem::GestureFeedbackItem(int parentWidth, int parentHeight)
	: m_state(StaticFeedback)
	, m_feedbackProg(0)
{	
	QPixmap* glow = glowPixmap();
	
	m_parentBounds = QRect(-parentWidth/2, -parentHeight/2, parentWidth,parentHeight);
	m_bounds = QRect(-glow->width()/2, -glow->height()/2, glow->width(),glow->height());
	
	m_glowAnimation.setPropertyName("opacity");
	m_glowAnimation.setEasingCurve(QEasingCurve::Linear);
	m_glowAnimation.setTargetObject(this);
	connect(&m_glowAnimation, SIGNAL(finished()), SLOT(slotGlowAnimationFinished()));
	
	m_posAnimation.setPropertyName("pos");
	m_posAnimation.setEasingCurve(QEasingCurve::Linear);
	m_posAnimation.setTargetObject(this);
	connect(&m_posAnimation, SIGNAL(finished()), SLOT(slotPosAnimationFinished()));

	m_tapAnimation.setPropertyName("tapFeedbackProgress");
	m_tapAnimation.setEasingCurve(QEasingCurve::Linear);
	m_tapAnimation.setTargetObject(this);
}

GestureFeedbackItem::~GestureFeedbackItem()
{    
}

void GestureFeedbackItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QPixmap* glow = glowPixmap(); 
	
	switch(m_state) {
		case StaticFeedback: {
			painter->drawPixmap(-(glow->width()/2), -(glow->height()/2), *glow);
		}
		break;
		
		case DraggedFeedback: {			
			if(m_dragLength >= 0) {
				painter->drawPixmap(-m_dragLength, -(glow->height()/2), m_dragLength,  glow->height(),
				                    *glow, 0, 0, glow->width()/2, glow->height());
				painter->drawPixmap(0, -(glow->height()/2), glow->width()/2,  glow->height(),
				                    *glow, glow->width()/2, 0, glow->width()/2, glow->height());
			} else {
				painter->drawPixmap(0, -(glow->height()/2), -m_dragLength,  glow->height(),
				                    *glow, glow->width()/2, 0, glow->width()/2, glow->height());				
				painter->drawPixmap(-(glow->width()/2), -(glow->height()/2), glow->width()/2,  glow->height(),
				                    *glow, 0, 0, glow->width()/2, glow->height());				
			}
		}
		break;
		
		case FlickFeedback: {
			switch(m_direction) {
				case FlickRight: {
					painter->drawPixmap(-m_dragLength, -(glow->height()/2), m_dragLength,  glow->height(),
					                    *glow, 0, 0, glow->width()/2, glow->height());
					painter->drawPixmap(0, -(glow->height()/2), glow->width()/2,  glow->height(),
					                    *glow, glow->width()/2, 0, glow->width()/2, glow->height());				
				}
				break;
				
				case FlickLeft: {
					painter->drawPixmap(0, -(glow->height()/2), m_dragLength,  glow->height(),
					                    *glow, glow->width()/2, 0, glow->width()/2, glow->height());				
					painter->drawPixmap(-(glow->width()/2), -(glow->height()/2), glow->width()/2,  glow->height(),
					                    *glow, 0, 0, glow->width()/2, glow->height());								
				}
				break;
			
				case FlickUp: {
					painter->rotate(-90);
					painter->drawPixmap(-m_dragLength, -(glow->height()), m_dragLength,  glow->height() * 2,
					                    *glow, 0, 0, glow->width()/2, glow->height());
					painter->drawPixmap(0, -(glow->height()), glow->width()/2,  glow->height() * 2,
					                    *glow, glow->width()/2, 0, glow->width()/2, glow->height());				
					painter->rotate(90);
				}
				break;
				
				default: {
					
				}
			}
		}
		break;
		
		case TapFeedback: {		
			int x1, length1, x2, length2;
			
			x1 = -glow->width()/2;
			length1 = m_dragLength * m_feedbackProg + glow->width()/2;
			x2 = x1 + length1;
			length2 = glow->width()/2;
						
			painter->drawPixmap(-(glow->width()/2), -(glow->height()/2), *glow);

			painter->drawPixmap(x1, -(glow->height()*TAP_GESTURE_SCALE/2), length1,  glow->height()*TAP_GESTURE_SCALE,
			                    *glow, 0, 0, glow->width()/2, glow->height());
			painter->drawPixmap(x2, -(glow->height()*TAP_GESTURE_SCALE/2), length2,  glow->height()*TAP_GESTURE_SCALE,
				                    *glow, glow->width()/2, 0, glow->width()/2, glow->height());

			painter->rotate(-90);
			painter->drawPixmap(x1, -(glow->height()*TAP_GESTURE_SCALE/2), length1,  glow->height()*TAP_GESTURE_SCALE,
			                    *glow, 0, 0, glow->width()/2, glow->height());
			painter->drawPixmap(x2, -(glow->height()*TAP_GESTURE_SCALE/2), length2,  glow->height()*TAP_GESTURE_SCALE,
				                    *glow, glow->width()/2, 0, glow->width()/2, glow->height());
			painter->rotate(90);

			painter->rotate(-180);
			painter->drawPixmap(x1, -(glow->height()*TAP_GESTURE_SCALE/2), length1,  glow->height()*TAP_GESTURE_SCALE,
			                    *glow, 0, 0, glow->width()/2, glow->height());
			painter->drawPixmap(x2, -(glow->height()*TAP_GESTURE_SCALE/2), length2,  glow->height()*TAP_GESTURE_SCALE,
				                    *glow, glow->width()/2, 0, glow->width()/2, glow->height());
			painter->rotate(180);
		}
		break;
		

		default: {
			
		}
	}
	
}

QRectF GestureFeedbackItem::boundingRect() const 
{
	return m_bounds;
}

void GestureFeedbackItem::setTapFeedbackProgress(const qreal progress)
{
	m_feedbackProg = progress;
//	update();
}

void GestureFeedbackItem::setFeedbackState(FeedbackState state) 
{ 
	if(m_state != state) {
		prepareGeometryChange();
	
		// change the bounding rect to the appropriate size
		switch(m_state) {
		case StaticFeedback: {
			m_bounds = QRect(-glowPixmap()->width()/2, -glowPixmap()->height()/2, glowPixmap()->width(), glowPixmap()->height());
			}
		break;
		
		case DraggedFeedback: {
			// Optmize this
			m_bounds = QRect(-m_parentBounds.width()/2, -m_parentBounds.height()/2, m_parentBounds.width(), m_parentBounds.height());
			}
		break;
			
		case FlickFeedback:   {
			if(m_direction != FlickUp) {
				// Optmize this
				m_bounds = QRect(-m_parentBounds.width()/2, -m_parentBounds.height()/2, m_parentBounds.width(), m_parentBounds.height());
			} else {
				m_bounds = QRect(-glowPixmap()->height(), -glowPixmap()->width()/2, glowPixmap()->height()*2, UP_GESTURE_LENGTH + glowPixmap()->width()/2);
			}
		}
		break;
			
			default: {
				m_bounds = QRect(-glowPixmap()->width()/2, -glowPixmap()->height()/2, glowPixmap()->width(), glowPixmap()->height());
			}
		}
	}
	
	m_state = state; 
}

void GestureFeedbackItem::fingerDown(int x)
{
	setFeedbackState(StaticFeedback); 
	setPos(x, 0);
	setVisible(true);
	
	m_posAnimation.stop();
	m_tapAnimation.stop();
	
	// animate the glow in
	m_glowAnimation.stop();
	m_glowAnimation.setDuration((1.0 - opacity()) * GLOW_ANIM_DURATION);
	m_glowAnimation.setStartValue(opacity());
	m_glowAnimation.setEndValue(1.0);
	m_glowAnimation.start();
}

void GestureFeedbackItem::fingerMove(int x)
{
	setFeedbackState(StaticFeedback); 
	setX(x);	
}

void GestureFeedbackItem::fingerUp(int x)
{
	if(m_state != StaticFeedback)
	{
		// animate the glow out
		m_glowAnimation.stop();
		m_glowAnimation.setStartValue(opacity());
		m_glowAnimation.setEndValue(0.0);
		if(m_state != FlickFeedback) {
			setX(x);	
			m_glowAnimation.setDuration((opacity()) * GLOW_ANIM_DURATION);
		} else {
			m_glowAnimation.setDuration(450);
		}
		m_glowAnimation.start();
	} else {
		setX(x);	
		// animate the tap feedback
		setFeedbackState(TapFeedback); 
		m_dragLength = UP_GESTURE_LENGTH;
		m_tapAnimation.stop();
		m_tapAnimation.setDuration(500);
		m_tapAnimation.setStartValue(0.0);
		m_tapAnimation.setEndValue(1.0);

		m_glowAnimation.stop();
		m_glowAnimation.setStartValue(opacity());
		m_glowAnimation.setEndValue(0.0);
		m_glowAnimation.setDuration(500);
		
		m_glowAnimation.start();
		m_tapAnimation.start();
	}
}


void GestureFeedbackItem::slotGlowAnimationFinished()
{
	if(opacity() == 0.0) {
		// finished fading OUT 
		setVisible(false);
	}
}

void GestureFeedbackItem::slotPosAnimationFinished()
{

}

void GestureFeedbackItem::fingerDrag(int start, int current)
{
	setFeedbackState(DraggedFeedback); 
	m_dragLength = current - start;
	if(m_dragLength >= 0) {
		m_dragLength += glowPixmap()->width()/2;
	}
	else {
		m_dragLength -= glowPixmap()->width()/2;
	}
	setX(current);
}

void GestureFeedbackItem::gesture(Qt::Key gestureKey, QPoint startPos, QPoint velocity)
{
	switch (gestureKey) {
		case Qt::Key_CoreNavi_Next:
		case Qt::Key_CoreNavi_Menu: {
			setFeedbackState(FlickFeedback); 
			m_direction = FlickRight;
						
			setX(startPos.x());

			// animate the feedback to the right
			m_posAnimation.stop();
			m_tapAnimation.stop();
			m_posAnimation.setDuration(500);
			m_posAnimation.setStartValue(pos());
			m_posAnimation.setEndValue(QPoint(m_bounds.width(), pos().y()));
			m_posAnimation.start();
			
//			SoundPlayerPool::instance()->play(kBackGestureSound, "feedback", false, -1);
		}
		break;
		
		case Qt::Key_CoreNavi_Previous:
		case Qt::Key_CoreNavi_Back: {
			setFeedbackState(FlickFeedback); 
			m_direction = FlickLeft;
			setX(startPos.x());

			// animate the feedback to the left
			m_posAnimation.stop();
			m_tapAnimation.stop();
			m_posAnimation.setDuration(500);
			m_posAnimation.setStartValue(pos());
			m_posAnimation.setEndValue(QPoint(-m_bounds.width(), pos().y()));
			m_posAnimation.start();

//			SoundPlayerPool::instance()->play(kBackGestureSound, "feedback", false, -1);
		}
		break;
		
		case Qt::Key_CoreNavi_Launcher: {
			setFeedbackState(FlickFeedback); 
			m_direction = FlickUp;
			setPos(startPos.x(), 0);
			m_dragLength = UP_GESTURE_LENGTH / 2;
			
			// animate the feedback to the left
			m_posAnimation.stop();
			m_tapAnimation.stop();
			m_posAnimation.setDuration(450);
			m_posAnimation.setStartValue(pos());
			m_posAnimation.setEndValue(QPoint(pos().x(), pos().y() - UP_GESTURE_LENGTH));
			m_posAnimation.start();

//			SoundPlayerPool::instance()->play(kUpGestureSound, "feedback", false, -1);		
		}
		break;
		
		default:
		{
			
		}
	}
}


