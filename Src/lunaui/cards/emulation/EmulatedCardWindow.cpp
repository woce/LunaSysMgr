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




#include "Common.h"
#include "Logging.h"
#include "EmulatedCardWindow.h"
#include "SystemUiController.h"
#include "CardTransition.h"
#include "Settings.h"
#include "CardDropShadowEffect.h"
#include "WindowServer.h"
#include "ApplicationManager.h"
#include "ApplicationDescription.h"

#include "IpcClientHost.h"
#include "WindowServer.h"
#include "WindowServerLuna.h"

#include <PIpcBuffer.h>
#include <PIpcChannel.h>
#include <SysMgrDefs.h>

#define MESSAGES_INTERNAL_FILE "SysMgrMessagesInternal.h"
#include <PIpcMessageMacros.h>

#include "QtUtils.h"
#include "Time.h"

#include "PixmapButton.h"
#include "IMEController.h"

#include <QGesture>


#define UPPER_LEFT_CORNER (Settings::LunaSettings()->lunaSystemResourcesPath + std::string("/wm-corner-top-left.png"))
#define UPPER_RIGHT_CORNER (Settings::LunaSettings()->lunaSystemResourcesPath + std::string("/wm-corner-top-right.png"))
#define LOWER_LEFT_CORNER (Settings::LunaSettings()->lunaSystemResourcesPath + std::string("/wm-corner-bottom-left.png"))
#define LOWER_RIGHT_CORNER (Settings::LunaSettings()->lunaSystemResourcesPath + std::string("/wm-corner-bottom-right.png"))
#define BACKGROUND (Settings::LunaSettings()->lunaSystemResourcesPath + std::string("/emucard-bg.png"))
#define DEVICE_FRAME (Settings::LunaSettings()->lunaSystemResourcesPath + std::string("/emucard-device-frame.png"))
#define KEYBOARD_UP_BUTTON (Settings::LunaSettings()->lunaSystemResourcesPath + std::string("/emucard-kb-up_icon.png"))

QWeakPointer<QPixmap> EmulatedCardWindow::s_upperLeftCorner;
QWeakPointer<QPixmap> EmulatedCardWindow::s_upperRightCorner;
QWeakPointer<QPixmap> EmulatedCardWindow::s_lowerLeftCorner;
QWeakPointer<QPixmap> EmulatedCardWindow::s_lowerRightCorner;
QWeakPointer<QPixmap> EmulatedCardWindow::s_deviceFrame;
QWeakPointer<QPixmap> EmulatedCardWindow::s_background;

