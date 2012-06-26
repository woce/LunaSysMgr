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




#ifndef __ProcessBase_h__
#define __ProcessBase_h__

#include "Common.h"

#include <string>


class ProcessBase
{
	public:
		ProcessBase() { }
		virtual ~ProcessBase() { }
		
		virtual void		setProcessId( const std::string& inId ) { m_procId=inId; }
		const std::string&	processId() const { return m_procId; }
		virtual void		setAppId( const std::string& inId ) { m_appId=inId; }
		const std::string&	appId() const { return m_appId; }
		void				setLaunchingAppId(const std::string& id) { m_launchingAppId=id; }
		const std::string&  launchingAppId() const { return m_launchingAppId; }
		void 				setLaunchingProcessId(const std::string& id) { m_launchingProcId = id; }
		const std::string&	launchingProcessId() const { return m_launchingProcId; }
	
	private:	
		std::string			m_procId;
		std::string			m_appId;
		std::string			m_launchingAppId;
		std::string 		m_launchingProcId;
};


#endif

