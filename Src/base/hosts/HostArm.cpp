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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>          
#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/input.h> 
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <dlfcn.h>
#include <glib.h>
#include <list>
#include <algorithm>
#include <map>

#include <QWidget>
#include <QApplication>
#include <qsocketnotifier.h>

#include "CustomEvents.h"
#include "Time.h"
#include "HostBase.h"
#include "Event.h"
#include "MutexLocker.h"
#include "WindowServer.h"
#include "Settings.h"
#include "HostArm.h"
#include "Logging.h"
#include "SystemUiController.h"
#include "hal/HalInputControl.h"

#include "HidLib.h"

#define HOSTARM_LOG	"HostArm"

#if !defined(FBIO_WAITFORVSYNC)
#define FBIO_WAITFORVSYNC	_IOW('F', 0x20, u_int32_t)
#endif

// TODO: these should come from hidd headers
#define MAX_HIDD_EVENTS 100 

static int s_currentOrientation = ORIENTATION_UNKNOWN;

#ifdef HAVE_QPA
//TODO: Move me to a header!
extern "C" void setTransform(QTransform*);
extern "C" InputControl* getTouchpanel(void);
extern "C" void setBluetoothCallback(void (*fun)(bool));
#endif

#ifdef HAVE_QPA
static void bluetoothCallback(bool enable)
{
    HostBase::instance()->setBluetoothKeyboardActive(enable);
}
#endif

HostArm::HostArm() :
      m_halLightNotifier(NULL)
	, m_halProxNotifier(NULL)
	, m_hwRev(HidHardwareRevisionEVT1)
	, m_hwPlatform (HidHardwarePlatformCastle)
	, m_fb0Fd(-1)
	, m_fb1Fd(-1)
	, m_fb0Buffer(0)
	, m_fb0NumBuffers(0)
	, m_fb1Buffer(0)
	, m_fb1NumBuffers(1)
	, m_service(NULL)
	, m_halInputControlALS(0)
	, m_halInputControlBluetoothInputDetect(0)
	, m_halInputControlProx(0)
	, m_halInputControlKeys(0)
	, m_halInputControlTouchpanel(0)
    , m_bluetoothKeyboardActive(false)
    , m_OrientationSensor(0)
{
	m_hwRev = HidGetHardwareRevision();
	m_hwPlatform = HidGetHardwarePlatform();
#ifdef HAVE_QPA
	setBluetoothCallback(&bluetoothCallback);
#endif
}

HostArm::~HostArm()
{
	stopService();
	shutdownInput();
	hal_deinit();
}

