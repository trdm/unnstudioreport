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

#include "debug.h"

#include <QFile>
#include <QMessageBox>

#ifdef Q_OS_WIN32
#include <windows.h>
#endif // Q_OS_WIN32

#include "params.h"


QFile DbgLogFile("");
bool DebugIsInit = false;
QString DebugFileName;

void DebugMsgHandler(QtMsgType type, const char *msg)
{
    if (!DebugIsInit)
    {
        if (DebugFileName.isEmpty())
            DebugFileName = "debug.log";
        DbgLogFile.setFileName(Params::debugPath(DebugFileName));
        DbgLogFile.open(QIODevice::WriteOnly | QIODevice::Text);
        DebugIsInit = true;
    }
    QByteArray buf;
    switch (type)
    {

    case QtDebugMsg:
        buf = "Debug: ";
        break;
    case QtWarningMsg:
        buf = "Warning: ";
        break;
    case QtCriticalMsg:
        buf = "Critical: ";
        break;
    case QtFatalMsg:
        buf = "Fatal: ";
    }
    buf += msg;
    buf += "\n";
    DbgLogFile.write(buf);
    DbgLogFile.flush();

#ifdef Q_OS_WIN32
#ifdef UNICODE
    QString buf2(buf);
    OutputDebugString((const WCHAR*)buf2.constData());
#else // UNICODE
    OutputDebugString(buf.constData());
#endif // UNICODE
#endif // Q_OS_WIN32

    int res = QMessageBox::Abort;
	switch (type)
    {
        case QtDebugMsg:
            break;
        case QtWarningMsg:
            QMessageBox::information(0, QString("Warning:"), QString(msg));
            break;
        case QtCriticalMsg:
            QMessageBox::warning(0, QString("Critical"), QString(msg));
            break;
        case QtFatalMsg:
            res = QMessageBox::critical(0, QString("Fatal"), QString(msg),
				QMessageBox::Abort, QMessageBox::Ignore);
    }

    if((type == QtFatalMsg) && (res == QMessageBox::Abort))
    {
		DbgLogFile.close();
#ifdef __GNUC__
		asm("int $0x3");
#endif
		abort();
    }
}

void InstallMsgHandler(const char *fileName)
{
    DebugFileName = fileName;
    qInstallMsgHandler(DebugMsgHandler);
}
