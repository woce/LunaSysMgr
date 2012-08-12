/**
 * @file
 * 
 * Device-specific functionality for the Qemu ARM emulator
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

#include <QObject>
#include <QWidget>
#include <QtDebug>
#include <QGraphicsView>

/**
 * Event filter which translates PC keyboard keys to simulated hardware device buttons
 */
class HostArmQemuKeyFilter : public QObject
{

protected:
	/**
	 * Translates a KeyPress or KeyRelease event from a keyboard key to a hardware button and posts the translated event to the queue
	 * 
	 * The keys which are translated are as follows:
	 * - Left arrow key = Previous button
	 * - Right arrow key = Next button
	 * - Home key = Home button
	 * - Escape key = Back button
	 * - End key = Launcher button
	 * - Pause/Break key = Power button
	 * - F6 key = simulate orientation with the top of the screen facing up
	 * - F7 key = simulate orientation with the right of the screen facing up
	 * - F8 key = simulate orientation with the bottom of the screen facing up
	 * - F9 key = simulate orientation with the left of the screen facing up
	 * 
	 * @param	obj			Object the KeyPress/KeyRelease event was sent to, presumably.
	 * @param	event			Event which, if it is a KeyPress or KeyRelease event that we know how to translate, is transleted.
	 * @return				true if we translated the event, false if we did not.
	 */
	bool eventFilter(QObject* obj, QEvent* event)
	{
		bool handled = false;
		if ((event->type() == QEvent::KeyPress))
		{
			QWidget* window = NULL;
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

			switch (keyEvent->key())
			{
			case Qt::Key_Left:
				window = QApplication::focusWidget();
				if (window) {
					QApplication::postEvent(window, new QKeyEvent(QEvent::KeyPress, Qt::Key_CoreNavi_Previous, 0));
				}
				handled = true;
				break;
			case Qt::Key_Right:
				window = QApplication::focusWidget();
				if (window) {
					QApplication::postEvent(window, new QKeyEvent(QEvent::KeyPress, Qt::Key_CoreNavi_Next, 0));
				}
				handled = true;
				break;
			case Qt::Key_Home:
				window = QApplication::focusWidget();
				if (window) {
					QApplication::postEvent(window, new QKeyEvent(QEvent::KeyPress, Qt::Key_CoreNavi_Home, 0));
				}
				handled = true;
				break;
			case Qt::Key_Escape:
				window = QApplication::focusWidget();
				if (window) {
					QApplication::postEvent(window, new QKeyEvent(QEvent::KeyPress, Qt::Key_CoreNavi_Back, 0));
				}
				handled = true;
				break;
			case Qt::Key_End:
				window = QApplication::focusWidget();
				if (window) {
					QApplication::postEvent(window, new QKeyEvent(QEvent::KeyPress, Qt::Key_CoreNavi_Launcher, 0));
				}
				handled = true;
				break;
			case Qt::Key_Pause:
				window = QApplication::focusWidget();
				if (window) {
					QApplication::postEvent(window, new QKeyEvent(QEvent::KeyPress, Qt::Key_Power, keyEvent->modifiers()));
				}
				handled = true;
				break;
			case Qt::Key_F6:
				window = QApplication::focusWidget();
				if (window) {
                    QApplication::postEvent(window, new OrientationEvent(OrientationEvent::Orientation_Up));
				}
				handled = true;
				break;

			case Qt::Key_F7:
				window = QApplication::focusWidget();
				if (window) {
                    QApplication::postEvent(window, new OrientationEvent(OrientationEvent::Orientation_Right));
				}
				handled = true;
				break;

			case Qt::Key_F8:
				window = QApplication::focusWidget();
				if (window) {
                    QApplication::postEvent(window, new OrientationEvent(OrientationEvent::Orientation_Down));
				}
				handled = true;
				break;

			case Qt::Key_F9:
				window = QApplication::focusWidget();
				if (window) {
                    QApplication::postEvent(window, new OrientationEvent(OrientationEvent::Orientation_Left));
				}
				handled = true;
				break;
			}
		} else if (event->type() == QEvent::KeyRelease) {
			QWidget *window = NULL;
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

			switch (keyEvent->key()) {
			case Qt::Key_Left:
				window = QApplication::focusWidget();
				if (window) {
					QApplication::postEvent(window, new QKeyEvent(QEvent::KeyRelease, Qt::Key_CoreNavi_Previous, 0));
				}
				handled = true;
				break;
			case Qt::Key_Right:
				window = QApplication::focusWidget();
				if (window) {
					QApplication::postEvent(window, new QKeyEvent(QEvent::KeyRelease, Qt::Key_CoreNavi_Next, 0));
				}
				handled = true;
				break;
			case Qt::Key_Home:
				window = QApplication::focusWidget();
				if (window) {
					QApplication::postEvent(window, new QKeyEvent(QEvent::KeyRelease, Qt::Key_CoreNavi_Home, 0));
				}
				handled = true;
				break;
			case Qt::Key_Escape:
				window = QApplication::focusWidget();
				if (window) {
					QApplication::postEvent(window, new QKeyEvent(QEvent::KeyRelease, Qt::Key_CoreNavi_Back, 0));
				}
				handled = true;
				break;
			case Qt::Key_End:
				window = QApplication::focusWidget();
				if (window) {
					QApplication::postEvent(window, new QKeyEvent(QEvent::KeyRelease, Qt::Key_CoreNavi_Launcher, 0));
				}
				handled = true;
				break;
			case Qt::Key_Pause:
				window = QApplication::focusWidget();
				if (window) {
					QApplication::postEvent(window, new QKeyEvent(QEvent::KeyRelease, Qt::Key_Power, keyEvent->modifiers()));
				}
				handled = true;
				break;
			}
		}

		return handled;
	}
};

/**
 * Device-specific functionality for the Qemu ARM emulator
 * 
 * Device details:
 * - Requires a bunch of key remapping.
 * - Switches (0)
 * - Translates a bunch of desktop keys to hardware keycodes.   See {@link HostArmQemuKeyFilter HostArmQemuKeyFilter} for information on which keys are remapped to which phone keycodes.
 * 
 * @see HostArmQemuKeyFilter
 */
class HostArmQemu : public HostArm
{
public:
	/**
	 * Constructs a Qemu device host
	 */
	HostArmQemu();
	
	/**
	 * Destroys a Qemu device host
	 */
	virtual ~HostArmQemu();

	/**
	 * @copybrief HostArm::hardwareName()
	 * 
	 * @return				Returns the string "ARM Emulator".
	 */
	virtual const char* hardwareName() const{ return "ARM Emulator"; }
	
	//Documented in parent
	virtual void setCentralWidget(QWidget* view);

	// switches aren't available in the emulator
	virtual void getInitialSwitchStates() { }
	
	//Documented in parent
	virtual int getNumberOfSwitches() const { return 0; }

private:
	/**
	 * The key filter that remaps desktop keybaord keys to phone keycodes
	 */
	HostArmQemuKeyFilter* m_keyFilter;
};


HostArmQemu::HostArmQemu()
	: m_keyFilter(NULL)
{
}

HostArmQemu::~HostArmQemu()
{
	delete m_keyFilter;
}


void HostArmQemu::setCentralWidget(QWidget* view)
{
	m_keyFilter = new HostArmQemuKeyFilter;
	view->installEventFilter(m_keyFilter);
	view->show();
}

