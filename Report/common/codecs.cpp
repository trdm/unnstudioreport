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

#include "codecs.h"

#include <QDebug>


QTextCodec* CodecCP1251 = 0;
QTextCodec* CodecOEM866 = 0;
QTextCodec* CodecUTF8 = 0;


// Инициализация текстовых кодеков
bool InitCodecs()
{
	CodecCP1251 = QTextCodec::codecForName("Windows-1251");
	CodecOEM866 = QTextCodec::codecForName("IBM 866");
	CodecUTF8 = QTextCodec::codecForName("UTF8");

	QTextCodec::setCodecForCStrings(CodecUTF8);
	QTextCodec::setCodecForLocale(CodecUTF8);

	return true;
};
