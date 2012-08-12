/**
 * @file
 * 
 * Base class for different types of app containers to derive from.
 *
 * @author Hewlett-Packard Development Company, L.P.
 * @author tyrok1
 *
 * @section LICENSE
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
 */




#ifndef WEBAPPBASE_H
#define WEBAPPBASE_H

#include "Common.h"

#include <glib.h>
#include <list>
#include <lunaservice.h>
#include <QMap>
#include <QPointer>

#include "ProcessBase.h"
#include "WebPageClient.h"
#include "WebAppManager.h"

#if defined (TARGET_DEVICE)
    #include "hal/HALSensorConnector.h"
#endif

class WebPage;
class ApplicationDescription;
namespace Palm {
	class WebGLES2Context;
}

/**
 * Base class for different types of app containers to derive from.
 */
class WebAppBase : public WebPageClient
        #if defined (TARGET_DEVICE)
            ,public HALConnectorObserver
        #endif
{
public:
    // Don't change these enums. They are published API
    typedef enum {
        WebKitShake_Invalid =-1,
        WebKitShake_Start   = 0,
        WebKitShake_Shaking = 1,
        WebKitShake_End     = 2
    }WebKitShakeEvent;

    typedef enum {
        WebKitOrientation_Invalid   =-1,
        WebKitOrientation_FaceUp    = 0,
        WebKitOrientation_FaceDown  = 1,
        WebKitOrientation_Up        = 2,
        WebKitOrientation_Down      = 3,
        WebKitOrientation_Left      = 4,
        WebKitOrientation_Right     = 5
    }WebKitOrientationEvent;

	/**
	 * Initializes this app container
	 * 
	 * Just initializes things to blank values.  DOES
	 * NOT report to {@link WebAppManager WebAppManager}
	 * that we've launched.
	 */
	WebAppBase();
	
	/**
	 * Cleans up this app container
	 * 
	 * This cleans up any resources allocated within
	 * this app container.  It does not detach from or
	 * clean up the WebPage it's attached to - that is
	 * expected to be handled by the caller.
	 * 
	 * Implementation details:
	 * 
	 * - Deletes our app from the global app cache
	 *   ({@link WebAppCache WebAppCache}).
	 * - Lets {@link WebAppManager WebAppManager} know
	 *   that we're being deleted.
	 * - Cleans up resources, including deleting the
	 *   content WebPage from memory.
	 * - Disconnects from sensors.
	 * - Asks {@link WebAppManager WebAppManager} to
	 *   remove us from the headless watch list.
	 * 
	 * @note This deletes the attached WebPage from memory.  After deleting this WebAppBase, do not attempt to access any WebPage attached to this instance when it was destroyed.
	 */
	virtual ~WebAppBase();
	
	/**
	 * Attaches to a WebPage instance to allow us to manage it
	 * 
	 * This method sets up the content to display for
	 * this app.
	 * 
	 * In detail, this method:
	 * - Reports to {@link WebAppManager WebAppManager}
	 *   that this app is launched.
	 * - Creates a new activity.
	 * - Loads and applies app properties from the app
	 *   description.
	 * 
	 * It's assumed that this WebPage instance will
	 * either already have a URL loaded or it will be
	 * loaded by the caller after calling this.  In
	 * other words, this attaches a WebPage, but does
	 * not load anything into it.
	 * 
	 * @note If still attached when this WebAppBase instance is destroyed, this page will also be deleted from memory.  Do not access this page again after attaching it without getting a new pointer to it via {@link WebAppBase::page() WebAppBase::page()}.
	 * 
	 * @param	page			WebPage of content for this app.
	 */
	virtual void attach(WebPage* page);
	
	/**
	 * Detaches this instance from the WebPage it was previously managing
	 * 
	 * This essentially removes the content from this
	 * app.
	 * 
	 * In detail, this method:
	 * - Reports to {@link WebAppManager WebAppManager}
	 *   that this app is closed.
	 * - Destroys the activity for the app.
	 * 
	 * @return				The previously-managed WebPage instance containing this app's content.
	 */
	virtual WebPage* detach();

	virtual void thawFromCache() {}
	virtual void freezeInCache() {}
	void markInCache(bool val);

	void setKeepAlive(bool val);
	bool keepAlive() const { return m_keepAlive; }
	
	WebPage* page() const { return m_page; }

	virtual bool isWindowed() const { return false; }
	virtual bool isCardApp() const { return false; }
	virtual bool isChildApp() const { return false; }
	virtual bool isDashboardApp() const { return false; }
	virtual bool isAlertApp() const { return false; }

	void relaunch(const char* args, const char* launchingAppId, const char* launchingProcId);
    virtual void stagePreparing();
    virtual void stageReady();
	
	/**
	 * Gets this app's app ID
	 * 
	 * This is set up when this instance is attached
	 * to a page and gets the app ID of the page it's
	 * attached to.
	 * 
	 * @return				App ID of this app.
	 */
	std::string appId() const { return m_appId; }
	
	std::string processId() const { return m_processId; }
	std::string url() const { return m_url; }
	ApplicationDescription* getAppDescription();
	void setAppDescription(ApplicationDescription* appDesc);

	virtual Palm::WebGLES2Context* getGLES2Context() { return 0; }

	virtual void setExplicitEditorFocus(bool focused, const PalmIME::EditorState & editorState);

    void setManualEditorFocusEnabled(bool enable);
    virtual void setManualEditorFocus(bool focused, const PalmIME::EditorState & editorState);

	virtual void suspendAppRendering() {}
	virtual void resumeAppRendering() {}

    /**
     * Function initializes(opens)/De-initializes the appropriate sensor
     *
     * @param[in] aSensorType   - Sensor Type
     * @param[in] aNeedEvents   - flag indicating whether to start or stop the sensor
     *
     * @return true if successful, false otherwise
     */
    virtual bool enableSensor(Palm::SensorType aSensorType, bool aNeedEvents);

    /**
      * Function sets the Accelerometer rate to highest level.
      * Currently the there is no means of setting fastAccelerometer to OFF.
      * So, the enable argument is not utilized at this point of time.
      *
      * @param[in] enable   - Whether to enable/disable
      */
    virtual void fastAccelerometerOn(bool enable);
protected:

	// WebPage Client
	
	//Documented in parent
	virtual int  getKey() const { return 0; }
	
	//Documented in parent
	virtual void focus() {}
	
	//Documented in parent
	virtual void unfocus() {}
	
	//Documented in parent
	virtual void close();
	
	virtual void windowSize(int& width, int& height) { width = 0; height = 0; }
	virtual void screenSize(int& width, int& height) {	width = WebAppManager::instance()->currentUiWidth();
														height = WebAppManager::instance()->currentUiHeight();}

	virtual void resizedContents(int contentsWidth, int contentsHeight);
	virtual void zoomedContents(double scaleFactor, int contentsWidth, int contentsHeight,
								int newScrollOffsetX, int newScrollOffsetY);
	virtual void invalContents(int x, int y, int width, int height);
	virtual void scrolledContents(int newContentsX, int newContentsY);
	virtual void uriChanged(const char* url);
	virtual void titleChanged(const char* title);
	virtual void statusMessage(const char* msg);
	
	//Documented in parent
	virtual void dispatchFailedLoad(const char* domain, int errorCode,
			const char* failingURL, const char* localizedDescription);
	virtual void loadFinished() {}
//	virtual void enableCompass(bool enable);
	
	//Documented in parent
	virtual void editorFocusChanged(bool focused, const PalmIME::EditorState& state) {}
	
	/**
	 * Enables/disables auto-capitalization in fields in this app
	 * 
	 * @note Currently doesn't do anything - see derived class to see if they actually did anything with this.
	 * 
	 * @param	enabled			true to enable auto-capitalization, false to disable it.
	 */
	virtual void autoCapEnabled(bool enabled) {}
	virtual void needTouchEvents(bool needTouchEvents) {}

protected:
	/**
	 * Asks the Activity Manager service to create an activity for us
	 * 
	 * @todo Document this further once the palm://com.palm.activitymanager/create IPC call is fully documented.
	 */
	void createActivity();
	
	/**
	 * Destroys our activity
	 * 
	 * @todo Fully document this once LSCallCancel() is documented.
	 */
	void destroyActivity();
	void focusActivity();
	
	/**
	 * Asks the Activity Manager service to unfocus our app
	 * 
	 * @todo Document this further once the palm://com.palm.activitymanager/unfocus IPC call is fully documented.
	 */
	void blurActivity();
	
	/**
	 * Cleans up this app
	 * 
	 * Implementation details:
	 * - Lets {@link WebAppManager WebAppManager}
	 *   know that the app is closed.
	 * - Destroys the activity that was set up in
	 *   {@link WebAppBase::attach() WebAppBase::attach()}.
	 * - Deletes the page.
	 * - Deletes the desc image.
	 * 
	 * @todo Document this more fully once we know what a "desc image" is.
	 */
	void cleanResources();
	
	/**
	 * Cleans up all open sensor connections
	 * 
	 * Goes through our list of sensors enabled using
	 * {@link WebAppBase::enableSensor() WebAppBase::enableSensor()}
	 * and disables them.
	 * 
	 * (Was documented in original as:
	 * Destroys all the sensors)
	 */
	void destroyAllSensors();

    /**
      * Function Maps the Screen orientation to Current Window Orientation
      */
    virtual Event::Orientation orientationForThisCard(Event::Orientation orient)
    {
        return orient;
    }

    /**
      * Function allows derived class members to do extra post processing before the orientation
      * event is given to the WebApp
      */
    virtual Event::Orientation postProcessOrientationEvent(Event::Orientation aInputEvent)
    {
        return aInputEvent;
    }

protected:

	WebPage* m_page;

	bool m_inCache;
	bool m_keepAlive;
	std::string m_appId;
	std::string m_processId;
	std::string m_url;
	ApplicationDescription *appDescImage;
	LSMessageToken m_activityManagerToken;
#if defined (TARGET_DEVICE)
    int m_OrientationAngle;
#endif
private:

    WebAppBase& operator=(const WebAppBase&);
    WebAppBase(const WebAppBase&);

#if defined (TARGET_DEVICE)
    /**
     * Function gets called whenever there is some data
     * available from HAL.
     *
     * @param[in]   - aSensorType - Sensor which has got some data to report
     */
    virtual void HALDataAvailable (HALConnectorBase::Sensor aSensorType);

    /**
     * Function initializes(opens) the appropriate sensor
     *
     * @param[in] aSensorType   - Sensor Type
     *
     * @return true if successful, false otherwise
     */
    bool enableSensor(HALConnectorBase::Sensor aSensorType);

    /**
     * Function disables(closes) the appropriate sensor
     *
     * @param[in] aSensorType   - Sensor Type
     *
     * @return true if successful, false otherwise
     */
    bool disableSensor(HALConnectorBase::Sensor aSensorType);

    bool sendCompassEvent();
    bool sendAccelerationEvent();
    bool sendShakeEvent();
    bool sendLogicalOrientationEvent();

    WebKitShakeEvent mapShakeEvent(ShakeEvent::Shake aShakeState);
    WebKitOrientationEvent mapOrientationEvent(OrientationEvent::Orientation aOrientation);
    HALConnectorBase::Sensor mapSensorType(Palm::SensorType aSensorType);

    QMap<HALConnectorBase::Sensor, QPointer<HALConnectorBase> > m_SensorList;
#endif
};


#endif /* WEBAPPBASE_H */
