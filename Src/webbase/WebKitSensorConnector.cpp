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




#include "WebKitSensorConnector.h"

WebKitSensorConnector::WebKitSensorConnector(Palm::SensorType aSensorType, Palm::fnSensorDataCallback aDataCB, Palm::fnSensorErrorCallback aErrCB, void *pUserData)
    : m_PalmSensorType(aSensorType)
    , m_DataCB(aDataCB)
    , m_ErrCB(aErrCB)
    , m_UserData(pUserData)
{
#if defined (TARGET_DEVICE)
    m_HALSensorType = WebKitToHAL(aSensorType);

    // Create the sensor
    m_Sensor = HALConnectorBase::getSensor(m_HALSensorType, this);

    // Immediately switch-off the sensor, as HAL starts sending the HAL data immediately
    off();
#endif
}


WebKitSensorConnector::~WebKitSensorConnector()
{
#if defined (TARGET_DEVICE)
    if (m_Sensor)
    {
        m_Sensor->scheduleDeletion();
    }
    m_Sensor = 0;
#endif
}

WebKitSensorConnector* WebKitSensorConnector::createSensor(Palm::SensorType aType, Palm::fnSensorDataCallback aDataCB, Palm::fnSensorErrorCallback aErrCB, void *pUserData)
{
    WebKitSensorConnector *pWebKitSensor = 0;

#if defined (TARGET_DEVICE)
    if ((aDataCB) && (aErrCB))
    {
        HALConnectorBase::Sensor halSensorType = WebKitToHAL(aType);
        if (HALConnectorBase::SensorIllegal != halSensorType)
        {
            pWebKitSensor = new WebKitSensorConnector(aType, aDataCB, aErrCB, pUserData);
        }
    }
#endif

    return pWebKitSensor;
}

#if defined (TARGET_DEVICE)
HALConnectorBase::Sensor WebKitSensorConnector::WebKitToHAL(Palm::SensorType aSensorType)
{
    HALConnectorBase::Sensor mappedSensor = HALConnectorBase::SensorIllegal;

    switch(aSensorType)
    {
        case Palm::SensorAcceleration:
        {
            mappedSensor = HALConnectorBase::SensorAcceleration;
            break;
        }

        case Palm::SensorOrientation:
        {
            mappedSensor = HALConnectorBase::SensorOrientation;
            break;
        }

        case Palm::SensorShake:
        {
            mappedSensor = HALConnectorBase::SensorShake;
            break;
        }

        case Palm::SensorBearing:
        {
            mappedSensor = HALConnectorBase::SensorBearing;
            break;
        }

        case Palm::SensorALS:
        {
            mappedSensor = HALConnectorBase::SensorALS;
            break;
        }

        case Palm::SensorAngularVelocity:
        {
            mappedSensor = HALConnectorBase::SensorAngularVelocity;
            break;
        }

        case Palm::SensorGravity:
        {
            mappedSensor = HALConnectorBase::SensorGravity;
            break;
        }

        case Palm::SensorLinearAcceleration:
        {
            mappedSensor = HALConnectorBase::SensorLinearAcceleration;
            break;
        }

        case Palm::SensorMagneticField:
        {
            mappedSensor = HALConnectorBase::SensorMagneticField;
            break;
        }

        case Palm::SensorScreenProximity:
        {
            mappedSensor = HALConnectorBase::SensorScreenProximity;
            break;
        }

        case Palm::SensorRotation:
        {
            mappedSensor = HALConnectorBase::SensorRotation;
            break;
        }

        case Palm::SensorLogicalDeviceOrientation:
        {
            mappedSensor = HALConnectorBase::SensorLogicalDeviceOrientation;
            break;
        }

        case Palm::SensorLogicalDeviceMotion:
        {
            mappedSensor = HALConnectorBase::SensorLogicalMotion;
            break;
        }

        default:
        {
            g_critical("[%s : %d] : Mustn't have reached here : Sensor Type : [%d]", __PRETTY_FUNCTION__, __LINE__, aSensorType);
            break;
        }
    }

    return mappedSensor;
}

