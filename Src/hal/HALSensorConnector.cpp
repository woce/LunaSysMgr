/* @@@LICENSE
*
*      Copyright (c) 2011-2012 Hewlett-Packard Development Company, L.P.
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



#include <QApplication>
#include <QWidget>
#include <QMap>
#include <glib.h>
#include <cjson/json.h>
#include "palmwebtypes.h"
#include "Settings.h"
#include "HALSensorConnector.h"

#define CHECK_ERROR(err, msg)                                                                               \
        do {                                                                                                \
             if (HAL_ERROR_SUCCESS != err)                                                                  \
             {                                                                                              \
               g_critical("[%s : %d] : %s : Error Code -> [%d]", __PRETTY_FUNCTION__, __LINE__, msg, err);  \
               return;                                                                                      \
             }                                                                                              \
           } while(0)

/**
  * Macro is designed to safely call the HAL APIs due to the asynchronous nature of these HAL connectors
  * m_Hanlde can be closed during reading the sensor data from HAL. (i.e. readSensorData())
  */
#define SAFE_HAL_CALL(expr)                     \
        do {                                    \
              if ((m_Handle) && (!m_Finished))  \
              {                                 \
                expr;                           \
              }                                 \
           } while(0)

// Forward Declration
static void InitSensorMap();

// Idle callback for deletion
static gboolean deleteCallback(gpointer apObject)
{
    if (apObject)
    {
        HALConnectorBase* pObj = (HALConnectorBase*)apObject;
        delete pObj;
        apObject = 0;
    }
    return false;
}

HALConnectorBase::HALConnectorBase (Sensor aSensorType, hal_device_type_t aDevType, hal_device_id_t aDevID, HALConnectorObserver *aObserver, bool bCanPostEvent)
    : m_SensorType(aSensorType),
      m_HALSensorNotifier(0),
      m_Handle(0),
      m_HALDeviceType(aDevType),
      m_HALDeviceId(aDevID),
      m_Observer(aObserver),
      m_SensorFD(0),
      m_CanPostEvent(bCanPostEvent),
      m_OrientationAngle(INVALID_ANGLE),
      m_Finished(false)
{
    InitSensorMap();
}

HALConnectorBase::~HALConnectorBase()
{
    if (m_Handle)
    {
        hal_error_t nError = hal_device_close(m_Handle);
        if (HAL_ERROR_SUCCESS != nError)
        {
            g_critical("Unable to release Sensor handle : [%d]", nError);
        }

        m_Handle = 0;
    }

    m_Observer = 0;
    delete m_HALSensorNotifier;
}

hal_error_t HALConnectorBase::openSensor ()
{
    hal_error_t nError = hal_device_open(m_HALDeviceType, m_HALDeviceId, &m_Handle);
    if ((HAL_ERROR_SUCCESS != nError) || (0 == m_Handle))
    {
        g_critical("Unable to open Sensor : [%s : %d] : [HAL Sensor Type = %d]", __PRETTY_FUNCTION__, __LINE__, type());
    }

    return nError;
}

bool HALConnectorBase::on()
{
    if (m_Handle)
    {
        hal_error_t nError;
        nError = hal_device_set_operating_mode(m_Handle, HAL_OPERATING_MODE_ON);
        return (nError == HAL_ERROR_SUCCESS || nError == HAL_ERROR_NOT_IMPLEMENTED);
    }

    return false;
}

bool HALConnectorBase::off()
{
    if (m_Handle)
    {
        hal_error_t error;
        error = hal_device_set_operating_mode(m_Handle, HAL_OPERATING_MODE_OFF);
        return (error == HAL_ERROR_SUCCESS || error == HAL_ERROR_NOT_IMPLEMENTED);
    }

    return false;
}

bool HALConnectorBase::setRate(SensorReportRate aRate)
{
    if (m_Handle)
    {
        hal_error_t nError;
        nError = hal_device_set_report_rate(m_Handle, (hal_report_rate_t)aRate);
        return (nError == HAL_ERROR_SUCCESS || nError == HAL_ERROR_NOT_IMPLEMENTED);
    }

    return false;
}

std::string HALConnectorBase::toJSONString()
{
    std::string strJson = "";

    json_object *jsonObj = toJSONObject();

    if (jsonObj)
    {
        strJson = json_object_to_json_string(jsonObj);

        json_object_put(jsonObj);
    }

    return strJson;
}

void HALConnectorBase::connectSensorSignalToSlot()
{
    if ((m_Handle) && (0 == m_HALSensorNotifier))
    {
        // get the event source from the HAL
        int nError = hal_device_get_event_source(m_Handle, &m_SensorFD);
        CHECK_ERROR(nError, "Unable to obtain Sensor Handle event_source");

        // Connect the Sensor FD from HAL to a SLOT
        m_HALSensorNotifier = new QSocketNotifier(m_SensorFD, QSocketNotifier::Read, this);
        connect(m_HALSensorNotifier, SIGNAL(activated(int)), this, SLOT(readSensorData(int)));
    }
}

std::vector<HALConnectorBase::Sensor> HALConnectorBase::getSupportedSensors()
{
    std::vector<HALConnectorBase::Sensor> sensorList;
    QMap<int, int> sensorMap;

    hal_device_iterator_handle_t    dev_iterator    = 0;
    hal_device_id_t                 dev_id;
    hal_error_t                     error;

    // Main loop, iterate over all possible device types
    for (int deviceType = HAL_DEVICE_SENSOR_FIRST; deviceType <= HAL_DEVICE_SENSOR_LAST; ++deviceType)
    {
        error = hal_get_device_iterator((hal_device_type_t)deviceType, HAL_FILTER_DEFAULT, &dev_iterator);

        if (HAL_ERROR_SUCCESS != error)
            g_critical("hal_device_get_iterator() reported an error of %d\n", error);

        /*
         * Use the iterator we just got and get a count for each of the device types
         * that we have on this device.
         */
        do
        {
            dev_id = 0;

            if (0 != dev_iterator)
            {
                error = hal_device_iterator_get_next_id(dev_iterator, &dev_id);

                if (HAL_ERROR_SUCCESS != error)
                    g_critical("hal_device_get_iterator_get_next_id() reported an error of %d\n", error);
            }


            if (0 != dev_id)
            {
                if (!sensorMap.contains(deviceType))
                {
                    sensorMap.insert(deviceType, 1);

                    HALConnectorBase::Sensor mappedSensor = MapFromHAL((hal_device_type_t)deviceType);
                    if (HALConnectorBase::SensorIllegal != mappedSensor)
                    {
                        sensorList.push_back(mappedSensor);
                    }
                }
            }
        } while (0 != dev_id);

        /* Release the iterator if we are one */
        if (0 != dev_iterator)
        {
            error = hal_release_device_iterator(dev_iterator);
            if (HAL_ERROR_SUCCESS != error)
                g_critical("hal_device_release_iterator() reported an error of %d\n", error);
        }

        dev_iterator = 0;
    }

    return sensorList;
}

HALConnectorBase::Sensor HALConnectorBase::MapFromHAL(hal_device_type_t aHALDevType)
{
    HALConnectorBase::Sensor mappedType = HALConnectorBase::SensorIllegal;

    switch(aHALDevType)
    {
        case HAL_DEVICE_SENSOR_ACCELERATION:
        {
            mappedType = HALConnectorBase::SensorAcceleration;
            break;
        }

        case HAL_DEVICE_SENSOR_ALS:
        {
            mappedType = HALConnectorBase::SensorALS;
            break;
        }

        case HAL_DEVICE_SENSOR_ANGULAR_VELOCITY:
        {
            mappedType = HALConnectorBase::SensorAngularVelocity;
            break;
        }

        case HAL_DEVICE_SENSOR_BEARING:
        {
            mappedType = HALConnectorBase::SensorBearing;
            break;
        }

        case HAL_DEVICE_SENSOR_GRAVITY:
        {
            mappedType = HALConnectorBase::SensorGravity;
            break;
        }

        case HAL_DEVICE_SENSOR_LINEAR_ACCELERATION:
        {
            mappedType = HALConnectorBase::SensorLinearAcceleration;
            break;
        }

        case HAL_DEVICE_SENSOR_MAGNETIC_FIELD:
        {
            mappedType = HALConnectorBase::SensorMagneticField;
            break;
        }

        case HAL_DEVICE_SENSOR_ORIENTATION:
        {
            mappedType = HALConnectorBase::SensorOrientation;
            break;
        }

        case HAL_DEVICE_SENSOR_PROXIMITY:
        {
            mappedType = HALConnectorBase::SensorScreenProximity;
            break;
        }

        case HAL_DEVICE_SENSOR_ROTATION:
        {
            mappedType = HALConnectorBase::SensorRotation;
            break;
        }

        case HAL_DEVICE_SENSOR_SHAKE:
        {
            mappedType = HALConnectorBase::SensorShake;
            break;
        }

        default:
        {
            break;
        }
    }

    return mappedType;
}

