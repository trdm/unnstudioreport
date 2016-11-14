//
// This file is part of the Qt 1L project
//
// Copyright (C) 2006 Dmitriy Pavlyuk <dm-p@rambler.ru>
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifndef __QT1L_DEBUG_H__
#define __QT1L_DEBUG_H__

#include <QtGlobal>

#include "export.h"


// Стандартный обработчик отладочных сообщений
void DebugMsgHandler(QtMsgType type, const char *msg);

void InstallMsgHandler(const char *fileName);

#endif // __QT1L_DEBUG_H__
