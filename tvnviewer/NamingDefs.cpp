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

#include "NamingDefs.h"
#ifdef _DEBUG
const TCHAR ProductNames::PRODUCT_NAME[] = _T("Gwupe_Dev");
const TCHAR ProductNames::VIEWER_PRODUCT_NAME[] = _T("Gwupe_Dev Support Screen");

const TCHAR LogNames::VIEWER_LOG_FILE_STUB_NAME[] = _T("gwupess_Dev");
const TCHAR LogNames::LOG_DIR_NAME[] = _T("Gwupe_Dev");

const TCHAR RegistryPaths::VIEWER_PATH[] = _T("Software\\BlitsMe_Dev\\Viewer");

const TCHAR ApplicationNames::WINDOW_CLASS_NAME[] = 
  _T("GwupedvApplicationClass");

const TCHAR WindowNames::TVN_WINDOW_CLASS_NAME[] = _T("GwupeDVWindowClass");
const TCHAR WindowNames::TVN_WINDOW_TITLE_NAME[] = _T("Gwupe_Dev Support Screen");
const TCHAR WindowNames::TVN_SUB_WINDOW_TITLE_NAME[] = _T("Viewer");
#else
const TCHAR ProductNames::PRODUCT_NAME[] = _T("Gwupe");
const TCHAR ProductNames::VIEWER_PRODUCT_NAME[] = _T("Gwupe Support Screen");

const TCHAR LogNames::VIEWER_LOG_FILE_STUB_NAME[] = _T("gwupess");
const TCHAR LogNames::LOG_DIR_NAME[] = _T("Gwupe");

const TCHAR RegistryPaths::VIEWER_PATH[] = _T("Software\\BlitsMe\\Viewer");

const TCHAR ApplicationNames::WINDOW_CLASS_NAME[] = 
  _T("GwupevApplicationClass");

const TCHAR WindowNames::TVN_WINDOW_CLASS_NAME[] = _T("GwupeVWindowClass");
const TCHAR WindowNames::TVN_WINDOW_TITLE_NAME[] = _T("Gwupe Support Screen");
const TCHAR WindowNames::TVN_SUB_WINDOW_TITLE_NAME[] = _T("Viewer");
#endif