typedef QMap<HALConnectorBase::Sensor, QString> SensorMap;
static SensorMap sSensorMap;

static void InitSensorMap()
{
    static bool sInitialized = false;
    if (!sInitialized)
    {
        // Add all the available sensors
        sSensorMap.insert(HALConnectorBase::SensorAcceleration,         QString(Palm::SensorNames::strAccelerometer()));
        sSensorMap.insert(HALConnectorBase::SensorOrientation,          QString(Palm::SensorNames::strOrientation()));
        sSensorMap.insert(HALConnectorBase::SensorShake,                QString(Palm::SensorNames::strShake()));
        sSensorMap.insert(HALConnectorBase::SensorALS,                  QString(Palm::SensorNames::strALS()));
        sSensorMap.insert(HALConnectorBase::SensorAngularVelocity,      QString(Palm::SensorNames::strAngularVelocity()));
        sSensorMap.insert(HALConnectorBase::SensorBearing,              QString(Palm::SensorNames::strBearing()));
        sSensorMap.insert(HALConnectorBase::SensorGravity,              QString(Palm::SensorNames::strGravity()));
        sSensorMap.insert(HALConnectorBase::SensorLinearAcceleration,   QString(Palm::SensorNames::strLinearAcceleration()));
        sSensorMap.insert(HALConnectorBase::SensorMagneticField,        QString(Palm::SensorNames::strMagneticField()));
        sSensorMap.insert(HALConnectorBase::SensorScreenProximity,      QString(Palm::SensorNames::strScreenProximity()));
        sSensorMap.insert(HALConnectorBase::SensorRotation,             QString(Palm::SensorNames::strRotation()));
        sSensorMap.insert(HALConnectorBase::SensorLogicalOrientation,   QString(Palm::SensorNames::strLogicalDeviceOrientation()));
        sSensorMap.insert(HALConnectorBase::SensorLogicalMotion,        QString(Palm::SensorNames::strLogicalDeviceMotion()));

        sInitialized = true;
    }
}

std::string HALConnectorBase::getSupportedSensors(bool bJson)
{
    std::string                             strJson     = "";
    std::vector<HALConnectorBase::Sensor>   sensorList  = getSupportedSensors();

    json_object *jsonSensorList = json_object_new_array();

    for (unsigned int nCounter = 0; nCounter < sensorList.size(); ++nCounter)
    {
        if (sSensorMap.contains(sensorList[nCounter]))
        {
            json_object_array_add(jsonSensorList, json_object_new_string(sSensorMap.value(sensorList[nCounter]).toStdString().c_str()));
        }
        else
        {
            g_critical("[%s : %d] : This shouldn't have happened.!!!", __PRETTY_FUNCTION__, __LINE__);
        }
    }

    strJson = json_object_to_json_string(jsonSensorList);

    json_object_put (jsonSensorList);

    return strJson;
}

HALConnectorBase* HALConnectorBase::getSensor (Sensor aSensorType, HALConnectorObserver *aObserver, bool bCanPostEvent)
{
    HALConnectorBase *pSensorConnector = 0;

    switch (aSensorType)
    {
        case SensorAcceleration:
        {
            pSensorConnector = new HALAccelerationSensorConnector(aObserver, bCanPostEvent);
            break;
        }

        case SensorALS:
        {
            pSensorConnector = new HALAlsSensorConnector(aObserver, bCanPostEvent);
            break;
        }

        case SensorAngularVelocity:
        {
            pSensorConnector = new HALAngularVelocitySensorConnector(aObserver, bCanPostEvent);
            break;
        }

        case SensorBearing:
        {
            pSensorConnector = new HALBearingSensorConnector(aObserver, bCanPostEvent);
            break;
        }

        case SensorGravity:
        {
            pSensorConnector =  new HALGravitySensorConnector(aObserver, bCanPostEvent);
            break;
        }

        case SensorLinearAcceleration:
        {
            pSensorConnector =  new HALLinearAccelearationSensorConnector(aObserver, bCanPostEvent);
            break;
        }

        case SensorMagneticField:
        {
            pSensorConnector =  new HALMagneticFieldSensorConnector(aObserver, bCanPostEvent);
            break;
        }

        case SensorOrientation:
        {
            pSensorConnector =  new HALOrientationSensorConnector(aObserver, bCanPostEvent);
            break;
        }

        case SensorScreenProximity:
        {
            pSensorConnector = new HALScreenProximitySensorConnector(aObserver, bCanPostEvent);
            break;
        }

        case SensorRotation:
        {
            pSensorConnector =  new HALRotationSensorConnector(aObserver, bCanPostEvent);
            break;
        }

        case SensorShake:
        {
            pSensorConnector =  new HALShakeSensorConnector(aObserver, bCanPostEvent);
            break;
        }

        case SensorLogicalAccelerometer:
        {
            pSensorConnector = new HALLogicalAccelerometerSensorConnector(aObserver, bCanPostEvent);
            break;
        }

        case SensorLogicalOrientation:
        {
            pSensorConnector = new HALLogicalOrientationSensorConnector(aObserver, bCanPostEvent);
            break;
        }

        case SensorLogicalDeviceOrientation:
        {
            pSensorConnector = new HALLogicalDeviceOrientationSensorConnector(aObserver, bCanPostEvent);
            break;
        }

        case SensorLogicalMotion:
        {
            pSensorConnector = new HALLogicalDeviceMotionSensorConnector(aObserver, bCanPostEvent);
            break;
        }

        default:
        {
            g_critical("Asked to open Invalid Sensor Type. : [%s : %d]", __PRETTY_FUNCTION__, __LINE__);
            break;
        }
    }

    if (pSensorConnector)
    {
        if (HAL_ERROR_SUCCESS == pSensorConnector->openSensor())
        {
            pSensorConnector->connectSensorSignalToSlot();
        }
        else
        {
            delete pSensorConnector;
            pSensorConnector = 0;
            g_critical("Unable to open the requested sensor type : [%s : %d] : [SensorType = %d]", __PRETTY_FUNCTION__, __LINE__, aSensorType);
        }
    }

    return pSensorConnector;
}

void HALConnectorBase::callObserver(bool aShouldEmit)
{
    if (!m_Finished)
    {
        if (aShouldEmit)
        {
            Q_EMIT sensorDataAvailable();
        }

        if ((canPostEvent()) && (m_Observer))
        {
            m_Observer->HALDataAvailable(type());
        }
    }
}

void HALConnectorBase::scheduleDeletion()
{
    if (!m_Finished)
    {
        // Mark the sensor as finished (i.e. no longer usable)
        m_Finished = true;

        // Turn off the sensor
        off();

        // DFISH-30217: Use timeout source at normal priority instead of an idle source
        // at idle priority, otherwise the idle callback gets queued behind the spewing
        // sensor and the sensor never gets destroyed and the spewing sensor causes
        // idle sources to never fire
        //g_idle_add(deleteCallback, (gpointer)this);
        g_timeout_add(0, deleteCallback, (gpointer) this);
    }
}

/**
 * Acceleration Sensor Connector
 */
HALAccelerationSensorConnector::HALAccelerationSensorConnector(HALConnectorObserver *aObserver, bool bCanPostEvent)
    : HALConnectorBase(SensorAcceleration, HAL_DEVICE_SENSOR_ACCELERATION, "Default", aObserver, bCanPostEvent)
{
    memset(&m_AccelerationData, 0x00, sizeof(hal_sensor_acceleration_event_item_t));
}

