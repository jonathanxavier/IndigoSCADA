/**********************************************************************
** Copyright (C) 2001 Trolltech AS.  All rights reserved.
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

void GotoLineDialog::init()
{
    editor = 0;
}

void GotoLineDialog::destroy()
{
    if ( editor )
	editor->release();
}

void GotoLineDialog::gotoLine()
{
    if ( editor )
	editor->gotoLine( spinLine->value() - 1 );
    accept();
}

void GotoLineDialog::setEditor( EditorInterface *e )
{
    editor = e;
    editor->addRef();
}

