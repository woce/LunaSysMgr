/**
 * @file
 * 
 * Device-specific functionality for the never-released and rather mysterious Chile device(s)
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
 * Device-specific functionality for the never-released and rather mysterious Chile device(s)
 * 
 * Device details:
 * - Unknown instruction set
 * - No turbo mode
 * - Switches (1): headphones inserted.
 */
class HostArmChile : public HostArm
{
public:
	/**
	 * Constructs a Chile device host
	 */
	HostArmChile();
	
	/**
	 * Destroys a Chile device host
	 */
	virtual ~HostArmChile();

	
	/**
	 * @copybrief HostArm::hardwareName()
	 * 
	 * @return				Returns a string "Chile".
	 */
	virtual const char* hardwareName() const;

	int getNumberOfSwitches() const;
};

HostArmChile::HostArmChile()
{
}

HostArmChile::~HostArmChile()
{
}

const char* HostArmChile::hardwareName() const
{
	return "Chile";
}

int HostArmChile::getNumberOfSwitches() const
{
	/* headset only, no slider or ringer */
	return 1;
}
