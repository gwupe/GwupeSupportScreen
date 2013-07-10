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

#include "StandardScreenDriver.h"

StandardScreenDriver::StandardScreenDriver(UpdateKeeper *updateKeeper,
                                           UpdateListener *updateListener,
                                           FrameBuffer *fb,
                                           LocalMutex *fbLocalMutex, LogWriter *log)
: m_poller(updateKeeper, updateListener, &m_screenGrabber, fb, fbLocalMutex, log),
  m_consolePoller(updateKeeper, updateListener, &m_screenGrabber, fb, fbLocalMutex, log),
  m_hooks(updateKeeper, updateListener, log)
{
}

StandardScreenDriver::~StandardScreenDriver()
{
  terminateDetection();
}

void StandardScreenDriver::executeDetection()
{
  m_poller.resume();
  m_consolePoller.resume();
  m_hooks.resume();
}

void StandardScreenDriver::terminateDetection()
{
  m_poller.terminate();
  m_consolePoller.terminate();
  m_hooks.terminate();

  m_poller.wait();
  m_consolePoller.wait();
  m_hooks.wait();
}

Dimension StandardScreenDriver::getScreenDimension()
{
  return Dimension(&m_screenGrabber.getScreenRect());
}

FrameBuffer *StandardScreenDriver::getScreenBuffer()
{
  return m_screenGrabber.getScreenBuffer();
}

bool StandardScreenDriver::grab(const Rect *rect)
{
  return m_screenGrabber.grab(rect);
}

bool StandardScreenDriver::getPropertiesChanged()
{
  return m_screenGrabber.getPropertiesChanged();
}

bool StandardScreenDriver::getScreenSizeChanged()
{
  return m_screenGrabber.getScreenSizeChanged();
}

bool StandardScreenDriver::applyNewProperties()
{
  return m_screenGrabber.applyNewProperties();
}
