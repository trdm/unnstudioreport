///
/// \file sys_conv.cpp
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

#include "sys_conv.h"

#include <QDebug>


void qt1lSysTrimL(QString &Str, QChar Ch)
{
    int i = Str.size();
	if (i == 0)
		return;
    int c = 0;
    const QChar *cur = Str.data();
	//while(c < i)
	while(true)
	{
	    if(*cur == Ch)
            c++;
        else
            break;
        cur++;
	}
	if(c)
        Str = Str.remove(0, c);
}

void qt1lSysTrimR(QString &Str, QChar Ch)
{
    int i = Str.size();
	if (i == 0)
		return;
    int c = 0;
    i--;
    const QChar *cur = (Str.data() + i);
	//while(i >= 0)
	while(true)
	{
	    if(*cur == Ch)
            c++;
        else
            break;
        cur--;
	}
	if(c)
        Str.chop(c);
}

QString qt1lSysUpper(QString Str)
{
	QString res = Str.trimmed();
	res = res.toUpper();
	return res;
}

QString qt1lDoubleToStr(double Num)
{
	QString Str = QString("%1").arg(Num, 0 , 'f');
	//qDebug() << Str;
	qt1lSysTrimR(Str, '0');
	qt1lSysTrimR(Str, '.');
	//qDebug() << Str;
	//Str << Num;
	//Str.sprintf("%1.6f", Num);
	//wx1lTrimRight(Str, '0');
	//wx1lTrimRight(Str, ',');
	// *** patch ru locale ***
	//Str.replace(",", ".", Qt::CaseInsensitive);
	return Str;
}

double qt1lStrToDouble(const QString& Str)
{
	//double Num = 0;
	QString Tmp = Str;
	//Str_.Trim(FALSE);
	// *** patch ru locale ***
	Tmp.replace(".", ",", Qt::CaseInsensitive);
	//Str_.ToDouble(&Num);
	//return Num;
	return Tmp.toDouble();
}

long qt1lStrToLong(QString& Str)
{
	//long Num = 0;
	//Str.ToLong(&Num);
	//return Num;
	return Str.toLong();
}

int qt1lBeginYear = 1946;

QDate qt1lCorrectDate(QDate date)
{
	if (date.year() > qt1lBeginYear)
		return date;
	return date.addYears(100);
}

QDate qt1lStrToDate(QString Date)
{
	QDate res;
	int len = Date.size();
	QString Format;
	//char* frmt = "%d.%m.%y";
	bool correct = false;
	if (len > 9)
        Format = "d.MM.yyyy";
    else
    {
        Format = "d.MM.yy";
        correct = true;
    }
	//else if (len > 10)
	//	if ((Date.GetChar(8) >= '0') && (Date.GetChar(8) <= '9'))
	//		frmt = "%d.%m.%Y";
	//res.ParseFormat(Date.c_str(), (wxChar*)frmt);
	res = QDate::fromString(Date, Format);
	if (correct)
		res = qt1lCorrectDate(res);
	return res;
}

int qt1lStrToInt(QString Str)
{
	//long Num = 0;
	//Str.ToLong(&Num);
	//return Num;
	return Str.toInt();
}