void HALAccelerationSensorConnector::readSensorData(int )
{
    hal_event_handle_t  eventHandle = 0;
    hal_error_t         nError      = HAL_ERROR_SUCCESS;

    if (0 == m_Handle) return;

    SAFE_HAL_CALL(nError = hal_device_get_event(m_Handle, &eventHandle));
    CHECK_ERROR(nError, "Unable to obtain HALAccelerationSensorConnector event handle");

    while ((!m_Finished) && (HAL_ERROR_SUCCESS == nError) && (eventHandle) && (m_Handle))
    {
        nError = hal_sensor_acceleration_event_get_item(eventHandle, &m_AccelerationData);
        CHECK_ERROR(nError, "Unable to obtain Acceleration event items");

        SAFE_HAL_CALL(nError = hal_device_release_event(m_Handle, eventHandle));
        CHECK_ERROR(nError, "Unable to release Acceleration event");

        postProcessSensorData();
        callObserver();

        eventHandle = 0;
        SAFE_HAL_CALL(nError = hal_device_get_event(m_Handle, &eventHandle));
    }
}

void HALAccelerationSensorConnector::postProcessSensorData()
{
    AccelerationEvent *event = static_cast<AccelerationEvent*>(getQSensorData());

    if (90 == m_OrientationAngle)
    {
        m_AccelerationData.x = event->y();
        m_AccelerationData.y = event->x();
    }
    else if (180 == m_OrientationAngle)
    {
        m_AccelerationData.x = -(event->x());
        m_AccelerationData.y = -(event->y());
    }
    else if (270 == m_OrientationAngle)
    {
        m_AccelerationData.x = -(event->y());
        m_AccelerationData.y = -(event->x());
    }

    delete event;
}

json_object* HALAccelerationSensorConnector::toJSONObject()
{
    json_object* jsonAccelerationValue = json_object_new_object();

    // Add x,y & z values
    json_object_object_add(jsonAccelerationValue, Palm::HALJsonStringConst::strX(), json_object_new_double(X()));
    json_object_object_add(jsonAccelerationValue, Palm::HALJsonStringConst::strY(), json_object_new_double(Y()));
    json_object_object_add(jsonAccelerationValue, Palm::HALJsonStringConst::strZ(), json_object_new_double(Z()));

    // Wrap above values to another json object
    json_object* jsonAccelerationObject = json_object_new_object();
    json_object_object_add(jsonAccelerationObject, Palm::SensorNames::strAccelerometer(), jsonAccelerationValue);

    return jsonAccelerationObject;
}

QEvent* HALAccelerationSensorConnector::getQSensorData()
{
    AccelerationEvent *e = new AccelerationEvent (X(),
                                                  Y(),
                                                  Z());

    return e;
}

/**
 * ALS Sensor Connector
 */
HALAlsSensorConnector::HALAlsSensorConnector(HALConnectorObserver *aObserver, bool bCanPostEvent)
    : HALConnectorBase(SensorALS, HAL_DEVICE_SENSOR_ALS, "Default", aObserver, bCanPostEvent),
      m_LightIntensity (0)
{
}

void HALAlsSensorConnector::readSensorData(int )
{
    hal_event_handle_t  eventHandle = 0;
    hal_error_t         nError      = HAL_ERROR_SUCCESS;

    if (0 == m_Handle) return;

    SAFE_HAL_CALL(nError = hal_device_get_event(m_Handle, &eventHandle));
    CHECK_ERROR(nError, "Unable to obtain ALS event handle");

    while ((!m_Finished) && (HAL_ERROR_SUCCESS == nError) && (eventHandle) && (m_Handle))
    {
        nError = hal_sensor_als_event_get_intensity(eventHandle, &m_LightIntensity);
        CHECK_ERROR(nError, "Unable to obtain ALS intensity");

        SAFE_HAL_CALL(nError = hal_device_release_event(m_Handle, eventHandle));
        CHECK_ERROR(nError, "Unable to release ALS event");

        callObserver();

        eventHandle = 0;
        SAFE_HAL_CALL(nError = hal_device_get_event(m_Handle, &eventHandle));
    }
}

json_object* HALAlsSensorConnector::toJSONObject()
{
    json_object* jsonLightIntensityValue = json_object_new_object();

    // Add lightIntensity value
    json_object_object_add(jsonLightIntensityValue, Palm::HALJsonStringConst::strLightIntensity(), json_object_new_int(getLightIntensity()));

    // Wrap above values to another json object
    json_object* jsonALSObject = json_object_new_object();
    json_object_object_add(jsonALSObject, Palm::SensorNames::strALS(), jsonLightIntensityValue);

    return jsonALSObject;
}

QEvent* HALAlsSensorConnector::getQSensorData()
{
    return (new AlsEvent(getLightIntensity()));
}

/**
 * Angular Velocity Sensor Connector
 */
HALAngularVelocitySensorConnector::HALAngularVelocitySensorConnector(HALConnectorObserver *aObserver, bool bCanPostEvent)
    : HALConnectorBase(SensorAngularVelocity, HAL_DEVICE_SENSOR_ANGULAR_VELOCITY, "Default", aObserver, bCanPostEvent)
{
    memset(&m_AngularVelocity, 0x00, sizeof(hal_sensor_angular_velocity_event_item_t));
}

void HALAngularVelocitySensorConnector::readSensorData(int )
{
    hal_event_handle_t  eventHandle = 0;
    hal_error_t         nError      = HAL_ERROR_SUCCESS;

    if (0 == m_Handle) return;

    SAFE_HAL_CALL(nError = hal_device_get_event(m_Handle, &eventHandle));
    CHECK_ERROR(nError, "Unable to obtain Angular Velocity Sensor event handle");

    while ((!m_Finished) && (HAL_ERROR_SUCCESS == nError) && (eventHandle) && (m_Handle))
    {
        nError = hal_sensor_angular_velocity_event_get_item(eventHandle, &m_AngularVelocity);
        CHECK_ERROR(nError, "Unable to obtain Angular Velocity data");

        SAFE_HAL_CALL(nError = hal_device_release_event(m_Handle, eventHandle));
        CHECK_ERROR(nError, "Unable to release Angular Velocity event");

        callObserver();

        eventHandle = 0;
        SAFE_HAL_CALL(nError = hal_device_get_event(m_Handle, &eventHandle));
    }
}

json_object* HALAngularVelocitySensorConnector::toJSONObject()
{
    json_object* jsonAngularVelocityValue = json_object_new_object();

    // Add x,y & z values
    json_object_object_add(jsonAngularVelocityValue, Palm::HALJsonStringConst::strX(), json_object_new_double(X()));
    json_object_object_add(jsonAngularVelocityValue, Palm::HALJsonStringConst::strY(), json_object_new_double(Y()));
    json_object_object_add(jsonAngularVelocityValue, Palm::HALJsonStringConst::strZ(), json_object_new_double(Z()));

    // Wrap above values to another json object
    json_object* jsonAngularVelocityObject = json_object_new_object();
    json_object_object_add(jsonAngularVelocityObject, Palm::SensorNames::strAngularVelocity(), jsonAngularVelocityValue);

    return jsonAngularVelocityObject;
}

QEvent* HALAngularVelocitySensorConnector::getQSensorData()
{
    AngularVelocityEvent *e = new AngularVelocityEvent(X(),
                                                       Y(),
                                                       Z());

    return e;
}

/**
 * Bearing Sensor Connector
 */
HALBearingSensorConnector::HALBearingSensorConnector(HALConnectorObserver *aObserver, bool bCanPostEvent)
    : HALConnectorBase(SensorBearing, HAL_DEVICE_SENSOR_BEARING, "Default", aObserver, bCanPostEvent)
{
    memset(&m_Bearing, 0x00, sizeof(hal_sensor_bearing_event_item_t));
}

void HALBearingSensorConnector::readSensorData(int )
{
    hal_event_handle_t  eventHandle = 0;
    hal_error_t         nError      = HAL_ERROR_SUCCESS;

    if (0 == m_Handle) return;

    SAFE_HAL_CALL(nError = hal_device_get_event(m_Handle, &eventHandle));
    CHECK_ERROR(nError, "Unable to obtain Bearing Sensor event handle");

    while ((!m_Finished) && (HAL_ERROR_SUCCESS == nError) && (eventHandle) && (m_Handle))
    {
        nError = hal_sensor_bearing_event_get_item(eventHandle, &m_Bearing);
        CHECK_ERROR(nError, "Unable to obtain bearing data");

        SAFE_HAL_CALL(nError = hal_device_release_event(m_Handle, eventHandle));
        CHECK_ERROR(nError, "Unable to release Bearing event");

        callObserver();

        eventHandle = 0;
        SAFE_HAL_CALL(nError = hal_device_get_event(m_Handle, &eventHandle));
    }
}

