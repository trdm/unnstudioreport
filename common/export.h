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

#ifndef __QT1L_EXPORT_H__
#define __QT1L_EXPORT_H__

#if defined(Q_OS_WIN32) && !defined(NO_DLL)
#  if defined(COMMON_DLL)
#    define COMMON_EXPORT __declspec(dllexport)
#  else
#    define COMMON_EXPORT __declspec(dllimport)
#  endif
#else
#  define COMMON_EXPORT
#endif

#if defined(Q_OS_WIN32) && !defined(NO_DLL)
#  if defined(CORE__DLL)
#    define CORE__EXPORT __declspec(dllexport)
#  else
#    define CORE__EXPORT __declspec(dllimport)
#  endif
#else
#  define CORE__EXPORT
#endif

#endif // __QT1L_EXPORT_H__