void HostArm::init(int w, int h)
{
	// turn off turbo mode if in case sysmgr died with it being previously enabled
	turboMode(false);
	
	// Need to wake up the display before opening the fb
	wakeUpLcd();

	// Get the display info ---------------------------------------------
	
	m_fb0Fd = open("/dev/fb0", O_RDWR);
	if (m_fb0Fd < 0) {
		fprintf(stderr, "Failed to open framebuffer device\n");
        return;
    }

	(void) ::ioctl(m_fb0Fd, FBIOBLANK, FB_BLANK_UNBLANK);
	
    struct fb_var_screeninfo varinfo;
	memset(&varinfo, 0, sizeof(varinfo));

    ioctl(m_fb0Fd, FBIOGET_VSCREENINFO, &varinfo);	
	
	m_info.displayWidth = varinfo.xres;
	m_info.displayHeight = varinfo.yres;
	m_info.displayRowBytes = varinfo.xres * varinfo.bits_per_pixel / 8;
	m_info.displayDepth = varinfo.bits_per_pixel;
	m_info.displayRedLength = varinfo.red.length;
	m_info.displayGreenLength = varinfo.green.length;
	m_info.displayBlueLength = varinfo.blue.length;
	m_info.displayAlphaLength = varinfo.transp.length;

	struct fb_fix_screeninfo fixinfo;
	int ret;

	memset(&fixinfo, 0, sizeof(fb_fix_screeninfo));
	ret = ::ioctl(m_fb0Fd, FBIOGET_FSCREENINFO, &fixinfo);

	if (ret < 0) {
		g_warning("Failed to get fscreeninfo: %s", strerror(errno));
		return;
	}

	m_fb0Buffer = ::mmap(0, fixinfo.smem_len, PROT_READ, MAP_SHARED, m_fb0Fd, 0);
	if (m_fb0Buffer == MAP_FAILED) {
		g_warning("Failed to map fb1 buffer: %s", strerror(errno));
		m_fb0Buffer = 0;
		return;
	}

	int bpp = varinfo.bits_per_pixel;
	int rowBytes = varinfo.xres * (bpp >> 3);
	m_fb0NumBuffers = fixinfo.smem_len / (rowBytes * varinfo.yres);
	
	// Setup the fb1 display.
	
	m_fb1Fd = open("/dev/fb1", O_RDWR, 0);
	if (m_fb1Fd < 0) {
		g_warning("Failed to open layer 1: %s", strerror(errno));
		return;
	}

#if defined(FB1_POWER_OPTIMIZATION)
	// Turn off fb1 if it was previously enabled
	(void) ::ioctl(m_fb1Fd, FBIOBLANK, FB_BLANK_POWERDOWN);
#else
	(void) ::ioctl(m_fb1Fd, FBIOBLANK, FB_BLANK_UNBLANK);
#endif	
	
	ret = ::ioctl(m_fb1Fd, FBIOGET_FSCREENINFO, &fixinfo);

	if (ret < 0) {
		g_warning("Failed to get fscreeninfo: %s", strerror(errno));
		return;
	}

	m_fb1Buffer = ::mmap(0, fixinfo.smem_len, PROT_READ, MAP_SHARED, m_fb1Fd, 0);
	if (m_fb1Buffer == MAP_FAILED) {
		g_warning("Failed to map fb1 buffer: %s", strerror(errno));
		m_fb1Buffer = 0;
		return;
	}

	memset(&varinfo, 0, sizeof(varinfo));
	ioctl(m_fb1Fd, FBIOGET_VSCREENINFO, &varinfo);	

	bpp = varinfo.bits_per_pixel;
	rowBytes = varinfo.xres * (bpp >> 3);
	m_fb1NumBuffers = fixinfo.smem_len / (rowBytes * varinfo.yres);

	printf("Linux Fb0: Num Buffers: %d, Fb1: Num Buffers: %d\n", m_fb0NumBuffers, m_fb1NumBuffers);
#ifdef HAVE_QPA
	setTransform(&m_trans);
#endif
}

void HostArm::disableScreenBlanking()
{
    int err;
    
	int fd = open ("/dev/tty0", O_RDWR);
    if (fd == -1) {
		fprintf(stderr, "failed to open /dev/tty0");
		return;
    }

	// This appears to be the magic, secret voodoo to keep the screen
	// from blanking while parts is running...

	err = ioctl(fd, TIOCLINUX, "\x4") ||    // Restore screen from sleep
          ioctl(fd, TIOCLINUX, "\xa\0");	// Disable screen sleep
    if (err != 0) {
        fprintf(stderr, "faild to disable screen blanking\n");
		return;
    }   

    err = ioctl(fd, KDSETMODE, KD_GRAPHICS);	// Get rid of the cursor
    if (err != 0) {
        fprintf(stderr, "faild to disable cursor\n");
		return;
    }   

    return;
}

#if 0
static bool
nopServiceResponse(LSHandle * /*sh*/, LSMessage * /*reply*/, void * /*ctx*/ )
{
    return true;
}
#endif

void HostArm::setupInput(void) {
    hal_init();

    hal_error_t error = HAL_ERROR_SUCCESS;

    InputControl *ic = getInputControlALS();
    if (NULL != ic)
    {
        hal_device_handle_t alsHandle = ic->getHandle();
        if (alsHandle)
        {
            int light_source_fd = 0;
            error = hal_device_get_event_source(alsHandle, &light_source_fd);

            if (error != HAL_ERROR_SUCCESS)
            {
                g_critical("Unable to obtain alsHandle event_source");
                return;
            }
            m_halLightNotifier = new QSocketNotifier(light_source_fd, QSocketNotifier::Read, this);
            connect(m_halLightNotifier, SIGNAL(activated(int)), this, SLOT(readALSData()));
        }
    }

    ic = getInputControlProximity();
    if (NULL != ic)
    {
        hal_device_handle_t proxHandle = ic->getHandle();
        if (proxHandle)
        {
            int proximity_source_fd = 0;
            error = hal_device_get_event_source(proxHandle, &proximity_source_fd);
            if (error != HAL_ERROR_SUCCESS)
            {
                g_critical("Unable to obtain proxHandle event_source");
                return;
            }
            m_halProxNotifier = new QSocketNotifier(proximity_source_fd, QSocketNotifier::Read, this);
            connect(m_halProxNotifier, SIGNAL(activated(int)), this, SLOT(readProxData()));
        }
    }
}

