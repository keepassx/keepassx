/*
 *  Copyright (C) 2012 Felix Geyer <debfx@fobos.de>
 *  Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies)
 *  Copyright (C) 2012 Intel Corporation
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KEEPASSX_GLOBAL_H
#define KEEPASSX_GLOBAL_H

#include <QtGlobal>

#if defined(Q_OS_WIN)
#  if defined(KEEPASSX_BUILDING_CORE)
#    define KEEPASSX_EXPORT Q_DECL_EXPORT
#  else
#    define KEEPASSX_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define KEEPASSX_EXPORT Q_DECL_EXPORT
#endif

#ifndef QUINT32_MAX
#define QUINT32_MAX 4294967295U
#endif

template <typename T> struct AddConst { typedef const T Type; };

// this adds const to non-const objects (like std::as_const)
template <typename T>
constexpr typename AddConst<T>::Type& asConst(T &t) noexcept { return t; }
// prevent rvalue arguments:
template <typename T>
void asConst(const T&&) = delete;

#endif // KEEPASSX_GLOBAL_H
