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




#ifndef CARDWINDOWMANAGER_H
#define CARDWINDOWMANAGER_H

#include "Common.h"

#include "WindowManagerBase.h"
#include "Preferences.h"
#include "BezelGesture.h"

#include <QStateMachine>
#include <QGraphicsSceneMouseEvent>
#include <QGestureEvent>
#include <QParallelAnimationGroup>
#include <QMap>
#include <QTimer>
#include <QSignalMapper>
#include <QEasingCurve>
#include <stdint.h>

class CardWindowManagerState;
class MinimizeState;
class GroupState;
class MaximizeState;
class PreparingState;
class LoadingState;
class FocusState;
class ReorderState;
class SwitchGestureState;
class MinimizeGestureState;
class SpreadGestureState;

QT_BEGIN_NAMESPACE
class QTapGesture;
class QTapAndHoldGesture;
class QPropertyAnimation;
QT_END_NAMESPACE

class CardWindow;
class CardGroup;


class CardWindowManager : public WindowManagerBase
{
	Q_OBJECT

public:
	static CardWindowManager* instance();

	CardWindowManager(int maxWidth, int maxHeight);
	virtual ~CardWindowManager();

	virtual void init();

	virtual void addWindow(Window* win);
	virtual void prepareAddWindow(Window* win);
	virtual void addWindowTimedOut(Window* win);
	virtual void removeWindow(Window* win);
	virtual void focusWindow(Window* win);

	CardWindow* activeWindow() const;
	CardGroup* activeGroup() const;

	void setInModeAnimation(bool animating);
	bool isLastWindowAddedModal() const { return m_addingModalWindow; }
	void resize(int width, int height);
	bool isModalDismissed() const {return m_modalDimissed; }
	void setModalDismissed(bool val) { m_modalDimissed = val; }
	void stopMoveGroupTimer() {m_groupMoveTimer->stop();}
	virtual bool okToResize();
	virtual bool isMinimized();
	virtual bool isMaximized();
	virtual bool isGroup();
	
	// cycle through cards in tabbed view
	void cycleGroupTabs();
	
	// convert tap events on the gesture border to single clicks
	void deadzoneTap(QTapGesture* event);

	CardWindow* modalParent() const { return m_parentOfModalCard; }

    void firstCardAlert();

    virtual bool handleNavigationEvent(QKeyEvent* keyEvent, bool& propogate);

protected:

	virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
	virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
	void tapGestureEvent(QTapGesture* event);
	void tapAndHoldGestureEvent(QTapAndHoldGesture* event);
	void flickGestureEvent(QGestureEvent* event);

	virtual bool sceneEvent(QEvent* event);

private Q_SLOTS:

	void slotPositiveSpaceAboutToChange(const QRect& r, bool fullScreen, bool screenResizing);
	void slotPositiveSpaceChangeFinished(const QRect& r);
	void slotPositiveSpaceChanged(const QRect& r);
	void slotLauncherVisible(bool val, bool fullyVisible);
	void slotLauncherShown(bool val);

	void slotAnimationsFinished();
	void slotDeletedAnimationFinished();
    void slotTouchToShareAnimationFinished();

	void slotMaximizeActiveCardWindow();
	void slotMinimizeActiveCardWindow();
    
	void slotChangeCardWindow(bool next);
	void slotSideSwipe(bool direction);  // Enables group view, for now

	void slotFocusMaximizedCardWindow(bool focus);
    
	void slotSwitchGesture(BezelGesture* gesture);
    
	void slotMinimizeGesture(BezelGesture* gesture);
    
	void slotSpreadGesture(QPinchGesture* gesture);

    void slotTouchToShareAppUrlTransfered(const std::string& appId);
    void slotOpacityAnimationFinished();
    void slotDismissActiveModalWindow();
    void slotDismissModalTimerStopped();

	void slotCloseGroupAdjustAfterAnimationFinished(QObject* winObject); // Adjusts group view if just one card remains after side closing
	void slotGroupTimerTimeout();  // Handles kinetic scrolling for tabbed cards

    void slotFinishFlyback();  // Finishes the animation for infinite card cycling option

Q_SIGNALS:

	void signalFocusWindow(CardWindow* win);
	void signalLoadingActiveWindow();
	void signalPreparingWindow(CardWindow* win);
	void signalMaximizeActiveWindow();
	void signalMinimizeActiveWindow();
	void signalEnterReorder(QPoint pt, int slice);
	void signalExitReorder(bool canceled = true);
	void signalSwitchGesture(BezelGesture* gesture);
	void signalEnterSwitchGestureState();
	void signalMinimizeGesture(BezelGesture* gesture);
	void signalEnterMinimizeGestureState();
	void signalSpreadGesture(QPinchGesture* gesture);
	void signalEnterSpreadGestureState();
    void signalFirstCardRun();
    void signalGroupWindow();

private:

