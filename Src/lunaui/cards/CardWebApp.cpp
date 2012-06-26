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

#include <palmwebview.h>
#include <palmwebpage.h>
#include <PMem.h>

#include "CardWebApp.h"
#include "EventThrottler.h"
#include "Logging.h"
#include "RemoteWindowData.h"
#include "RoundedCorners.h"
#include "Settings.h"
#include "Utils.h"
#include "WebAppCache.h"
#include "WebAppDeferredUpdateHandler.h"
#include "WebAppManager.h"
#include "WebAppFactory.h"
#include "WebPage.h"
#include "Window.h"
#include "WindowContentTransitionRunner.h"
#include "WindowMetaData.h"
#include "Time.h"
#include "EventReporter.h"
#include "ApplicationDescription.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <glib.h>
#include <string>
#include <PIpcBuffer.h>
#include <PIpcChannel.h>
#include <WebGLES2Context.h>

#define MESSAGES_INTERNAL_FILE "SysMgrMessagesInternal.h"
#include <PIpcMessageMacros.h>

PGContext* CardWebApp::s_rotatedDrawCtxt   = 0;
PGSurface* CardWebApp::s_landscapeDrawSurf = 0;
PGSurface* CardWebApp::s_portraitDrawSurf  = 0;
PGSurface* CardWebApp::s_emulatedLandscapeDrawSurf = 0;
PGSurface* CardWebApp::s_emulatedPortraitDrawSurf  = 0;
PGSurface* CardWebApp::s_modalPortraitDrawSurf  = 0;

static bool PrvIsCompositingMagicRect(int x, int y, int w, int h)
{
	return ((x == 0) && (y == 0) && (w == 1) && (h == 1));
}

CardWebApp::CardWebApp(Window::Type winType, PIpcChannel *channel, ApplicationDescription* desc)
	: WindowedWebApp(0, 0, winType, channel)
	, m_state(StateNormal)
	, m_transitionIsPop(false)
	, m_parentWebApp(0)
	, m_childWebApp(0)
    , m_CardOrientation(Event::Orientation_Invalid)
    , m_orientation(Event::Orientation_Up)
	, m_fixedOrientation(Event::Orientation_Invalid)
	, m_allowsOrientationChange(false)
	, m_pendingResizeWidth(-1)
	, m_pendingResizeHeight(-1)
	, m_pendingOrientation(Event::Orientation_Invalid)
	, m_pendingFullScreenMode(-1)
	, m_enableFullScreen(false)
	, m_doPageUpDownInLandscape(false)
	, m_directRendering(false)
	, m_renderOffsetX(0)
	, m_renderOffsetY(0)
	, m_renderOrientation(Event::Orientation_Up)
	, m_paintingDisabled(false)
	, m_renderingSuspended(false)
{
	if(desc != 0) {
		std::string request = desc->requestedWindowOrientation();


		if(request.length() > 0) {
			if (strcasecmp(request.c_str(), "free") == 0) {
				m_fixedOrientation = Event::Orientation_Invalid;
				m_allowsOrientationChange = true;
			}
			else {
				Event::Orientation appOrient = Event::Orientation_Invalid;
				if (strcasecmp(request.c_str(), "up") == 0)
					appOrient = Event::Orientation_Up;
				else if (strcasecmp(request.c_str(), "down") == 0)
					appOrient = Event::Orientation_Down;
				else if (strcasecmp(request.c_str(), "left") == 0)
					appOrient = Event::Orientation_Left;
				else if (strcasecmp(request.c_str(), "right") == 0)
					appOrient = Event::Orientation_Right;
				else if (strcasecmp(request.c_str(), "landscape") == 0)
					appOrient = Event::Orientation_Landscape;
				else if (strcasecmp(request.c_str(), "portrait") == 0)
					appOrient = Event::Orientation_Portrait;

				m_fixedOrientation = orientationForThisCard(appOrient);

			}
		}

		if (m_winType == Window::Type_Emulated_Card) {
			m_fixedOrientation = orientationForThisCard(Event::Orientation_Up);
		}

	}

	int widthAdj = 0;
	int heightAdj = Settings::LunaSettings()->positiveSpaceTopPadding;

	if (winType == Window::Type_Emulated_Card) {
		m_width = Settings::LunaSettings()->emulatedCardWidth;
		m_height = Settings::LunaSettings()->emulatedCardHeight;
	} else {
		if(winType == Window::Type_ModalChildWindowCard) {
			heightAdj = 0;
			m_width = Settings::LunaSettings()->modalWindowWidth;
			m_height = Settings::LunaSettings()->modalWindowHeight;
		}
		else if(!Settings::LunaSettings()->displayUiRotates || m_fixedOrientation == Event::Orientation_Invalid) {
			m_width = WebAppManager::instance()->currentUiWidth();
			m_height = WebAppManager::instance()->currentUiHeight();
		} else {
			getFixedOrientationDimensions(m_width, m_height, widthAdj, heightAdj);
		}
	}

	m_appBufWidth = m_width;
	m_appBufHeight = m_height;

	m_windowWidth = m_width - widthAdj;
	m_windowHeight = m_height - heightAdj;
	m_setWindowWidth = m_windowWidth;
	m_setWindowHeight = m_windowHeight;

	// --------------------------------------------------------------------------

	if (winType==Window::Type_Emulated_Card || winType == Window::Type_Card ||
	    winType == Window::Type_PIN || winType == Window::Type_Emergency ||
	    winType == Window::Type_ModalChildWindowCard) {
		
		init();

		initGfxSurfacesIfNeeded();
		setOrientation(appOrientationFor(WebAppManager::instance()->orientation()));

	}
	else if (winType == Window::Type_ChildCard) {
		m_state = StateWaitingForNewScene;		
	}

	// for child windows the setup will be done in the attach page call

	if (winType == Window::Type_Card || winType == Window::Type_ChildCard)
		WebAppDeferredUpdateHandler::registerApp(this);
}

CardWebApp::~CardWebApp()
{
	if( page() ) {
		// We do not want to report this if it was parked; it was already reported WHEN it was parked.
		EventReporter::instance()->report( "close", static_cast<ProcessBase*>(page())->appId().c_str() );
	}

	if (m_childWebApp) {
		m_childWebApp->setParentCardWebApp(0);
		m_childWebApp->close();
	}		
	
	if (m_parentWebApp)
		m_parentWebApp->removeChildCardWebApp(this);

	m_data->cleanupSceneTransition();

	if (m_winType == Window::Type_ChildCard) {
		m_data = 0;
		m_metaDataBuffer = 0;
	}

	if (m_winType == Window::Type_Card || m_winType == Window::Type_ChildCard)
		WebAppDeferredUpdateHandler::unregisterApp(this);
}

void CardWebApp::inputEvent(sptr<Event> e)
{
	if (m_childWebApp) {
		m_childWebApp->inputEvent(e);
		return;
	}
	
	if (!((m_state == StateNormal) || (m_state == StateRunningTransition))) {
		// drop all input events
		return;
	}

	switch (e->type) {
	case (Event::KeyDown):
	case (Event::KeyUp): {

		bool inLandScapeMode = false;
		if (m_doPageUpDownInLandscape &&
			(m_orientation == Event::Orientation_Left ||
			 m_orientation == Event::Orientation_Right)) {
			inLandScapeMode = true;
		}

		if (inLandScapeMode) {
			if (e->key == Event::Key_CoreNavi_Back || e->key == Event::Key_CoreNavi_Previous) {
				e->key = (m_orientation == Event::Orientation_Right) ? Event::Key_PageDown : Event::Key_PageUp;
				e->setGestureKey(false);
			}
			else if (e->key == Event::Key_CoreNavi_Menu || e->key == Event::Key_CoreNavi_Next) {
				e->key = (m_orientation == Event::Orientation_Right) ? Event::Key_PageUp : Event::Key_PageDown;
				e->setGestureKey(false);
			}
		}

		if (e->key >= Event::Key_Up && e->key <= Event::Key_Right) {

			// Reject cursor events if we are in the middle of a gesture
			// (indicated by m_blockPenEvents)
			if (m_blockPenEvents)
				return;

			switch (m_orientation) {
			case (Event::Orientation_Down): {

				switch (e->key) {
				case (Event::Key_Left):
					e->key = Event::Key_Right;
					break;
				case (Event::Key_Right):
					e->key = Event::Key_Left;
					break;
				case (Event::Key_Up):
					e->key = Event::Key_Down;
					break;
				case (Event::Key_Down):
					e->key = Event::Key_Up;
					break;
				default:
					break;
				}

				break;
			}
			case (Event::Orientation_Left): {

				switch (e->key) {
				case (Event::Key_Left):
					e->key = Event::Key_Down;
					break;
				case (Event::Key_Right):
					e->key = Event::Key_Up;
					break;
				case (Event::Key_Up):
					e->key = Event::Key_Left;
					break;
				case (Event::Key_Down):
					e->key = Event::Key_Right;
					break;
				default:
					break;
				}

				
				break;
			}
			case (Event::Orientation_Right): {

				switch (e->key) {
				case (Event::Key_Left):
					e->key = Event::Key_Up;
					break;
				case (Event::Key_Right):
					e->key = Event::Key_Down;
					break;
				case (Event::Key_Up):
					e->key = Event::Key_Right;
					break;
				case (Event::Key_Down):
					e->key = Event::Key_Left;
					break;
				default:
					break;
				}

				break;
			}
			default:
				break;
			}

		}

		break;
	}
	case (Event::PenDown):
	case (Event::PenUp):
	case (Event::PenMove):
	case (Event::PenFlick):
	case (Event::PenPressAndHold):
	case (Event::PenCancel):
	case (Event::PenCancelAll):
	case (Event::PenSingleTap): {
	
		switch (m_orientation) {
		case (Event::Orientation_Right): {

			int tmp = e->x;
			e->x = m_windowWidth - e->y;
			e->y = tmp;

			tmp = e->flickXVel;
			e->flickXVel = - e->flickYVel;
			e->flickYVel = tmp;			
			break;
		}
		case (Event::Orientation_Left): {

			int tmp = e->x;
			e->x = e->y;
			e->y = m_windowHeight - tmp;

			tmp = e->flickXVel;
			e->flickXVel = e->flickYVel;
			e->flickYVel = - tmp;						
			break;
		}
		case (Event::Orientation_Down): {
			e->x = m_windowWidth - e->x;
			e->y = m_windowHeight - e->y;
			e->flickXVel = - e->flickXVel;
			e->flickYVel = - e->flickYVel;
			break;
		}
		default:
			break;
		}

		break;
	}
	case (Event::GestureStart):
	case (Event::GestureChange):
	case (Event::GestureEnd): {

		switch (m_orientation) {
		case (Event::Orientation_Right): {
			int tmp = e->gestureCenterX;
			e->gestureCenterX = m_windowWidth - e->gestureCenterY;
			e->gestureCenterY = tmp;			
			break;
		}
		case (Event::Orientation_Left): {
			int tmp = e->gestureCenterX;
			e->gestureCenterX = e->gestureCenterY;
			e->gestureCenterY = m_windowHeight - tmp;
			break;
		}
		case (Event::Orientation_Down): {
			e->gestureCenterX = m_windowWidth - e->gestureCenterX;
			e->gestureCenterY = m_windowHeight - e->gestureCenterY;		
			break;
		}
		default:
			break;
		}

		e->gestureCenterX = CLAMP(e->gestureCenterX, 0, (int) m_windowWidth - 1);
		e->gestureCenterY = CLAMP(e->gestureCenterY, 0, (int) m_windowHeight - 1);

	}
	default:
		break;
	}
			
	WindowedWebApp::inputEvent(e);	
}

