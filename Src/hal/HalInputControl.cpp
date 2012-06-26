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




#include "hal/HalInputControl.h"
#include <glib.h>

HalInputControl::HalInputControl(hal_device_type_t type, hal_device_id_t id) : m_handle(0)
{
    hal_error_t error = hal_device_open(type, id, &m_handle);
    if ((error != HAL_ERROR_SUCCESS) || (m_handle == NULL))
    {
        g_critical("Failed to open HAL device %d: %d", type, error);
    }
}

HalInputControl::~HalInputControl()
{
    if (m_handle)
    {
        hal_error_t error = hal_device_close(m_handle);
        if (error != HAL_ERROR_SUCCESS)
            g_critical("Unable to release m_handle");
    }
}

bool HalInputControl::on()
{
    if (m_handle)
    {
        hal_error_t error = HAL_ERROR_SUCCESS;
        error = hal_device_set_operating_mode(m_handle, HAL_OPERATING_MODE_ON);
        return (error == HAL_ERROR_SUCCESS || error == HAL_ERROR_NOT_IMPLEMENTED);
    }
    return true;
}

bool HalInputControl::off()
{
    if (m_handle)
    {
        hal_error_t error = HAL_ERROR_SUCCESS;
        error = hal_device_set_operating_mode(m_handle, HAL_OPERATING_MODE_OFF);
        return (error == HAL_ERROR_SUCCESS || error == HAL_ERROR_NOT_IMPLEMENTED);
    }
    return true;
}

bool HalInputControl::setRate(hal_report_rate_t rate)
{
    if (m_handle)
    {
        hal_error_t error = HAL_ERROR_SUCCESS;
        error = hal_device_set_report_rate(m_handle, rate);
        return (error == HAL_ERROR_SUCCESS || error == HAL_ERROR_NOT_IMPLEMENTED);
    }
    return true;
}