EmulatedCardWindow::EmulatedCardWindow(Window::Type type, HostWindowData* data, IpcClientHost* clientHost)
	: CardWindow(type, data, clientHost)
	, m_fullScreenMode(false)
	, m_appFreeOrientation(false)
	, m_positiveSpaceChanging(false)
        , m_smoothEdgeShaderStage(0)
	, m_keyboardUpManually(false)
{

	m_positiveSpaceAdjustment = 0;
	m_screenMidLine = 0;
	//NOTE: it seems the bounding rect may have a width and height of 0 at this time
	//making brect's use in the calculation below pointless

	QRectF brect = boundingRect();
	int emuWidth = Settings::LunaSettings()->emulatedCardWidth;
	int emuHeight = Settings::LunaSettings()->emulatedCardHeight - Settings::LunaSettings()->positiveSpaceTopPadding;

	//Center emu card in its bounding card
	int x = (brect.x() + (brect.width()/2)) - (emuWidth/2);
	int y = (brect.y() + (brect.height()/2)) - (emuHeight/2);

	QRectF emuRect(x,y,emuWidth,emuHeight);
	m_emulationBoundingRect = emuRect;

	//Start out with a screen mid-line that is half the buffer height.
	m_screenMidLine = (SystemUiController::instance()->currentUiHeight() - Settings::LunaSettings()->positiveSpaceTopPadding)/2;

	//Find the right angle to start bringing in this card at.
	//This will control the rotation angle for rendering the
	//card, how the widgets are laid out, etc.

	refreshAdjustmentAngle();

	//Note that these assets are hidden at initialization.  They will be shown the
	//first time the card is maximized.

	m_gestureStrip = new VirtualCoreNavi(emuWidth, 66);
	m_gestureStrip->setParentItem(this);
	m_gestureStrip->hide();

	//Set the accepted corenavi gestures.  We're currently supporting...only back.
	QList<Qt::Key> acceptedKeys;
	acceptedKeys.append(Qt::Key_CoreNavi_Back);
	m_gestureStrip->setAcceptedKeys(acceptedKeys);
	m_gestureStrip->setTapGesture(Qt::Key_CoreNavi_Back);

	m_statusBar = new StatusBar(StatusBar::TypeEmulatedCard, emuWidth, Settings::LunaSettings()->positiveSpaceTopPadding);
	m_statusBar->setParentItem(this);
	m_statusBar->init();
	m_statusBar->hide();

	m_keyboardButton = new PixmapButton(QPixmap (qFromUtf8Stl(KEYBOARD_UP_BUTTON)),
										QRect(0,0,52,44), QRect(0,44,52,44));
	m_keyboardButton->setParentItem(this);
	m_keyboardButton->setManualMode(true);
	m_keyboardButton->hide();
	m_keyboardButton->createLargerHitTargetRect(10,10);
		connect(m_keyboardButton,SIGNAL(clickComplete()),SLOT(slotKeyboardUpClicked()));
	layoutWidgets();

	//And now to load the rounded corners.  These are drawn in by paintBase()

	m_upperLeftCorner = s_upperLeftCorner.toStrongRef();
	if (m_upperLeftCorner.isNull()) {
		QPixmap corner = QPixmap::fromImage(QImage(qFromUtf8Stl(UPPER_LEFT_CORNER)));
		if (corner.isNull()) {
			corner = QPixmap(10, 10);
			corner.fill(Qt::transparent);
		}
		m_upperLeftCorner = QSharedPointer<QPixmap>(new QPixmap(corner));
		s_upperLeftCorner = m_upperLeftCorner.toWeakRef();
	}

	m_upperRightCorner = s_upperRightCorner.toStrongRef();
	if (m_upperRightCorner.isNull()) {
		QPixmap corner = QPixmap::fromImage(QImage(qFromUtf8Stl(UPPER_RIGHT_CORNER)));
		if (corner.isNull()) {
			corner = QPixmap(10, 10);
			corner.fill(Qt::transparent);
		}
		m_upperRightCorner = QSharedPointer<QPixmap>(new QPixmap(corner));
		s_upperRightCorner = m_upperRightCorner.toWeakRef();
	}

	m_lowerLeftCorner = s_lowerLeftCorner.toStrongRef();
	if (m_lowerLeftCorner.isNull()) {
		QPixmap corner = QPixmap::fromImage(QImage(qFromUtf8Stl(LOWER_LEFT_CORNER)));
		if (corner.isNull()) {
			corner = QPixmap(10, 10);
			corner.fill(Qt::transparent);
		}
		m_lowerLeftCorner = QSharedPointer<QPixmap>(new QPixmap(corner));
		s_lowerLeftCorner = m_lowerLeftCorner.toWeakRef();
	}

	m_lowerRightCorner = s_lowerRightCorner.toStrongRef();
	if (m_lowerRightCorner.isNull()) {
		QPixmap corner = QPixmap::fromImage(QImage(qFromUtf8Stl(LOWER_RIGHT_CORNER)));
		if (corner.isNull()) {
			corner = QPixmap(10, 10);
			corner.fill(Qt::transparent);
		}
		m_lowerRightCorner = QSharedPointer<QPixmap>(new QPixmap(corner));
		s_lowerRightCorner = m_lowerRightCorner.toWeakRef();
	}

	m_deviceFrame = s_deviceFrame.toStrongRef();
	if (m_deviceFrame.isNull()) {
		QPixmap corner = QPixmap::fromImage(QImage(qFromUtf8Stl(DEVICE_FRAME)));
		if (corner.isNull()) {
			corner = QPixmap(10, 10);
			corner.fill(Qt::transparent);
		}
		m_deviceFrame = QSharedPointer<QPixmap>(new QPixmap(corner));
		s_deviceFrame = m_deviceFrame.toWeakRef();
        }

        m_background = s_background.toStrongRef();
        if (m_background.isNull()) {
                QPixmap corner = QPixmap::fromImage(QImage(qFromUtf8Stl(BACKGROUND)));
                if (corner.isNull()) {
                        corner = QPixmap(10, 10);
                        corner.fill(Qt::transparent);
                }
                m_background = QSharedPointer<QPixmap>(new QPixmap(corner));
                s_background = m_background.toWeakRef();
        }

	grabGesture(Qt::TapGesture);
}

EmulatedCardWindow::EmulatedCardWindow(Window::Type type, const QPixmap& pixmap)
	: CardWindow(type, pixmap)
	, m_fullScreenMode(false)
	, m_appFreeOrientation(false)
        , m_smoothEdgeShaderStage(0)
{

}

EmulatedCardWindow::~EmulatedCardWindow() {
	ungrabGesture(Qt::TapGesture);
	delete m_keyboardButton;
	delete m_gestureStrip;
	delete m_statusBar;
}

void EmulatedCardWindow::setAppId(const std::string& id)
{
	m_appId = id;
	ApplicationDescription* appDesc = ApplicationManager::instance()->getAppById(m_appId);

	m_statusBar->setMaximizedAppTitle(true, appDesc->launchPoints().front()->menuName().c_str());
}


void EmulatedCardWindow::resizeEvent(int w, int h)
{
	//Emulated cards cannot resize to full screen, ever.
	int maxWidth = Settings::LunaSettings()->emulatedCardWidth;
	int maxHeight = Settings::LunaSettings()->emulatedCardHeight;

	if (!m_fullScreenMode) {
		maxHeight = maxHeight - Settings::LunaSettings()->positiveSpaceTopPadding;
	}

	w = (w > maxWidth ? maxWidth : w);
	h = (h > maxHeight ? maxHeight : h);

	if (w>h) {
		//Going to a landscape card
		setNewEmuRect(maxHeight, maxWidth);
		if (m_channel)
			m_channel->sendAsyncMessage(new View_Resize(routingId(), maxHeight, maxWidth, false));
	} else {
		//Going to a portrait card
		setNewEmuRect(maxWidth, maxHeight);
		if (m_channel)
			m_channel->sendAsyncMessage(new View_Resize(routingId(), maxWidth, maxHeight, false));
	}


}

