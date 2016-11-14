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

#include "params.h"

//#include <QFileInfo>
#include <QDir>
#include <QApplication>
#include <QDebug>

/*
QString qt1lExePath;
QString qt1lExeDir;
QString qt1lDBsDir;
QString qt1lSysDir;
QString qt1lDBDir;
//QString qt1lResDir;
QString qt1lPicsDir;
QString qt1lI18nDir;
QString qt1lTempDir;

QString qt1lUserDir;
QString qt1lUserName;
QString qt1lUserPasw;

bool qt1lExclusiveMode = false;
int qt1lRunMode = 0;
QString qt1lDBName;

//QString wx1lMainTitle;

QChar qt1lPS = PS;
QString qt1lsPS = sPS;
QStringList qt1lLibraryPaths;*/

#ifdef Q_OS_WIN32
QString Params::m_debugDir = QString::fromLatin1("");
#else
QString Params::m_debugDir = QString::fromLatin1("");
#endif
QString Params::m_appDir;
QString Params::m_resourceDir;
QString Params::m_translatorDir;
QString Params::m_translatorLocalDir;
QString Params::m_tempDir;

QString Params::m_dbsDir;
int Params::m_runMode;
QString Params::m_dbDir;
QString Params::m_dbIni;
QString Params::m_dbName;
QString Params::m_userDir;
QString Params::m_userName;
QString Params::m_userPasw;
QString Params::m_lang;
bool Params::m_exclusiveMode;

QString JoinPaths(const QString& dir, const QString& file)
{
    if (file.isEmpty())
        return dir;
    if (dir.isEmpty())
        return file;
    return dir+"/"+file;
}

bool Params::initPaths()
{
    // Путь к приложению
    m_appDir = QApplication::applicationDirPath();
    m_dbsDir = JoinPaths(m_appDir, "db");
    // Путь к рессурсам
#ifdef Q_OS_WIN32
    m_resourceDir = m_appDir;
#else
    m_resourceDir = QString::fromLatin1("/usr/share/1l");
#endif
    // Путь к переводам
    m_translatorDir = m_resourceDir + QString::fromLatin1("/translations");
    // Поиск локальных переводов (без установки программы)
    QDir dir(m_appDir);
    dir.cdUp();
    if (dir.cd(QString::fromLatin1("translations")))
        m_translatorLocalDir = dir.absolutePath();

    // Временная папка
	m_tempDir = QDir::tempPath();

	// Инициализация пути к плагинам
	//QStringList LibraryPaths = QCoreApplication::libraryPaths();
	//LibraryPaths.insert(0, qt1lExeDir + "plugins");
	//QCoreApplication::setLibraryPaths(qt1lLibraryPaths);
	// FI XME Необходим правильный путь
	QApplication::addLibraryPath(m_appDir+"/plugins");

    return true;
}

QString Params::debugPath(const QString& file)
{
    return JoinPaths(m_debugDir, file);
}

QString Params::translatorPath(const QString& file)
{
    return JoinPaths(m_translatorDir, file);
}

QString Params::translatorLocalPath(const QString& file)
{
    return JoinPaths(m_translatorLocalDir, file);
}

/*
bool qt1lInitPaths(const QString &sExePath)
{

	QString Msg;
	// Системная папка
	qt1lSysDir = qt1lExeDir + "system" + qt1lPS;
	FileInfo.setFile(qt1lSysDir);
	if (!FileInfo.exists())
	{
		//qt1lSysDir = wxGetCwd();
		Msg = QString("Not exist system directory: '%1.'").arg(qt1lSysDir);
		qCritical(qPrintable(Msg));
		//Q_ASSERT(false);
		return false;
	}
	// TO DO Переделать формирование путей
	FileInfo.setFile(qt1lExeDir + ".." + qt1lPS + "pics");
	qt1lPicsDir = FileInfo.canonicalFilePath() + qt1lPS;
	//qDebug(qPrintable(qt1lPicsDir));
	FileInfo.setFile(qt1lExeDir + ".." + qt1lPS + "i18n");
	qt1lI18nDir = FileInfo.canonicalFilePath() + qt1lPS;

	// Путь к ресурсам
	*//*qt1lResDir = qt1lSysDir + "resource" + qt1lPS;
	FileInfo.setFile(qt1lResDir);
	if (!FileInfo.exists())
	{
		Msg = QString("Not exist resource directory: '%1.'").arg(qt1lResDir);
		qCritical(qPrintable(Msg));
		//Q_ASSERT(false);
		return false;
	}*//*

	return true;
}*/

QString Params::dbWorkPath(const QString& file)
{
    return JoinPaths(m_dbDir, file);
}

QString Params::dbIniPath()
{
    return JoinPaths(m_dbDir, m_dbIni);
}

// Разбор парамметров
//
// CONFIG
// ENTERPRISE
// DEBUG
// MONITOR
// /D<Путь> - Каталог ИБ
// /U<Путь> - Каталог пользователя
// /N<Имя> - Имя пользователя
// /P<Пароль> - Пароль пользователя
// /M - Монопольный режим
// /L<Язык> - Язык интерфейса
// /I<Файл> - Настройки ИБ, по умолчанию ib.ini
//
void Params::parseArgs(QStringList args)
{
    if (args.size() == 0)
        return;
    int i;

    for (i=1; i<args.size(); i++)
    {
        QString Param = args[i];
        QString ParamUp = Param.toUpper();

        if (ParamUp == "CONFIG")
            m_runMode = CONFIG_MODE;
        else if (ParamUp == "ENTERPRISE")
            m_runMode = ENTERPRISE_MODE;
        else if (ParamUp == "DEBUG")
            m_runMode = DEBUG_MODE;
        else if (ParamUp == "MONITOR")
            m_runMode = MONITOR_MODE;
        else if (ParamUp.left(2) == "/D")
        {
            m_dbDir = QDir::fromNativeSeparators(Param.mid(2));
            //qDebug() << Param;
            // TO DO нормализация пути
            //qt1lSysTrimR(qt1lDBDir, '"');
            //qt1lSysTrimR(qt1lDBDir, '\\');
        }
        else if (ParamUp.left(2) == "/U")
        {
            m_userDir = QDir::fromNativeSeparators(Param.mid(2));
            // TO DO нормализация пути
            //csIBDir.TrimLeft('"');
            //qt1lSysTrimR(qt1lUserDir, '"');
            //qt1lSysTrimR(qt1lUserDir, '\\');
        }
        else if (ParamUp.left(2) == "/N")
        {
            m_userName = Param.mid(2);
        }
        else if (ParamUp.left(2) == "/P")
        {
            m_userPasw = Param.mid(2);
        }
        else if (ParamUp.left(2) == "/M")
        {
            m_exclusiveMode = true;
        }
        else if (ParamUp.left(2) == "/L")
        {
            m_lang = Param.mid(2);
        }
        else if (ParamUp.left(2) == "/I")
        {
            m_dbIni = Param.mid(2);
        }
        else
        {
            qDebug() << QObject::tr("Unknown start parametr: %1").arg(Param);
        }
    }
}
