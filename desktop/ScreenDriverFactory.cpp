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

#include "ScreenDriverFactory.h"
#include "server-config-lib/Configurator.h"

ScreenDriverFactory::ScreenDriverFactory()
{
}

ScreenDriverFactory::~ScreenDriverFactory()
{
}

ScreenDriver *ScreenDriverFactory::
createScreenDriver(UpdateKeeper *updateKeeper,
                   UpdateListener *updateListener,
                   FrameBuffer *fb,
                   LocalMutex *fbLocalMutex,
                   LogWriter *log)
{
  if (isMirrorDriverAllowed()) {
    log->info(_T("Mirror driver usage is allowed, try to start it..."));
    try {
      return createMirrorScreenDriver(updateKeeper, updateListener,
                                      fbLocalMutex, log);
    } catch (Exception &e) {
      log->error(_T("The mirror driver factory has failed: %s"),
                 e.getMessage());
    }
  } else {
    log->info(_T("Mirror driver usage is disallowed"));
  }
  log->info(_T("Using the standart screen driver"));
  return createStandardScreenDriver(updateKeeper,
                                    updateListener,
                                    fb,
                                    fbLocalMutex, log);
}

ScreenDriver *ScreenDriverFactory::
createStandardScreenDriver(UpdateKeeper *updateKeeper,
                           UpdateListener *updateListener,
                           FrameBuffer *fb,
                           LocalMutex *fbLocalMutex,
                           LogWriter *log)
{
  return new StandardScreenDriver(updateKeeper, updateListener, fb, fbLocalMutex, log);
}

ScreenDriver *ScreenDriverFactory::
createMirrorScreenDriver(UpdateKeeper *updateKeeper,
                         UpdateListener *updateListener,
                         LocalMutex *fbLocalMutex,
                         LogWriter *log)
{
  return new MirrorScreenDriver(updateKeeper, updateListener, fbLocalMutex, log);
}

bool ScreenDriverFactory::isMirrorDriverAllowed()
{
  ServerConfig *srvConf = Configurator::getInstance()->getServerConfig();
  return srvConf->getMirrorIsAllowed();
}