void EmulatedCardWindow::resizeEventSync(int w, int h)
{
	//Emulated cards cannot resize to full screen, ever.
	int maxWidth = Settings::LunaSettings()->emulatedCardWidth;
	int maxHeight = Settings::LunaSettings()->emulatedCardHeight;

	if (!m_fullScreenMode) {
		maxHeight = maxHeight - Settings::LunaSettings()->positiveSpaceTopPadding;

	}

	w = (w > maxWidth ? maxWidth : w);
	h = (h > maxHeight ? maxHeight : h);

	if (w>h) {
		//Going to a landscape card
		setNewEmuRect(maxHeight, maxWidth);
		if (m_channel) {

			SystemUiController::instance()->aboutToSendSyncMessage();

			int dummy = 0;
			m_channel->sendSyncMessage(new View_SyncResize(routingId(), maxHeight, maxWidth, false, &dummy));
		}
	} else {
		//Going to a portrait card
		setNewEmuRect(maxWidth, maxHeight);
		if (m_channel) {

			SystemUiController::instance()->aboutToSendSyncMessage();

			int dummy = 0;
			m_channel->sendSyncMessage(new View_SyncResize(routingId(), maxWidth, maxHeight, false, &dummy));
		}
	}


}

void EmulatedCardWindow::resizeWindowBufferEvent(int w, int h, QRect windowScreenBounds, bool forceSync, bool ignoreFixedOrient)
{
	bool directRendering = m_maximized;
	bool visible = isVisible() && this->sceneBoundingRect().intersects(WindowServer::instance()->sceneRect());
	bool obscured = SystemUiController::instance()->maximizedCardWindow() ? (SystemUiController::instance()->maximizedCardWindow() != this) : false;
	bool synch = forceSync || m_maximized || (visible  && !obscured);

	if((w == (int)m_bufWidth) && (h == (int)m_bufHeight)) {
		// already the right size, so do nothing
		return;
	}

	if(Settings::LunaSettings()->displayUiRotates) {
		if(directRendering)
			setMaximized(false); // disable direct rendering for the resize event

			//Emulated cards cannot resize to full screen, ever.
			int maxWidth = Settings::LunaSettings()->emulatedCardWidth;
			int maxHeight = Settings::LunaSettings()->emulatedCardHeight;

			m_screenMidLine = (SystemUiController::instance()->currentUiHeight() - Settings::LunaSettings()->positiveSpaceTopPadding)/2;

			if (!m_fullScreenMode) {
				maxHeight = maxHeight - Settings::LunaSettings()->positiveSpaceTopPadding;

			}

			w = (w > maxWidth ? maxWidth : w);
			h = (h > maxHeight ? maxHeight : h);

			if (w>h) {
				//Going to a landscape card
				setNewEmuRect(maxHeight, maxWidth);
				HostWindow::resizeEventSync(maxHeight,maxWidth);
			} else {
				//Going to a portrait card
				setNewEmuRect(maxWidth, maxHeight);
				HostWindow::resizeEventSync(maxWidth,maxHeight);
			}


		setBoundingRect(windowScreenBounds.width(), windowScreenBounds.height());
		setVisibleDimensions(windowScreenBounds.width(), windowScreenBounds.height());

		m_paintPath = QPainterPath();
		m_paintPath.addRoundedRect(m_boundingRect, 25, 25);

		// reconstruct shadow
		CardDropShadowEffect* shadow = static_cast<CardDropShadowEffect*>(graphicsEffect());
		if (shadow) {
			bool enable = shadow->isEnabled();
			setGraphicsEffect(0);
			shadow = new CardDropShadowEffect(this);
			setGraphicsEffect(shadow);
			shadow->setEnabled(enable);
		}

		if(directRendering)
			setMaximized(true); // re-enable direct rendering
	}

}

void EmulatedCardWindow::initializeSmoothEdgeStage()
{
#if defined(USE_SMOOTHEDGE_SHADER)
        if (!m_smoothEdgeShaderStage){
            m_smoothEdgeShaderStage = new CardSmoothEdgeShaderStage();
        }

        if (m_adjustmentAngle != 90 && m_adjustmentAngle != -90){
                if (m_data) {
                    m_smoothEdgeShaderStage->setParameters(m_data->width(),
                                                           m_data->height(),
                                                           emulationBoundingRect().toRect().width(),
                                                           emulationBoundingRect().toRect().height());
                }
        } else {
                if (m_data) {
                        m_smoothEdgeShaderStage->setParameters(m_data->width(),
                                                               m_data->height(),
                                                               emulationBoundingRect().toRect().width(),
                                                               emulationBoundingRect().toRect().height());
                }
        }

#endif


}

void EmulatedCardWindow::initializeRoundedCornerStage()
{
#if defined(USE_ROUNDEDCORNER_SHADER)
        if (!m_roundedCornerShaderStage){
            m_roundedCornerShaderStage = new CardRoundedCornerShaderStage();
        }

        if (m_adjustmentAngle != 90 && m_adjustmentAngle != -90){
                if (m_background) {
                    m_roundedCornerShaderStage->setParameters(m_background->width(),
                                                           m_background->height(),
                                                           m_background->width(),
                                                           m_background->height(), 77);
                }
        }
        else {
                if (m_background) {
                        m_roundedCornerShaderStage->setParameters(m_background->height(),
                                                               m_background->width(),
                                                               m_background->height(),
                                                               m_background->width(), 77);
                }
        }



#endif


}