void HostArm::readALSData() {

    hal_error_t error = HAL_ERROR_SUCCESS;
    hal_event_handle_t event_handle = NULL;

    if (m_halInputControlALS == NULL) return;

    hal_device_handle_t m_halHandle = m_halInputControlALS->getHandle();
    if (m_halHandle == NULL) return;

    error = hal_device_get_event(m_halHandle, &event_handle);
    if (error != HAL_ERROR_SUCCESS)
    {
		g_critical("Unable to obtain m_halHandle event");
		return;
    }

    while ((error == HAL_ERROR_SUCCESS) && (event_handle != NULL))
    {
	    static int lightVal = -1;
        error = hal_sensor_als_event_get_intensity(event_handle, &lightVal);
        if (error != HAL_ERROR_SUCCESS)
        {
		    g_critical("Unable to obtain m_halLightHandle event intensity");
		    return;
        }

        QApplication::postEvent(QApplication::activeWindow(), new AlsEvent(lightVal));

        error = hal_device_release_event(m_halHandle, event_handle);
        if (error != HAL_ERROR_SUCCESS)
        {
		    g_critical("Unable to release m_halLightHandle event");
		    return;
        }

        event_handle = NULL;
        error = hal_device_get_event(m_halHandle, &event_handle);
    }

}

void HostArm::readProxData() {
    hal_error_t error = HAL_ERROR_SUCCESS;
    hal_event_handle_t event_handle = NULL;

    if (m_halInputControlProx == NULL) return;

    hal_device_handle_t m_halHandle = m_halInputControlProx->getHandle();
    if (m_halHandle == NULL) return;

    error = hal_device_get_event(m_halHandle, &event_handle);
    if (error != HAL_ERROR_SUCCESS)
    {
		g_critical("Unable to obtain m_halHandle event");
		return;
    }

    while ((error == HAL_ERROR_SUCCESS) && (event_handle != NULL))
    {
	    int presence = 0;
        error = hal_sensor_proximity_event_get_presence(event_handle, &presence);
        if (error != HAL_ERROR_SUCCESS)
        {
		    g_critical("Unable to get hal proximity event");
		    return;
        }


        luna_log(HOSTARM_LOG, "Proximity sensor event: value: %d", presence);
        QApplication::postEvent(QApplication::activeWindow(), new ProximityEvent(presence));

        error = hal_device_release_event(m_halHandle, event_handle);
        if (error != HAL_ERROR_SUCCESS)
        {
		    g_critical("Unable to release m_halHandle event");
		    return;
        }

        event_handle = NULL;

        error = hal_device_get_event(m_halHandle, &event_handle);
    }
}

// returns -1 on failure
// returns the number of events read on success (may be 0)
//
// bufSize - size of eventBuf in bytes
int HostArm::readHidEvents(int fd, struct input_event* eventBuf, int bufSize)
{
	struct sockaddr_un sender;
	int fromLen = sizeof(sender);
	int numBytes = recvfrom(fd, eventBuf, bufSize, O_NONBLOCK,
				(struct sockaddr*)&sender, (socklen_t*)&fromLen);

	if (numBytes < 0) {
		if (errno != EAGAIN) {
			luna_critical(HOSTARM_LOG, "Error reading input events: %s",
						strerror(errno));
			return -1;
		}
		luna_critical(HOSTARM_LOG, "An input event should have been" \
				           " available, but the non-blocking call" \
					   " returned -EAGAIN: %s", strerror(errno));
		return 0;	// the recvfrom would have blocked, so we read 0 events
	}

	return (numBytes / sizeof(struct input_event));
}

