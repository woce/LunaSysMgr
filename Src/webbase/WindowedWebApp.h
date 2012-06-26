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




#ifndef WINDOWEDWEBAPP_H
#define WINDOWEDWEBAPP_H

#include "Common.h"

#include <webkitpalmtimer.h>
#include <list>

#include "AsyncCaller.h"
#include "GraphicsDefs.h"
#include "WebAppBase.h"
#include "sptr.h"
#include "Event.h"
#include "Timer.h"
#include "Window.h"
#include <PIpcChannelListener.h>
#include <PIpcBuffer.h>
#include <QRect>

class SysMgrKeyEvent;
class QKeyEvent;
class SysMgrTouchEvent;
class RemoteWindowData;
class WindowMetaData;
namespace Palm {
	class WebGLES2Context;
}

class WindowedWebApp : public WebAppBase , public PIpcChannelListener
{
public:

	WindowedWebApp(int width, int height, Window::Type type, PIpcChannel *channel = 0);
	virtual ~WindowedWebApp();

	virtual void attach(WebPage* page);
	
	virtual void paint(bool clipToWindow);

	virtual void inputEvent(sptr<Event> e);
	virtual void keyEvent(QKeyEvent* e);
	virtual void focusedEvent(bool focused);
	virtual int resizeEvent(int newWidth, int newHeight, bool resizeBuffer);
	virtual void flipEvent(int newWidth, int newHeight);
	virtual void asyncFlipEvent(int newWidth, int newHeight, int newScreenWidth, int newScreenHeight);

	virtual bool isWindowed() const { return true; }
	virtual bool isCardApp() const { return false; }
	virtual bool isDashboardApp() const { return false; }
    virtual bool isLeafApp() const { return true; }

	virtual Window::Type windowType() const { return m_winType; };

	virtual void setOrientation(Event::Orientation orient) {}

	virtual void invalidate();

	virtual bool isFocused() const { return m_focused; }

	virtual void sceneTransitionFinished() {}

	virtual void applyLaunchFeedback(int cx, int cy);

	int windowWidth() const { return m_windowWidth; }
	int windowHeight() const { return m_windowHeight; }
	
	virtual void onMessageReceived(const PIpcMessage& msg);
	virtual void onDisconnected();
	virtual int  getKey() const;
	int routingId() const;
	int metadataId() const;

	virtual void onResize(int width, int height, bool resizeBuffer);
	virtual void onFlip(int newWidth, int newHeight);
	virtual void onAsyncFlip(int newWidth, int newHeight, int newScreenWidth, int newScreenHeight);
	virtual void onSyncResize(int width, int height, bool resizeBuffer, int* newKey);
	virtual void onAdjustForPositiveSpace(int width, int height);
	virtual void onKeyboardShow(bool val);
	virtual void onClose(bool disableKeepAlive);
	virtual void onInputEvent(const SysMgrEventWrapper& wrapper);
	virtual void onKeyEvent(const SysMgrKeyEvent& keyEvent);
	virtual void onTouchEvent(const SysMgrTouchEvent& touchEvent);
	virtual void onDirectRenderingChanged();
	virtual void onSceneTransitionFinished();
	virtual void onClipboardEvent_Cut();
	virtual void onClipboardEvent_Copy();
	virtual void onClipboardEvent_Paste();
    virtual void onSelectAll();

	virtual void onSetComposingText(const std::string& text);
	virtual void onCommitComposingText();

	virtual void onCommitText(const std::string& text);

	virtual void onPerformEditorAction(int action);

	virtual void onRemoveInputFocus();

	virtual void windowSize(int& width, int& height);
	virtual void screenSize(int& width, int& height);
	
	virtual void setWindowProperties(WindowProperties &winProp);

	virtual void displayOn() {}
	virtual void displayOff() {}
	
	virtual Palm::WebGLES2Context* getGLES2Context();

protected:

	// Call this from subclasses after setting width and height
	void init();

	virtual void focus();
	virtual void unfocus();
    virtual void invalContents(int x, int y, int width, int height);
    virtual void scrollContents(int newContentsX, int newContentsY);
	virtual void loadFinished();
	virtual void stagePreparing();
	virtual void stageReady();
	virtual void editorFocusChanged(bool focused, const PalmIME::EditorState& state);
    virtual void autoCapEnabled(bool enabled);
	virtual void needTouchEvents(bool needTouchEvents);
	virtual void getWindowPropertiesString(WindowProperties &winProp, std::string &propString) const;
	
protected:

	void startPaintTimer();
	void stopPaintTimer();

	enum PendingFocus {
		PendingFocusNone = 0,
		PendingFocusTrue,
		PendingFocusFalse
	};

	RemoteWindowData* m_data;
	PIpcBuffer* m_metaDataBuffer;
	WindowMetaData* m_metaData;

	WebKitPalmTimer*	m_paintTimer;
	Window::Type		m_winType;
	int					m_width;
	int					m_height;
	bool                m_beingDeleted;
	bool                m_stagePreparing;
	bool                m_stageReady;
	bool				m_addedToWindowMgr;

	uint32_t            m_windowWidth;
	uint32_t            m_windowHeight;

	QRect m_paintRect;

	int  m_blockCount; //Keeps track on how many PenDown's we blocked.
	bool m_blockPenEvents;
	uint32_t m_lastGestureEndTime;
	bool m_showPageStats;
	bool m_focused;
	PendingFocus m_pendingFocus;

	NativeGraphicsSurface* m_metaCaretHint;

	Timer<WindowedWebApp> m_showWindowTimer;

    bool m_generateMouseClick;

	WindowProperties m_winProps;

protected:
	
	void renderPageStatistics(int offsetX=0, int offsetY=0);
	void renderMetaHint(int offsetX=0, int offsetY=0);
	bool showWindowTimeout();
	bool appLoaded() const;
	bool isTransparent() const;
	
private:

	struct RecordedGestureEntry {
		RecordedGestureEntry(float s, float r, int cX, int cY)
			: scale(s)
			, rotate(r)
			, centerX(cX)
			, centerY(cY) {
		}
		
		float scale;
		float rotate;
		int centerX;
		int centerY;
	};

	std::list<RecordedGestureEntry> m_recordedGestures;	

	void recordGesture(float s, float r, int cX, int cY);
	RecordedGestureEntry getAveragedGesture() const;

	void keyGesture(QKeyEvent* e);

private:	
	
	WindowedWebApp& operator=(const WindowedWebApp&);
	WindowedWebApp(const WindowedWebApp&);

	friend class WebAppBase;
	friend class JsSysObject;
};

#endif /* WINDOWEDWEBAPP_H */