json_object* HALBearingSensorConnector::toJSONObject()
{
    json_object* jsonBearingValues = json_object_new_object();

    json_object_object_add(jsonBearingValues, Palm::HALJsonStringConst::strMagnetic(),    json_object_new_double(bearingMagnitude()));
    json_object_object_add(jsonBearingValues, Palm::HALJsonStringConst::strTrueBearing(), json_object_new_double(trueBearing()));
    json_object_object_add(jsonBearingValues, Palm::HALJsonStringConst::strConfidence(),  json_object_new_double(confidence()));

    // Wrap above values to another json object
    json_object* jsonBearingObject = json_object_new_object();
    json_object_object_add(jsonBearingObject, Palm::SensorNames::strBearing(), jsonBearingValues);

    return jsonBearingObject;
}

QEvent* HALBearingSensorConnector::getQSensorData()
{
    CompassEvent* e = new CompassEvent (bearingMagnitude(),
                                        trueBearing(),
                                        (int)confidence());

    return e;
}

bool HALBearingSensorConnector::setLocation(double aLatitude, double aLongitude)
{
    if (m_Handle)
    {
        hal_error_t                     nError = HAL_ERROR_SUCCESS;
        hal_sensor_bearing_location_t   location;

        location.latitude   = aLatitude;
        location.longitude  = aLongitude;
        location.altitude   = 0.0;

        SAFE_HAL_CALL(nError = hal_sensor_bearing_set_location(m_Handle, &location));

        // g_debug("Set bearing location to %f, %f, hal-error = %d", aLatitude, aLongitude, nError);
        return (nError == HAL_ERROR_SUCCESS || nError == HAL_ERROR_NOT_IMPLEMENTED);
    }

    return false;
}

/**
 * Gravity Sensor Connector
 */
HALGravitySensorConnector::HALGravitySensorConnector(HALConnectorObserver *aObserver, bool bCanPostEvent)
    : HALConnectorBase(SensorGravity, HAL_DEVICE_SENSOR_GRAVITY, "Default", aObserver, bCanPostEvent)
{
    memset(&m_Gravity, 0x00, sizeof(hal_sensor_gravity_event_item_t));
}

void HALGravitySensorConnector::readSensorData(int )
{
    hal_event_handle_t  eventHandle = 0;
    hal_error_t         nError      = HAL_ERROR_SUCCESS;

    if (0 == m_Handle) return;

    SAFE_HAL_CALL(nError = hal_device_get_event(m_Handle, &eventHandle));
    CHECK_ERROR(nError, "Unable to obtain Gravity Sensor event handle");

    while ((!m_Finished) && (HAL_ERROR_SUCCESS == nError) && (eventHandle) && (m_Handle))
    {
        nError = hal_sensor_gravity_event_get_item(eventHandle, &m_Gravity);
        CHECK_ERROR(nError, "Unable to obtain gravity data");

        SAFE_HAL_CALL(nError = hal_device_release_event(m_Handle, eventHandle));
        CHECK_ERROR(nError, "Unable to release Gravity event");

        callObserver();

        eventHandle = 0;
        SAFE_HAL_CALL(nError = hal_device_get_event(m_Handle, &eventHandle));
    }
}

json_object* HALGravitySensorConnector::toJSONObject()
{
    json_object* jsonGravityValues = json_object_new_object();

    // Add x,y & z values
    json_object_object_add(jsonGravityValues, Palm::HALJsonStringConst::strX(), json_object_new_double(X()));
    json_object_object_add(jsonGravityValues, Palm::HALJsonStringConst::strY(), json_object_new_double(Y()));
    json_object_object_add(jsonGravityValues, Palm::HALJsonStringConst::strZ(), json_object_new_double(Z()));

    // Wrap above values to another json object
    json_object* jsonGravityObject = json_object_new_object();
    json_object_object_add(jsonGravityObject, Palm::SensorNames::strGravity(), jsonGravityValues);

    return jsonGravityObject;
}

QEvent* HALGravitySensorConnector::getQSensorData()
{
    GravityEvent *e = new GravityEvent (X(),
                                        Y(),
                                        Z());

    return e;
}

/**
 * Linear Acceleration Sensor Connector
 */
HALLinearAccelearationSensorConnector::HALLinearAccelearationSensorConnector(HALConnectorObserver *aObserver, bool bCanPostEvent)
    : HALConnectorBase(SensorLinearAcceleration, HAL_DEVICE_SENSOR_LINEAR_ACCELERATION, "Default", aObserver, bCanPostEvent)
{
    memset(&m_LinearAcceleration, 0x00, sizeof(hal_sensor_linear_acceleration_event_item_t));
}

void HALLinearAccelearationSensorConnector::readSensorData(int )
{
    hal_event_handle_t  eventHandle = 0;
    hal_error_t         nError      = HAL_ERROR_SUCCESS;

    if (0 == m_Handle) return;

    SAFE_HAL_CALL(nError = hal_device_get_event(m_Handle, &eventHandle));
    CHECK_ERROR(nError, "Unable to obtain Linear Acceleration Sensor event handle");

    while ((!m_Finished) && (HAL_ERROR_SUCCESS == nError) && (eventHandle) && (m_Handle))
    {
        nError = hal_sensor_linear_acceleration_event_get_item(eventHandle, &m_LinearAcceleration);
        CHECK_ERROR(nError, "Unable to obtain linear acceleration data");

        SAFE_HAL_CALL(nError = hal_device_release_event(m_Handle, eventHandle));
        CHECK_ERROR(nError, "Unable to release linear acceleration event");

        callObserver();

        eventHandle = 0;
        SAFE_HAL_CALL(nError = hal_device_get_event(m_Handle, &eventHandle));
    }
}

json_object* HALLinearAccelearationSensorConnector::toJSONObject()
{
    json_object* jsonLinearAccelerationValues = json_object_new_object();

    json_object_object_add(jsonLinearAccelerationValues, Palm::HALJsonStringConst::strX(),        json_object_new_double(X()));
    json_object_object_add(jsonLinearAccelerationValues, Palm::HALJsonStringConst::strY(),        json_object_new_double(Y()));
    json_object_object_add(jsonLinearAccelerationValues, Palm::HALJsonStringConst::strZ(),        json_object_new_double(Z()));
    json_object_object_add(jsonLinearAccelerationValues, Palm::HALJsonStringConst::strWorldX(),   json_object_new_double(WorldX()));
    json_object_object_add(jsonLinearAccelerationValues, Palm::HALJsonStringConst::strWorldY(),   json_object_new_double(WorldY()));
    json_object_object_add(jsonLinearAccelerationValues, Palm::HALJsonStringConst::strWorldZ(),   json_object_new_double(WorldZ()));

    // Wrap above values to another json object
    json_object* jsonLinearAccelerationObject = json_object_new_object();
    json_object_object_add(jsonLinearAccelerationObject, Palm::SensorNames::strLinearAcceleration(), jsonLinearAccelerationValues);

    return jsonLinearAccelerationObject;
}

QEvent* HALLinearAccelearationSensorConnector::getQSensorData()
{
    LinearAccelerationEvent *e = new LinearAccelerationEvent (X(),
                                                              Y(),
                                                              Z(),
                                                              WorldX(),
                                                              WorldY(),
                                                              WorldZ());

    return e;
}

/**
 * Magnetic Field Sensor Connector
 */
HALMagneticFieldSensorConnector::HALMagneticFieldSensorConnector(HALConnectorObserver *aObserver, bool bCanPostEvent)
    : HALConnectorBase(SensorMagneticField, HAL_DEVICE_SENSOR_MAGNETIC_FIELD, "Default", aObserver, bCanPostEvent)
{
    memset(&m_MagneticField, 0x00, sizeof(hal_sensor_magnetic_field_event_item_t));
}

