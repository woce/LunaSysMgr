/* @@@LICENSE
*
*      Copyright (c) 2008-2012 Hewlett-Packard Development Company, L.P.
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




#include "Common.h"

#include <PGSurface.h>
#include <PGContext.h>

#include "CardWebApp.h"
#include "PIpcChannel.h"
#include "PIpcBuffer.h"
#include "WindowContentTransitionRunner.h"
#include "SyncTask.h"

#define MESSAGES_INTERNAL_FILE "SysMgrMessagesInternal.h"
#include <PIpcMessageMacros.h>


WindowContentTransitionRunner* WindowContentTransitionRunner::instance()
{
	static WindowContentTransitionRunner* s_instance = 0;
	if (G_UNLIKELY(s_instance == 0))
		s_instance = new WindowContentTransitionRunner;

	return s_instance;
}

WindowContentTransitionRunner::WindowContentTransitionRunner()
	: m_mainLoop(0)
	, m_mainCtxt(0)
	, m_transitionType(Transition_Invalid)
	, m_app(0)
	, m_channel(0)
	, m_ipcBuffer(0)
	, m_dstContext(0)
	, m_fromSceneSurface(0)
	, m_toSceneSurface(0)
	, m_transitionIsPop(false)
	, m_currRotateAngle(0)
	, m_targRotateAngle(0)
{
	m_mainCtxt = g_main_context_new();
	m_mainLoop = g_main_loop_new(m_mainCtxt, TRUE);
}

WindowContentTransitionRunner::~WindowContentTransitionRunner()
{
    // not reached
}

void WindowContentTransitionRunner::runSceneTransition(PIpcBuffer* ipcBuff, 
													   PIpcChannel* ipcChannel,
													   PGSurface* fromSurface,
													   PGSurface* toSurface,
													   PGContext* dstContext,
													   const char* transitionName,
													   bool isPop)
{
	m_transitionType = Transition_ZoomAndCrossFade;
	m_channel = ipcChannel;
	m_ipcBuffer = ipcBuff;
	m_fromSceneSurface = fromSurface;
	m_toSceneSurface = toSurface;
	m_dstContext = dstContext;
	m_transitionIsPop = isPop;

	if (transitionName) {
		if (strcmp(transitionName, "cross-fade") == 0)
			m_transitionType = Transition_CrossFade;
		else if (strcmp(transitionName, "zoom-fade") == 0)
			m_transitionType = Transition_ZoomAndCrossFade;
	}
	
	// -----------------------------------------------------------------------

	AnimObject& aObj = m_fromSceneAnimObject;
	AnimObject& bObj = m_toSceneAnimObject;
	PGSurface* aSurf = m_fromSceneSurface;
	PGSurface* bSurf = m_toSceneSurface;
	if (m_transitionIsPop) {
		aObj = m_toSceneAnimObject;
		bObj = m_fromSceneAnimObject;
		aSurf = m_toSceneSurface;
		bSurf = m_fromSceneSurface;
	}

	float scaleFactor;

	if (m_transitionIsPop)
		scaleFactor = 0.75f;
	else
		scaleFactor = 1.25f;

	int W, H, w, h;

	aObj.currX = 0;	
	aObj.currY = 0;
	aObj.currR = aSurf->width();
	aObj.currB = aSurf->height();
	aObj.currA = 0xFF;

	W = aSurf->width();
	H = aSurf->height();
	w = (int) (W * scaleFactor);
	h = (int) (H * scaleFactor);
	
	aObj.targX = (W - w)/2;
	aObj.targY = (H - h)/2;
	aObj.targR = (W + w)/2;
	aObj.targB = (H + h)/2;
	aObj.targA = 0x00;

	if (m_transitionIsPop)
		scaleFactor = 1.25f;
	else
		scaleFactor = 0.75f;

	bObj.targX = 0;
	bObj.targY = 0;
	bObj.targR = bSurf->width();
	bObj.targB = bSurf->height();
	bObj.targA = 0xFF;

	W = bSurf->width();
	H = bSurf->height();
	w = (int) (W * scaleFactor);
	h = (int) (H * scaleFactor);
		
	bObj.currX = (W - w)/2;
	bObj.currY = (H - h)/2;
	bObj.currR = (W + w)/2;
	bObj.currB = (H + h)/2;
	bObj.currA = 0x80;
	

	// -----------------------------------------------------------------------	

	runLoop();

	// -----------------------------------------------------------------------	
	
	m_transitionType = Transition_Invalid;
	m_ipcBuffer = 0;
	m_channel = 0;
	m_fromSceneSurface = 0;
	m_toSceneSurface = 0;
	m_dstContext = 0;
	m_transitionIsPop = false;
}

void WindowContentTransitionRunner::runRotateTransition(CardWebApp* app,
														PGSurface* toSurface,
														PGContext* dstContext,
														int currAngle,
														int targAngle)
{
	m_transitionType = Transition_Rotate;
	m_app = app;
	m_toSceneSurface = toSurface;
	m_dstContext = dstContext;
	m_currRotateAngle = currAngle;
	m_targRotateAngle = targAngle;

	// -----------------------------------------------------------------------	

	runLoop();

	// -----------------------------------------------------------------------	
	
	m_transitionType = Transition_Invalid;
	m_app = 0;
	m_toSceneSurface = 0;
	m_dstContext = 0;
	m_currRotateAngle = 0;
	m_targRotateAngle = 0;
}

void WindowContentTransitionRunner::runLoop()
{
	GSource* src = g_timeout_source_new(33);
	g_source_set_callback(src, WindowContentTransitionRunner::sourceCallback, this, NULL);
	g_source_attach(src, m_mainCtxt);
	g_source_unref(src);

	g_main_loop_run(m_mainLoop);    
}

#define SCALE_DELTA(val, numer, denom)									\
	if (val > 0)														\
		val = MAX((val * numer)/denom, 1);								\
	else if (val < 0)													\
		val = MIN((val * numer)/denom, -1);								\

bool WindowContentTransitionRunner::timerTicked()
{
	bool ret = false;
	
	switch (m_transitionType) {
	case Transition_ZoomAndCrossFade:
		ret = zoomAndCrossFadeTick();
		break;
	case Transition_CrossFade:
		ret = crossFadeTick();
		break;
	case Transition_Rotate:
		ret = rotateTick();
		break;
	default:
		return false;
	}

	if(m_channel && m_ipcBuffer)
		m_channel->sendAsyncMessage(new ViewHost_UpdateFullWindow(m_ipcBuffer->key()));	
	
    return ret;
}

gboolean WindowContentTransitionRunner::sourceCallback(gpointer arg)
{
	WindowContentTransitionRunner* runner = (WindowContentTransitionRunner*) arg;
	bool ret = runner->timerTicked();
	if (!ret)
		g_main_loop_quit(runner->m_mainLoop);

	return ret;
}

bool WindowContentTransitionRunner::zoomAndCrossFadeTick()
{
	static const int danielNumer = 3;
	static const int danielDenom = 5;
	
	if (m_toSceneAnimObject.currX == m_toSceneAnimObject.targX &&
		m_toSceneAnimObject.currY == m_toSceneAnimObject.targY &&
		m_toSceneAnimObject.currR == m_toSceneAnimObject.targR &&
		m_toSceneAnimObject.currB == m_toSceneAnimObject.targB &&
		m_toSceneAnimObject.currA == m_toSceneAnimObject.targA) {

		return false;
	}

	int deltaX = m_toSceneAnimObject.targX - m_toSceneAnimObject.currX;
	int deltaY = m_toSceneAnimObject.targY - m_toSceneAnimObject.currY;
	int deltaR = m_toSceneAnimObject.targR - m_toSceneAnimObject.currR;
	int deltaH = m_toSceneAnimObject.targB - m_toSceneAnimObject.currB;
	int deltaA = m_toSceneAnimObject.targA - m_toSceneAnimObject.currA;

	SCALE_DELTA(deltaX, danielNumer, danielDenom);
	SCALE_DELTA(deltaY, danielNumer, danielDenom);
	SCALE_DELTA(deltaR, danielNumer, danielDenom);
	SCALE_DELTA(deltaH, danielNumer, danielDenom);
	SCALE_DELTA(deltaA, danielNumer, danielDenom);

	
	m_toSceneAnimObject.currX += deltaX;
	m_toSceneAnimObject.currY += deltaY;
	m_toSceneAnimObject.currR += deltaR;
	m_toSceneAnimObject.currB += deltaH;
	m_toSceneAnimObject.currA += deltaA;

	deltaX = m_fromSceneAnimObject.targX - m_fromSceneAnimObject.currX;
	deltaY = m_fromSceneAnimObject.targY - m_fromSceneAnimObject.currY;
	deltaR = m_fromSceneAnimObject.targR - m_fromSceneAnimObject.currR;
	deltaH = m_fromSceneAnimObject.targB - m_fromSceneAnimObject.currB;
	deltaA = m_fromSceneAnimObject.targA - m_fromSceneAnimObject.currA;

	SCALE_DELTA(deltaX, danielNumer, danielDenom);
	SCALE_DELTA(deltaY, danielNumer, danielDenom);
	SCALE_DELTA(deltaR, danielNumer, danielDenom);
	SCALE_DELTA(deltaH, danielNumer, danielDenom);
	SCALE_DELTA(deltaA, danielNumer, danielDenom);

	
	m_fromSceneAnimObject.currX += deltaX;
	m_fromSceneAnimObject.currY += deltaY;
	m_fromSceneAnimObject.currR += deltaR;
	m_fromSceneAnimObject.currB += deltaH;
	m_fromSceneAnimObject.currA += deltaA;
		
	m_ipcBuffer->lock();

	m_dstContext->push();

	m_dstContext->setStrokeColor(PColor32(0x00, 0x00, 0x00, 0x00));
	m_dstContext->setFillColor(PColor32(0x00, 0x00, 0x00, 0xFF));
	m_dstContext->drawRect(0, 0, (int) m_fromSceneSurface->width(), (int) m_fromSceneSurface->height());

	if (m_transitionIsPop) {

		m_dstContext->setFillOpacity(m_fromSceneAnimObject.currA);

		m_dstContext->bitblt(m_fromSceneSurface, m_fromSceneAnimObject.currX, m_fromSceneAnimObject.currY,
							 m_fromSceneAnimObject.currR, m_fromSceneAnimObject.currB);

		m_dstContext->setFillOpacity(m_toSceneAnimObject.currA);

		m_dstContext->bitblt(m_toSceneSurface, m_toSceneAnimObject.currX, m_toSceneAnimObject.currY,
								 m_toSceneAnimObject.currR, m_toSceneAnimObject.currB);
	}
	else {

		m_dstContext->setFillOpacity(m_toSceneAnimObject.currA);

		m_dstContext->bitblt(m_toSceneSurface, m_toSceneAnimObject.currX, m_toSceneAnimObject.currY,
								 m_toSceneAnimObject.currR, m_toSceneAnimObject.currB);

		m_dstContext->setFillOpacity(m_fromSceneAnimObject.currA);

		m_dstContext->bitblt(m_fromSceneSurface, m_fromSceneAnimObject.currX, m_fromSceneAnimObject.currY,
								 m_fromSceneAnimObject.currR, m_fromSceneAnimObject.currB);
	}

	
	m_dstContext->pop();

	m_ipcBuffer->unlock();

	return true;    
}

bool WindowContentTransitionRunner::crossFadeTick()
{
	static const int danielNumer = 2;
	static const int danielDenom = 5;
	
	if (m_toSceneAnimObject.currA == m_toSceneAnimObject.targA) {

		return false;
	}

	int deltaA = m_toSceneAnimObject.targA - m_toSceneAnimObject.currA;

	SCALE_DELTA(deltaA, danielNumer, danielDenom);
	m_toSceneAnimObject.currA += deltaA;

	deltaA = m_fromSceneAnimObject.targA - m_fromSceneAnimObject.currA;
	SCALE_DELTA(deltaA, danielNumer, danielDenom);
	m_fromSceneAnimObject.currA += deltaA;
		
	m_ipcBuffer->lock();

	m_dstContext->push();

	m_dstContext->setStrokeColor(PColor32(0x00, 0x00, 0x00, 0x00));
	m_dstContext->setFillColor(PColor32(0x00, 0x00, 0x00, 0xFF));
	m_dstContext->drawRect(0, 0, (int) m_fromSceneSurface->width(), (int) m_fromSceneSurface->height());

	m_dstContext->setFillOpacity(0xFF);

	m_dstContext->bitblt(m_toSceneSurface, 0, 0,
						 (int) m_toSceneSurface->width(),
						 (int) m_toSceneSurface->height());
	
	m_dstContext->setFillOpacity(m_fromSceneAnimObject.currA);

	m_dstContext->bitblt(m_fromSceneSurface, 0, 0,
							 (int) m_fromSceneSurface->width(),
							 (int) m_fromSceneSurface->height());

	
	m_dstContext->pop();

	m_ipcBuffer->unlock();

	return true;    
}

bool WindowContentTransitionRunner::rotateTick()
{
	static const int danielNumer = 3;
	static const int danielDenom = 5;
	
	if (m_targRotateAngle == m_currRotateAngle) {

		return false;
	}

	int delta = m_targRotateAngle - m_currRotateAngle;
	SCALE_DELTA(delta, danielNumer, danielDenom);

	m_currRotateAngle += delta;
	
	// Painting --------------------------------------------------------------

	m_app->beginPaint();
	
	m_dstContext->push();

	// Fill with black
	m_dstContext->setStrokeColor(PColor32(0x00, 0x00, 0x00, 0x00));
	m_dstContext->setFillColor(PColor32(0x00, 0x00, 0x00, 0xFF));
	m_dstContext->drawRect(0, 0, (int) m_toSceneSurface->width(), (int) m_toSceneSurface->height());

	m_dstContext->translate((int) m_toSceneSurface->width()/2, (int) m_toSceneSurface->height()/2);
	m_dstContext->rotate(m_currRotateAngle);
	m_dstContext->translate(-(int) m_toSceneSurface->width()/2, -(int) m_toSceneSurface->height()/2);
	m_dstContext->bitblt(m_toSceneSurface, 0, 0,
						 (int) m_toSceneSurface->width(),
						 (int) m_toSceneSurface->height());

	m_dstContext->pop();

	m_app->endPaint();
	
	return true;    
}