void EmulatedCardWindow::paintBase(QPainter* painter, bool maximized)
{
	if (G_UNLIKELY(m_transition)) {
		painter->fillRect(boundingRect(), QColor(0x0f,0x0f,0x0f,0xff));

		bool clipping = painter->hasClipping();
		QPainterPath clipPath;
		if (clipping) {
			clipPath = painter->clipPath();
		}

		painter->rotate(rotationAdjustmentAngle());
		translateForPositiveSpaceChange(painter, false);
		drawDeviceFrame(painter);
		painter->setClipRect(transitionBoundingRect());
		m_transition->draw(painter, m_paintPath, m_maximized);
		drawRoundedCorners(painter);
		//Undo changes to the painter.
		translateForPositiveSpaceChange(painter, true);
		painter->rotate(-rotationAdjustmentAngle());

		if (m_transition->completed()) {

			m_data->onSceneTransitionFinish();

			delete m_transition;
			m_transition = NULL;

			m_transitionTimer.stop();
		}


		if (G_UNLIKELY(clipping)) {
			painter->setClipPath(clipPath);
		} else {
			painter->setClipping(false);
		}

	} else if (G_UNLIKELY(m_data->acquireTransitionPixmap())) {

		QRect targetRect = boundingRect().toRect();
		QRect sourceRect = QRect(0, 0, emulationBoundingRect().width(), emulationBoundingRect().height());

		painter->setBrushOrigin(targetRect.x(), targetRect.y());

		if (maximized) {
			painter->fillRect(boundingRect(), QColor(0x0f,0x0f,0x0f,0xff));
		} else {
			painter->fillPath(m_paintPath, QColor(0x0f,0x0f,0x0f,0xff));
		}

		painter->rotate(rotationAdjustmentAngle());
		translateForPositiveSpaceChange(painter, false);
		drawDeviceFrame(painter);

		if (G_LIKELY(maximized)) {
			painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
			painter->drawPixmap(emulationBoundingRect().toRect(), *m_data->acquireTransitionPixmap(), sourceRect);
			drawRoundedCorners(painter);
			//painter->drawPixmap(QRect(x,y,emuWidth,emuHeight), *m_data->acquireTransitionPixmap());
		} else {
			painter->drawPixmap(emulationBoundingRect().toRect(), *m_data->acquireTransitionPixmap(), sourceRect);
			drawRoundedCorners(painter);
			//painter->drawPixmap(QRect(x,y,emuWidth,emuHeight), *m_data->acquireTransitionPixmap());
		}

		if (G_LIKELY(maximized))
			painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
		painter->setBrushOrigin(0, 0);
		//Undo changes to the painter
		translateForPositiveSpaceChange(painter, true);
		painter->rotate(-rotationAdjustmentAngle());
	} else {
		if (maximized) {
			// faster, rectangular blit
			painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
			painter->fillRect(boundingRect(), QColor(0x0f,0x0f,0x0f,0xff));
			painter->rotate(rotationAdjustmentAngle());
			translateForPositiveSpaceChange(painter, false);
			drawDeviceFrame(painter);
			Window::paint(painter, 0, 0);
			drawRoundedCorners(painter);

			//Undo changes to the painter
			translateForPositiveSpaceChange(painter, true);

			painter->rotate(-rotationAdjustmentAngle());
			painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
		} else {
			// draw with rounded corners
                        const QPixmap* pix = acquireScreenPixmap();
                        if (pix) {
                                initializeSmoothEdgeStage();
                                initializeRoundedCornerStage();

                                painter->setCompositionMode(QPainter::CompositionMode_SourceOver);

                                //painter->fillRect(boundingRect(), QColor(0,0,0, 0xFF));
#if defined(USE_ROUNDEDCORNER_SHADER)
                                QRectF brect = boundingRect();
                                QRect sourceRectBG = QRect(0,0,boundingRect().toRect().width(), boundingRect().toRect().height());
                                m_roundedCornerShaderStage->setOnPainter(painter);
                                painter->drawPixmap(brect, *m_background, m_background->rect());
                                m_roundedCornerShaderStage->removeFromPainter(painter);
#else
                                painter->fillPath(m_paintPath, QColor(0x0f,0x0f,0x0f,0xff));
#endif
				painter->rotate(rotationAdjustmentAngle());
				drawDeviceFrame(painter);
				painter->save();
                                qreal zPosition = m_position.trans.z();
				//zPosition is a value between 0 and 1, with a minimized card being
				//approximately 0.5.  At 1, we want scaling to be 1 (i.e. no difference)
				//and at zPosition of 0.5, a scale of 1.5 looks pretty good.  So, we'll
				//subtract zPosition from 2.0 to get the scaling factor.
				painter->scale(2.0 - zPosition, 2.0 - zPosition);
                                QRect sourceRect = QRect(0,0,emulationBoundingRect().toRect().width(), emulationBoundingRect().toRect().height());
#if defined(USE_SMOOTHEDGE_SHADER)
                                m_smoothEdgeShaderStage->setOnPainter(painter);
                                painter->drawPixmap(emulationBoundingRect().toRect(), *pix, sourceRect);
                                m_smoothEdgeShaderStage->removeFromPainter(painter);
#else
                                painter->drawPixmap(emulationBoundingRect().toRect(), *pix, sourceRect);
#endif
                                drawRoundedCorners(painter);
				painter->rotate(-rotationAdjustmentAngle());
				painter->restore();
                                painter->setBrushOrigin(0, 0);
			}
		}
	}
}