void HALMagneticFieldSensorConnector::readSensorData(int )
{
    hal_event_handle_t  eventHandle = 0;
    hal_error_t         nError      = HAL_ERROR_SUCCESS;

    if (0 == m_Handle) return;

    SAFE_HAL_CALL(nError = hal_device_get_event(m_Handle, &eventHandle));
    CHECK_ERROR(nError, "Unable to obtain Magnetic Field Sensor event handle");

    while ((!m_Finished) && (HAL_ERROR_SUCCESS == nError) && (eventHandle) && (m_Handle))
    {
        nError = hal_sensor_magnetic_field_event_get_item(eventHandle, &m_MagneticField);
        CHECK_ERROR(nError, "Unable to obtain Magnetic field data");

        SAFE_HAL_CALL(nError = hal_device_release_event(m_Handle, eventHandle));
        CHECK_ERROR(nError, "Unable to release magnetic field event");

        callObserver();

        eventHandle = 0;
        SAFE_HAL_CALL(nError = hal_device_get_event(m_Handle, &eventHandle));
    }
}

json_object* HALMagneticFieldSensorConnector::toJSONObject()
{
    json_object* jsonMagneticFieldValues = json_object_new_object();

    json_object_object_add(jsonMagneticFieldValues, Palm::HALJsonStringConst::strX(),     json_object_new_double(X()));
    json_object_object_add(jsonMagneticFieldValues, Palm::HALJsonStringConst::strY(),     json_object_new_double(Y()));
    json_object_object_add(jsonMagneticFieldValues, Palm::HALJsonStringConst::strZ(),     json_object_new_double(Z()));
    json_object_object_add(jsonMagneticFieldValues, Palm::HALJsonStringConst::strRawX(),  json_object_new_double(rawX()));
    json_object_object_add(jsonMagneticFieldValues, Palm::HALJsonStringConst::strRawY(),  json_object_new_double(rawY()));
    json_object_object_add(jsonMagneticFieldValues, Palm::HALJsonStringConst::strRawZ(),  json_object_new_double(rawZ()));

    // Wrap above values to another json object
    json_object* jsonMagneticFieldObject = json_object_new_object();
    json_object_object_add(jsonMagneticFieldObject, Palm::SensorNames::strMagneticField(), jsonMagneticFieldValues);

    return jsonMagneticFieldObject;
}

QEvent* HALMagneticFieldSensorConnector::getQSensorData()
{
    MagneticFieldEvent *e = new MagneticFieldEvent (X(),
                                                    Y(),
                                                    Z(),
                                                    rawX(),
                                                    rawY(),
                                                    rawZ());

    return e;
}

/**
 * Orientation Sensor Connector
 */
HALOrientationSensorConnector::HALOrientationSensorConnector(HALConnectorObserver *aObserver, bool bCanPostEvent)
    : HALConnectorBase(SensorOrientation, HAL_DEVICE_SENSOR_ORIENTATION, "Default", aObserver, bCanPostEvent),
      m_Orientation(OrientationEvent::Orientation_Invalid)
{
}

void HALOrientationSensorConnector::readSensorData(int )
{
    hal_event_handle_t  eventHandle = 0;
    hal_error_t         nError      = HAL_ERROR_SUCCESS;

    if (0 == m_Handle) return;

    SAFE_HAL_CALL(nError = hal_device_get_event(m_Handle, &eventHandle));
    CHECK_ERROR(nError, "Unable to obtain Orientation Sensor event handle");

    while ((!m_Finished) && (HAL_ERROR_SUCCESS == nError) && (eventHandle) && (m_Handle))
    {
        hal_sensor_orientation_event_item_t orientationData;
        nError = hal_sensor_orientation_event_get_item(eventHandle, &orientationData);
        CHECK_ERROR((nError != HAL_ERROR_SUCCESS), "Unable to obtain Orientation event items");

        m_Orientation = mapAccelerometerOrientation(orientationData.value);

        SAFE_HAL_CALL(nError = hal_device_release_event(m_Handle, eventHandle));
        CHECK_ERROR(nError, "Unable to release Orientation event");

        postProcessSensorData();
        callObserver();

        eventHandle = 0;
        SAFE_HAL_CALL(nError = hal_device_get_event(m_Handle, &eventHandle));
    }
}

void HALOrientationSensorConnector::postProcessSensorData()
{
    OrientationEvent *event = static_cast<OrientationEvent *>(getQSensorData());

    if (INVALID_ANGLE != m_OrientationAngle && 0 != m_OrientationAngle)
    {
        switch(event->orientation())
        {
            case OrientationEvent::Orientation_Up:
            {
                m_Orientation = OrientationEvent::Orientation_Left;
                break;
            }

            case OrientationEvent::Orientation_Down:
            {
                m_Orientation = OrientationEvent::Orientation_Right;
                break;
            }

            case OrientationEvent::Orientation_Left:
            {
                m_Orientation = OrientationEvent::Orientation_Down;
                break;
            }

            case OrientationEvent::Orientation_Right:
            {
                m_Orientation = OrientationEvent::Orientation_Up;
                break;
            }
            default:
            {
                break;
            }
        }
    }

    delete event;
}

const char* HALOrientationSensorConnector::toPositionString()
{
    switch(getOrientation())
    {
        case OrientationEvent::Orientation_FaceUp:
        {
            return Palm::HALJsonStringConst::strOrientationFaceUp();
        }

        case OrientationEvent::Orientation_FaceDown:
        {
            return Palm::HALJsonStringConst::strOrientationFaceDown();
        }

        case OrientationEvent::Orientation_Up:
        {
            return Palm::HALJsonStringConst::strOrientationFaceForward();
        }

        case OrientationEvent::Orientation_Down:
        {
            return Palm::HALJsonStringConst::strOrientationFaceBack();
        }

        case OrientationEvent::Orientation_Left:
        {
            return Palm::HALJsonStringConst::strOrientationLeft();
        }

        case OrientationEvent::Orientation_Right:
        {
            return Palm::HALJsonStringConst::strOrientationRight();
        }

        default:
        {
            return Palm::HALJsonStringConst::strEmpty();
        }
    }
}

json_object* HALOrientationSensorConnector::toJSONObject()
{
    json_object* jsonOrientationValue = json_object_new_object();

    json_object_object_add(jsonOrientationValue, Palm::HALJsonStringConst::strPosition(), json_object_new_string(toPositionString()));

    // Wrap above values to another json object
    json_object* jsonOrientationObject = json_object_new_object();
    json_object_object_add(jsonOrientationObject, Palm::SensorNames::strOrientation(), jsonOrientationValue);

    return jsonOrientationObject;
}

QEvent* HALOrientationSensorConnector::getQSensorData()
{
    OrientationEvent *e = new OrientationEvent ((OrientationEvent::Orientation)m_Orientation);

    return e;
}

/**
 * convert the accelerometer orientation from the lower level into
 * an Orientation type
 */
OrientationEvent::Orientation HALOrientationSensorConnector::mapAccelerometerOrientation(int aValue)
{
#if defined(MACHINE_TOPAZ)
    // Temporary hack to flip around orientations till Topaz framebuffer is aligned right
    switch (aValue) {
    case HAL_SENSOR_ORIENTATION_FACE_DOWN:
        return OrientationEvent::Orientation_FaceDown;
    case HAL_SENSOR_ORIENTATION_FACE_UP:
        return OrientationEvent::Orientation_FaceUp;
    case HAL_SENSOR_ORIENTATION_RIGHT:
        return OrientationEvent::Orientation_Left;
    case HAL_SENSOR_ORIENTATION_LEFT:
        return OrientationEvent::Orientation_Right;
    case HAL_SENSOR_ORIENTATION_FACE_FORWARD:
        return OrientationEvent::Orientation_Down;
    case HAL_SENSOR_ORIENTATION_FACE_BACK:
        return OrientationEvent::Orientation_Up;
    default:
        return OrientationEvent::Orientation_Invalid;
    }
#else
    switch (aValue)
    {
        case HAL_SENSOR_ORIENTATION_FACE_DOWN:
        {
            return OrientationEvent::Orientation_FaceDown;
        }

        case HAL_SENSOR_ORIENTATION_FACE_UP:
        {
            return OrientationEvent::Orientation_FaceUp;
        }

        case HAL_SENSOR_ORIENTATION_RIGHT:
        {
            return OrientationEvent::Orientation_Right;
        }

        case HAL_SENSOR_ORIENTATION_LEFT:
        {
            return OrientationEvent::Orientation_Left;
        }

        case HAL_SENSOR_ORIENTATION_FACE_BACK:
        {
            return OrientationEvent::Orientation_Down;
        }

        case HAL_SENSOR_ORIENTATION_FACE_FORWARD:
        {
            return OrientationEvent::Orientation_Up;
        }

        default:
        {
            return OrientationEvent::Orientation_Invalid;
        }
    }
#endif
}

