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

#ifndef __QT1L_CODECS_H__
#define __QT1L_CODECS_H__

#include <QTextCodec>

#include "export.h"


extern QTextCodec* CodecCP1251;
extern QTextCodec* CodecOEM866;
extern QTextCodec* CodecUTF8;

//#define RU(s) CodecUTF8->toUnicode(s)
//#define R(s) CodecUTF8->fromUnicode(s)
#define RU(s) QString::fromUtf8(s)
#define _SWU(s) CodecCP1251->toUnicode(s)
#define _SUW(s) CodecCP1251->fromUnicode(s)

// Для внутреннего использования
bool InitCodecs();

#endif // __QT1L_CODECS_H__