void CardWebApp::keyEvent(QKeyEvent* e)
{
	if (m_childWebApp) {
		m_childWebApp->keyEvent(e);
		return;
	}	
	
	bool inLandScapeMode = false;
	if (m_doPageUpDownInLandscape &&
		(m_orientation == Event::Orientation_Left ||
		 m_orientation == Event::Orientation_Right)) {
		inLandScapeMode = true;
	}

	if (inLandScapeMode) {
		if (e->key() == Qt::Key_CoreNavi_Back ||
			e->key() == Qt::Key_CoreNavi_Previous) {
			int ev_key = (m_orientation == Event::Orientation_Right) ? Qt::Key_PageDown : Qt::Key_PageUp;
			QKeyEvent ev(e->type(), ev_key, e->modifiers());
			WindowedWebApp::keyEvent(&ev);
			return;
		}
		else if (e->key() == Qt::Key_CoreNavi_Menu ||
				 e->key() == Qt::Key_CoreNavi_Next) {
			int ev_key = (m_orientation == Event::Orientation_Right) ? Qt::Key_PageUp : Qt::Key_PageDown;
			QKeyEvent ev(e->type(), ev_key, e->modifiers());
			WindowedWebApp::keyEvent(&ev);
			return;
		}
	}

	if (e->key() >= Qt::Key_Left && e->key() <= Qt::Key_Down) {

		// Reject cursor events if we are in the middle of a gesture
		// (indicated by m_blockPenEvents)
		if (m_blockPenEvents)
			return;

		int orientedKey = e->key();

		switch (m_orientation) {
		case (Event::Orientation_Down): {

			switch (e->key()) {
			case (Qt::Key_Left):
				orientedKey = Qt::Key_Right;
				break;
			case (Qt::Key_Right):
				orientedKey = Qt::Key_Left;
				break;
			case (Qt::Key_Up):
				orientedKey = Qt::Key_Down;
				break;
			case (Qt::Key_Down):
				orientedKey = Qt::Key_Up;
				break;
			default:
				break;
			}

			break;
		}
		case (Event::Orientation_Left): {

			switch (e->key()) {
			case (Qt::Key_Left):
				orientedKey = Qt::Key_Down;
				break;
			case (Qt::Key_Right):
				orientedKey = Qt::Key_Up;
				break;
			case (Qt::Key_Up):
				orientedKey = Qt::Key_Left;
				break;
			case (Qt::Key_Down):
				orientedKey = Qt::Key_Right;
				break;
			default:
				break;
			}

				
			break;
		}
		case (Event::Orientation_Right): {

			switch (e->key()) {
			case (Qt::Key_Left):
				orientedKey = Qt::Key_Up;
				break;
			case (Qt::Key_Right):
				orientedKey = Qt::Key_Down;
				break;
			case (Qt::Key_Up):
				orientedKey = Qt::Key_Right;
				break;
			case (Qt::Key_Down):
				orientedKey = Qt::Key_Left;
				break;
			default:
				break;
			}

			break;
		}
		default:
			break;
		}

		QKeyEvent ev(e->type(), orientedKey, e->modifiers());
		WindowedWebApp::keyEvent(&ev);
		return;
	}

	
	WindowedWebApp::keyEvent(e);
}

void CardWebApp::sceneTransitionFinished()
{
	if (G_UNLIKELY(Settings::LunaSettings()->perfTesting)) {
		g_message("SYSMGR PERF: SCENE FINISH appid: %s, processid: %s, type: %s, time: %d",
				  m_page ? m_page->appId().c_str() : "unknown",
				  m_page ? m_page->processId().c_str() : "unknown",
				  WebAppFactory::nameForWindowType(m_winType).c_str(),
				  Time::curTimeMs());
	}

	if (m_childWebApp) {
		m_childWebApp->sceneTransitionFinished();
		return;
	}
	
	animationFinished();

	if (m_page && m_page->webkitPage())
		m_page->webkitPage()->evaluateScript("Mojo.sceneTransitionCompleted()");    
}

void CardWebApp::paint(bool clipToWindow)
{
	if(m_renderingSuspended)
		return;

	stopPaintTimer();
	
	if (!appLoaded()) {
		// If clipToWindow is set to false, then we are forcing paint. ignore the
		// fact that the app has not fully loaded yet
		if (clipToWindow)
			return;
	}

	if (m_childWebApp) {
		m_paintRect = QRect();
		return;
	}
		
	luna_assert(!m_beingDeleted);

	if (m_beingDeleted) {
		g_critical("FATAL ERROR: Being painted when deleted\n");
	}

	if (m_paintingDisabled) {
		return;
	}

	if (m_paintRect.isEmpty())
		return;

	if (clipToWindow) {
		QRect windowRect(0, 0, m_appBufWidth, m_appBufHeight);
		m_paintRect &= windowRect;
		if (m_paintRect.isEmpty())
			return;
	}

	if (m_state == StateChildScene || m_state == StateWaitingForNewScene) {
		m_paintRect = QRect();
		return;
	}

	// px, py, pw, ph are the painted coords in unrotated coords
	int px = m_paintRect.x();
	int py = m_paintRect.y();
	int pw = m_paintRect.width();
	int ph = m_paintRect.height();

	if (!m_data->supportsPartialUpdates()) {
		px = 0;
		py = 0;
		pw = m_appBufWidth;
		ph = m_appBufHeight;
	}

	int tx = px;
	int ty = py;
	int tw = pw;
	int th = ph;
	
    m_paintRect.setRect(0, 0, 0, 0);

	//uint32_t paintStartTime = Time::curTimeMs();
	
	PGContext* gc = m_data->renderingContext();
	m_data->beginPaint();

	bool requiresScreenUpdate = paint(px, py, pw, ph, tx, ty, tw, th);
	
	if (G_UNLIKELY(m_showPageStats))
		renderPageStatistics(m_renderOffsetX, m_renderOffsetY);

	bool rotatedBlit = ((m_orientation != Event::Orientation_Up) &&
						(m_orientation != Event::Orientation_Invalid)) ||
					   ((m_renderOrientation != Event::Orientation_Up) &&
						(m_renderOrientation != Event::Orientation_Invalid));
	
	if (!rotatedBlit) {
		int preserveX = tx + m_renderOffsetX;
		int preserveY = ty + m_renderOffsetY;
		int preserveR = preserveX + tw;
		int preserveB = preserveY + th;

		preserveX = MAX(0, preserveX);
		preserveY = MAX(0, preserveY);
		preserveR = MIN(m_width, preserveR);
		preserveB = MIN(m_height, preserveB);

		preserveX = MIN(preserveX, preserveR-1);
		preserveY = MIN(preserveY, preserveB-1);

		m_data->endPaint(true, PRect(preserveX, preserveY, preserveR, preserveB), requiresScreenUpdate);
	}
	else {
		m_data->endPaint(false, PRect(), requiresScreenUpdate);
	}

	//uint32_t paintEndTime = Time::curTimeMs();
	//printf("Paint took: %d ms\n", paintEndTime - paintStartTime);

	if (!m_showPageStats) {
		if (!rotatedBlit) {
			m_data->sendWindowUpdate(tx, ty, tw, th);
		} else { 
			// FIXME: Optimize
			m_data->sendWindowUpdate(0, 0, m_width, m_height);
		}
	}
	
	if (!m_paintRect.isEmpty())
		startPaintTimer();

	if (false)
	{
		static uint32_t startTime = Time::curTimeMs();
		static int frameCount = 0;
		const uint32_t interval = 100;

		uint32_t currTime = Time::curTimeMs();

		if ((currTime - startTime) > interval) {
			int fps = (frameCount * 1000.0) / (currTime - startTime);
			startTime = currTime;
			frameCount = 0;
			
			printf("FPS: %d\n", fps);
		}
		else {
			frameCount++;
		}
	}
}

