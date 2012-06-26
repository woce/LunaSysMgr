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

#include "Settings.h"
#include "VirtualCoreNavi.h"

#define BAR_GLOW_ANIM_DURATION  100
#define BAR_BRIGHT_OPACITY      0.60

#define DRAG_THRESHOLD     8
#define GESTURE_THRESHOLD  100
#define GESTURE_FADE_SPEED 45

VirtualCoreNavi::VirtualCoreNavi(int width, int height)
	: VirtualGestureStrip(width, height)
	, m_feedbackItem(width, height)
	, m_lightBarBrightness(0)
{
	m_feedbackItem.setParentItem(this);
	m_feedbackItem.setVisible(false);
	m_feedbackItem.setPos(0,0);
	
	std::string path;
	
	// Load the Light Bar images
	path = Settings::LunaSettings()->lunaSystemResourcesPath + "/corenavi/light-bar-dark-left.png";
	m_pixBarDarkLeft.load(path.c_str());
	if(m_pixBarDarkLeft.isNull())
		g_warning("failed to load CoreNavi image: %s", path.c_str());
	
	path = Settings::LunaSettings()->lunaSystemResourcesPath + "/corenavi/light-bar-dark-center.png";
	m_pixBarDarkCenter.load(path.c_str());
	if(m_pixBarDarkCenter.isNull())
		g_warning("failed to load CoreNavi image: %s", path.c_str());
	
	path = Settings::LunaSettings()->lunaSystemResourcesPath + "/corenavi/light-bar-dark-right.png";
	m_pixBarDarkRight.load(path.c_str());
	if(m_pixBarDarkRight.isNull())
		g_warning("failed to load CoreNavi image: %s", path.c_str());
	
	path = Settings::LunaSettings()->lunaSystemResourcesPath + "/corenavi/light-bar-bright-left.png";
	m_pixBarBrightLeft.load(path.c_str());
	if(m_pixBarBrightLeft.isNull())
		g_warning("failed to load CoreNavi image: %s", path.c_str());
	
	path = Settings::LunaSettings()->lunaSystemResourcesPath + "/corenavi/light-bar-bright-center.png";
	m_pixBarBrightCenter.load(path.c_str());
	if(m_pixBarBrightCenter.isNull())
		g_warning("failed to load CoreNavi image: %s", path.c_str());
	
	path = Settings::LunaSettings()->lunaSystemResourcesPath + "/corenavi/light-bar-bright-right.png";
	m_pixBarBrightRight.load(path.c_str());
	if(m_pixBarBrightRight.isNull())
		g_warning("failed to load CoreNavi image: %s", path.c_str());
	
	// setup the glow animation
	m_barGlowAnimation.setPropertyName("lightBarBrightness");
	m_barGlowAnimation.setEasingCurve(QEasingCurve::Linear);
	m_barGlowAnimation.setTargetObject(this);
}

VirtualCoreNavi::~VirtualCoreNavi()
{    
}

void VirtualCoreNavi::setLightBarBrightness(const qreal brightness)
{
	m_lightBarBrightness = brightness;
//	update();
}

void VirtualCoreNavi::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->fillRect(m_bounds, QColor(0,0,0,0));
	
	// always draw the dark light bar
	painter->drawPixmap(m_bounds.x(), -m_pixBarDarkLeft.height()/2, m_pixBarDarkLeft);
	painter->drawPixmap(m_bounds.x() + m_bounds.width() - m_pixBarDarkRight.width(), -m_pixBarDarkRight.height()/2, m_pixBarDarkRight);
	painter->drawPixmap(m_bounds.x() + m_pixBarDarkLeft.width(), -m_pixBarDarkCenter.height()/2, 
			            m_bounds.width() - m_pixBarDarkLeft.width() - m_pixBarDarkRight.width(), m_pixBarDarkCenter.height(),
			            m_pixBarDarkCenter);
	
	// now draw the bright light bar if the opacity its is non-zero
	if(m_lightBarBrightness > 0.0) {
		qreal prevOpacity = painter->opacity();
		painter->setOpacity(prevOpacity * m_lightBarBrightness);
		
		painter->drawPixmap(m_bounds.x(), -m_pixBarBrightLeft.height()/2, m_pixBarBrightLeft);
		painter->drawPixmap(m_bounds.x() + m_bounds.width() - m_pixBarBrightRight.width(), -m_pixBarBrightRight.height()/2, m_pixBarBrightRight);
		painter->drawPixmap(m_bounds.x() + m_pixBarBrightLeft.width(), -m_pixBarDarkCenter.height()/2, 
				            m_bounds.width() - m_pixBarBrightLeft.width() - m_pixBarBrightRight.width(), m_pixBarBrightCenter.height(),
				            m_pixBarBrightCenter);
		
		painter->setOpacity(prevOpacity);
	}
}


void VirtualCoreNavi::fingerDown(int xPos)
{
	m_barGlowAnimation.stop();
	m_barGlowAnimation.setDuration((1.0 - m_lightBarBrightness) * BAR_GLOW_ANIM_DURATION);
	m_barGlowAnimation.setStartValue(m_lightBarBrightness);
	m_barGlowAnimation.setEndValue(BAR_BRIGHT_OPACITY);
	m_barGlowAnimation.start();

	m_feedbackItem.fingerDown(xPos);
}

void VirtualCoreNavi::fingerMove(int xPos)
{
	m_feedbackItem.fingerMove(xPos);
}

void VirtualCoreNavi::fingerDrag(int xStart, int xEnd)
{
	m_feedbackItem.fingerDrag(xStart, xEnd);	
}

void VirtualCoreNavi::fingerUp(int xPos)
{
	m_barGlowAnimation.stop();
	m_barGlowAnimation.setDuration((m_lightBarBrightness) * BAR_GLOW_ANIM_DURATION);
	m_barGlowAnimation.setStartValue(m_lightBarBrightness);
	m_barGlowAnimation.setEndValue(0.0);
	m_barGlowAnimation.start();
	
	m_feedbackItem.fingerUp(xPos);
}


void VirtualCoreNavi::gesturePerformed(Qt::Key key, QPoint startPos, QPoint velocity)
{
	m_feedbackItem.gesture(key, startPos, velocity);
}

