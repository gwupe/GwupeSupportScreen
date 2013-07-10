// Copyright (C) 2009,2010,2011,2012 GlavSoft LLC.
// All rights reserved.
//
//-------------------------------------------------------------------------
// This file is part of the TightVNC software.  Please visit our Web site:
//
//                       http://www.tightvnc.com/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//-------------------------------------------------------------------------
//

#include "LocalUpdateHandler.h"
#include "Poller.h"
#include "ConsolePoller.h"
#include "HooksUpdateDetector.h"
#include "MouseShapeDetector.h"
#include "server-config-lib/Configurator.h"
#include "gui/WindowFinder.h"
#include "ScreenDriverFactory.h"

LocalUpdateHandler::LocalUpdateHandler(UpdateListener *externalUpdateListener,
                                       LogWriter *log)
: m_externalUpdateListener(externalUpdateListener),
  m_fullUpdateRequested(false)
{
  // FIXME: Maybe the UpdateKeeper constructor can be empty?
  m_updateKeeper = new UpdateKeeper();
  m_screenDriver = ScreenDriverFactory::createScreenDriver(m_updateKeeper,
                                                           this,
                                                           &m_backupFrameBuffer,
                                                           &m_fbLocMut, log);
  m_updateKeeper->setBorderRect(&m_screenDriver->getScreenDimension().getRect());

  m_mouseDetector = new MouseDetector(m_updateKeeper, this, log);
  m_mouseShapeDetector = new MouseShapeDetector(m_updateKeeper, this,
                                                &m_mouseGrabber,
                                                &m_mouseGrabLocMut,
                                                log);
  m_updateFilter = new UpdateFilter(m_screenDriver,
                                    &m_backupFrameBuffer,
                                    &m_fbLocMut, log);

  executeDetectors();

  // Force first update with full screen grab
  m_absoluteRect = m_screenDriver->getScreenBuffer()->getDimension().getRect();
  m_updateKeeper->addChangedRect(&m_absoluteRect);
  doUpdate();
}

LocalUpdateHandler::~LocalUpdateHandler()
{
  terminateDetectors();
  delete m_mouseShapeDetector;
  delete m_mouseDetector;
  delete m_screenDriver;
  delete m_updateKeeper;
  delete m_updateFilter;
}

void LocalUpdateHandler::extract(UpdateContainer *updateContainer)
{
  {
    AutoLock al(&m_fbLocMut);
    Rect copyRect;
    Point copySrc;
    m_copyRectDetector.detectWindowMovements(&copyRect, &copySrc);

    {
      AutoLock al(m_updateKeeper);
      m_updateKeeper->addCopyRect(&copyRect, &copySrc);
      m_updateKeeper->extract(updateContainer);
    }
    updateVideoRegion();
    updateContainer->videoRegion = m_vidRegion;
    // Constrain the video region to the current frame buffer border.
    Region fbRect(&m_backupFrameBuffer.getDimension().getRect());
    updateContainer->videoRegion.intersect(&fbRect);

    m_updateFilter->filter(updateContainer);

    if (!m_absoluteRect.isEmpty()) {
      updateContainer->changedRegion.addRect(&m_screenDriver->getScreenBuffer()->
                                             getDimension().getRect());
      m_absoluteRect.clear();
    }

    // Checking for screen properties changing or frame buffers differ
    if (m_screenDriver->getPropertiesChanged() ||
      !m_backupFrameBuffer.isEqualTo(m_screenDriver->getScreenBuffer())) {
      if (m_screenDriver->getScreenSizeChanged()) {
        updateContainer->screenSizeChanged = true;
      }
      m_screenDriver->applyNewProperties();
      m_backupFrameBuffer.clone(m_screenDriver->getScreenBuffer());
      updateContainer->changedRegion.clear();
      updateContainer->copiedRegion.clear();
      m_absoluteRect = m_backupFrameBuffer.getDimension().getRect();
      m_updateKeeper->setBorderRect(&m_absoluteRect);
    }
  }
  // Cursor position must always be present.
  updateContainer->cursorPos = m_mouseDetector->getCursorPos();
  // Checking for mouse shape changing
  if (updateContainer->cursorShapeChanged || m_fullUpdateRequested) {
    // Update cursor shape
    AutoLock al(&m_mouseGrabLocMut);
    m_mouseGrabber.grab(&m_backupFrameBuffer.getPixelFormat());
    // Store cursor shape
    m_cursorShape.clone(m_mouseGrabber.getCursorShape());

    m_fullUpdateRequested = false;
  }
}

void LocalUpdateHandler::setFullUpdateRequested(const Region *region)
{
  m_updateKeeper->addChangedRegion(region);
  m_fullUpdateRequested = true;
}

void LocalUpdateHandler::executeDetectors()
{
  m_backupFrameBuffer.assignProperties(m_screenDriver->getScreenBuffer());
  m_screenDriver->executeDetection();
  m_mouseDetector->resume();
  m_mouseShapeDetector->resume();
}

void LocalUpdateHandler::terminateDetectors()
{
  m_mouseDetector->terminate();
  m_screenDriver->terminateDetection();
  m_mouseDetector->wait();
}

void LocalUpdateHandler::onUpdate()
{
  AutoLock al(&m_fbLocMut);

  UpdateContainer updCont;
  m_updateKeeper->getUpdateContainer(&updCont);
  if (!updCont.isEmpty()) {
    doUpdate();
  }
}

bool LocalUpdateHandler::checkForUpdates(Region *region)
{
  UpdateContainer updateContainer;
  m_updateKeeper->getUpdateContainer(&updateContainer);

  Region resultRegion = updateContainer.changedRegion;
  resultRegion.add(&updateContainer.copiedRegion);
  resultRegion.intersect(region);

  bool result = updateContainer.cursorPosChanged ||
                updateContainer.cursorShapeChanged ||
                updateContainer.screenSizeChanged ||
                !resultRegion.isEmpty();

  return result;
}

void LocalUpdateHandler::setExcludedRegion(const Region *excludedRegion)
{
  m_updateKeeper->setExcludedRegion(excludedRegion);
}

void LocalUpdateHandler::updateVideoRegion()
{
  ServerConfig *srvConf = Configurator::getInstance()->getServerConfig();
  unsigned int interval = srvConf->getVideoRecognitionInterval();

  DateTime curTime = DateTime::now();
  if ((curTime - m_lastVidUpdTime).getTime() > interval) {
    m_lastVidUpdTime = DateTime::now();
    m_vidRegion.clear();
    AutoLock al(srvConf);
    StringVector *classNames = srvConf->getVideoClassNames();
    std::vector<HWND> hwndVector;
    std::vector<HWND>::iterator hwndIter;

    WindowFinder::findWindowsByClass(classNames, &hwndVector);

    for (hwndIter = hwndVector.begin(); hwndIter != hwndVector.end(); hwndIter++) {
      HWND videoHWND = *hwndIter;
      if (videoHWND != 0) {
        WINDOWINFO wi;
        wi.cbSize = sizeof(WINDOWINFO);
        if (GetWindowInfo(videoHWND, &wi)) {
          Rect videoRect(wi.rcClient.left, wi.rcClient.top,
                         wi.rcClient.right, wi.rcClient.bottom);
          if (videoRect.isValid()) {
            videoRect.move(-GetSystemMetrics(SM_XVIRTUALSCREEN),
                           -GetSystemMetrics(SM_YVIRTUALSCREEN));
            m_vidRegion.addRect(&videoRect);
          }
        }
      }
    }
  }
}