bool CardWebApp::paint(int px, int py, int pw, int ph,
					   int& tx, int& ty, int& tw, int& th)
{
	PGContext* paintContext = m_data->renderingContext();
	PGSurface* tmpSurface = 0;

	Event::Orientation orientation = m_orientation;
	if ((m_renderOrientation != Event::Orientation_Up) &&
		(m_renderOrientation != Event::Orientation_Invalid))
		orientation = (Event::Orientation) m_renderOrientation;
	
	bool rotatedBlit = ((orientation != Event::Orientation_Up) &&
						(orientation != Event::Orientation_Invalid));

	if (rotatedBlit) {

		if (!PrvIsCompositingMagicRect(px, py, pw, ph)) {
			px = 0;
			py = 0;
			pw = m_appBufWidth;
			ph = m_appBufHeight;
		}

		paintContext = s_rotatedDrawCtxt;

		if (isEmulatedCardOrChild()) {
			if (orientation == Event::Orientation_Down)
				tmpSurface = s_emulatedPortraitDrawSurf;
			else
				tmpSurface = s_emulatedLandscapeDrawSurf;
		}
		else {
			if (m_winType == Window::Type_ModalChildWindowCard) {
				tmpSurface = s_modalPortraitDrawSurf;
			}
			else {
				if (orientation == Event::Orientation_Down)
					tmpSurface = s_portraitDrawSurf;
				else
					tmpSurface = s_landscapeDrawSurf;
			}
		}

		paintContext->setSurface(tmpSurface);
	}
	
	paintContext->push();

	int angle = 0;

	bool emulatedOrModalCard = (isEmulatedCardOrChild() || m_winType == Window::Type_ModalChildWindowCard)? true:false;

	if (false == emulatedOrModalCard && !rotatedBlit) {
		paintContext->setGlobalClipRect(px + m_renderOffsetX,
										py + m_renderOffsetY,
										px + pw + m_renderOffsetX,
										py + ph + m_renderOffsetY);

		paintContext->translate(m_renderOffsetX, m_renderOffsetY);
		m_data->rotate(0);

	} else {
		m_data->rotate(angleForOrientation(orientation));
		paintContext->setGlobalClipRect(px, py, px + pw, py + ph);
	}

	if (m_data->needsClear())
		paintContext->clearRect(px, py, px+pw, py+ph, PColor32(0, 0, 0, 0));

	paintContext->translate(px, py);
	m_data->translate(m_renderOffsetX, m_renderOffsetY);

	bool onlyComposite = PrvIsCompositingMagicRect(px, py, pw, ph);
	bool usedContextParam = m_page->webkitView()->paint(paintContext, px, py, pw, ph, onlyComposite, false);

	// This currently forces webapps to release their texture mappings after each offscreen paint.
	// We may want to be smarter about handling apps that update frequently while carded.
	if (!m_directRendering) {
		m_page->webkitView()->unmapCompositingTextures();
	}

	paintContext->pop();

	if (rotatedBlit && usedContextParam) {

		//uint32_t paintStartTime = Time::curTimeMs();

		paintContext = m_data->renderingContext();
		paintContext->push();

		tmpSurface->pgPixmap()->SetQuality(PSNearest);
		paintContext->pgContext()->SetAttribute(0, 0); // turn off anti-aliasing
		paintContext->setCompositeOperation(PCopyMode); // no need to alpha-blend
		
		int centerX = tmpSurface->width()/2;
		int centerY = tmpSurface->height()/2;

		if (orientation != Event::Orientation_Down) {
			int tmp;
			tmp = centerX;
			centerX = centerY;
			centerY = tmp;
		}

		paintContext->translate(centerX, centerY);
		paintContext->rotate(angleForOrientation(orientation));

		if (orientation != Event::Orientation_Down) {
			int tmp;
			tmp = centerX;
			centerX = centerY;
			centerY = tmp;
		}
		
		paintContext->translate(-centerX, -centerY);

		paintContext->translate(m_renderOffsetX, m_renderOffsetY);		
		
		paintContext->bitblt(tmpSurface,
							 px, py, px + pw, py + ph,
							 px, py, px + pw, py + ph);

		tmpSurface->pgPixmap()->SetQuality();
		paintContext->pgContext()->SetAttribute(0, 1); // turn off anti-aliasing

		paintContext->pop();

		//uint32_t paintEndTime = Time::curTimeMs();
		//printf("Rotated blit took: %d at %d, %d with rect: %d:%d, %d:%dms\n",
		//	   paintEndTime - paintStartTime, m_renderOffsetX, m_renderOffsetY,
		//	   px, px+pw, py, py+ph);
			   
	}

	return usedContextParam;
}

void CardWebApp::paint(NativeGraphicsContext* dstContext, NativeGraphicsSurface* dstSurface,
					   int px, int py, int pw, int ph,
					   int& tx, int& ty, int& tw, int& th)
{
	PGContext* paintContext = dstContext;
	PGSurface* paintSurface = dstSurface;

	if (m_orientation != Event::Orientation_Up) {

		if (isEmulatedCardOrChild()) {
			if (m_orientation == Event::Orientation_Down)
				paintSurface = s_emulatedPortraitDrawSurf;
			else
				paintSurface = s_emulatedLandscapeDrawSurf;
		}
		else {
			if (m_winType == Window::Type_ModalChildWindowCard) {
				paintSurface = s_modalPortraitDrawSurf;
			}
			else {
				if (m_orientation == Event::Orientation_Down)
					paintSurface = s_portraitDrawSurf;
				else
					paintSurface = s_landscapeDrawSurf;
			}
		}

		paintContext = s_rotatedDrawCtxt;
	}
	
	paintContext->push();

	paintContext->translate(px, py);

	paintContext->setGlobalClipRect(px, py, px + pw, py + ph);

	paintContext->setSurface(paintSurface);

	m_page->webkitView()->paint(paintContext, px, py, pw, ph, false, false);

	paintContext->pop();

	if (m_orientation != Event::Orientation_Up) {

		dstContext->push();

		dstContext->setSurface(dstSurface);

		dstSurface->pgPixmap()->SetQuality(PSNearest);
		paintSurface->pgPixmap()->SetQuality(PSNearest);
		dstContext->pgContext()->SetAttribute(0, 0); // turn off anti-aliasing
		
		dstContext->translate((int) dstSurface->width() / 2,
							  (int) dstSurface->height() / 2);
		dstContext->rotate(angleForOrientation(m_orientation));
		dstContext->translate(-(int) paintSurface->width() / 2,
							  -(int) paintSurface->height() / 2);

		dstContext->bitblt(paintSurface, px, py, px + pw, py + ph,
						   px, py, px + pw, py + ph);

		dstSurface->pgPixmap()->SetQuality();
		paintSurface->pgPixmap()->SetQuality();
		dstContext->pgContext()->SetAttribute(0, 1); // turn on anti-aliasing

		dstContext->pop();

		switch (m_orientation) {
		case (Event::Orientation_Down): {

			tx = m_width - (px + pw);
			ty = m_height - (py + ph);
			tw = pw;
			th = ph;
			break;
		}
		case (Event::Orientation_Right): {

			tx = py;
			ty = m_width - (px + pw);
			tw = ph;
			th = pw;
			break;
		}
		case (Event::Orientation_Left): {

			tx = m_height - (py + ph);
			ty = px;
			tw = ph;
			th = pw;
			break;
		}
		default:
			break;
		}
	}
}

void CardWebApp::focusedEvent(bool focused)
{
	if (m_childWebApp)
		m_childWebApp->focusedEvent(focused);

	WindowedWebApp::focusedEvent(focused);

	if (focused)
		focusActivity();
	else
		blurActivity();
}

int CardWebApp::resizeEvent(int newWidth, int newHeight, bool resizeBuffer)
{
	if (m_setWindowWidth == newWidth && m_setWindowHeight == newHeight) {
		// Force a full app repaint (not clipped to window dimensions)
		m_paintRect.setRect(0, 0, m_appBufWidth, m_appBufHeight);

		forcePaint(false);

		g_message("ROTATION: [%s]: Skipping ResizeEvent, but forcing app repaint. newWidth = %d, newHeight = %d",
				   __PRETTY_FUNCTION__, newWidth, newHeight);
		return -1;
	}

	m_setWindowWidth = newWidth;
	m_setWindowHeight = newHeight;

	if (m_childWebApp) {
		m_childWebApp->resizeEvent(newWidth, newHeight, resizeBuffer);
		m_pendingResizeWidth = newWidth;
		m_pendingResizeHeight = newHeight;
		return -1;
	}
	
	if (!m_page || !m_page->webkitPage())
		goto Done;

	// We allow window resize only in UP orientation mode (Except for Dock Mode Windows)
	if ((m_orientation != Event::Orientation_Up) && (m_winType != Window::Type_DockModeWindow))
		goto Done;

	if ((int) m_windowWidth == newWidth &&
		(int) m_windowHeight == newHeight)
	{
		if((m_pendingResizeWidth != -1) && (m_pendingResizeHeight != -1))
		{
			// if there are pending changes, make sure they are updated to the new values
			m_pendingResizeWidth = newWidth;
			m_pendingResizeHeight = newHeight;
		}
		goto Done;
	}

	if (m_state != StateNormal) {
		m_pendingResizeWidth = newWidth;
		m_pendingResizeHeight = newHeight;
		goto Done;
	}

	if(resizeBuffer) {
		int oldKey = m_data->key();
		m_data->resize(newWidth, newHeight);

		int newKey = m_data->key();

		m_appBufWidth = newWidth;
		m_appBufHeight = newHeight;

		m_windowWidth = newWidth;

		m_windowHeight = newHeight - (m_enableFullScreen ? 0 : Settings::LunaSettings()->positiveSpaceTopPadding);

		m_width = newWidth;
		m_height = newHeight;

		//Note that no special code is needed for emulated cards here as windowheight
		//was appropriately calculated above.
		m_page->webkitView()->resize(newWidth, newHeight);

		m_paintRect.setRect(0, 0, m_appBufWidth, m_appBufHeight);

		forcePaint(true);

		WebAppManager::instance()->windowedAppKeyChanged(this, oldKey);

		setVisibleDimensions(m_width, m_height - (m_enableFullScreen ? 0 : Settings::LunaSettings()->positiveSpaceTopPadding));


		g_message("%s: Resized window (%s) to new Dimensions: %d, %d. New Key: %d",
				  __PRETTY_FUNCTION__, this->appId().c_str(), newWidth, newHeight, newKey);
		return newKey;
	} else {
		m_windowHeight = newHeight;
		if(m_fixedOrientation == Event::Orientation_Invalid) {
			m_windowWidth = newWidth;
		} else {
			m_windowWidth = m_width;
		}

		if(Window::Type_ModalChildWindowCard == windowType()) {
			m_page->webkitView()->resize(m_width, newHeight);
		}
		else {
			m_page->webkitView()->resize(m_windowWidth, m_windowHeight);
		}

		if(Window::Type_ModalChildWindowCard != windowType())
			setVisibleDimensions(m_width, m_height - (m_enableFullScreen ? 0 : Settings::LunaSettings()->positiveSpaceTopPadding));
		else
			setVisibleDimensions(m_width, newHeight);

		// Force a full app repaint (not clipped to window dimensions)
		m_paintRect.setRect(0, 0, m_appBufWidth, m_appBufHeight);

		forcePaint(false);
	}

Done:

	return -1;
}

void CardWebApp::flipEvent(int newWidth, int newHeight)
{
	if (m_childWebApp) {
		m_childWebApp->flipEvent(newWidth, newHeight);
		return;
	}

	m_data->flip();

	int tempWidth = m_width;
	m_width = m_height;
	m_height = tempWidth;

	m_appBufWidth = m_width;
	m_appBufHeight = m_height;

	m_pendingResizeWidth = -1;
	m_pendingResizeHeight = -1;

	m_windowWidth = m_width;
	if (isEmulatedCardOrChild()) {
		m_windowHeight = m_height;
	} else if(m_winType == Window::Type_ModalChildWindowCard) {
		m_windowHeight = newHeight;
	}
	else {
		m_windowHeight = m_height - (m_enableFullScreen ? 0 : Settings::LunaSettings()->positiveSpaceTopPadding);
	}

	m_setWindowWidth = m_windowWidth;
	m_setWindowHeight = m_windowHeight;
	m_page->webkitView()->resize(m_windowWidth, m_windowHeight);

	m_paintRect.setRect(0, 0, m_appBufWidth, m_appBufHeight);

	forcePaint(true);

	setVisibleDimensions(m_windowWidth, m_windowHeight);
}