HALConnectorBase::SensorReportRate WebKitSensorConnector::WebKitToHAL(Palm::SensorRate aRate)
{
    HALConnectorBase::SensorReportRate mappedRate = HALConnectorBase::SensorReportRateDefault;

    switch(aRate)
    {
        case Palm::RATE_DEFAULT:
        {
            mappedRate = HALConnectorBase::SensorReportRateDefault;
            break;
        }

        case Palm::RATE_LOW:
        {
            mappedRate = HALConnectorBase::SensorReportRateLow;
            break;
        }

        case Palm::RATE_MEDIUM:
        {
            mappedRate = HALConnectorBase::SensorReportRateMedium;
            break;
        }

        case Palm::RATE_HIGH:
        {
            mappedRate = HALConnectorBase::SensorReportRateHigh;
            break;
        }

        case Palm::RATE_HIGHEST:
        {
            mappedRate = HALConnectorBase::SensorReportRateHighest;
            break;
        }

        default:
        {
            g_critical("[%s : %d] : Mustn't have reached here : Sensor Type : Sensor Type.", __PRETTY_FUNCTION__, __LINE__);
            break;
        }
    }

    return mappedRate;
}

void WebKitSensorConnector::HALDataAvailable (HALConnectorBase::Sensor aSensorType)
{
    if ((HALConnectorBase::SensorIllegal != aSensorType) && (m_Sensor) && (m_DataCB))
    {
        std::string jsonData = m_Sensor->toJSONString();

        m_DataCB(m_PalmSensorType, jsonData, m_UserData);
    }
}

#endif

bool WebKitSensorConnector::on()
{
    bool bRetValue = false;

#if defined (TARGET_DEVICE)
    if (m_Sensor)
    {
        bRetValue = m_Sensor->on();
        if (!bRetValue)
        {
            g_critical("[%s : %d] : Critical Error Occurred while trying to turn the sensor on : Sensor Type : [%d]", __PRETTY_FUNCTION__, __LINE__, m_HALSensorType);
        }
    }
#endif

    return bRetValue;
}


bool WebKitSensorConnector::off()
{
    bool bRetValue = false;

#if defined (TARGET_DEVICE)
    if (m_Sensor)
    {
        bRetValue = m_Sensor->off();
        if (!bRetValue)
        {
            g_critical("[%s : %d] : Critical Error Occurred while trying to turn the sensor off : Sensor Type : [%d]", __PRETTY_FUNCTION__, __LINE__, m_HALSensorType);
        }
    }
#endif

    return bRetValue;

}

bool WebKitSensorConnector::setRate(Palm::SensorRate aRate)
{
    bool bRetValue = false;

#if defined (TARGET_DEVICE)
    if (m_Sensor)
    {
        HALConnectorBase::SensorReportRate sensorRate = WebKitToHAL(aRate);
        if ((HALConnectorBase::SensorReportRateUnknown != sensorRate) &&
            (HALConnectorBase::SensorReportRateCount   != sensorRate))
        {
            bRetValue = m_Sensor->setRate(sensorRate);
            if (!bRetValue)
            {
                g_critical("[%s : %d] : Critical Error Occurred while trying to set the sensor rate : Sensor Type : [%d]", __PRETTY_FUNCTION__, __LINE__, m_HALSensorType);
            }
        }
    }
#endif

    return bRetValue;
}

std::string WebKitSensorConnector::getSupportedSensors()
{
    std::string strSensorList = "";

#if defined (TARGET_DEVICE)
    strSensorList = HALConnectorBase::getSupportedSensors(true);
    if (strSensorList.empty())
    {
        g_critical("[%s : %d] : Critical Error Occurred while trying to get the Sensor List: Sensor Type", __PRETTY_FUNCTION__, __LINE__);
    }
#endif

    return strSensorList;
}

void WebKitSensorConnector::CallErrorCB(std::string aMsg)
{
#if defined (TARGET_DEVICE)
    if ((m_Sensor) && (m_ErrCB))
    {
        m_ErrCB(m_PalmSensorType, aMsg, m_UserData);
    }
#endif
}