/**
 * Screen Proximity Sensor Connector
 */
HALScreenProximitySensorConnector::HALScreenProximitySensorConnector(HALConnectorObserver *aObserver, bool bCanPostEvent)
    : HALConnectorBase(SensorScreenProximity, HAL_DEVICE_SENSOR_PROXIMITY, "Screen", aObserver, bCanPostEvent),
      m_Present(0)
{
}

void HALScreenProximitySensorConnector::readSensorData(int )
{
    hal_event_handle_t  eventHandle = 0;
    hal_error_t         nError      = HAL_ERROR_SUCCESS;

    if (0 == m_Handle) return;

    SAFE_HAL_CALL(nError = hal_device_get_event(m_Handle, &eventHandle));
    CHECK_ERROR(nError, "Unable to obtain Proximity event handle");

    while ((!m_Finished) && (HAL_ERROR_SUCCESS == nError) && (eventHandle) && (m_Handle))
    {
        nError = hal_sensor_proximity_event_get_presence(eventHandle, &m_Present);
        CHECK_ERROR(nError, "Unable to get proximity event");

        SAFE_HAL_CALL(nError = hal_device_release_event(m_Handle, eventHandle));
        CHECK_ERROR(nError, "Unable to release proximity event");

        callObserver();

        eventHandle = 0;
        SAFE_HAL_CALL(nError = hal_device_get_event(m_Handle, &eventHandle));
    }
}

QEvent* HALScreenProximitySensorConnector::getQSensorData()
{
    return (new ProximityEvent(m_Present));
}

/**
 * Rotation Sensor Connector
 */
HALRotationSensorConnector::HALRotationSensorConnector(HALConnectorObserver *aObserver, bool bCanPostEvent)
    : HALConnectorBase(SensorRotation, HAL_DEVICE_SENSOR_ROTATION, "Default", aObserver, bCanPostEvent)
{
    memset(&m_RotationData, 0x00, sizeof(hal_sensor_rotation_event_item_t));
}

void HALRotationSensorConnector::readSensorData(int )
{
    hal_event_handle_t  eventHandle = 0;
    hal_error_t         nError      = HAL_ERROR_SUCCESS;

    if (0 == m_Handle) return;

    SAFE_HAL_CALL(nError = hal_device_get_event(m_Handle, &eventHandle));
    CHECK_ERROR(nError, "Unable to obtain Rotation event handle");

    while ((!m_Finished) && (HAL_ERROR_SUCCESS == nError) && (eventHandle) && (m_Handle))
    {
        nError = hal_sensor_rotation_event_get_item(eventHandle, &m_RotationData);
        CHECK_ERROR(nError, "Unable to get Rotation event");

        SAFE_HAL_CALL(nError = hal_device_release_event(m_Handle, eventHandle));
        CHECK_ERROR(nError, "Unable to release Rotation event");

        postProcessSensorData();
        callObserver();

        eventHandle = 0;
        SAFE_HAL_CALL(nError = hal_device_get_event(m_Handle, &eventHandle));
    }
}

void HALRotationSensorConnector::postProcessSensorData()
{
    RotationEvent *event = static_cast<RotationEvent *>(getQSensorData());

    if (0 == m_OrientationAngle)
    {
        m_RotationData.euler_angle.pitch = -(event->pitch());
        m_RotationData.euler_angle.roll  =   event->roll();
    }
    else if (90 == m_OrientationAngle)
    {
        m_RotationData.euler_angle.pitch = -(event->roll());
        m_RotationData.euler_angle.roll  = -(event->pitch());
    }
    else if (180 == m_OrientationAngle)
    {
        m_RotationData.euler_angle.pitch =   event->pitch();
        m_RotationData.euler_angle.roll  = -(event->roll());
    }
    else if (270 == m_OrientationAngle)
    {
        m_RotationData.euler_angle.pitch =   event->roll();
        m_RotationData.euler_angle.roll  = -(event->pitch());
    }

    delete event;
}

json_object* HALRotationSensorConnector::toJSONObject()
{
    // Create rotationMatrix Array
    json_object* rotationMatrix = json_object_new_array();

    int nArraySize = sizeof(m_RotationData.matrix)/sizeof(m_RotationData.matrix[0]);
    for (int nCounter = 0; nCounter < nArraySize; ++nCounter)
    {
        json_object_array_add(rotationMatrix, json_object_new_double(m_RotationData.matrix[nCounter]));
    }

    //Create quaternion Vector
    json_object* quaternionVector = json_object_new_object();

    json_object_object_add(quaternionVector, Palm::HALJsonStringConst::strW(), json_object_new_double(quaternionW()));
    json_object_object_add(quaternionVector, Palm::HALJsonStringConst::strX(), json_object_new_double(quaternionX()));
    json_object_object_add(quaternionVector, Palm::HALJsonStringConst::strY(), json_object_new_double(quaternionY()));
    json_object_object_add(quaternionVector, Palm::HALJsonStringConst::strZ(), json_object_new_double(quaternionZ()));

    // Create eulerAngle Vector
    json_object* eulerAngleVector = json_object_new_object();

    json_object_object_add(eulerAngleVector, Palm::HALJsonStringConst::strRoll(), json_object_new_double(roll()));
    json_object_object_add(eulerAngleVector, Palm::HALJsonStringConst::strPitch(),json_object_new_double(pitch()));
    json_object_object_add(eulerAngleVector, Palm::HALJsonStringConst::strYaw(),  json_object_new_double(yaw()));

    // Wrap above values to another json inner object
    json_object* jsonRotationInnerObject = json_object_new_object();
    json_object_object_add(jsonRotationInnerObject, Palm::HALJsonStringConst::strRotationMatrix(),     rotationMatrix);
    json_object_object_add(jsonRotationInnerObject, Palm::HALJsonStringConst::strQuaternionVector(),   quaternionVector);
    json_object_object_add(jsonRotationInnerObject, Palm::HALJsonStringConst::strEulerAngle(),         eulerAngleVector);

    // Wrap the json inner object to final object
    json_object* jsonRotationObject = json_object_new_object();
    json_object_object_add(jsonRotationObject, Palm::SensorNames::strRotation(), jsonRotationInnerObject);

    return jsonRotationObject;
}

QEvent* HALRotationSensorConnector::getQSensorData()
{
    RotationEvent *e = new RotationEvent(m_RotationData.euler_angle.pitch,
                                         m_RotationData.euler_angle.roll);

    return e;
}

/**
 * Shake Sensor Connector
 */
HALShakeSensorConnector::HALShakeSensorConnector(HALConnectorObserver *aObserver, bool bCanPostEvent)
    : HALConnectorBase(SensorShake, HAL_DEVICE_SENSOR_SHAKE, "Default", aObserver, bCanPostEvent),
      m_ShakeState(ShakeEvent::Shake_Invalid),
      m_ShakeMagnitude(0.0)
{
}

void HALShakeSensorConnector::readSensorData(int )
{
    hal_event_handle_t  eventHandle = 0;
    hal_error_t         nError      = HAL_ERROR_SUCCESS;

    if (0 == m_Handle) return;

    SAFE_HAL_CALL(nError = hal_device_get_event(m_Handle, &eventHandle));
    CHECK_ERROR(nError, "Unable to obtain Shake Sensor event handle");

    while ((!m_Finished) && (HAL_ERROR_SUCCESS == nError) && (eventHandle) && (m_Handle))
    {
        hal_sensor_shake_event_item_t   shakeData;
        nError = hal_sensor_shake_event_get_item(eventHandle, &shakeData);
        CHECK_ERROR((nError != HAL_ERROR_SUCCESS), "Unable to obtain Shake Handle event items");

        SAFE_HAL_CALL(nError = hal_device_release_event(m_Handle, eventHandle));
        CHECK_ERROR(nError, "Unable to release Shake event");

        m_ShakeMagnitude = shakeData.magnitude;
        m_ShakeState     = mapShakeSensorEvent(shakeData.state);

        callObserver();

        eventHandle = 0;
        SAFE_HAL_CALL(nError = hal_device_get_event(m_Handle, &eventHandle));
    }
}