	void performPostModalWindowRemovedActions(Window* win, bool restore = true);
	void initiateRemovalOfActiveModalWindow();

	void handleMousePressMinimized(QGraphicsSceneMouseEvent* event);

	void handleMouseMoveMinimized(QGraphicsSceneMouseEvent* event);
	void handleMouseMoveReorder(QGraphicsSceneMouseEvent* event);

	void handleMouseReleaseMinimized(QGraphicsSceneMouseEvent* event);
	void handleMouseReleaseReorder(QGraphicsSceneMouseEvent* event);
    
    void handleSwitchGesture(BezelGesture* gesture);
    
    void handleMinimizeGesture(BezelGesture* gesture);
    
    void handleSpreadGesture(QPinchGesture* gesture);

	void handleFlickGestureMinimized(QGestureEvent* event);

	void handleTapGestureMinimized(QTapGesture* event);

	void handleTapAndHoldGestureMinimized(QTapAndHoldGesture* event);

	// For tabbed cards input handling
	void handleMousePressGroup(QGraphicsSceneMouseEvent* event);
	void handleMouseMoveGroup(QGraphicsSceneMouseEvent* event);
	void handleMouseReleaseGroup(QGraphicsSceneMouseEvent* event);
	void handleTapGestureGroup(QTapGesture* event);
	void handleFlickGestureGroup(QGestureEvent* event);
	void handleKeyNavigationGroup(QKeyEvent* keyEvent);


    void handleKeyNavigationMinimized(QKeyEvent* keyEvent);

	void setCurrentState(CardWindowManagerState* newState) { m_curState = newState; }

	void maximizeActiveWindow(bool animate=true);
	void minimizeActiveWindow(bool animate=true);

	// places the active window just off the bottom of the screen
	// either unscaled or scaled
	void setActiveCardOffScreen(bool fullsize=true);

	QRect normalOrScreenBounds(CardWindow* win) const;
	QRect targetPositiveSpace() const { return m_targetPositiveSpace; }

	void prepareAddWindowSibling(CardWindow* win);
	void addWindowTimedOutNormal(CardWindow* win);

	void removeCardFromGroupMaximized(CardWindow* win);
	void removeCardFromGroup(CardWindow* win, bool adjustLayout=true, bool dir = false);

	void closeWindow(CardWindow* win, bool angryCard=false);

	CardGroup* groupClosestToCenterHorizontally() const;

	void enterReorder(QPoint pt);
	void cycleReorderSlot();
	void moveReorderSlotRight();
	void moveReorderSlotLeft();
	void moveReorderSlotCenter(QPointF pt);
	void arrangeWindowsAfterReorderChange(int duration, QEasingCurve::Type curve);

	// animate to the group next to the active group
	void switchToNextGroup();
	// animate to the group before the current active group
	void switchToPrevGroup();

	void switchToNextApp();
	void switchToPrevApp();
	void switchToNextAppMaximized();
	void switchToPrevAppMaximized();

	/* Functions Required for Grouped Cards */
	void moveGroupCards();
	bool whichSideOfScreen(QPointF p);
	void closeWindowGroup(CardWindow* win, bool dir, bool angryCard=false);

	// animate all groups to center around the active group.
	// optionally include the active card in the active group.
	// NOTE: If you exclude the active card, the animations will
	// not be started automatically, you will have to call m_anims.start()
	void slideAllGroups(bool includeActiveCard = true, int flyback = 0);
	void slideAllGroupsTo(int xOffset);

	// Does the same as slideAllGroups, but sets the positions immediately,
	// with no animations
	void layoutAllGroups(bool includeActiveCard = true);

	// immediately update the positions of all groups in the closed
	// orientation, shift the centers of all groups by xDiff
	void layoutGroups(qreal xDiff);

	// slide all non-active groups to their closed positions
	// and snap the active group to the closest card to the center
	// of the group
	void slideToActiveCard();

	void setActiveGroup(CardGroup* group);
    
    void setGroupsStack();
    void setGroupsLinear();
    void setGroupsMinimize();
    void setGroupsTab();
    
    void setGroupsMiniScale(qreal scale);

	void disableCardRestoreToMaximized();
	void restoreCardToMaximized();

	void cancelReorder(bool dueToPenCancel=false);

	void queueFocusAction(CardWindow* win, bool focused);
	void performPendingFocusActions();

    void queueTouchToShareAction(CardWindow* win);
    void performPendingTouchToShareActions();

	void removePendingActionWindow(CardWindow* win);