void EmulatedCardWindow::drawDeviceFrame(QPainter* painter) {

	QPainter::CompositionMode compositionMode = painter->compositionMode();
	painter->setCompositionMode(QPainter::CompositionMode_SourceOver);

	QRectF emuRect = rotateEmulationBoundingRect(0);

	qreal x = emuRect.center().x() - (m_deviceFrame->width()/2);
	qreal y = emuRect.center().y() - (m_deviceFrame->height()/2);

	//This is such an irritation.  The painter is currently rotated in,
	//but this asset needs its own rotation to be laid correctly.  So, first
	//we're going to un-rotate the painter...

	painter->rotate(-rotationAdjustmentAngle());


	qreal assetRotationAngle = 0.0;
	int uiOrientation = WindowServer::instance()->getUiOrientation();

	//And now rotate it so it stays "fixed" at any orientation.

	switch (uiOrientation) {
        case OrientationEvent::Orientation_Up: {assetRotationAngle = -90; break;}
        case OrientationEvent::Orientation_Left: {assetRotationAngle = 180; break;}
        case OrientationEvent::Orientation_Right: {assetRotationAngle = 0; break;}
        case OrientationEvent::Orientation_Down: {assetRotationAngle = 90; break;}
	}

	painter->rotate(assetRotationAngle);
	painter->drawPixmap(x, y, *m_deviceFrame);

	//Now undo this.
	painter->rotate(-assetRotationAngle);
	painter->rotate(rotationAdjustmentAngle());

	//It's handy debugging to uncomment these lines
	//painter->setPen(0xffffff);
	//painter->drawRect(emuRect);

	painter->setCompositionMode(compositionMode);

}


void EmulatedCardWindow::drawRoundedCorners(QPainter* painter) {

	QPainter::CompositionMode compositionMode = painter->compositionMode();


	QRectF emuRect;
    OrientationEvent::Orientation orient = WindowServer::instance()->getUiOrientation();

	//No rounded corners in full screen mode.
	if (m_fullScreenMode) return;

	emuRect = rotateEmulationBoundingRect(0);

	painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
	painter->drawPixmap(emuRect.x(), emuRect.y(),
						*m_upperLeftCorner);

	painter->drawPixmap(emuRect.topRight().x() - m_upperRightCorner->width(),
						emuRect.topRight().y(),
						*m_upperRightCorner);

	painter->drawPixmap(emuRect.bottomLeft().x(),
						emuRect.bottomLeft().y() - m_lowerLeftCorner->height(),
						*m_lowerLeftCorner);

	painter->drawPixmap(emuRect.bottomRight().x() - m_lowerRightCorner->width(),
						emuRect.bottomRight().y() - m_lowerRightCorner->height(),
						*m_lowerRightCorner);

	//It's handy debugging to uncomment these lines
	//painter->setPen(0xffffff);
	//painter->drawRect(emuRect);

	painter->setCompositionMode(compositionMode);

}

void EmulatedCardWindow::mapCoordinates(qreal& x, qreal& y) {

	//Before we even get started, the keyboard activation button
	//is always in the CardWindow's coordinate space and isn't
	//relative to the emulated app, so its touch target is
	//"in a DMZ" of this algorithm.

	QRectF hitTarget = mapRectFromItem(m_keyboardButton, m_keyboardButton->boundingRect());
	if (hitTarget.contains(x,y)) {
		//The tap coordinates landed on the keyboard button, so let them through
		//without adjusting them
		return;
	}


	qreal adjustmentAngle = rotationAdjustmentAngle();

	qreal temp;

	if (adjustmentAngle == 90) {
		temp = -x;
		x = y;
		y = temp;
	}
	else if (adjustmentAngle == -90) {
		temp = x;
		x = -y;
		y = temp;
	} else if (adjustmentAngle == 0) {
		//Do nothing
	}
	else if (adjustmentAngle == 180) {
		x = -x;
		y = -y;
	}

	int emuWidth = emulationBoundingRect().toRect().width();
	int emuHeight = emulationBoundingRect().toRect().height();
	QRectF br = boundingRect();
	x = x - br.x() - (br.width()/2) + (emuWidth/2);
	y = y - br.y() - (br.height()/2) + (emuHeight/2);

	if (adjustmentAngle == 90) {
		x = x + m_positiveSpaceAdjustment;
	}
	else if (adjustmentAngle == -90) {
		x = x - m_positiveSpaceAdjustment;
	} else if (adjustmentAngle == 0) {
		y = y + m_positiveSpaceAdjustment;
	}
	else if (adjustmentAngle == 180) {
		y = y - m_positiveSpaceAdjustment;
	}

}

void EmulatedCardWindow::mapFlickVelocities(qreal& x, qreal& y) {
	qreal adjustmentAngle = rotationAdjustmentAngle();
	qreal temp;

	if (adjustmentAngle == 90) {
		temp = -x;
		x = y;
		y = temp;
	}
	else if (adjustmentAngle == -90) {
		temp = x;
		x = -y;
		y = temp;
	} else if (adjustmentAngle == 0) {
		//Do nothing
	}
	else if (adjustmentAngle == 180) {
		x = -x;
		y = -y;
	}
}


//This is created as a public method to keep the card transitions bounded in
//but it's also a handy way to get the region rendered to for emulation mode
//purposes.

//This actually now redirects to emulationBoundingRect() so that transition
//animations don't leave the rounded corners, but I'm leaving the separate
//function in case it becomes useful to do some separate logic in here one day.

QRectF EmulatedCardWindow::transitionBoundingRect() {

	return emulationBoundingRect();
}


//This at one point calculated a different rect than transitionBoundingRect()
//it used to adjust for the status bar height but now we lock in that figure
//in the constructor.  This is still a private function though and so it
//might be useful to keep transitionBoundingRect() as the public function.

QRectF EmulatedCardWindow::emulationBoundingRect() {
	return m_emulationBoundingRect;
}


//I should really document this better and probably use an enum to make this
//just a dose more typesafe (or something).  This represents the number of
//degrees to rotate the card and contents versus the UI orientation.

