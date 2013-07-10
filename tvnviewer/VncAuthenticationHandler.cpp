// Copyright (C) 2012 GlavSoft LLC.
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

#include "VncAuthenticationHandler.h"

#include "rfb/AuthDefs.h"
#include "rfb/VendorDefs.h"

#include "viewer-core/VncAuthentication.h"

#include "AuthenticationDialog.h"

VncAuthenticationHandler::VncAuthenticationHandler(ConnectionData *connectionData)
: m_connectionData(connectionData)
{
  m_id = AuthDefs::VNC;
}

VncAuthenticationHandler::~VncAuthenticationHandler()
{
}


void VncAuthenticationHandler::authenticate(DataInputStream *input,
                                            DataOutputStream *output)
{
  // get password from ConnectionData or User Interface
  StringStorage password;
  if (!m_connectionData->isSetPassword()) {
    getPassword();
  }
  password = m_connectionData->getPlainPassword();

  VncAuthentication::vncAuthenticate(input, output, &password);
}

void VncAuthenticationHandler::addAuthCapability(CapabilitiesManager *capManager)
{
  capManager->addAuthCapability(this, AuthDefs::VNC, VendorDefs::STANDARD, AuthDefs::SIG_VNC);
}

void VncAuthenticationHandler::getPassword()
{
  AuthenticationDialog authDialog;
  StringStorage hostname = m_connectionData->getHost();
  authDialog.setHostName(&hostname);
  if (authDialog.showModal()) {
    m_connectionData->setPlainPassword(authDialog.getPassword());
  } else {
    throw AuthCanceledException();
  }
}
