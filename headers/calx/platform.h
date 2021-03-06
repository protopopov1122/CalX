/*
        SPDX-License-Identifier: LGPL-3.0

        Copyright (c) 2017-2019 Jevgenijs Protopopovs

        This file is part of CalX project.

        CalX is free software: you can redistribute it and/or modify
        it under the terms of the GNU Lesser General Public License version 3 as published
   by
        the Free Software Foundation.


        CalX is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU Lesser General Public License for more details.

        You should have received a copy of the GNU Lesser General Public License
        along with CalX.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CALX_PLATFORM_H_
#define CALX_PLATFORM_H_

/* This file contains platform-specific macros to make other code portable */

// Target platform
#if defined(__WIN32) | defined(_WIN32) | defined(__WIN32__)
#define OS_WIN32
#define OS_WIN
#elif defined(_WIN64)
#define OS_WIN64
#elif defined(__linux__)
#define OS_LINUX
#else
#define OS_UNKNOWN
#endif

// Library export/import defines
#ifdef OS_WIN
#define LIBEXPORT __declspec(dllexport)
#define LIBIMPORT __declspec(dllimport)
#define PATH_SEPARATOR '\\'
#else
#define LIBEXPORT
#define LIBIMPORT
#define PATH_SEPARATOR '/'
#endif

#define TMP_STRINGIZE(x) #x
#define STRINGIZE(x) TMP_STRINGIZE(x)

#endif