//Up: 90 degrees clockwise
//Left: do nothing
//Right: flip the card up-down and left-right / rotate 180 degrees
//Down: 90 degrees counterclockwise.

//This keeps the card "bottom" pointing at the hardware home button.

void EmulatedCardWindow::refreshAdjustmentAngle() {

	int uiOrientation = WindowServer::instance()->getUiOrientation();
	int homeButtonAngle = Settings::LunaSettings()->homeButtonOrientationAngle;

	if (homeButtonAngle == 0 || homeButtonAngle == 180) {
		if (m_appFreeOrientation) {
			switch (uiOrientation) {
            case OrientationEvent::Orientation_Up: {m_adjustmentAngle = -90; break;}
            case OrientationEvent::Orientation_Left: {m_adjustmentAngle = 180; break;}
            case OrientationEvent::Orientation_Right: {m_adjustmentAngle = 0; break;}
            case OrientationEvent::Orientation_Down: {m_adjustmentAngle = 90; break;}
			}
		} else {
			switch (uiOrientation) {
            case OrientationEvent::Orientation_Up: {m_adjustmentAngle = -90; break;}
            case OrientationEvent::Orientation_Left: {m_adjustmentAngle = 180; break;}
            case OrientationEvent::Orientation_Right: {m_adjustmentAngle = 0; break;}
            case OrientationEvent::Orientation_Down: {m_adjustmentAngle = 90; break;}
			}
		}
	} else {
		if (m_appFreeOrientation) {
			switch (uiOrientation) {
            case OrientationEvent::Orientation_Up: {m_adjustmentAngle = 90; break;}
            case OrientationEvent::Orientation_Left: {m_adjustmentAngle = 180; break;}
            case OrientationEvent::Orientation_Right: {m_adjustmentAngle = 0; break;}
            case OrientationEvent::Orientation_Down: {m_adjustmentAngle = -90; break;}
			}
		} else {
			switch (uiOrientation) {
            case OrientationEvent::Orientation_Up: {m_adjustmentAngle = -90; break;}
            case OrientationEvent::Orientation_Left: {m_adjustmentAngle = 180; break;}
            case OrientationEvent::Orientation_Right: {m_adjustmentAngle = 0; break;}
            case OrientationEvent::Orientation_Down: {m_adjustmentAngle = 90; break;}
			}
		}
	}
}

void EmulatedCardWindow::translateForPositiveSpaceChange(QPainter* painter, bool isUndo) {
	float angle = rotationAdjustmentAngle();
	int xoffset = 0;
	int yoffset = 0;

	if (angle == 0) {
		//Keys from the bottom
		yoffset -= m_positiveSpaceAdjustment;
	} else if (angle == 180) {
		//Keys from the top
		yoffset += m_positiveSpaceAdjustment;
	} else if (angle == -90 || angle == 270) {
		//Keys from the left
		xoffset += m_positiveSpaceAdjustment;
	} else if (angle == 90 ) {
		//Keys from the right
		xoffset -= m_positiveSpaceAdjustment;
	}

	if (isUndo) {
		//On an undo, invert the translation
		xoffset= -xoffset;
		yoffset= -yoffset;
	}

	painter->translate(xoffset,yoffset);

}

qreal EmulatedCardWindow::rotationAdjustmentAngle() {
	return m_adjustmentAngle;
}

QRectF EmulatedCardWindow::rotateEmulationBoundingRect(qreal angle) {
	int emuWidth = m_emulationBoundingRect.width();
	int emuHeight = m_emulationBoundingRect.height();

	QRectF brect = boundingRect();

	//Center emu card in its bounding card
	int x = (brect.x() + (brect.width()/2)) - (emuWidth/2);
	int y = (brect.y() + (brect.height()/2)) - (emuHeight/2);

	if (angle == 90 || angle == -90) {
		return QRectF(y,x,emuHeight,emuWidth);
	}
	return QRectF(x,y,emuWidth,emuHeight);

}

