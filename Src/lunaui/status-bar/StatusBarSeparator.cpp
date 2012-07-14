/* @@@LICENSE
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
* LICENSE@@@ */


#include "StatusBarSeparator.h"
#include "Settings.h"
#include "HostBase.h"
#include "StatusBarServicesConnector.h"
#include "SystemUiController.h"
#include "SoundPlayerPool.h"

#include <QPainter>

StatusBarSeparator::StatusBarSeparator()
{
	int width, height;

	Settings* settings = Settings::LunaSettings();
	
	std::string statusBarImagesPath = settings->lunaSystemResourcesPath + "/launcher3/";
	
	std::string imagePath = statusBarImagesPath + "search-button-launcher.png";
	
	m_pixmap = QPixmap(imagePath.c_str());
	
	width  = 4096;
	height = 32;

	m_bounds = QRectF(-width/2, -height/2, width, height);

	// implicit assumption that the dimensions of the errorPixmap is the same as the others
}

StatusBarSeparator::~StatusBarSeparator()
{

}

QRectF StatusBarSeparator::boundingRect() const
{
	return m_bounds;
}

void StatusBarSeparator::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	if (!m_pixmap.isNull()) {
		painter->setOpacity(0.0);
		painter->drawPixmap(m_bounds.x() + (m_bounds.width() - m_pixmap.width())/2.0,
							m_bounds.y() + (m_bounds.height() - m_pixmap.height())/2.0,
							m_pixmap);
	}
}