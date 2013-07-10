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

#ifndef __SCREENDRIVER_H__
#define __SCREENDRIVER_H__

#include "region/Dimension.h"
#include "rfb/FrameBuffer.h"

class ScreenDriver
{
public:
  ScreenDriver();
  virtual ~ScreenDriver();

  // Starts screen update detection if it not started yet.
  virtual void executeDetection() = 0;

  // Stops screen update detection.
  virtual void terminateDetection() = 0;

  // Return a current screen Dimension.
  virtual Dimension getScreenDimension() = 0;

  // Returns a pointer an internal screen driver FrameBuffer
  virtual FrameBuffer *getScreenBuffer() = 0;

  /* Provides grabbing.
  Parameters:     *rect - Pointer to a Rect object with relative workRect coordinates.
  Return value:   true if success.
  */
  virtual bool grab(const Rect *rect = 0) = 0;

  // Checks screen(desktop) properties on changes
  virtual bool getPropertiesChanged() = 0;
  virtual bool getScreenSizeChanged() = 0;

  // Set new values of the WorkRect to default (to full screen rectangle coordinates)
  // if desktop properties has been changed.
  // Also the frame buffer pixel format set to actual value.
  virtual bool applyNewProperties() = 0;
};

#endif // __SCREENDRIVER_H__