void EmulatedCardWindow::layoutWidgets() {

	qreal angle = rotationAdjustmentAngle();

	int side = 0;

	QRectF emuRect = rotateEmulationBoundingRect(angle);

	QPointF center = emuRect.center();

	qreal height = emuRect.height();
	qreal width = emuRect.width();

	QPointF bottomCenter = QPointF(center.x(), center.y()+(height/2));
	QPointF topCenter = QPointF(center.x(), center.y()-(height/2));
	QPointF leftCenter = QPointF(center.x()-(width/2), center.y());
	QPointF rightCenter = QPointF(center.x()+(width/2), center.y());

	int orient = WindowServer::instance()->getUiOrientation();

	//The keyboard button actually tracks the system UI rather than the
	//app, so it's laid out independently

	QRectF brect = boundingRect();

	//X-axis margin: 17px
	//Y-axis margin: 18px

	m_keyboardButton->setPos(brect.bottomRight().x() - m_keyboardButton->boundingRect().width() - 17,
							brect.bottomRight().y() - m_keyboardButton->boundingRect().height() - 18);

	//The button should be "fixed", meaning the system orientations map
	//as follows:
	//
	// Up->left
	// Left->Up
	// Right->down
	// down->right

	const Event::Orientation app_orientations[] = {Event::Orientation_Up,
										Event::Orientation_Left,
										Event::Orientation_Down,
										Event::Orientation_Right};

	const Event::Orientation system_orientations[] = {Event::Orientation_Left,
										Event::Orientation_Up,
										Event::Orientation_Right,
										Event::Orientation_Down};

	for (int i=0; i<4; i++) {
		if (orient == system_orientations[i]) {
			side = app_orientations[i];
		}
	}

	int back_button_offset = 14;

	/* Terminology note-- here, "side" means the following:
	 *
	 * up -> buttons go on bottom, status bar on top
	 * right -> buttons go on right, status bar (which shouldn't show) on left
	 * left -> opposite of right
	 * down -> opposite of up
	 *
	 * This is slightly better self-documenting than relying strictly on the
	 * adjustment angle.
	 */

	if (side == Event::Orientation_Up) {
		m_gestureStrip->setPos(topCenter.x(), topCenter.y()-m_gestureStrip->boundingRect().height()/2);
		m_gestureStrip->setRotation(180);


		m_statusBar->setPos(emuRect.right() - emuRect.width()/2,
							emuRect.bottom() + Settings::LunaSettings()->positiveSpaceTopPadding/2);
		m_statusBar->setRotation(180);

	}
	else if (side == Event::Orientation_Down) {
		m_gestureStrip->setPos(bottomCenter.x(), bottomCenter.y()+m_gestureStrip->boundingRect().height()/2);
		m_gestureStrip->setRotation(0);

		m_statusBar->setPos(emuRect.right() - emuRect.width()/2,
							emuRect.top() - Settings::LunaSettings()->positiveSpaceTopPadding/2);
		m_statusBar->setRotation(0);

	}
	else if (side == Event::Orientation_Left) {
		m_gestureStrip->setRotation(-90);
		m_gestureStrip->setPos(rightCenter.x()+m_gestureStrip->boundingRect().height()/2, rightCenter.y());

		m_statusBar->setPos(emuRect.left()-Settings::LunaSettings()->positiveSpaceTopPadding/2,
							emuRect.bottom() - emuRect.height()/2);
		m_statusBar->setRotation(-90);

	}
	else if (side == Event::Orientation_Right) {
		m_gestureStrip->setRotation(90);
		m_gestureStrip->setPos(leftCenter.x()-m_gestureStrip->boundingRect().height()/2, leftCenter.y());


		m_statusBar->setPos(emuRect.right()+Settings::LunaSettings()->positiveSpaceTopPadding/2,
							emuRect.top() + emuRect.height()/2);
		m_statusBar->setRotation(90);

	}

	//Translate the widgets to adjust for changes to positive space
	m_gestureStrip->setPos(m_gestureStrip->pos().x(), m_gestureStrip->pos().y()-m_positiveSpaceAdjustment);
	m_statusBar->setPos(m_statusBar->pos().x(), m_statusBar->pos().y()-m_positiveSpaceAdjustment);
}

void EmulatedCardWindow::setMaximized(bool enable) {

	refreshAdjustmentAngle();

	if (enable) {
		layoutWidgets();
		if (!m_fullScreenMode) { m_statusBar->show(); }
		m_gestureStrip->show();
		m_keyboardButton->show();
	} else {
		m_statusBar->hide();
		m_gestureStrip->hide();
		m_keyboardButton->hide();
	}
	CardWindow::setMaximized(enable);
}

void EmulatedCardWindow::slotUiRotated() {
	refreshAdjustmentAngle();
	layoutWidgets();

}

void EmulatedCardWindow::setFullScreenEnabled(bool val) {

	m_fullScreenMode = val;
	//Adjust the emulation bounding rect.
	int emuWidth = Settings::LunaSettings()->emulatedCardWidth;
	int emuHeight = Settings::LunaSettings()->emulatedCardHeight;

	if (!val) {
		emuHeight = emuHeight - Settings::LunaSettings()->positiveSpaceTopPadding;
	}

	setNewEmuRect(emuWidth, emuHeight);

	//hide the status bar
	if (m_maximized) {
		if (val) {m_statusBar->hide();}
		else {m_statusBar->show();}
	}

	resizeEvent(emuWidth, emuHeight);
	//Move the nav buttons
	layoutWidgets();

	//Force a repaint.
	update();
}

void EmulatedCardWindow::setWindowProperties (const WindowProperties& props) {
	CardWindow::setWindowProperties(props);

	if (props.flags & WindowProperties::isSetFullScreen) {
		if (props.fullScreen)
			setFullScreenEnabled(true);
		else
			setFullScreenEnabled(false);
	}

}

void EmulatedCardWindow::setNewEmuRect(int width, int height) {

	QRectF brect = boundingRect();

	//Center emu card in its bounding card
	int x = (brect.x() + (brect.width()/2)) - (width/2);
	int y = (brect.y() + (brect.height()/2)) - (height/2);

	QRectF emuRect(x,y,width,height);
	m_emulationBoundingRect = emuRect;

}

void EmulatedCardWindow::onSetAppOrientation(int orientation) {
	refreshAdjustmentAngle();
	layoutWidgets();
}

void EmulatedCardWindow::onSetAppFixedOrientation(int orientation, bool isPortrait){

    if (orientation != OrientationEvent::Orientation_Invalid) {
		orientation = (int)systemOrientationFor(orientation);
		isPortrait = true;
	}

	CardWindow::onSetAppFixedOrientation(orientation, isPortrait);
}


void EmulatedCardWindow::positiveSpaceAboutToChange(const QRect& r, bool fullScreen)
{
	CardWindow::positiveSpaceAboutToChange(r, fullScreen);
	m_positiveSpaceChanging = true;
}