void CardWebApp::asyncFlipEvent(int newWidth, int newHeight, int newScreenWidth, int newScreenHeight)
{
	if (m_childWebApp) {
		m_childWebApp->asyncFlipEvent(newWidth, newHeight, newScreenWidth, newScreenHeight);
		return;
	}

	if (m_data)
		m_data->flip();

	int tempWidth = m_width;
	m_width = m_height;
	m_height = tempWidth;

	m_pendingResizeWidth = -1;
	m_pendingResizeHeight = -1;

	m_appBufWidth = m_width;
	m_appBufHeight = m_height;

	m_windowWidth = m_width;
	if (isEmulatedCardOrChild()) {
		m_windowHeight = m_height;
	} else if (m_winType == Window::Window::Type_ModalChildWindowCard ){
		m_windowHeight = newHeight;
	}
	else {
		m_windowHeight = m_height - (m_enableFullScreen ? 0 : Settings::LunaSettings()->positiveSpaceTopPadding);
	}

	m_page->webkitView()->resize(m_windowWidth, m_windowHeight);

	m_paintRect.setRect(0, 0, m_windowWidth, m_windowHeight);
	forcePaint(true);

	// notify Host that this window is done resizing
	m_channel->sendAsyncMessage(new ViewHost_AsyncFlipCompleted(routingId(), newWidth, newHeight, newScreenWidth, newScreenHeight));

	setVisibleDimensions(m_windowWidth, m_windowHeight);
}

void CardWebApp::setOrientation(Event::Orientation orient)
{
	switch (orient) {
	case Event::Orientation_Up:
		break;
	case Event::Orientation_Left:
		break;
	case Event::Orientation_Down:
		break;
	case Event::Orientation_Right:
		break;
	default:
		break;
	}
	if (m_childWebApp) {
		m_childWebApp->setOrientation(orient);
		return;
	}
	
	switch (orient) {
	case (Event::Orientation_Up):    
	case (Event::Orientation_Down):  
	case (Event::Orientation_Left):  
	case (Event::Orientation_Right):
		break;
	default:
		return;
	}
	
    // Keep this variable always up-to-date
    m_CardOrientation = orient;

	if(Settings::LunaSettings()->displayUiRotates && (!isEmulatedCardOrChild())) {
		return;
	} else {

		if (m_state != StateNormal) {
			m_pendingOrientation = orient;
			return;
		}

		orient = orientationForThisCard(orient);

		resizeWindowForOrientation(orient);

		callMojoScreenOrientationChange();
	}
}

Event::Orientation CardWebApp::orientation() const
{
    if (Event::Orientation_Invalid != m_fixedOrientation)
    {
        return m_fixedOrientation;
    }
    else
    {
        return m_CardOrientation;
    }
}

void CardWebApp::resizeWindowForOrientation(Event::Orientation orient)
{
	if (!m_allowsOrientationChange) {
		return;
	}

	if (orient == m_orientation)
		return;

	Event::Orientation oldOrientation = m_orientation;
	m_orientation = orient;

	switch (orient) {
	case (Event::Orientation_Left):
	case (Event::Orientation_Right): {
		// Full screen in this mode
		m_windowWidth = m_height;
		m_windowHeight = m_width;
		m_appBufWidth = m_height;
		m_appBufHeight = m_width;
		break;
	}
	case (Event::Orientation_Down): {
		// Full screen in this mode
		m_windowWidth = m_width;
		m_windowHeight = m_height;
		m_appBufWidth = m_width;
		m_appBufHeight = m_height;
		break;
	}
	case (Event::Orientation_Up):
	default: {
		m_windowWidth = m_width;
		m_windowHeight = m_height - (m_enableFullScreen ? 0 :
									 Settings::LunaSettings()->positiveSpaceTopPadding);
		m_appBufWidth = m_width;
		m_appBufHeight = m_height;
		break;
	}
	}

	int savedWindowWidth = m_windowWidth;
	int savedWindowHeight = m_windowHeight;
	m_windowWidth = m_appBufWidth;
	m_windowHeight = m_appBufHeight;

	m_page->webkitView()->resize(m_windowWidth, m_windowHeight);

	if (m_orientation == Event::Orientation_Up && !m_enableFullScreen) {
		setVisibleDimensions(m_windowWidth, m_windowHeight);
	}
	else {
		setVisibleDimensions(m_width, m_height);
	}	
	
	// Force a full paint onto a temporary surface
	PGSurface* toSceneSurface = PGSurface::create(m_width, m_height, false);
	PGContext* toSceneContext = PGContext::create();
	toSceneContext->setSurface(toSceneSurface);

	// Force a full app repaint (not clipped to window dimensions)
	int tx = 0;
	int ty = 0;
	int tw = m_appBufWidth;
	int th = m_appBufHeight;
	m_paintRect.setRect(tx, ty, tw, th);
	paint(toSceneContext, toSceneSurface,
		  tx, ty, tw, th, tx, ty, tw, th);

	// Now we can kick off the animation
	int currAngleForAnim = angleForOrientation(oldOrientation) - angleForOrientation(m_orientation);
	if (currAngleForAnim > 180)
		currAngleForAnim = -90;
	else if (currAngleForAnim < -180)
		currAngleForAnim = 90;
	int targAngleForAnim = 0;

	updateWindowProperties();

	PGContext* gc = m_data->renderingContext();
	
	WindowContentTransitionRunner::instance()->runRotateTransition(this,
																   toSceneSurface,
																   gc,
																   currAngleForAnim,
																   targAngleForAnim);

	toSceneSurface->releaseRef();
	toSceneContext->releaseRef();

	if (savedWindowWidth != (int) m_windowWidth ||
		savedWindowHeight != (int) m_windowHeight) {
		m_windowWidth = savedWindowWidth;
		m_windowHeight = savedWindowHeight;
		m_page->webkitView()->resize(m_windowWidth, m_windowHeight);
		//The visible dimensions were artificially expanded above.  After resizing webkit, we need
		//to also resize the visible dimensions

		if (m_orientation == Event::Orientation_Up && !m_enableFullScreen) {
			setVisibleDimensions(m_windowWidth, m_windowHeight);
		}
		else {
			setVisibleDimensions(m_width, m_height);
		}
	}

	m_channel->sendAsyncMessage(new ViewHost_Card_SetAppOrientation(routingId(), orient));
	
	animationFinished();

}

void CardWebApp::resizeWindowForFixedOrientation(Event::Orientation orient)
{
	if (!Settings::LunaSettings()->displayUiRotates || !m_allowsOrientationChange)
		return;

	if (orient == Event::Orientation_Invalid)
		return;

	// already there?
//	if (orient == m_fixedOrientation)
//		return;

	// sanity check (when the UI rotates the app orientation should always be UP)
	if (Event::Orientation_Up != m_orientation)
		return;

	m_fixedOrientation = orient;

	int widthAdj = Settings::LunaSettings()->positiveSpaceTopPadding;
	int heightAdj = 0;

	getFixedOrientationDimensions(m_width, m_height, widthAdj, heightAdj);

	m_windowWidth = m_width - (m_enableFullScreen ? 0 : widthAdj);
	m_windowHeight = m_height - (m_enableFullScreen ? 0 : heightAdj);

	m_appBufWidth = m_width;
	m_appBufHeight = m_height;

	m_setWindowWidth = m_windowWidth;
	m_setWindowHeight = m_windowHeight;

	if(m_page) {
		m_page->webkitView()->resize(m_windowWidth, m_windowHeight);
	}

	setVisibleDimensions(m_windowWidth, m_windowHeight);
}

void CardWebApp::getFixedOrientationDimensions(int& width, int& height, int& wAdjust, int& hAdjust)
{
	bool appRequestedPortrait = false;
	bool uiIsInPortraitMode = false;

	if(WebAppManager::instance()->currentUiWidth() >= WebAppManager::instance()->currentUiHeight()) {
		uiIsInPortraitMode = false;
	} else {
		uiIsInPortraitMode = true;
	}

	appRequestedPortrait = isOrientationPortrait(m_fixedOrientation);

	if(appRequestedPortrait == uiIsInPortraitMode) {
		width = WebAppManager::instance()->currentUiWidth();
		height = WebAppManager::instance()->currentUiHeight();

		wAdjust = 0;
		hAdjust = Settings::LunaSettings()->positiveSpaceTopPadding;
	} else {
		width = WebAppManager::instance()->currentUiHeight();
		height = WebAppManager::instance()->currentUiWidth();

		wAdjust = Settings::LunaSettings()->positiveSpaceTopPadding;
		hAdjust = 0;
	}
}

bool CardWebApp::isOrientationPortrait(Event::Orientation orient)
{
	bool isPortrait = false;

	switch (orient) {
		case (Event::Orientation_Left):
		case (Event::Orientation_Right):
		{
			if(Settings::LunaSettings()->homeButtonOrientationAngle == 0 || Settings::LunaSettings()->homeButtonOrientationAngle == 180)
				isPortrait = !WebAppManager::instance()->isDevicePortraitType();
			else
				isPortrait = WebAppManager::instance()->isDevicePortraitType();
			break;
		}

		case (Event::Orientation_Landscape): {
			isPortrait = false;
			break;
		}

		case (Event::Orientation_Up):
		case (Event::Orientation_Down):
		{
			if(Settings::LunaSettings()->homeButtonOrientationAngle == 0 || Settings::LunaSettings()->homeButtonOrientationAngle == 180)
				isPortrait = WebAppManager::instance()->isDevicePortraitType();
			else
				isPortrait = !WebAppManager::instance()->isDevicePortraitType();
			break;
		}

		case (Event::Orientation_Portrait): {
			isPortrait = true;
			break;
		}

		default: {
			isPortrait = false;
		}
	}
	return isPortrait;
}

void CardWebApp::setFixedOrientation(Event::Orientation orient)
{

	if(!Settings::LunaSettings()->displayUiRotates) {
		m_fixedOrientation = orientationForThisCard(orient);
		setAllowOrientationChange(true);
		setOrientation(m_fixedOrientation);
	} else {

		if (isEmulatedCardOrChild())
        {
            //Concepts of "left" and "right" are backwards on emulated cards because of
            //legacy compatibility with the phones.
            Event::Orientation sysOrient = adjustEmuModeLeftRight(orient);

			m_fixedOrientation = orientationForThisCard(sysOrient);
        }
		else
			m_fixedOrientation = orientationForThisCard(orient);

		if (isEmulatedCardOrChild()) {
			if (orient == Event::Orientation_Invalid) {
				setAllowOrientationChange(true);
			} else {

				//Must change this field in order to allow the
				//orientation change to happen.
				m_allowsOrientationChange = true;
				//Bypassing setOrientation() to prevent any translating
				//of the incoming orientation.  This function deals with
				//app orientations rather than system ones, as its
				//main caller is JsSysObject.
				resizeWindowForOrientation(orient);
				callMojoScreenOrientationChange(orient);
				//Setting a fixed orientation blocks orientation changes.
				setAllowOrientationChange(false);
			}
		}

		m_channel->sendAsyncMessage(new ViewHost_Card_SetAppFixedOrientation(routingId(), m_fixedOrientation, isOrientationPortrait(m_fixedOrientation)));

	}
}

