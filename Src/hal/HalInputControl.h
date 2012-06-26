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




#ifndef _HALINPUTCONTROL_H_
#define _HALINPUTCONTROL_H_

#include "Common.h"
#include "InputControl.h"

class HalInputControl: public InputControl {
public:
    HalInputControl(hal_device_type_t type, hal_device_id_t id);
    virtual ~HalInputControl();

    virtual bool on();
    virtual bool off();
    virtual bool setRate(hal_report_rate_t rate);

    virtual hal_device_handle_t getHandle() { return m_handle; }

private:
    hal_device_handle_t m_handle;
};

#endif /* _HALINPUTCONTROL_H_ */