void EmulatedCardWindow::positiveSpaceChanged(const QRect& r)
{
	CardWindow::positiveSpaceChanged(r);
	m_positiveSpace = r.height();
	int positiveSpaceMax = SystemUiController::instance()->currentUiHeight() - Settings::LunaSettings()->positiveSpaceTopPadding;

	//Don't accept positive space changes that indicate there's more
	//positive space than the screen

	if (m_positiveSpace > positiveSpaceMax) {
		m_positiveSpace = positiveSpaceMax;
	}

	calculatePositiveSpaceAdjustment();
}

void EmulatedCardWindow::positiveSpaceChangeFinished(const QRect& r)
{
	CardWindow::positiveSpaceChangeFinished(r);
	m_positiveSpace = r.height();
	int positiveSpaceMax = SystemUiController::instance()->currentUiHeight() - Settings::LunaSettings()->positiveSpaceTopPadding;

	//Don't accept positive space changes that indicate there's more
	//positive space than the screen

	if (m_positiveSpace > positiveSpaceMax) {
		m_positiveSpace = positiveSpaceMax;
	}

	calculatePositiveSpaceAdjustment();
	m_positiveSpaceChanging = false;
}

void EmulatedCardWindow::calculatePositiveSpaceAdjustment() {
	m_positiveSpaceAdjustment = m_screenMidLine - (m_positiveSpace/2);
	layoutWidgets();
}

void EmulatedCardWindow::onSetAppFreeOrientation(bool freeOrientation) {
	m_appFreeOrientation = freeOrientation;
	refreshAdjustmentAngle();
	layoutWidgets();
}

bool EmulatedCardWindow::isAppFreeOrientation() {
	return m_appFreeOrientation;
}

bool EmulatedCardWindow::touchEvent(QTouchEvent* event) {
	if (!m_positiveSpaceChanging) {
		return CardWindow::touchEvent(event);
	} else {
		return true;
	}
}

OrientationEvent::Orientation EmulatedCardWindow::systemOrientationFor(int orient) {

    OrientationEvent::Orientation orientations[4] = {OrientationEvent::Orientation_Up,
            OrientationEvent::Orientation_Right, OrientationEvent::Orientation_Down,
            OrientationEvent::Orientation_Left};
	int emuModeOrientationAngle = Settings::LunaSettings()->emuModeOrientationAngle;

	if (emuModeOrientationAngle == -90) {
		emuModeOrientationAngle = 270;
	}

	//Convert this into an offset of the array
	if (emuModeOrientationAngle == 0) {
		emuModeOrientationAngle = 0;
	} else if (emuModeOrientationAngle == 90) {
		emuModeOrientationAngle = 1;
	} else if (emuModeOrientationAngle == 180) {
		emuModeOrientationAngle = 2;
	} else if (emuModeOrientationAngle == 270) {
		emuModeOrientationAngle = 3;
	}

	//Convert input into offset of array
	int inputOrient = 0;
    if (orient == OrientationEvent::Orientation_Up) {
		inputOrient = 0;
    } else if (orient == OrientationEvent::Orientation_Right) {
		inputOrient = 1;
    } else if (orient == OrientationEvent::Orientation_Down) {
		inputOrient = 2;
    } else if (orient == OrientationEvent::Orientation_Left) {
		inputOrient = 3;
	}

	inputOrient = (inputOrient + emuModeOrientationAngle) % 4;

	return orientations[inputOrient];

}

bool EmulatedCardWindow::sceneEvent(QEvent* event)
{
	if (isMaximized()) {
		if (event->type() == QEvent::GestureOverride) {
				event->accept();
		}
		else if (event->type() == QEvent::Gesture) {

			QGestureEvent* ge = static_cast<QGestureEvent*>(event);
			QGesture* g = ge->gesture(Qt::TapGesture);
			if (g && g->state() == Qt::GestureFinished) {
				QTapGesture* gt = static_cast<QTapGesture*>(g);
				QPoint touchPoint = mapFromScene(gt->position()).toPoint();
				QRectF hitTarget = mapRectFromItem(m_keyboardButton, m_keyboardButton->boundingRect());
				if (hitTarget.contains(touchPoint)) {
					//The tap coordinates landed on the keyboard button, so let them through
					//without adjusting them
					m_keyboardButton->simulateClick();
					return true;
				}
			}
		}
	}
	return CardWindow::sceneEvent(event);
}


void EmulatedCardWindow::slotKeyboardUpClicked() {
	PalmIME::EditorState state; //just using its default constructor
	onEditorFocusChanged(true, state);
	m_keyboardUpManually = true;
}

void EmulatedCardWindow::onEditorFocusChanged(bool focused, const PalmIME::EditorState& state)
{
	g_debug("IME: Window got focus change in sysmgr %s focused: %d, fieldtype: %d, fieldactions: 0x%02x",
			 appId().c_str(), focused, state.type, state.actions);
	// cache input focus state for this window
	if (m_keyboardUpManually) {
		setInputFocus(true);
	} else {
		setInputFocus(focused);
	}
	setInputState(state);

	IMEController::instance()->notifyInputFocusChange(this, (m_keyboardUpManually ? m_keyboardUpManually : focused));
}

void EmulatedCardWindow::removeInputFocus()
{
	if (m_keyboardUpManually) {
		m_keyboardUpManually = false;
	}

	HostWindow::removeInputFocus();
}

#if !defined(TARGET_EMULATOR)
void EmulatedCardWindow::onUpdateWindowRegion(int x, int y, int w, int h)
{
	onUpdateFullWindow();
}

void EmulatedCardWindow::onUpdateFullWindow()
{
	if (scene())
		scene()->update();
}
#endif