const char* HALShakeSensorConnector::toShakeStateString()
{
    switch(shakeState())
    {
        case ShakeEvent::Shake_Start:
        {
            return Palm::HALJsonStringConst::strShakeStart();
        }

        case ShakeEvent::Shake_Shaking:
        {
            return Palm::HALJsonStringConst::strShaking();
        }

        case ShakeEvent::Shake_End:
        {
            return Palm::HALJsonStringConst::strShakeEnd();
        }

        default:
        {
            return "";
        }
    }
}

json_object* HALShakeSensorConnector::toJSONObject()
{
    json_object* jsonShakeSensorValue = json_object_new_object();

    json_object_object_add(jsonShakeSensorValue, Palm::HALJsonStringConst::strShakeState(),     json_object_new_string(toShakeStateString()));
    json_object_object_add(jsonShakeSensorValue, Palm::HALJsonStringConst::strShakeMagnitude(), json_object_new_double(shakeMagnitude()));

    // Wrap above values to another json object
    json_object* jsonShakeObject = json_object_new_object();
    json_object_object_add(jsonShakeObject, Palm::SensorNames::strShake(), jsonShakeSensorValue);

    return jsonShakeObject;
}

QEvent* HALShakeSensorConnector::getQSensorData()
{
    return (new ShakeEvent(m_ShakeState, m_ShakeMagnitude));
}

ShakeEvent::Shake HALShakeSensorConnector::mapShakeSensorEvent(int aValue)
{
    switch (aValue)
    {
        case HAL_SENSOR_SHAKE_START:
            return ShakeEvent::Shake_Start;

        case HAL_SENSOR_SHAKE_SHAKING:
            return ShakeEvent::Shake_Shaking;

        case HAL_SENSOR_SHAKE_STOP:
            return ShakeEvent::Shake_End;

        case HAL_SENSOR_SHAKE_NONE:
        default:
            return ShakeEvent::Shake_Invalid;
    }
}

/**
 * HAL Logical sensor connector base class
 */
HALLogicalSensorConnectorBase::HALLogicalSensorConnectorBase(Sensor aSensorType, HALConnectorObserver *aObserver, bool bCanPostEvent)
    : HALConnectorBase(aSensorType, HAL_DEVICE_ILLEGAL_DEVICE, "Default", aObserver, bCanPostEvent)
{
}

HALLogicalSensorConnectorBase::~HALLogicalSensorConnectorBase()
{
}

bool HALLogicalSensorConnectorBase::on()
{
    HALConnectorBase  *pSensor = 0;
    std::vector<HALConnectorBase *>::iterator it;

    for (it = m_SensorList.begin() ; it < m_SensorList.end(); ++it )
    {
        pSensor = *it;

        if ((pSensor) && (!(pSensor->on())))
        {
            g_critical("Unable to start a Sensor : [%s : %d] : [Sensor Type = %d]]", __PRETTY_FUNCTION__, __LINE__, pSensor->type());
        }
    }

    return true;
}

bool HALLogicalSensorConnectorBase::off()
{
    HALConnectorBase  *pSensor = 0;
    std::vector<HALConnectorBase *>::iterator it;

    for (it = m_SensorList.begin() ; it < m_SensorList.end(); ++it )
    {
        pSensor = *it;

        if ((pSensor) && (!(pSensor->off())))
        {
            g_critical("Unable to stop a Sensor : [%s : %d] : [Sensor Type = %d]]", __PRETTY_FUNCTION__, __LINE__, pSensor->type());
        }
    }

    return true;
}

bool HALLogicalSensorConnectorBase::setRate(SensorReportRate aRate)
{
    HALConnectorBase  *pSensor = 0;
    std::vector<HALConnectorBase *>::iterator it;

    for (it = m_SensorList.begin() ; it < m_SensorList.end(); ++it )
    {
        pSensor = *it;

        if ((pSensor) && (!(pSensor->setRate(aRate))))
        {
            g_critical("Unable to set rate for the Sensor : [%s : %d] : [Sensor Type = %d]]", __PRETTY_FUNCTION__, __LINE__, pSensor->type());
        }
    }

    return true;
}

void HALLogicalSensorConnectorBase::setOrientationAngle(int aAngle)
{
    HALConnectorBase  *pSensor = 0;
    std::vector<HALConnectorBase *>::iterator it;

    m_OrientationAngle = aAngle;

    for (it = m_SensorList.begin() ; it < m_SensorList.end(); ++it )
    {
        pSensor = *it;

        if (pSensor)
        {
            pSensor->setOrientationAngle(aAngle);
        }
    }
}

void HALLogicalSensorConnectorBase::scheduleDeletion()
{
    HALConnectorBase  *pSensor = 0;
    std::vector<HALConnectorBase *>::iterator it;

    HALConnectorBase::scheduleDeletion();

    for (it = m_SensorList.begin() ; it < m_SensorList.end(); ++it )
    {
        pSensor = *it;

        if (pSensor)
        {
            pSensor->scheduleDeletion();
        }
    }
}

void HALLogicalSensorConnectorBase::logicalSensorDataAvailable()
{
    if ((m_Observer) && (!m_Finished))
    {
        m_Observer->HALDataAvailable(type());
    }
}

json_object* HALLogicalSensorConnectorBase::toJSONObject()
{
    HALConnectorBase   *pSensor         = 0;
    std::string         strJson         = "";

    std::vector<HALConnectorBase *>::iterator it;

    json_object* jsonLogicalSensorInnerObjects = json_object_new_array();
    for (it = m_SensorList.begin() ; it < m_SensorList.end(); ++it )
    {
        pSensor = *it;
        if (pSensor)
        {
            json_object_array_add(jsonLogicalSensorInnerObjects, pSensor->toJSONObject());
        }
    }

    // Create the final object
    json_object* jsonLogicalSensorObject = json_object_new_object();
    json_object_object_add(jsonLogicalSensorObject, sSensorMap.value(type()).toStdString().c_str(), jsonLogicalSensorInnerObjects);

    return jsonLogicalSensorObject;
}

/**
 * Logical Accelerometer Sensor Connector
 *  - Accelerometer
 *  - Orientation
 *  - Shake
 *  - Rotation
 */
HALLogicalAccelerometerSensorConnector::HALLogicalAccelerometerSensorConnector(HALConnectorObserver *aObserver, bool bCanPostEvent)
    : HALLogicalSensorConnectorBase(SensorLogicalAccelerometer, aObserver, bCanPostEvent)
{
    HALConnectorBase *pSensor = HALConnectorBase::getSensor(SensorAcceleration, 0, false);
    if (pSensor)
    {
        m_SensorList.push_back(pSensor);
        connect(pSensor, SIGNAL(sensorDataAvailable()), this, SLOT(logicalSensorDataAvailable()));
        pSensor = 0;
    }

    pSensor = HALConnectorBase::getSensor(SensorOrientation, 0, false);
    if (pSensor)
    {
        m_SensorList.push_back(pSensor);
        connect(pSensor, SIGNAL(sensorDataAvailable()), this, SLOT(logicalSensorDataAvailable()));
        pSensor = 0;
    }

    pSensor = HALConnectorBase::getSensor(SensorShake, 0, false);
    if (pSensor)
    {
        m_SensorList.push_back(pSensor);
        connect(pSensor, SIGNAL(sensorDataAvailable()), this, SLOT(logicalSensorDataAvailable()));
        pSensor = 0;
    }

    pSensor = HALConnectorBase::getSensor(SensorRotation, 0, false);
    if (pSensor)
    {
        m_SensorList.push_back(pSensor);
        connect(pSensor, SIGNAL(sensorDataAvailable()), this, SLOT(logicalSensorDataAvailable()));
        pSensor = 0;
    }
}

