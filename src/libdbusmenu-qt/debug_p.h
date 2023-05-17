/* This file is part of the dbusmenu-qt library
   Copyright 2009 Canonical
   Author: Aurelien Gateau <aurelien.gateau@canonical.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef DEBUG_P_H
#define DEBUG_P_H

#include <QDebug>

#define _DMBLUE "\033[34m"
#define _DMRED "\033[31m"
#define _DMRESET "\033[0m"
#define _DMTRACE(level, color) (level().nospace() << color << __PRETTY_FUNCTION__ << _DMRESET ":").space()

// Simple macros to get KDebug like support
#define DMDEBUG _DMTRACE(qDebug, _DMBLUE)
#define DMWARNING _DMTRACE(qWarning, _DMRED)

// Log a variable name and value
#define DMVAR(var) DMDEBUG << #var ":" << var

#define DMRETURN_IF_FAIL(cond)                                                                                                                                 \
    if (!(cond)) {                                                                                                                                             \
        DMWARNING << "Condition failed: " #cond;                                                                                                               \
        return;                                                                                                                                                \
    }

#define DMRETURN_VALUE_IF_FAIL(cond, value)                                                                                                                    \
    if (!(cond)) {                                                                                                                                             \
        DMWARNING << "Condition failed: " #cond;                                                                                                               \
        return (value);                                                                                                                                        \
    }

#endif /* DEBUG_P_H */
