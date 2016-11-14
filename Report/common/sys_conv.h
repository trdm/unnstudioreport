///
/// \file sys_conv.h
///
/// \brief Системные функции преобразования
//
//  qt1L - платформа для написания программ автоматизации учета
//
//  Copyright (C) 2004-2006 Dmitriy Pavlyuk <dm-p@rambler.ru>
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Library General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Library General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this library; if not, write to the Free
//  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#ifndef __QT1L_SYS_CONV_H__
#define __QT1L_SYS_CONV_H__

#include <QString>
#include <QDate>

#include "export.h"

/*
COMMON_EXPORT void qt1lSysTrimL(QString &Str, QChar Ch);
COMMON_EXPORT void qt1lSysTrimR(QString &Str, QChar Ch);

//QString qt1lSysUpper(QString Str);

COMMON_EXPORT QString qt1lDoubleToStr(double Num);
COMMON_EXPORT double qt1lStrToDouble(const QString& Str);
//long qt1lStrToLong(QString& Str);
COMMON_EXPORT QDate qt1lStrToDate(QString Date);
//int qt1lStrToInt(QString Str);
*/




void qt1lSysTrimL(QString &Str, QChar Ch);
void qt1lSysTrimR(QString &Str, QChar Ch);

//QString qt1lSysUpper(QString Str);

QString qt1lDoubleToStr(double Num);
double qt1lStrToDouble(const QString& Str);
//long qt1lStrToLong(QString& Str);
QDate qt1lStrToDate(QString Date);

QString qt1lDoubleToStr(double Num);

#endif // __QT1L_SYS_CONV_H__