void HostArm::shutdownInput(void)
{
    delete m_OrientationSensor;
    m_OrientationSensor = 0;

	if (m_halInputControlALS)
	{
		delete m_halInputControlALS;
		m_halInputControlALS = NULL;
	}

	if (m_halInputControlProx)
	{
		delete m_halInputControlProx;
		m_halInputControlProx = NULL;
	}

	if (m_halInputControlTouchpanel)
	{
		delete m_halInputControlTouchpanel;
		m_halInputControlTouchpanel = NULL;
	}

	if (m_halInputControlKeys)
	{
        delete m_halInputControlKeys;
		m_halInputControlKeys = NULL;
	}

	delete m_halProxNotifier;
	m_halProxNotifier = NULL;

	delete m_halLightNotifier;
	m_halLightNotifier = NULL;
}

void HostArm::startService()
{
	LSError lserror;
	LSErrorInit(&lserror);
	bool retVal = false;

	GMainLoop* mainLoop = HostBase::instance()->mainLoop();

	retVal = LSRegister(NULL, &m_service, &lserror);
	if (!retVal) goto Error;

	retVal = LSGmainAttach(m_service, mainLoop, &lserror);
	if (!retVal) goto Error;

	return;

Error:
	if (LSErrorIsSet(&lserror)) {
		LSErrorPrint(&lserror, stderr);
		LSErrorFree(&lserror);
	}

	g_debug(":%s: Unable to start service", __PRETTY_FUNCTION__);
}

void HostArm::stopService()
{
	LSError lserror;
	LSErrorInit(&lserror);
	bool result;

	result = LSUnregister(m_service, &lserror);
	if (!result) {
		LSErrorPrint(&lserror, stderr);
		LSErrorFree(&lserror);
	}
}

const char* HostArm::hardwareName() const
{
    return "ARM Device";
}

void HostArm::setCentralWidget(QWidget* view)
{
	view->setWindowFlags(Qt::CustomizeWindowHint |
						 Qt::WindowStaysOnTopHint);
	QApplication::setActiveWindow(view);
	view->show();    
}

void HostArm::show()
{
    disableScreenBlanking();
    startService();
    setupInput();
	getInitialSwitchStates();
}

int HostArm::getNumberOfSwitches() const
{
	return 3;
}

bool HostArm::getMsgValueInt(LSMessage* msg, int& value)
{
	const char* str = LSMessageGetPayload(msg);
	if (!str)
		return false;

	json_object* json = json_tokener_parse(str);
	if (!json || is_error(json))
		return false;

	const char* valueStr = json_object_get_string(json_object_object_get(json, "value"));
	if (!valueStr) {
		json_object_put(json);
		return false;
	}

	value = atoi(valueStr);
	json_object_put(json);
	return true;
}

bool HostArm::switchStateCallback(LSHandle* handle, LSMessage* msg, void* data)
{
	int switchCode = (int)data;
	int value = -1;
	
	if (!HostArm::getMsgValueInt(msg, value))
		return true;

	Qt::Key switchKey = Qt::Key_unknown;
	switch (switchCode) {
	case SW_RINGER:
		switchKey = Qt::Key_Ringer;
		break;
	case SW_SLIDER:
		switchKey = Qt::Key_Slider;
		break;
	case SW_HEADPHONE_INSERT:
		if (value == HID_HEADSET_MIC_VAL) {
			switchKey = Qt::Key_HeadsetMic;
		}
		else {
			switchKey = Qt::Key_Headset;
		}
		break;
	default: 
		return true;
	}

	// SysMgrNativeKeyboardModifier_InitialState is used to signify key events which are sent as initial state events
	QApplication::postEvent(QApplication::activeWindow(),
							QKeyEvent::createExtendedKeyEvent(value == 0 ?  QEvent::KeyRelease : QEvent::KeyPress,
															  switchKey, 0, 0, 0,
															  SysMgrNativeKeyboardModifier_InitialState));
	return true;
}

