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




#include "StatusBarSearch.h"
#include "Settings.h"
#include "Preferences.h"
#include "HostBase.h"
#include "StatusBarServicesConnector.h"
#include "SystemUiController.h"

#include <QPainter>

StatusBarSearch::StatusBarSearch()
{
	int width, height;

	Settings* settings = Settings::LunaSettings();
	
	std::string statusBarImagesPath = settings->lunaSystemResourcesPath + "/launcher3/";
	
	std::string imagePath = statusBarImagesPath + "search-button-launcher.png";
	
	m_pixmap = QPixmap(imagePath.c_str());
	
	width  = 28;
	height = 28;

	m_bounds = QRectF(-width/2, -height/2, width, height);
}

StatusBarSearch::~StatusBarSearch()
{

}

QRectF StatusBarSearch::boundingRect() const
{
	return m_bounds;
}

void StatusBarSearch::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	if(Preferences::instance()->sysUiEnableStatusBarSearch())
	{
		if (!m_pixmap.isNull()) {
			painter->setRenderHint(QPainter::SmoothPixmapTransform); //Resize the icon smoothly
			painter->scale(0.9,0.9); //Scale the icon down to fit inside the StatusBar
			painter->drawPixmap(m_bounds.x() + (m_bounds.width() - m_pixmap.width())/2.0,
								m_bounds.y() + (m_bounds.height() - m_pixmap.height())/2.0,
								m_pixmap);
		}
	}
}