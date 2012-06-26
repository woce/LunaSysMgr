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




#ifndef EMULATEDCARDWINDOW_H_
#define EMULATEDCARDWINDOW_H_

#include "CardWindow.h"
#include "Event.h"
#include "PixmapButton.h"
#include "StatusBar.h"
#include "VirtualCoreNavi.h"

#include "CardSmoothEdgeShaderStage.h"
#include "CardRoundedCornerShaderStage.h"

class CardSmoothEdgeShaderStage;
class CardRoundedCornerShaderStage;

class EmulatedCardWindow: public CardWindow {
Q_OBJECT

public:
	EmulatedCardWindow(Window::Type type, HostWindowData* data, IpcClientHost* clientHost);
	EmulatedCardWindow(Window::Type type, const QPixmap& pixmap);
	virtual ~EmulatedCardWindow();
	virtual QRectF transitionBoundingRect();
	virtual void setAppId(const std::string& id);
	virtual void setMaximized(bool enable);
	virtual void setWindowProperties (const WindowProperties& props);
	virtual void positiveSpaceChanged(const QRect& r);
	bool isAppFreeOrientation();
	virtual void positiveSpaceAboutToChange(const QRect& r, bool fullScreen);
	virtual void positiveSpaceChangeFinished(const QRect& r);
	bool touchEvent(QTouchEvent* event);
        virtual bool sceneEvent(QEvent* event);

public Q_SLOTS:
	void slotKeyboardUpClicked();


protected:
	virtual void paintBase(QPainter* painter, bool maximized);
	virtual void resizeEvent(int w, int h);
	virtual void resizeEventSync(int w, int h);
	virtual void resizeWindowBufferEvent(int w, int h, QRect windowScreenBounds, bool forceSync=false, bool ignoreFixedOrient=false);
	virtual void mapCoordinates(qreal& x, qreal& y);
	virtual void mapFlickVelocities(qreal& x, qreal& y);
	virtual void onSetAppOrientation(int orientation);
	virtual void onSetAppFixedOrientation(int orientation, bool isPortrait);
	virtual void onSetAppFreeOrientation(bool free);
    void initializeSmoothEdgeStage();

   	virtual void onEditorFocusChanged(bool focus, const PalmIME::EditorState& state);
	virtual void removeInputFocus();

#if !defined(TARGET_EMULATOR)
	virtual void onUpdateWindowRegion(int x, int y, int w, int h);
	virtual void onUpdateFullWindow();
#endif
        void initializeRoundedCornerStage();

	virtual void fullScreenEnabled(bool val) {}

        //CardRoundedCornerShaderStage* m_roundedCornerShaderStage;
	CardSmoothEdgeShaderStage* m_smoothEdgeShaderStage;

private:
	VirtualCoreNavi* m_gestureStrip;
	StatusBar* m_statusBar;
	PixmapButton* m_keyboardButton;

	QRectF m_emulationBoundingRect;
	static QWeakPointer<QPixmap> s_upperLeftCorner;
	static QWeakPointer<QPixmap> s_upperRightCorner;
	static QWeakPointer<QPixmap> s_lowerLeftCorner;
	static QWeakPointer<QPixmap> s_lowerRightCorner;
	static QWeakPointer<QPixmap> s_background;
	static QWeakPointer<QPixmap> s_deviceFrame;

	bool m_fullScreenMode; //Not to be confused with m_fullScreenEnabled!
	qreal m_positiveSpaceAdjustment;
	int m_screenMidLine;
	int m_positiveSpace;
	Event::Orientation   m_appOrientation;
	bool  m_appFreeOrientation;
	bool  m_positiveSpaceChanging;


	QSharedPointer<QPixmap> m_upperLeftCorner;
	QSharedPointer<QPixmap> m_upperRightCorner;
	QSharedPointer<QPixmap> m_lowerLeftCorner;
	QSharedPointer<QPixmap> m_lowerRightCorner;
        QSharedPointer<QPixmap> m_background;
	QSharedPointer<QPixmap> m_deviceFrame;

	void drawDeviceFrame(QPainter* painter);
	void drawRoundedCorners(QPainter* painter);
	QRectF emulationBoundingRect();
	qreal rotationAdjustmentAngle();
	void refreshAdjustmentAngle();
	QRectF rotateEmulationBoundingRect(qreal angle);
	void layoutWidgets();
	void setFullScreenEnabled(bool val);
	void setNewEmuRect(int width, int height);
	void calculatePositiveSpaceAdjustment();
	void translateForPositiveSpaceChange(QPainter* painter, bool isUndo);
    OrientationEvent::Orientation systemOrientationFor(int orient);

	bool m_keyboardUpManually;

private Q_SLOTS:

	void slotUiRotated();

};

#endif /* EMULATEDCARDWINDOW_H_ */