void HostArm::getInitialSwitchStates()
{
	LSError err;
	LSErrorInit(&err);

	if (!LSCall(m_service, HIDD_RINGER_URI, HIDD_GET_STATE, HostArm::switchStateCallback, (void*)SW_RINGER, NULL, &err))
		goto Error;

	if (!LSCall(m_service, HIDD_SLIDER_URI, HIDD_GET_STATE, HostArm::switchStateCallback, (void*)SW_SLIDER, NULL, &err))
		goto Error;

	if (!LSCall(m_service, HIDD_HEADSET_URI, HIDD_GET_STATE, HostArm::switchStateCallback, (void*)SW_HEADPHONE_INSERT, NULL, &err))
		goto Error;

	return;

Error:

	if (LSErrorIsSet(&err)) {
		LSErrorPrint(&err, stderr);
		LSErrorFree(&err);
	}
}

void HostArm::flip()
{
	bool needFlip = true;

#if defined(HAVE_OPENGL)
	if (!Settings::LunaSettings()->forceSoftwareRendering)
		needFlip = false;
#endif

	if (!needFlip)
		return;

	// force a buffer "flip" in software
	if (m_fb0Fd < 0)
		return;

	struct fb_var_screeninfo screeninfo;
	int rc = ::ioctl(m_fb0Fd, FBIOGET_VSCREENINFO, &screeninfo);
	Q_ASSERT_X(rc >= 0, __PRETTY_FUNCTION__, "ioctl(FBIOGET_VSCREENINFO) failed");

	if (rc < 0)
		return;

	screeninfo.xoffset = 0;
	screeninfo.yoffset = 0;

	rc = ioctl(m_fb0Fd, FBIOPAN_DISPLAY, &screeninfo);
	Q_ASSERT_X(rc >= 0, __PRETTY_FUNCTION__, "ioctl(FBIOPAN_DISPLAY) failed");
}

QImage HostArm::takeScreenShot() const
{
	if (m_fb0Fd < 0 || m_fb0Buffer == 0)
		return QImage();

	int yoffset = 0;

	struct fb_var_screeninfo screeninfo;
	int rc = ::ioctl(m_fb0Fd, FBIOGET_VSCREENINFO, &screeninfo);
	if (rc >= 0)
		yoffset = screeninfo.yoffset;

	yoffset = qMax(yoffset, 0);
	yoffset = qMin(yoffset, m_info.displayHeight * (m_fb0NumBuffers - 1));

	QImage image((const unsigned char*) m_fb0Buffer +
				 m_info.displayWidth * yoffset * 4,
				 m_info.displayWidth, m_info.displayHeight,
				 QImage::Format_ARGB32_Premultiplied);

	return image;
}

QImage HostArm::takeAppDirectRenderingScreenShot() const
{
	if (m_fb1Fd < 0 || m_fb1Buffer == 0)
		return QImage();

	int yoffset = 0;

	struct fb_var_screeninfo screeninfo;
	int rc = ::ioctl(m_fb1Fd, FBIOGET_VSCREENINFO, &screeninfo);
	if (rc >= 0)
		yoffset = screeninfo.yoffset;

	yoffset = qMax(yoffset, 0);
	yoffset = qMin(yoffset, m_info.displayHeight * (m_fb1NumBuffers - 1));

	QImage image((const unsigned char*) m_fb1Buffer +
				 m_info.displayWidth * yoffset * 4,
				 m_info.displayWidth, m_info.displayHeight,
				 QImage::Format_ARGB32_Premultiplied);
	
	return image;    
}

void HostArm::setAppDirectRenderingLayerEnabled(bool enable)
{
#if !defined(FB1_POWER_OPTIMIZATION)
	return;
#endif	
	
	if (m_fb1Fd < 0)
		return;

	g_message("%s fb1", enable ? "Unblanking" : "Blanking");
	
	(void) ::ioctl(m_fb1Fd, FBIOBLANK, enable ? FB_BLANK_UNBLANK : FB_BLANK_POWERDOWN);
}

void HostArm::setRenderingLayerEnabled(bool enable)
{
#if !defined(FB1_POWER_OPTIMIZATION)
	return;
#endif	

	if (m_fb0Fd < 0)
		return;

	g_message("%s fb0", enable ? "Unblanking" : "Blanking");

	(void) ::ioctl(m_fb0Fd, FBIOBLANK, enable ? FB_BLANK_UNBLANK : FB_BLANK_POWERDOWN);	
}

