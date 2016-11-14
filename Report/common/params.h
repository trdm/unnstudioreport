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

#ifndef __QT1L_PARAMS_H__
#define __QT1L_PARAMS_H__

//#include <QString>
#include <QStringList>

#include "export.h"
#include "types.h"


// Режимы запуска
enum runModes
{
    CONFIG_MODE = 1,
    ENTERPRISE_MODE,
    DEBUG_MODE,
    MONITOR_MODE
};

// Разделитель папок.
//extern QChar qt1lPS;
// Разделитель папок.
//extern QString qt1lsPS;

//class COMMON_EXPORT Params
class Params
{
    Params() {};

public:
    // Возвращает путь, где лежат файды с отладочной информацией
    static QString debugDir() { return m_debugDir; };
    static QString debugPath(const QString& file);

    static QString appDir() { return m_appDir; };
    static QString tempDir() { return m_tempDir; };

    static QString translatorDir() { return m_translatorDir; };
    static QString translatorPath(const QString& file);

    static QString translatorLocalDir() { return m_translatorLocalDir; };
    static QString translatorLocalPath(const QString& file);

    static bool initPaths();

    /// Режим запуска программы
    static int runMode() { return m_runMode; };
    /// Имя БД
    static QString dbName() { return m_dbName; };
    /// Папка текущей БД.
    static QString dbDir() { return m_dbDir; };
    static QString dbWorkPath(const QString& file);
    /// Файл настроек БД
    static QString dbIni() { return m_dbIni; };
    static QString dbIniPath();
    /// Папка пользователя
    static QString userDir() { return m_userDir; };
    /// Имя пользователя
    static QString userName() { return m_userName; };
    /// Пароль пользователя
    static QString userPasw() { return m_userPasw; };
    static QString lang() { return m_lang; };
    /// Монопольный режим запуска программы
    static bool exclusiveMode() { return m_exclusiveMode; };

    static void parseArgs(QStringList args);

private:
    // Папка исполнительного файла.
    static QString m_appDir;
    // Временная папка.
    static QString m_tempDir;
    // Папка баз данных по умолчанию.
    static QString m_dbsDir;
    static QString m_debugDir;
    static QString m_resourceDir;
    static QString m_translatorDir;
    static QString m_translatorLocalDir;

    static int m_runMode;
    static QString m_dbDir;
    // Файл настройки ИБ
    static QString m_dbIni;
    static QString m_dbName;
    static QString m_userDir;
    static QString m_userName;
    static QString m_userPasw;
    static QString m_lang;
    static bool m_exclusiveMode;
};

#endif // __QT1L_PARAMS_H__
