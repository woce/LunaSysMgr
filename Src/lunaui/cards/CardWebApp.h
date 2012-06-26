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



#ifndef __CardWebApp_h__
#define __CardWebApp_h__

#include "Common.h"

#include <sptr.h>

#include "WindowedWebApp.h"
#include "SysMgrDefs.h"

class PIpcChannel;

class CardWebApp : public WindowedWebApp
{
public:

	CardWebApp(Window::Type winType, PIpcChannel *channel, ApplicationDescription* desc = 0);
	~CardWebApp( );

	virtual void thawFromCache();
	virtual void freezeInCache();

	virtual bool isCardApp() const { return true; }
	virtual bool isChildApp() const;

	virtual void paint(bool clipToWindow);

	virtual void inputEvent(sptr<Event> e);
	virtual void keyEvent(QKeyEvent* e);
	virtual void focusedEvent(bool focused);
	virtual int  resizeEvent(int newWidth, int newHeight, bool resizeBuffer);
	virtual void flipEvent(int newWidth, int newHeight);
	virtual void asyncFlipEvent(int newWidth, int newHeight, int newScreenWidth, int newScreenHeight);

	virtual void setOrientation(Event::Orientation orient);
	Event::Orientation orientation() const;
	
	void setFixedOrientation(Event::Orientation orient);
	void setAllowOrientationChange(bool value);
	bool allowsOrientationChange() const;

	virtual void enableFullScreenMode(bool enable);
	
	void prepareSceneTransition();
	void runSceneTransition(const char* transitionType, bool isPop);
	void cancelSceneTransition();
	void runCrossAppTransition(bool isPop);
	void crossAppSceneActive();
	void cancelCrossAppScene();
	void sceneTransitionFinished();
	void receivePageUpDownInLandscape(bool val);

	virtual void invalidate();

	void beginPaint();
	void endPaint();
	
	void paint(NativeGraphicsContext* context, NativeGraphicsSurface* dstSurface,
			   int px, int py, int pw, int ph,
			   int& tx, int& ty, int& tw, int& th);
	void drawCornerWindows(NativeGraphicsContext* context);

	virtual void displayOn();
	virtual void displayOff();

	void allowResizeOnPositiveSpaceChange(bool allowResize);

	bool isRenderingSuspended() { return m_renderingSuspended; }
	virtual Event::Orientation appOrientationFor(int orient);

	CardWebApp* parentWebApp() const;
	
private:

	virtual void attach(WebPage* page);
	virtual WebPage* detach();
	virtual bool isWindowed() const;
    virtual bool isLeafApp() const;

	void addChildCardWebApp(CardWebApp* app);
	void removeChildCardWebApp(CardWebApp* app);
	void setParentCardWebApp(CardWebApp* app);

	virtual void resizeWindowForOrientation(Event::Orientation orient);
	virtual void resizeWindowForFixedOrientation(Event::Orientation orient);
	virtual void getFixedOrientationDimensions(int& width, int& height, int& wAdjust, int& hAdjust);
	bool isOrientationPortrait(Event::Orientation orient);
	void handlePendingChanges();
	
    virtual void invalContents(int x, int y, int width, int height);
	virtual void loadFinished();

	void callMojoScreenOrientationChange();
	void callMojoScreenOrientationChange(Event::Orientation orient);
	virtual void onSetComposingText(const std::string& text);
	virtual void onCommitComposingText();
	virtual void onCommitText(const std::string& text);
	virtual void onPerformEditorAction(int action);
	virtual void onRemoveInputFocus();

    virtual void onInputEvent(const SysMgrEventWrapper& wrapper);
    virtual Event::Orientation orientationForThisCard(Event::Orientation orient);
    virtual Event::Orientation adjustEmuModeLeftRight(Event::Orientation orient);
    virtual Event::Orientation postProcessOrientationEvent(Event::Orientation aInputEvent);

protected:

	void initGfxSurfacesIfNeeded();
	
	bool paint(int px, int py, int pw, int ph,
			   int& tx, int& ty, int& tw, int& th);

	int  angleForOrientation(Event::Orientation orient) const;
	void updateWindowProperties();
	void animationFinished();

	virtual void setVisibleDimensions(int width, int height);
	virtual void onDirectRenderingChanged();

	virtual void directRenderingChanged(bool directRendering, int renderX, int renderY, int angle);
	virtual void directRenderingAllowed();
	virtual void directRenderingDisallowed();

	virtual void focus();
	virtual void screenSize(int& width, int& height);

	void forcePaint(bool clipToWindow);

	bool isEmulatedCardOrChild() const;

	static PGContext* s_rotatedDrawCtxt;
	static PGSurface* s_landscapeDrawSurf;
	static PGSurface* s_portraitDrawSurf;
	static PGSurface* s_emulatedLandscapeDrawSurf;
	static PGSurface* s_emulatedPortraitDrawSurf;
	static PGSurface* s_modalPortraitDrawSurf;

public:
	virtual void suspendAppRendering();
	virtual void resumeAppRendering();

protected:
	enum State {
		StateNormal,
		StateWaitingForNewScene,
		StateRunningTransition,
		StateChildScene
	};

	enum SceneTransition {
		SceneTransitionInvalid = -1,
		SceneTransitionPush,
		SceneTransitionPop
	};

	State m_state;
	bool m_transitionIsPop;

	CardWebApp* m_parentWebApp;
	CardWebApp* m_childWebApp;
	
	// these are the dimensions of the paint buffer seen by webkit
	// (are adjusted to app rotation)
	int m_appBufWidth;
	int m_appBufHeight;
		
    // m_CardOrientation is to keep track of the current orientation
    // of the card. m_orientation & m_fixedOrientation cannot be used
    // reliably always to query current orientation of the CardWebApp
    Event::Orientation m_CardOrientation;
	Event::Orientation m_orientation;
	Event::Orientation m_fixedOrientation;
	bool m_allowsOrientationChange;

	int m_currAngleForAnim;
	int m_targAngleForAnim;
	
	int m_pendingResizeWidth;
	int m_pendingResizeHeight;	
	Event::Orientation m_pendingOrientation;
	int m_pendingFullScreenMode;

	int m_setWindowWidth;
	int m_setWindowHeight;

	bool m_enableFullScreen;

	bool m_doPageUpDownInLandscape;

	bool m_directRendering;
	int m_renderOffsetX;
	int m_renderOffsetY;
	int m_renderOrientation;
	bool m_paintingDisabled;

	bool m_renderingSuspended;
	
private:
	
	CardWebApp& operator=( const CardWebApp& );
	CardWebApp( const CardWebApp& );
};

#endif


