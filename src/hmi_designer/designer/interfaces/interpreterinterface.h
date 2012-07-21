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

#ifndef INTERPRETERINTERFACE_H
#define INTERPRETERINTERFACE_H

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

class QObject;

// {11cad9ec-4e3c-418b-8e90-e1b8c0c1f48f}
#ifndef IID_Interpreter
#define IID_Interpreter QUuid( 0x11cad9ec, 0x4e3c, 0x418b, 0x8e, 0x90, 0xe1, 0xb8, 0xc0, 0xc1, 0xf4, 0x8f )
#endif

struct InterpreterInterface : public QUnknownInterface
{
    virtual void init() = 0;
    virtual bool exec( QObject *obj, const QString &code ) = 0;
    virtual void setBreakPoints( QObject *obj, const QValueList<uint> &lst ) = 0;
    virtual QString createVariableDeclaration( const QString &var ) = 0;
    virtual QString createFunctionDeclaration( const QString &signature, const QString &body ) = 0;

    virtual void onShowDebugStep( QObject *obj, const char *slot ) = 0;
    virtual void onShowError( QObject *obj, const char *slot ) = 0;
    virtual void onShowStackFrame( QObject *obj, const char *slot ) = 0;
    virtual void onFinish( QObject *obj, const char *slot ) = 0;
};



#endif
