 /**********************************************************************
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
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

#ifndef PROGRAMINTERFACE_H
#define PROGRAMINTERFACE_H

//
//  W A R N I N G  --  PRIVATE INTERFACES
//  --------------------------------------
//
// This file and the interfaces declared in the file are not
// public. It exists for internal purpose. This header file and
// interfaces may change from version to version (even binary
// incompatible) without notice, or even be removed.
//
// We mean it.
//
//

#include <private/qcom_p.h>
#include <qmap.h>

// {87ced303-884f-449a-881a-ae8104932e3e}
#ifndef IID_Program
#define IID_Program QUuid( 0x87ced303, 0x884f, 0x449a, 0x88, 0x1a, 0xae, 0x81, 0x04, 0x93, 0x2e, 0x3e )
#endif

struct ProgramInterface : public QUnknownInterface
{
    virtual bool check( const QString &, QStringList &errors, QValueList<uint> &line ) = 0;
    virtual bool build( const QString &projectFile, QMap< QString, QMap<QStringList, int > > &errors ) = 0;
    virtual int run( const QStringList &projectFile ) = 0;
};

#endif