void CardWebApp::setAllowOrientationChange(bool value)
{
	if (!isEmulatedCardOrChild())
		return;

	m_allowsOrientationChange = value;
	m_channel->sendAsyncMessage(new ViewHost_Card_SetFreeOrientation(routingId(), value));
}

bool CardWebApp::allowsOrientationChange() const
{
	return m_allowsOrientationChange;    
}

void CardWebApp::drawCornerWindows(NativeGraphicsContext* context)
{
#ifndef FIX_FOR_QT
	PGSurface* topLeftCornerSurf = RoundedCorners::topLeft();
	PGSurface* topRightCornerSurf = RoundedCorners::topRight();
	PGSurface* bottomLeftCornerSurf = RoundedCorners::bottomLeft();
	PGSurface* bottomRightCornerSurf = RoundedCorners::bottomRight();
	
	if (!topLeftCornerSurf || !topRightCornerSurf || !bottomLeftCornerSurf || !bottomRightCornerSurf)
		return;

	int width = m_windowWidth;
	int height = m_windowHeight;
	int fullWidth = m_appBufWidth;
	int fullHeight = m_appBufHeight;
	if (m_orientation == Event::Orientation_Left ||
		m_orientation == Event::Orientation_Right) {
		width = m_windowHeight;
		height = m_windowWidth;
		fullWidth = m_appBufHeight;
		fullHeight = m_appBufWidth;
	}
	
	Rectangle paintRect(0, 0, m_windowWidth, m_windowHeight);
		
	Rectangle cornerRect(0, 0, topLeftCornerSurf->width(), topLeftCornerSurf->height());
	if (cornerRect.intersects(paintRect))
		context->bitblt(topLeftCornerSurf, 0, 0, cornerRect.w(), cornerRect.h());

	cornerRect.set(width - topRightCornerSurf->width(), 0, width, topRightCornerSurf->height());
	if (cornerRect.intersects(paintRect))
		context->bitblt(topRightCornerSurf, cornerRect.x(), cornerRect.y(), cornerRect.r(), cornerRect.b());

	cornerRect.set(0, height - bottomLeftCornerSurf->height(), bottomLeftCornerSurf->width(), height);
	if (cornerRect.intersects(paintRect))
		context->bitblt(bottomLeftCornerSurf, cornerRect.x(), cornerRect.y(), cornerRect.r(), cornerRect.b());

	cornerRect.set(width - bottomRightCornerSurf->width(),
				   height - bottomLeftCornerSurf->height(),
				   width, height);
	if (cornerRect.intersects(paintRect))
		context->bitblt(bottomRightCornerSurf, cornerRect.x(), cornerRect.y(), cornerRect.r(), cornerRect.b());

	if (height != fullHeight) {
		context->push();
		context->setStrokeColor(PColor32(0x00, 0x00, 0x00, 0x00));
		context->setFillColor(PColor32(0x00, 0x00, 0x00, 0xFF));
		context->drawRect(0, height, fullWidth, fullHeight);
		context->pop();
	}
#endif
}

void CardWebApp::prepareSceneTransition()
{
	if (G_UNLIKELY(Settings::LunaSettings()->perfTesting)) {
		g_message("SYSMGR PERF: SCENE PREPARE appid: %s, processid: %s, type: %s, time: %d",
				  m_page ? m_page->appId().c_str() : "unknown",
				  m_page ? m_page->processId().c_str() : "unknown",
				  WebAppFactory::nameForWindowType(m_winType).c_str(),
				  Time::curTimeMs());
	}

	if (m_state == StateRunningTransition) {
		// A sloppy app can call prepare and run twice in a row. While the first animation is running we prepare and try to run
		// the second animation (this happens often on faster hardware), which causes lots of screen schmutz. The solution is to not do the prepare
		// if a transition is running (therefore m_state doesn't move to StateWaitingForNewScene) so that once run gets called we cancel the
		// transition and display the target scene immediately.
		g_critical("%s: prepareSceneTransition called while another transition was running. Ignoring...", __PRETTY_FUNCTION__);
		return;
	}

	if (m_state == StateChildScene) {
		// disable direct rendering before starting the transition
		if (m_directRendering) {
			if (m_parentWebApp)
				m_parentWebApp->directRenderingChanged(false, 0, 0, Event::Orientation_Invalid);
			else
				directRenderingChanged(false, 0, 0, Event::Orientation_Invalid);
		}
		m_data->prepareCrossAppSceneTransition();
		return;
	}

	m_state = StateWaitingForNewScene;

	// disable direct rendering before starting the transition
	if (m_directRendering) {
		if (m_parentWebApp)
			m_parentWebApp->directRenderingChanged(false, 0, 0, Event::Orientation_Invalid);
		else
			directRenderingChanged(false, 0, 0, Event::Orientation_Invalid);
	}

	m_data->prepareSceneTransition(this);
}

void CardWebApp::runSceneTransition(const char* transitionType, bool isPop)
{
	if (G_UNLIKELY(Settings::LunaSettings()->perfTesting)) {
		g_message("SYSMGR PERF: SCENE RUN appid: %s, processid: %s, type: %s, time: %d",
				  m_page ? m_page->appId().c_str() : "unknown",
				  m_page ? m_page->processId().c_str() : "unknown",
				  WebAppFactory::nameForWindowType(m_winType).c_str(),
				  Time::curTimeMs());
	}

	if (m_state != StateWaitingForNewScene) {
		g_critical("%s: runSceneTransition called multiple times or without a prior prepareSceneTransition", __PRETTY_FUNCTION__);
		cancelSceneTransition();
		return;
	}

	m_transitionIsPop = isPop;

	handlePendingChanges();

	m_data->runSceneTransition(this, transitionType, isPop, m_appBufWidth, m_appBufHeight);

	m_state = StateRunningTransition;
}

void CardWebApp::runCrossAppTransition(bool isPop)
{
	g_message("%s: %s %s", __PRETTY_FUNCTION__, isPop ? "is pop" : "is push", m_page->appId().c_str());

	if (m_state == StateChildScene) {
		cancelSceneTransition();
		return;
	}

	if (m_state != StateWaitingForNewScene) {
		g_critical("%s: runCrossAppTransition called multiple times or without a prior prepareSceneTransition",
				  __PRETTY_FUNCTION__);
		return;
	}

	m_transitionIsPop = isPop;
		
	// Wait for the child app to come up and run its animations
	m_state = StateChildScene;
}

void CardWebApp::cancelSceneTransition()
{
	g_message("%s: %s", __PRETTY_FUNCTION__, m_page->appId().c_str());

	if (m_state != StateChildScene) {
		animationFinished();
		// let the remote side know that the transition is canceled so that it can free the resources in case the prepare message was already sent
		m_channel->sendAsyncMessage(new ViewHost_SceneTransitionCancel(routingId()));
		return;
	}

	g_message("%s: running cross app pop transition: %s", __PRETTY_FUNCTION__, m_page->appId().c_str());

	// Fake a pending full screen mode change so that full-screen reset
	// after a child card push happens correctly
	if (m_pendingFullScreenMode == -1 && windowType() != Window::Type_DockModeWindow) {
		m_pendingFullScreenMode = m_enableFullScreen;
	}
		
	handlePendingChanges();

	m_data->runSceneTransition(this, "zoom-fade-pop", false, m_appBufWidth, m_appBufHeight);
}

int CardWebApp::angleForOrientation(Event::Orientation orient) const
{
    switch (orient) {
	case (Event::Orientation_Down):
		return 180;
	case (Event::Orientation_Left):
		return 270;
	case (Event::Orientation_Right):
		return 90;
	default:
		break;
	}

	return 0;
}

void CardWebApp::animationFinished()
{
	m_state = StateNormal;

	m_data->cleanupSceneTransition();

	handlePendingChanges();

	invalidate();
}

void CardWebApp::enableFullScreenMode(bool enable)
{

	if (m_state != StateNormal) {
		m_pendingFullScreenMode = enable;
		return;
	}
	
	if (m_enableFullScreen == enable) {
		return;
	}

	m_enableFullScreen = enable;

	if (m_orientation == Event::Orientation_Up) {

		// We allow window resizes only in UP orientation mode

		int newWidth = m_width;
		int newHeight = m_height;
		
		if (!enable)
			newHeight -= Settings::LunaSettings()->positiveSpaceTopPadding;

		if ((int) m_windowWidth == newWidth &&
			(int) m_windowHeight == newHeight)
			goto Done;

		m_windowWidth = newWidth;
		m_windowHeight = newHeight;
		m_page->webkitView()->resize(m_windowWidth, m_windowHeight);

		setVisibleDimensions(newWidth, newHeight);
	}
	else {

		// in other orientation modes, full screen is always true
		m_enableFullScreen = true;
	}

Done:		

	updateWindowProperties();
}

void CardWebApp::invalContents(int x, int y, int width, int height)
{
	QRect r(x, y, width, height);
	r &= QRect(0, 0, m_appBufWidth, m_appBufHeight);
	if (r.isEmpty())
		return;
	
	m_paintRect |= r;

	startPaintTimer();
}

void CardWebApp::invalidate()
{
    invalContents(0, 0, m_appBufWidth, m_appBufHeight);    
}

void CardWebApp::initGfxSurfacesIfNeeded()
{
	if (s_rotatedDrawCtxt)
		return;

	s_rotatedDrawCtxt = PGContext::create();
	s_portraitDrawSurf = s_rotatedDrawCtxt->createSurface(m_width, m_height, false);
	s_landscapeDrawSurf = s_rotatedDrawCtxt->createSurface(m_height, m_width, false);

	int w = Settings::LunaSettings()->emulatedCardWidth;
	int h = Settings::LunaSettings()->emulatedCardHeight;
	s_emulatedPortraitDrawSurf = s_rotatedDrawCtxt->createSurface(w, h, false);
	s_emulatedLandscapeDrawSurf = s_rotatedDrawCtxt->createSurface(h, w, false);

	int wM = Settings::LunaSettings()->modalWindowWidth;
	int wH = Settings::LunaSettings()->modalWindowHeight;
	s_modalPortraitDrawSurf = s_rotatedDrawCtxt->createSurface(wM, wH, false);
}

