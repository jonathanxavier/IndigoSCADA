/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef DEFS_H
#define DEFS_H

#include <qsizepolicy.h>
#include <qstring.h>

#define POINTER_TOOL 32000
#define CONNECT_TOOL 32001
#define ORDER_TOOL 32002
#define BUDDY_TOOL 32004

int size_type_to_int( QSizePolicy::SizeType t );
QString size_type_to_string( QSizePolicy::SizeType t );
QSizePolicy::SizeType int_to_size_type( int i );

#endif
