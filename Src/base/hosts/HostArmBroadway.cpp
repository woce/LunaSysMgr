/**
 * @file
 * 
 * Device-specific functionality for the Broadway devices
 *
 * @author Hewlett-Packard Development Company, L.P.
 * @author tyrok1
 *
 * @section LICENSE
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
 */




#include "Common.h"

#include "HostArm.h"

/**
 * Device-specific functionality for the Broadway devices
 * 
 * Device details:
 * - ARMv7.
 * - No turbo mode.
 * - Switches (2): ringer and slider.
 * 
 * @see https://en.wikipedia.org/wiki/HP_Veer
 */
class HostArmBroadway : public HostArm
{
public:
	/**
	 * Constructs a Broadway device host
	 */
	HostArmBroadway();
	
	/**
	 * Destroys a Broadway device host
	 */
	virtual ~HostArmBroadway();
	
	/**
	 * @copybrief HostArm::hardwareName()
	 * 
	 * Returns one of the following depending on
	 * hardware model:
	 * 
	 * - Broadway EVT1
	 * - Broadway EVT2
	 * - Broadway EVT3
	 * - Broadway DVT1
	 * - Broadway DVT2
	 * - Broadway DVT3
	 * - Broadway -- unknown revision
	 * 
	 * @return				Returns a string starting with "Broadway".  See description for full list.
	 */
	virtual const char* hardwareName() const;

	int getNumberOfSwitches() const;
};

HostArmBroadway::HostArmBroadway()
{
}

HostArmBroadway::~HostArmBroadway()
{
}

const char* HostArmBroadway::hardwareName() const
{
	switch (m_hwRev) {
		case HidHardwareRevisionEVT1:
			return "Broadway EVT1";
		case HidHardwareRevisionEVT2:
			return "Broadway EVT2";
		case HidHardwareRevisionEVT3:
			return "Broadway EVT3";
		case HidHardwareRevisionDVT1:
			return "Broadway DVT1";
		case HidHardwareRevisionDVT2:
			return "Broadway DVT2";
		case HidHardwareRevisionDVT3:
			return "Broadway DVT3";
		default:
			return "Broadway -- unknown revision";
	}
}

int HostArmBroadway::getNumberOfSwitches() const
{
    // broadway only has a ringer and a slider, but no jack, so there is no headset switch.
	return 2;
}