void CardWebApp::addChildCardWebApp(CardWebApp* app)
{
	g_message("%s: state: %d", __PRETTY_FUNCTION__, m_state);

	if (m_childWebApp) {
		g_critical("%s: Trying to pile on multiple cross-app scenes", __PRETTY_FUNCTION__);
		return;
	}
	
    m_childWebApp = app;
}

void CardWebApp::removeChildCardWebApp(CardWebApp* app)
{
	g_message("%s: state: %d", __PRETTY_FUNCTION__, m_state);
    m_childWebApp = 0;
}

void CardWebApp::setParentCardWebApp(CardWebApp* app)
{
	m_parentWebApp = app; 
}

void CardWebApp::attach(WebPage* page)
{
	if (m_winType == Window::Type_ChildCard) {

		// this is a child card

		m_parentWebApp = static_cast<CardWebApp*>(WebAppManager::instance()->findApp(page->launchingProcessId()));
		m_parentWebApp->addChildCardWebApp(this);

		if (m_parentWebApp->windowType() == Window::Type_Emulated_Card) {

			if (page->webkitView())
				page->webkitView()->setSupportsAcceleratedCompositing(false);

			m_width = Settings::LunaSettings()->emulatedCardWidth;
			m_height = Settings::LunaSettings()->emulatedCardHeight;

			int widthAdj = 0;
			int heightAdj = Settings::LunaSettings()->positiveSpaceTopPadding;
			
			m_appBufWidth = m_width;
			m_appBufHeight = m_height;

			m_windowWidth = m_width - widthAdj;
			m_windowHeight = m_height - heightAdj;
			m_setWindowWidth = m_windowWidth;
			m_setWindowHeight = m_windowHeight;
		}

		m_data = m_parentWebApp->m_data;
		m_metaDataBuffer = m_parentWebApp->m_metaDataBuffer;
		m_directRendering = m_parentWebApp->m_directRendering;
		m_renderOffsetX = m_parentWebApp->m_renderOffsetX;
		m_renderOffsetY = m_parentWebApp->m_renderOffsetY;

		// --------------------------------------------------------------------------

		initGfxSurfacesIfNeeded();

		if (Settings::LunaSettings()->displayUiRotates && !isEmulatedCardOrChild()) {
			setOrientation(appOrientationFor(WebAppManager::instance()->orientation()));
		}
	} 

	//Perform the attach before trying to send any messages to it or else there
	//won't be a window to receive them.

	WindowedWebApp::attach(page);

	/*if(m_winType == Window::Type_Emulated_Card) {
		m_fixedOrientation = orientationForThisCard(Event::Orientation_Up);
	}*/

	if(m_fixedOrientation != Event::Orientation_Invalid) {
		if(!Settings::LunaSettings()->displayUiRotates) {
			m_allowsOrientationChange = true;
			setOrientation(m_fixedOrientation);
		} else {
			m_allowsOrientationChange = false;
			m_channel->sendAsyncMessage(new ViewHost_Card_SetAppFixedOrientation(routingId(), m_fixedOrientation, isOrientationPortrait(m_fixedOrientation)));
		    resizeWindowForFixedOrientation(m_fixedOrientation);
		}
	}

	if (m_winType == Window::Type_Emulated_Card || m_winType == Window::Type_Card || m_winType == Window::Type_ModalChildWindowCard) {

		AppLaunchOptionsEvent ops_ev;

		ops_ev.splashBackground = std::string();
		ops_ev.launchInNewGroup = false;

		// Cards have the option to specify a custom splash background
        const StringVariantMap& stageArgs = page->stageArguments();
        StringVariantMap::const_iterator it = stageArgs.find("splashbackgroundname");
        if (it == stageArgs.end())
			it = stageArgs.find("splashbackground");
		if (it != stageArgs.end()) {
			ops_ev.splashBackground = it->second.toString().toStdString();
		}

        it = stageArgs.find("launchinnewgroup");
		if (it != stageArgs.end()) {
			ops_ev.launchInNewGroup = it->second.toBool();
		}

        it = stageArgs.find("statusbarcolor");
		if (it != stageArgs.end()) {
			unsigned int color = (unsigned int)(it->second.toInt());
			WindowProperties prop;
			prop.setStatusBarColor(color);
			setWindowProperties(prop);
		}

		if (!ops_ev.splashBackground.empty() || ops_ev.launchInNewGroup != false) {
			m_channel->sendAsyncMessage(new ViewHost_Card_AppLaunchOptionsEvent(routingId(), AppLaunchOptionsEventWrapper(&ops_ev)));
		}
	}
	
	if(m_winType != Window::Type_ModalChildWindowCard)
		setVisibleDimensions(m_width, m_height - Settings::LunaSettings()->positiveSpaceTopPadding);
	else
		setVisibleDimensions(m_width, m_height);

}

WebPage* CardWebApp::detach()
{
	return WindowedWebApp::detach();
}

bool CardWebApp::isWindowed() const
{
    return m_winType != Window::Type_ChildCard;
}

bool CardWebApp::isChildApp() const
{
	return m_winType == Window::Type_ChildCard;
}

bool CardWebApp::isLeafApp() const
{
    return m_childWebApp == 0;
}

void CardWebApp::loadFinished()
{
	if (m_winType != Window::Type_Card) {
		WindowedWebApp::loadFinished();
	}

	// fake this
	if (m_winType == Window::Type_ChildCard) {
		m_addedToWindowMgr = true;
	}
}


void CardWebApp::crossAppSceneActive()
{
	g_message("%s: %s", __PRETTY_FUNCTION__, m_page->appId().c_str());

	if (m_state != StateWaitingForNewScene) {
		g_critical("%s: crossAppSceneActive called multiple times", __PRETTY_FUNCTION__);
		cancelSceneTransition();
		return;
	}

	if (m_parentWebApp && m_parentWebApp->m_state != StateChildScene) {
		g_critical("%s: This transition was canceled by the parent. Canceling on the child as well", __PRETTY_FUNCTION__);
		cancelSceneTransition();
		return;
	}

	// a child card should use parent's window current or pending dimensions
	// to determine its window dimensions
	if (m_parentWebApp->m_pendingResizeWidth != -1 &&
		m_parentWebApp->m_pendingResizeHeight != -1) {		
		m_pendingResizeWidth = m_parentWebApp->m_pendingResizeWidth;
		m_pendingResizeHeight = m_parentWebApp->m_pendingResizeHeight;
	}
	else {
		m_pendingResizeWidth = m_parentWebApp->m_windowWidth;
		m_pendingResizeHeight = m_parentWebApp->m_windowHeight;
	}

	// Sync up with parent webapp's focus state
	if (m_focused != m_parentWebApp->m_focused) {

		m_focused = m_parentWebApp->m_focused;
		const char* scriptActivated = "if (window.Mojo && Mojo.stageActivated) {Mojo.stageActivated();}";
		const char* scriptDeactivated = "if (window.Mojo && Mojo.stageDeactivated) {Mojo.stageDeactivated();}";

		m_page->webkitPage()->evaluateScript(m_focused ? scriptActivated : scriptDeactivated);		
	}

	handlePendingChanges();

	m_data->crossAppSceneActive(this, m_appBufWidth, m_appBufHeight);

	m_state = StateRunningTransition;
}

void CardWebApp::cancelCrossAppScene()
{
	g_message("%s: %s", __PRETTY_FUNCTION__, m_page->appId().c_str());

	// Two possibilities: we either have a child window on top of us or
	// we are waiting for a child window. FIXME: For now we handle only
	// the case where the child window is already created

	if (m_childWebApp) {
		m_childWebApp->setParentCardWebApp(0);
		m_childWebApp->close();
		m_childWebApp = 0;
	}
}

void CardWebApp::handlePendingChanges()
{

	if (m_pendingOrientation == Event::Orientation_Invalid &&
		m_pendingFullScreenMode == -1 &&
		m_pendingResizeWidth == -1 &&
		m_pendingResizeHeight == -1) {

		// nothing to do
		return;
	}

	if (m_pendingOrientation != Event::Orientation_Invalid &&
		Settings::LunaSettings()->displayUiRotates)  {
		if (m_fixedOrientation != Event::Orientation_Invalid) {
			//NOTE: m_fixedOrientation is in system orientation, but orientationForThisCard()
			//can be called to map app-to-system and system-to-app.
			m_orientation = orientationForThisCard(m_fixedOrientation);
		}
		else if (m_allowsOrientationChange) {
			m_orientation = orientationForThisCard(m_pendingOrientation);
		}
	}

	if (m_pendingFullScreenMode != -1 && windowType() != Window::Type_DockModeWindow) {
		if (m_enableFullScreen == m_pendingFullScreenMode) {
			//Requesting we switch to a state we're already in.  This means
			//there's nothing to do and we should suppress the processing
			//of the request.
			m_pendingFullScreenMode = -1;
		} else {
			m_enableFullScreen = m_pendingFullScreenMode;
		}
	}

	switch (m_orientation) {
	case (Event::Orientation_Left):
	case (Event::Orientation_Right): {
		// Full screen in this mode
		m_windowWidth = m_height;
		m_windowHeight = m_width;
		m_appBufWidth = m_height;
		m_appBufHeight = m_width;
		break;
	}
	case (Event::Orientation_Down): {
		// Full screen in this mode
		m_windowWidth = m_width;
		m_windowHeight = m_height;
		m_appBufWidth = m_width;
		m_appBufHeight = m_height;
		break;
	}
	case (Event::Orientation_Up):
	default: {
		m_appBufWidth = m_width;
		m_appBufHeight = m_height;

		if (m_enableFullScreen) {
			m_windowWidth = m_width;
			m_windowHeight = m_height;
		}
		else if (m_pendingResizeWidth != -1 &&
				 m_pendingResizeHeight != -1) {
			m_windowWidth = m_pendingResizeWidth;
			m_windowHeight = m_pendingResizeHeight;
		}
		else {
			m_windowWidth = m_setWindowWidth;
			m_windowHeight = m_setWindowHeight;
			if (m_pendingFullScreenMode != -1 && !m_enableFullScreen) {
				//If leaving full screen mode, adjust the window down
				//for the height of the status bar.
				m_windowHeight = m_windowHeight - Settings::LunaSettings()->positiveSpaceTopPadding;
			}
		}

		break;
	}
	}

	// only resize the window if something was pending that could trigger it
	if (m_pendingResizeWidth != -1 || m_pendingResizeHeight != -1 ||
		m_pendingFullScreenMode != -1 ||
		m_pendingOrientation != Event::Orientation_Invalid)	{
		m_page->webkitView()->resize(m_windowWidth, m_windowHeight);
	}

	if (m_orientation == Event::Orientation_Up && !m_enableFullScreen) {
		setVisibleDimensions(m_width, m_height - Settings::LunaSettings()->positiveSpaceTopPadding);
	}
	else {
		setVisibleDimensions(m_width, m_height);
	}	

	m_setWindowWidth = m_windowWidth;
	m_setWindowHeight = m_windowHeight;
	
	updateWindowProperties();

	Event::Orientation newOrient = m_pendingOrientation;
	
	m_pendingResizeWidth = -1;
	m_pendingResizeHeight = -1;
	m_pendingOrientation = Event::Orientation_Invalid;
	m_pendingFullScreenMode = -1;

	if (newOrient != Event::Orientation_Invalid) {
		callMojoScreenOrientationChange();
	}
}