void HostArm::wakeUpLcd()
{
	(void) ::system("echo 1 > /sys/class/display/lcd.0/state");
}

void HostArm::OrientationSensorOn(bool enable)
{
    if (0 == m_OrientationSensor)
    {
        m_OrientationSensor = static_cast<HALOrientationSensorConnector *>(HALConnectorBase::getSensor(HALConnectorBase::SensorOrientation, this));
    }

    if (m_OrientationSensor)
    {
        if (enable)
        {
            m_OrientationSensor->on();
        }
        else
        {
            m_OrientationSensor->off();
        }
    }
}

void HostArm::HALDataAvailable (HALConnectorBase::Sensor aSensorType)
{
    if (HALConnectorBase::SensorOrientation == aSensorType)
    {
        OrientationEvent *e = static_cast<OrientationEvent *>(m_OrientationSensor->getQSensorData());

        // Call device specific code for any post processing, if required
        OrientationEvent *postProcessedEvent = postProcessDeviceOrientation(e);

        // post the event to the world
        // Remember QApplication::postEvent function deletes the event passed to it
        QApplication::postEvent(QApplication::activeWindow(), postProcessedEvent);

        // delete the unUsed event.
        delete e;
    }
}

InputControl* HostArm::getInputControlALS()
{
    if (m_halInputControlALS)
        return m_halInputControlALS;

    m_halInputControlALS = new HalInputControl(HAL_DEVICE_SENSOR_ALS, "Default");
    if (NULL == m_halInputControlALS)
        g_critical("Unable to obtain m_halInputControlALS");

    return m_halInputControlALS;
}

InputControl* HostArm::getInputControlProximity()
{
    if (m_halInputControlProx)
        return m_halInputControlProx;

    m_halInputControlProx = new HalInputControl(HAL_DEVICE_SENSOR_PROXIMITY, "Screen");
    if (NULL == m_halInputControlProx)
        g_critical("Unable to obtain m_halInputControlProx");

    return m_halInputControlProx;
}

InputControl* HostArm::getInputControlTouchpanel()
{
    if (m_halInputControlTouchpanel)
        return m_halInputControlTouchpanel;
    
#ifndef HAVE_QPA
    m_halInputControlTouchpanel = new HalInputControl(HAL_DEVICE_TOUCHPANEL, "Main");
#else
    m_halInputControlTouchpanel = getTouchpanel();
#endif
    if (NULL == m_halInputControlTouchpanel)
        g_critical("Unable to obtain m_halInputControlTouchpanel");

    
    return m_halInputControlTouchpanel;
}

InputControl* HostArm::getInputControlKeys()
{
    if (m_halInputControlKeys)
        return m_halInputControlKeys;

    m_halInputControlKeys = new HalInputControl(HAL_DEVICE_KEYS, "Main");
    if (NULL == m_halInputControlKeys)
        g_critical("Unable to obtain m_halInputControlKeys");

    return m_halInputControlKeys;
}

InputControl* HostArm::getInputControlBluetoothInputDetect()
{
    if (m_halInputControlBluetoothInputDetect)
        return m_halInputControlBluetoothInputDetect;

    m_halInputControlBluetoothInputDetect = new HalInputControl(HAL_DEVICE_BLUETOOTH_INPUT_DETECT, "Main");
    if (NULL == m_halInputControlBluetoothInputDetect)
        g_critical("Unable to obtain m_halInputControlBluetoothInputDetect");

    return m_halInputControlBluetoothInputDetect;
}

void HostArm::setBluetoothKeyboardActive(bool active)
{
    if (m_bluetoothKeyboardActive != active) {
        m_bluetoothKeyboardActive = active;
        g_debug("Bluetooth keyboard is %s", active ? "active" : "inactive");
        Q_EMIT signalBluetoothKeyboardActive(m_bluetoothKeyboardActive);
    }
}

bool HostArm::bluetoothKeyboardActive() const
{
    return m_bluetoothKeyboardActive;
}
