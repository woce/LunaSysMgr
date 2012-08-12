/**
 * @file
 * 
 * Device-specific functionality for the never-released Windsor device(s)
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




#include "Common.h"

#include "HostArm.h"

/**
 * Device-specific functionality for the never-released Windsor device(s)
 * 
 * Device details:
 * - Little to nothing is known about this device.
 */
class HostArmWindsor : public HostArm
{
public:
	/**
	 * Constructs a Windsor device host
	 */
	HostArmWindsor();
	
	/**
	 * Destroys a Windsor device host
	 */
	virtual ~HostArmWindsor();

	/**
	 * @copybrief HostArm::hardwareName()
	 * 
	 * Returns one of the following depending on
	 * hardware model:
	 * 
	 * - Windsor EVT1
	 * - Windsor EVT2
	 * - Windsor EVT3
	 * - Windsor DVT1
	 * - Windsor DVT2
	 * - Windsor DVT3
	 * - Windsor -- unknown revision
	 * 
	 * @return				Returns a string starting with "Windsor".  See description for full list.
	 */
	virtual const char* hardwareName() const;
};

HostArmWindsor::HostArmWindsor()
{
}

HostArmWindsor::~HostArmWindsor()
{
}

const char* HostArmWindsor::hardwareName() const
{
	switch (m_hwRev) {
		case HidHardwareRevisionEVT1:
			return "Windsor EVT1";
		case HidHardwareRevisionEVT2:
			return "Windsor EVT2";
		case HidHardwareRevisionEVT3:
			return "Windsor EVT3";
		case HidHardwareRevisionDVT1:
			return "Windsor DVT1";
		case HidHardwareRevisionDVT2:
			return "Windsor DVT2";
		case HidHardwareRevisionDVT3:
			return "Windsor DVT3";
		default:
			return "Windsor -- unknown revision";
	}
}