void CardWebApp::receivePageUpDownInLandscape(bool val)
{
	m_doPageUpDownInLandscape = val;    
}

void CardWebApp::updateWindowProperties()
{
	WindowProperties winProps;
	
	if (m_enableFullScreen)
		winProps.setFullScreen(true);
	else
		winProps.setFullScreen((!Settings::LunaSettings()->displayUiRotates ||
								isEmulatedCardOrChild()) &&
							   (m_orientation != Event::Orientation_Up));

	switch (m_orientation) {
	case (Event::Orientation_Down):
		winProps.setOverlayNotificationsPosition(WindowProperties::OverlayNotificationsTop);
		break;
	case (Event::Orientation_Left):
		winProps.setOverlayNotificationsPosition(WindowProperties::OverlayNotificationsLeft);
		break;
	case (Event::Orientation_Right):
		winProps.setOverlayNotificationsPosition(WindowProperties::OverlayNotificationsRight);
		break;
	case (Event::Orientation_Up):
	default:
		winProps.setOverlayNotificationsPosition(WindowProperties::OverlayNotificationsBottom);
		break;
	}

	WindowedWebApp::setWindowProperties(winProps);

}

void CardWebApp::callMojoScreenOrientationChange()
{
    Event::Orientation orient = orientationForThisCard(WebAppManager::instance()->orientation());

	callMojoScreenOrientationChange(orient);
}

void CardWebApp::callMojoScreenOrientationChange(Event::Orientation orient)
{
	// FIXME: We should call the callback directly without using a script
	static const char* scriptUp    = "Mojo.screenOrientationChanged(\"up\")";
	static const char* scriptDown  = "Mojo.screenOrientationChanged(\"down\")";
	static const char* scriptLeft  = "Mojo.screenOrientationChanged(\"left\")";
	static const char* scriptRight = "Mojo.screenOrientationChanged(\"right\")";

	const char* script = 0;
	
	switch (orient) {
	case (Event::Orientation_Up):    script = scriptUp; break;
	case (Event::Orientation_Down):  script = scriptDown; break;
	case (Event::Orientation_Left):  script = scriptLeft; break;
	case (Event::Orientation_Right): script = scriptRight; break;
	default: return;
	}

	if (script && m_page && m_page->webkitPage()) {

		// Recursion protector
		static bool s_alreadyHere = false;
		if (!s_alreadyHere) {
			s_alreadyHere = true;
			m_page->webkitPage()->evaluateScript(script);
			s_alreadyHere = false;
		}
	}
}


void CardWebApp::onSetComposingText(const std::string& text)
{
	if (m_childWebApp) {
		m_childWebApp->onSetComposingText(text);
		return;
	}
	WindowedWebApp::onSetComposingText(text);
}

void CardWebApp::onCommitComposingText()
{
	if (m_childWebApp) {
		m_childWebApp->onCommitComposingText();
		return;
	}
	WindowedWebApp::onCommitComposingText();
}

void CardWebApp::onCommitText(const std::string& text)
{
	if (m_childWebApp) {
		m_childWebApp->onCommitText(text);
		return;
	}
	WindowedWebApp::onCommitText(text);

}

void CardWebApp::onPerformEditorAction(int action)
{
	if (m_childWebApp) {
		m_childWebApp->onPerformEditorAction(action);
		return;
	}
	WindowedWebApp::onPerformEditorAction(action);
}

void CardWebApp::onRemoveInputFocus()
{
	if (m_childWebApp) {
		m_childWebApp->onRemoveInputFocus();
		return;
	}
	WindowedWebApp::onRemoveInputFocus();
}

void CardWebApp::onInputEvent(const SysMgrEventWrapper& wrapper)
{
/*
    if (Settings::LunaSettings()->enableTouchEventsForWebApps && 
        page()->isTouchEventsNeeded()) {
        // filter out mouse related events
        switch (wrapper.event->type) {
        case Event::PenDown:
        case Event::PenMove:
        case Event::PenUp:
        case Event::PenCancel:
        case Event::PenCancelAll:
            return;
        default:
            break;
        }
    }
*/
    WindowedWebApp::onInputEvent(wrapper);
}

void CardWebApp::setVisibleDimensions(int width, int height)
{
	m_channel->sendAsyncMessage(new ViewHost_SetVisibleDimensions(routingId(), width, height));
}

void CardWebApp::beginPaint()
{
	m_data->beginPaint();
}

void CardWebApp::endPaint()
{
	m_data->endPaint(false, PRect());
	m_data->sendWindowUpdate(0, 0, m_width, m_height);
}

void CardWebApp::onDirectRenderingChanged()
{
	if (!m_data->hasDirectRendering() || isEmulatedCardOrChild())
		return;

	bool directRendering = false;
	int renderOffsetX = 0;
	int renderOffsetY = 0;
	int renderOrientation = Event::Orientation_Invalid;

	WindowMetaData* metaData = (WindowMetaData*) m_metaDataBuffer->data();
	m_metaDataBuffer->lock();
	renderOffsetX = metaData->directRenderingScreenX;
	renderOffsetY = metaData->directRenderingScreenY;
	renderOrientation = metaData->directRenderingOrientation;
	directRendering = metaData->allowDirectRendering;
	m_metaDataBuffer->unlock();

	directRenderingChanged(directRendering, renderOffsetX, renderOffsetY, renderOrientation);
}

void CardWebApp::directRenderingChanged(bool directRendering, int renderOffsetX, int renderOffsetY, int renderOrientation)
{
	g_message("directRendering appid: %s, processId: %s, enabled: %d, offset: %d, %d, orientation: %d",
			  m_page ? m_page->appId().c_str() : "unknown",
			  m_page ? m_page->processId().c_str() : "unknown",
			  directRendering, renderOffsetX, renderOffsetY, renderOrientation);

	if (directRendering && (m_state == StateWaitingForNewScene || m_state == StateRunningTransition)) {
		// do not turn on direct rendering while in the middle of a transition
		g_message("not allowing direct rendering while in the middle of a transition. state = %d", m_state);
		return;
	}

	m_renderOffsetX = renderOffsetX;
	m_renderOffsetY = renderOffsetY;
	m_renderOrientation = renderOrientation;

	if (m_childWebApp) {
		m_childWebApp->directRenderingChanged(directRendering, renderOffsetX, renderOffsetY, renderOrientation);
		m_directRendering = directRendering;
		return;
	}

	if (directRendering)
		directRenderingAllowed();
    else
		directRenderingDisallowed();
}

void CardWebApp::directRenderingAllowed()
{
	WebAppDeferredUpdateHandler::directRenderingActive(this);

	if (!m_data->hasDirectRendering())
		return;

	if (m_directRendering) {

		// Check state of Direct Rendering before clearing offscreen
		WindowMetaData* metaData = (WindowMetaData*) m_metaDataBuffer->data();
		if (!metaData->allowDirectRendering)
			return;

		// Already doing direct rendering. Just clear the offscreen
		m_data->clear();
		invalidate();
		return;
	}

	g_message("%s: %s doing direct rendering",
			  m_page ? m_page->appId().c_str() : "unknown",
			  m_page ? m_page->processId().c_str() : "unknown");			  

	// Step 1: Switch to direct rendering and paint the "screen"
	m_directRendering = true;
	m_data->directRenderingAllowed(m_directRendering);

	// FIXME: We need to flip three times to make sure that all three
	// buffers are updated. A better way to do is this to synchronously
	// wait for vsync. Need Piranha support

	bool oldPaintingDisabled = m_paintingDisabled;
	if (m_paintingDisabled)
		m_paintingDisabled = false;

	for (int i = 0; i < 4; i++) {
		invalidate();
		forcePaint(false);
	}

	m_paintingDisabled = oldPaintingDisabled;

	// Check state of Direct Rendering again before clearing offscreen
	WindowMetaData* metaData = (WindowMetaData*) m_metaDataBuffer->data();
	if (!metaData->allowDirectRendering)
		return;

	m_data->clear();
}

void CardWebApp::directRenderingDisallowed()
{
	WebAppDeferredUpdateHandler::directRenderingInactive(this);

	if (!m_data->hasDirectRendering())
		return;

	g_message("%s: %s doing offscreen rendering",
			  m_page ? m_page->appId().c_str() : "unknown",
			  m_page ? m_page->processId().c_str() : "unknown");			  
	
	m_directRendering = false;
	m_renderOffsetX = 0;
	m_renderOffsetY = 0;
	m_renderOrientation = Event::Orientation_Invalid;
	m_data->directRenderingAllowed(m_directRendering);

	invalidate();
	forcePaint(false);
}

void CardWebApp::displayOn()
{
	if (!m_paintingDisabled)
		return;
	
	g_message("%s: %s resuming paints",
			  m_page ? m_page->appId().c_str() : "unknown",
			  m_page ? m_page->processId().c_str() : "unknown");			  
	
	m_paintingDisabled = false;
	if (!m_paintRect.isEmpty())
		startPaintTimer();
}

void CardWebApp::displayOff()
{
	if (m_paintingDisabled)
		return;

	g_message("%s: %s suspending paints",
			  m_page ? m_page->appId().c_str() : "unknown",
			  m_page ? m_page->processId().c_str() : "unknown");			  
	
	m_paintingDisabled = true;
}

Event::Orientation CardWebApp::postProcessOrientationEvent(Event::Orientation aInputEvent)
{
    Event::Orientation outputEvent = aInputEvent;

    if (isEmulatedCardOrChild())
    {
        if (Event::Orientation_Left == aInputEvent)
        {
            outputEvent = Event::Orientation_Right;
        }
        else if (Event::Orientation_Right == aInputEvent)
        {
            outputEvent = Event::Orientation_Left;
        }
    }

    return outputEvent;
}

