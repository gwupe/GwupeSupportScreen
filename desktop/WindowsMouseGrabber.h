// Copyright (C) 2008,2009,2010,2011,2012 GlavSoft LLC.
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

#ifndef __WINDOWSMOUSEGRABBER_H__
#define __WINDOWSMOUSEGRABBER_H__

#include "MouseGrabber.h"
#include "util/CommonHeader.h"
#include "win-system/Screen.h"

class WindowsMouseGrabber : public MouseGrabber
{
public:
  WindowsMouseGrabber(void);
  virtual ~WindowsMouseGrabber(void);

  virtual bool grab(PixelFormat *pixelFormat);

  virtual bool isCursorShapeChanged();

private:
  bool grabPixels(PixelFormat *pixelFormat);

  HCURSOR getHCursor();

  static void inverse(char *bits, int count);
  void fixAlphaChannel(const FrameBuffer *pixels,
                       char *maskAND);
  static bool testBit(char byte, int index);

  // This function combines the windows cursor mask and image and convert
  // theirs to rfb format. This function uses for monochrome cursor image.
  static void winMonoShapeToRfb(const FrameBuffer *pixels,
                                char *maskAND, char *maskXOR);

  //   This function combines windows the cursor mask and image and convert
  // theirs to rfb format. This function uses for 16 or 24 bit color cursor
  // image.
  //   Also, this function determines whether image contains the alhpa channel
  // and returns true in this case.
  template< typename T >
  bool winColorShapeToRfb(const FrameBuffer *pixels,
                          char *maskAND);

  UINT32 getAlphaMask(const PixelFormat *pf);

  HCURSOR m_lastHCursor;
  Screen m_screen;
};

#endif // __WINDOWSMOUSEGRABBER_H__
