// Copyright (C) 2011,2012 GlavSoft LLC.
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

#ifndef __STANDARDSCREENDRIVER_H__
#define __STANDARDSCREENDRIVER_H__

#include "ScreenDriver.h"
#include "Poller.h"
#include "ConsolePoller.h"
#include "HooksUpdateDetector.h"
#include "WindowsScreenGrabber.h"

class StandardScreenDriver : public ScreenDriver
{
public:
  StandardScreenDriver(UpdateKeeper *updateKeeper,
                       UpdateListener *updateListener,
                       FrameBuffer *fb,
                       LocalMutex *fbLocalMutex, LogWriter *log);
  virtual ~StandardScreenDriver();

  // Starts screen update detection if it not started yet.
  virtual void executeDetection();

  // Stops screen update detection.
  virtual void terminateDetection();

  virtual Dimension getScreenDimension();
  virtual FrameBuffer *getScreenBuffer();
  virtual bool grab(const Rect *rect = 0);

  virtual bool getPropertiesChanged();
  virtual bool getScreenSizeChanged();

  virtual bool applyNewProperties();

private:
  WindowsScreenGrabber m_screenGrabber;
  Poller m_poller;
  ConsolePoller m_consolePoller;
  HooksUpdateDetector m_hooks;

};

#endif // __STANDARDSCREENDRIVER_H__