Event::Orientation CardWebApp::orientationForThisCard(Event::Orientation orient)
{
		int angle = Settings::LunaSettings()->homeButtonOrientationAngle;

		// translate the orientation requested by the card to an orientation that matches the physical layout of the device

		switch (angle) {
            // Adding case for handling orientation for Opal devices
            // As our orientations are reversed in topaz, we'll be
            // Continuing with the same values.
            case 0: // For Opal
            {
                switch(orient)
                {
                case (Event::Orientation_Left):
                     return Event::Orientation_Right;
                case (Event::Orientation_Right):
                    return Event::Orientation_Left;
                default:
                    return orient;
                }
                break;
            }

			case 90:
			{
				switch (orient) {
				case (Event::Orientation_Up):
					return Event::Orientation_Left; //rotation by 270 deg
					break;
				case (Event::Orientation_Down):
					return Event::Orientation_Right; //rotation by 270 deg
					break;
				case (Event::Orientation_Left):
					return Event::Orientation_Up; //rotation by 90 deg
					break;
				case (Event::Orientation_Right):
					return Event::Orientation_Down; //rotation by 90 deg
					break;
				case (Event::Orientation_Landscape):
					return Event::Orientation_Landscape;
					break;
				case (Event::Orientation_Portrait):
					return Event::Orientation_Portrait;
					break;
				default:
					return Event::Orientation_Invalid;
				}
			}
			break;

			case -180:
			case 180:
			{
				switch (orient) {
				case (Event::Orientation_Up):
						return Event::Orientation_Down;
						break;
				case (Event::Orientation_Down):
						return Event::Orientation_Up;
						break;
				case (Event::Orientation_Left):
					return Event::Orientation_Right;
					break;
				case (Event::Orientation_Right):
					return Event::Orientation_Left;
					break;
				case (Event::Orientation_Landscape):
					return Event::Orientation_Landscape;
					break;
				case (Event::Orientation_Portrait):
					return Event::Orientation_Portrait;
					break;
				default:
					return Event::Orientation_Invalid;
				}
			}
			break;
			case -90:
            case 270://For Topaz
			{
				switch (orient) {
				case (Event::Orientation_Up):
					return Event::Orientation_Right; //rotation by 90 deg
					break;
				case (Event::Orientation_Down):
					return Event::Orientation_Left; //rotation by 90 deg
					break;
				case (Event::Orientation_Left):
                        return Event::Orientation_Down;
					break;
				case (Event::Orientation_Right):
                        return Event::Orientation_Up;
					break;
				case (Event::Orientation_Landscape):
					return Event::Orientation_Landscape;
					break;
				case (Event::Orientation_Portrait):
					return Event::Orientation_Portrait;
					break;
			  default:
			      return Event::Orientation_Invalid;
			  }
			}
			break;

			default:
				return orient;
		}

}

/*
 * Emulation mode adheres to the following definitions of left and right:
 * left = 90 deg CW from up
 * right = 90 deg CCE from up
 *
 * This is at odds with Topaz and Opal and other devices, and so it is necessary
 * to correct this by changing the requested orientation to one that will end up
 * parsing to the correct one.
 */

Event::Orientation CardWebApp::adjustEmuModeLeftRight(Event::Orientation orient) {
	int angle = Settings::LunaSettings()->homeButtonOrientationAngle;

	// translate the orientation requested by the card to an orientation that matches the physical layout of the device

	if(angle == 0)
		return orient;

	switch (angle) {
		case 90:
		{
			if (orient == Event::Orientation_Left) {
				return Event::Orientation_Right;
			}
			else if (orient == Event::Orientation_Right){
				return Event::Orientation_Left;
			}
		}
		break;

		case -180:
		case 180:
		{
			return orient;
		}
		break;
		case -90:
		case 270:
		{
			if (orient == Event::Orientation_Left) {
				return Event::Orientation_Right;
			}
			else if (orient == Event::Orientation_Right){
				return Event::Orientation_Left;
			}
		}
		break;

		default:
			return orient;
	}

	return orient;
}


void CardWebApp::allowResizeOnPositiveSpaceChange(bool allowResize)
{
	WindowProperties props;
	props.setAllowResizeOnPositiveSpaceChange(allowResize);

	WindowedWebApp::setWindowProperties(props);
}


void CardWebApp::thawFromCache()
{
	luna_assert(m_page);

	if (!m_inCache) {
		focus();
		return;
	}

	if (m_winType == Window::Type_Card || m_winType == Window::Type_ChildCard)
		WebAppDeferredUpdateHandler::registerApp(this);

	// Resume all timers in the app before we do the rest of the thawing work
	// which may indirectly rely on timers working.
	m_page->webkitPage()->throttle(100, 0);

	WebAppCache::remove(this);
	m_page->webkitPage()->evaluateScript("if (window.Mojo && Mojo.show) Mojo.show()");

	g_message("THAWING app %s", m_page->appId().c_str());

	if (G_UNLIKELY(Settings::LunaSettings()->perfTesting)) {
		g_message("SYSMGR PERF: APP START appid: %s, processid: %s, type: %s, time: %d",
				  m_page->appId().c_str(), m_page->processId().c_str(),
				  WebAppFactory::nameForWindowType(Window::Type_Card).c_str(),
				  Time::curTimeMs());
	}

	m_page->webkitView()->setSupportsAcceleratedCompositing(true);
	
	if (m_fixedOrientation == Event::Orientation_Invalid) {
		WebAppManager* wam = WebAppManager::instance();
		if (m_width != wam->currentUiWidth() ||
			m_height != wam->currentUiHeight())
			flipEvent(wam->currentUiWidth(), wam->currentUiHeight());
	}
	
	m_channel->sendAsyncMessage(new ViewHost_PrepareAddWindowWithMetaData(routingId(), metadataId(),
																		  m_winType, m_width, m_height));		
	m_channel->sendAsyncMessage(new ViewHost_SetAppId(routingId(), m_page->appId()));
	m_channel->sendAsyncMessage(new ViewHost_SetProcessId(routingId(), m_page->processId()));
	m_channel->sendAsyncMessage(new ViewHost_SetLaunchingAppId(routingId(), m_page->launchingAppId()));
	m_channel->sendAsyncMessage(new ViewHost_SetLaunchingProcessId(routingId(), m_page->launchingProcessId()));
	m_channel->sendAsyncMessage(new ViewHost_SetName(routingId(), m_page->name()));

	
	stagePreparing();
	invalidate();
	stageReady();    

	EventReporter::instance()->report("launch", m_page->appId().c_str());

	// Send our window properties to the sysmgr side
	setWindowProperties(m_winProps);

	markInCache(false);
}

void CardWebApp::freezeInCache()
{
	luna_assert(m_page);

	g_message("CACHING app %s", m_page->appId().c_str());

	if (m_winType == Window::Type_Card || m_winType == Window::Type_ChildCard)
		WebAppDeferredUpdateHandler::unregisterApp(this);

	if (m_data)
		m_channel->sendAsyncMessage(new ViewHost_RemoveWindow(routingId()));

	m_page->webkitView()->setSupportsAcceleratedCompositing(false);
	m_page->webkitView()->unmapCompositingTextures();

	m_page->webkitPage()->evaluateScript("if (window.Mojo && Mojo.hide) Mojo.hide()");
	WebAppCache::put(this);

	m_stagePreparing = false;
	m_stageReady = false;
	m_addedToWindowMgr = false;

	EventReporter::instance()->report("close", m_page->appId().c_str());

	// Suspend all timers in the app after we finish the freezing work.
	m_page->webkitPage()->throttle(0, 0);

	markInCache(true);
}

void CardWebApp::focus()
{
	if (m_inCache) {
		thawFromCache();
		return;
	}

	WindowedWebApp::focus();
}

void CardWebApp::suspendAppRendering()
{
	m_renderingSuspended = true;
	stopPaintTimer();
}

void CardWebApp::resumeAppRendering()
{
	m_renderingSuspended = false;
}

void CardWebApp::screenSize(int& width, int& height) {
	if (isEmulatedCardOrChild()) {
		width = Settings::LunaSettings()->emulatedCardWidth;
		height = Settings::LunaSettings()->emulatedCardHeight;
	} else if(m_winType == Window::Type_ModalChildWindowCard){
		width = Settings::LunaSettings()->modalWindowWidth;
		height = Settings::LunaSettings()->modalWindowHeight;
	}
	else {
		width = WebAppManager::instance()->currentUiWidth();
		height = WebAppManager::instance()->currentUiHeight();
	}
}

//This is called primarily by WebAppManager (but also 1-2 places in CardWebApp)
//to translate system orientation data into correct orientations for this app.
//This is, as with many things, being done as part of the process of shoehorning
//emulation mode onto Opal (worked fine on Topaz, for the record).  Because the
//emulation mode can now run in an orientation space which is rotated relative
//to the device's, a system orientation must be converted before it is handed
//over to the app.

Event::Orientation CardWebApp::appOrientationFor(int orient) {

	Event::Orientation orientations[4] = {Event::Orientation_Up,
			Event::Orientation_Right, Event::Orientation_Down,
			Event::Orientation_Left};
	int emuModeOrientationAngle = Settings::LunaSettings()->emuModeOrientationAngle;

	if (emuModeOrientationAngle == -90) {
		emuModeOrientationAngle = 270;
	}

	//Convert this into an offset of the array (this is the inverse of systemOrientFor
	//in EmulatedCardWindow.cpp
	if (emuModeOrientationAngle == 0) {
		emuModeOrientationAngle = 0;
	} else if (emuModeOrientationAngle == 90) {
		emuModeOrientationAngle = 3;
	} else if (emuModeOrientationAngle == 180) {
		emuModeOrientationAngle = 2;
	} else if (emuModeOrientationAngle == 270) {
		emuModeOrientationAngle = 1;
	}

	//Convert input into offset of array
	int inputOrient = 0;
	if (orient == Event::Orientation_Up) {
		inputOrient = 0;
	} else if (orient == Event::Orientation_Right) {
		inputOrient = 1;
	} else if (orient == Event::Orientation_Down) {
		inputOrient = 2;
	} else if (orient == Event::Orientation_Left) {
		inputOrient = 3;
	}

	inputOrient = (inputOrient + emuModeOrientationAngle);
	inputOrient = inputOrient % 4;
	//if negative, make positive
	if (inputOrient<0) inputOrient = -inputOrient;

	return orientations[inputOrient];
}

void CardWebApp::forcePaint(bool clipToWindow)
{
	bool oldRenderingSuspend = m_renderingSuspended;
	m_renderingSuspended = false;
	paint(clipToWindow);
	m_renderingSuspended = oldRenderingSuspend;
}

CardWebApp* CardWebApp::parentWebApp() const
{
    return m_parentWebApp;
}

bool CardWebApp::isEmulatedCardOrChild() const
{
	return ((m_winType == Window::Type_Emulated_Card) ||
			(m_winType == Window::Type_ChildCard &&
			 m_parentWebApp &&
			 m_parentWebApp->windowType() == Window::Type_Emulated_Card));
}
