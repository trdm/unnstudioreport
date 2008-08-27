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

#ifndef __QT1L_TYPES_H__
#define __QT1L_TYPES_H__

#include <QString>
#include <QVector>
#include <QMap>
#include <QList>

typedef QVector<int> arrInt;
typedef QVector<QString> arrStr;

typedef QMap<QString, int> mapStrToInt;

// Разделитель папок - \ или /
//#ifdef Q_OS_WIN32
//  #define PS '\\'
//  #define sPS "\\"
//#else
  #define PS '/'
  #define sPS "/"
//#endif

// Константы
#define PageBreak "\f"
#define LineBreak "\r\n"
#define TabSymbol "\t"

#endif // __QT1L_TYPES_H__