	void setAnimationForWindow(CardWindow* win, QPropertyAnimation* anim);
	void removeAnimationForWindow(CardWindow* win, bool includeDeletedAnimations=false);
	bool windowHasAnimation(CardWindow* win) const;

	void setAnimationForGroup(CardGroup* group, QPropertyAnimation* anim);
	void removeAnimationForGroup(CardGroup* group);
	bool groupHasAnimation(CardGroup* group) const;
	void startAnimations();
	void clearAnimations();

	void updateAllowWindowUpdates();
	int proceedToAddModalWindow(CardWindow* win);

	void removeWindowNoModality(CardWindow* win);
	void removeWindowWithModality(CardWindow* win);
	bool performCommonWindowRemovalTasks(CardWindow* w, bool checkCardGroup=true);
	void resetMouseTrackState();
	void resetModalFlags(bool forceReset=false);
	void handleModalRemovalForDeletedParent(CardWindow* w);

    bool playAngryCardSounds() const;
    void updateAngryCardThreshold();

        void markFirstCardDone();

	QSet<CardWindow*> m_pendingActionWinSet;
    QSet<CardWindow*> m_pendingTouchToShareWinSet;

	QVector<CardGroup*> m_groups;
	CardGroup* m_activeGroup;

	QRect m_normalScreenBounds;
	QRect m_targetPositiveSpace;

	CardWindow* m_draggedWin;
	bool m_penDown;

	CardWindow* m_cardToRestoreToMaximized;
	CardWindow* m_parentOfModalCard;
	bool m_lowResMode;
	bool m_addingModalWindow;
	bool m_initModalMaximizing;
	bool m_modalDismissInProgress;
	bool m_animateWindowForModalDismisal;
	int m_dismissModalImmediately;
	bool m_modalDimissed;
	bool m_dismissedFirstCard;


	enum ModalWindowState {
		NoModalWindow = 0,
		ModalWindowAddInitCheckFail,
		ModalWindowDismissedParentSwitched,
		ModalWindowDismissedInternally,
		ModalWindowDismissedExternally,
		ModalParentDismissed,
		ModalParentDimissedWaitForChildDismissal
	};

	ModalWindowState m_modalWindowState;

	enum NotifySystemUiControllerAction {
		Invalid=0,
		ModalLaunch,
		ModalDismissNoAnimate,
		ModalDismissAnimate
	};

	void notifySysControllerOfModalStatus(int reason, bool restore, NotifySystemUiControllerAction type, Window* win = NULL);

	enum Movement {
		MovementUnlocked, // not locked to an axis
		MovementHLocked,  // locked to horizontal movement
		MovementVLocked	  // locked to vertical movement
	};
	Movement m_movement;
	bool m_trackWithinGroup;
	bool m_seenFlickOrTap;
    int m_activeGroupPivot;

	enum ReorderZone {
		ReorderZone_Center = 0,
		ReorderZone_Left,
		ReorderZone_Right
	};
	ReorderZone m_reorderZone;
	ReorderZone getReorderZone(QPoint pt);

	QStateMachine* m_stateMachine;
	MinimizeState* m_minimizeState;
  GroupState* m_groupState;
	MaximizeState* m_maximizeState;
	PreparingState* m_preparingState;
	LoadingState* m_loadingState;
	FocusState* m_focusState;
	ReorderState* m_reorderState;
	SwitchGestureState* m_switchGestureState;
	MinimizeGestureState* m_minimizeGestureState;
	SpreadGestureState* m_spreadGestureState;

	CardWindowManagerState* m_curState;

	// keep track of which windows have animations running
	QMap<CardWindow*,QPropertyAnimation*> m_cardAnimMap;
	QMap<CardGroup*,QPropertyAnimation*> m_groupAnimMap;

	// Collection of all running animations (Window + Group)
	// It is the union of the animations mapped above
	QParallelAnimationGroup m_anims;
	QMap<CardWindow*,QPropertyAnimation*> m_deletedAnimMap;

  /* Variables for Tabbed Cards Implementation */
  bool m_groupMove;
  int m_groupShift;
  int m_groupInitialMove;
  bool m_groupMoveDir;
  bool m_groupDir;
  float m_groupVelocity;
  QTimer *m_groupMoveTimer;
  
	// Mini Cards Scale
	qreal m_miniScale;

    bool m_playedAngryCardStretchSound;

	bool m_animationsActive;

	friend class CardWindowManagerState;
	friend class MinimizeState;
	friend class GroupState;
	friend class MaximizeState;
	friend class PreparingState;
	friend class LoadingState;
	friend class FocusState;
	friend class ReorderState;
	friend class SwitchGestureState;
	friend class MinimizeGestureState;
	friend class SpreadGestureState;
};

#endif /* CARDWINDOWMANAGER_H */