QEvent* HALLogicalAccelerometerSensorConnector::readAllSensorData()
{
    OrientationEvent::Orientation   aOrientation    = OrientationEvent::Orientation_Invalid;
    ShakeEvent::Shake               aShakeState     = ShakeEvent::Shake_Invalid;
    HALConnectorBase               *pSensor         = 0;
    float                           aRotationPitch  = 0.0;
    float                           aRotationRoll   = 0.0;
    float                           aAccelX         = 0.0;
    float                           aAccelY         = 0.0;
    float                           aAccelZ         = 0.0;
    float                           aShakeMagnitude = 0.0;

    std::vector<HALConnectorBase *>::iterator it;

    for (it = m_SensorList.begin() ; it < m_SensorList.end(); ++it )
    {
        pSensor = *it;
        if (pSensor)
        {
            switch (pSensor->type())
            {
                case SensorAcceleration:
                {
                    HALAccelerationSensorConnector *pAccelSensor = static_cast<HALAccelerationSensorConnector *>(pSensor);
                    aAccelX = pAccelSensor->X();
                    aAccelY = pAccelSensor->Y();
                    aAccelZ = pAccelSensor->Z();
                    break;
                }

                case SensorOrientation:
                {
                    HALOrientationSensorConnector *pOrientationSensor = static_cast<HALOrientationSensorConnector *>(pSensor);
                    aOrientation = (OrientationEvent::Orientation) pOrientationSensor->getOrientation();
                    break;
                }

                case SensorShake:
                {
                    HALShakeSensorConnector *pShakeSensor = static_cast<HALShakeSensorConnector *>(pSensor);
                    aShakeState     = pShakeSensor->shakeState();
                    aShakeMagnitude = pShakeSensor->shakeMagnitude();
                    break;
                }

                case SensorRotation:
                {
                    HALRotationSensorConnector *pRotationSensor = static_cast<HALRotationSensorConnector *>(pSensor);
                    aRotationPitch = pRotationSensor->pitch();
                    aRotationRoll  = pRotationSensor->roll();
                    break;
                }

                default:
                {
                    g_critical("Mustn't have reached here. [%s]:[%d] !!!!",__PRETTY_FUNCTION__, __LINE__);
                    break;
                }
            }
        }
    }

    AccelerometerEvent* e = new AccelerometerEvent(aOrientation,
                                                   aRotationPitch,
                                                   aRotationRoll,
                                                   aAccelX,
                                                   aAccelY,
                                                   aAccelZ,
                                                   aShakeState,
                                                   aShakeMagnitude);

    // g_debug("Accelerometer event A: floatX: %1.4f, floatY: %1.4f, floatZ: %1.4f, orientation: %d, \npitch: %3.4f, roll: %3.4f, shakeState: %d, shakeMagnitude: %1.4f\n", e->x(), e->y(), e->z(), e->orientation(), e->pitch(), e->roll(), e->shakeState(), e->shakeMagnitude());

    return e;
}

QEvent* HALLogicalAccelerometerSensorConnector::getQSensorData()
{
    return readAllSensorData();
}

/**
 * HAL Logical Orientation sensor connector class
 *  - Orientation
 *  - Rotation
 */
HALLogicalOrientationSensorConnector::HALLogicalOrientationSensorConnector(HALConnectorObserver *aObserver, bool bCanPostEvent)
    : HALLogicalSensorConnectorBase(SensorLogicalOrientation, aObserver, bCanPostEvent)
{
    HALConnectorBase *pSensor = HALConnectorBase::getSensor(SensorOrientation, 0, false);
    if (pSensor)
    {
        m_SensorList.push_back(pSensor);
        connect(pSensor, SIGNAL(sensorDataAvailable()), this, SLOT(logicalSensorDataAvailable()));
        pSensor = 0;
    }

    pSensor = HALConnectorBase::getSensor(SensorRotation, 0, false);
    if (pSensor)
    {
        m_SensorList.push_back(pSensor);
        connect(pSensor, SIGNAL(sensorDataAvailable()), this, SLOT(logicalSensorDataAvailable()));
        pSensor = 0;
    }
}

QEvent* HALLogicalOrientationSensorConnector::readAllSensorData()
{
    OrientationEvent::Orientation   aOrientation    = OrientationEvent::Orientation_Invalid;
    HALConnectorBase               *pSensor         = 0;
    float                           aRotationPitch  = 0.0;
    float                           aRotationRoll   = 0.0;

    std::vector<HALConnectorBase *>::iterator it;

    for (it = m_SensorList.begin() ; it < m_SensorList.end(); ++it )
    {
        pSensor = *it;
        if (pSensor)
        {
            switch (pSensor->type())
            {
                case SensorOrientation:
                {
                    HALOrientationSensorConnector *pOrientationSensor = static_cast<HALOrientationSensorConnector *>(pSensor);
                    aOrientation = (OrientationEvent::Orientation)pOrientationSensor->getOrientation();
                    break;
                }

                case SensorRotation:
                {
                    HALRotationSensorConnector *pRotationSensor = static_cast<HALRotationSensorConnector *>(pSensor);
                    aRotationPitch = pRotationSensor->pitch();
                    aRotationRoll  = pRotationSensor->roll();
                    break;
                }

                default:
                {
                    g_critical("Mustn't have reached here. [%s]:[%d] !!!!",__PRETTY_FUNCTION__, __LINE__);
                    break;
                }
            }
        }
    }

    OrientationEvent* e = new OrientationEvent(aOrientation,
                                               aRotationPitch,
                                               aRotationRoll);

    // g_debug("Logical OrientationEvent event : orientation: %d, pitch: %3.4f, roll: %3.4f", e->orientation(), e->pitch(), e->roll());

    return e;
}

QEvent* HALLogicalOrientationSensorConnector::getQSensorData()
{
    return readAllSensorData();
}

/**
 * HAL Logical Orientation sensor connector class
 *  - Rotation
 *  - Bearing
 *  - Acceleration
 *  - Linear Acceleration
 */
HALLogicalDeviceMotionSensorConnector::HALLogicalDeviceMotionSensorConnector(HALConnectorObserver *aObserver, bool bCanPostEvent)
    : HALLogicalSensorConnectorBase(SensorLogicalMotion, aObserver, bCanPostEvent)
{
    HALConnectorBase *pSensor = HALConnectorBase::getSensor(SensorRotation, 0, false);
    if (pSensor)
    {
        m_SensorList.push_back(pSensor);
        connect(pSensor, SIGNAL(sensorDataAvailable()), this, SLOT(logicalSensorDataAvailable()));
        pSensor = 0;
    }

    pSensor = HALConnectorBase::getSensor(SensorBearing, 0, false);
    if (pSensor)
    {
        m_SensorList.push_back(pSensor);
        connect(pSensor, SIGNAL(sensorDataAvailable()), this, SLOT(logicalSensorDataAvailable()));
        pSensor = 0;
    }

    pSensor = HALConnectorBase::getSensor(SensorAcceleration, 0, false);
    if (pSensor)
    {
        m_SensorList.push_back(pSensor);
        connect(pSensor, SIGNAL(sensorDataAvailable()), this, SLOT(logicalSensorDataAvailable()));
        pSensor = 0;
    }

    pSensor = HALConnectorBase::getSensor(SensorLinearAcceleration, 0, false);
    if (pSensor)
    {
        m_SensorList.push_back(pSensor);
        connect(pSensor, SIGNAL(sensorDataAvailable()), this, SLOT(logicalSensorDataAvailable()));
        pSensor = 0;
    }
}

/**
 * HAL Logical Device Orientation sensor connector class
 *  - Bearing
 *  - Rotation
 */
HALLogicalDeviceOrientationSensorConnector::HALLogicalDeviceOrientationSensorConnector(HALConnectorObserver *aObserver, bool bCanPostEvent)
    : HALLogicalSensorConnectorBase(SensorLogicalOrientation, aObserver, bCanPostEvent)
{
    HALConnectorBase *pSensor = HALConnectorBase::getSensor(SensorRotation, 0, false);
    if (pSensor)
    {
        m_SensorList.push_back(pSensor);
        connect(pSensor, SIGNAL(sensorDataAvailable()), this, SLOT(logicalSensorDataAvailable()));
        pSensor = 0;
    }

    pSensor = HALConnectorBase::getSensor(SensorBearing, 0, false);
    if (pSensor)
    {
        m_SensorList.push_back(pSensor);
        connect(pSensor, SIGNAL(sensorDataAvailable()), this, SLOT(logicalSensorDataAvailable()));
        pSensor = 0;
    }
}

