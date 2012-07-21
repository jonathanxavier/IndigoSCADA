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

void FindDialog::init()
{
    editor = 0;
    formWindow = 0;
}

void FindDialog::destroy()
{
    if ( editor )
	editor->release();
}

void FindDialog::doFind()
{
    if ( !editor )
	return;
    
    if ( !editor->find( comboFind->currentText(), checkCase->isChecked(),
	checkWords->isChecked(), radioForward->isChecked(), !checkBegin->isChecked() ) )
	checkBegin->setChecked( TRUE );
    else
	checkBegin->setChecked( FALSE );
    
}

void FindDialog::setEditor( EditorInterface * e, QObject * fw )
{
    if ( fw != formWindow )
	checkBegin->setChecked( TRUE );
    formWindow = fw;
    if ( editor )
	editor->release();
    editor = e;
    editor->addRef();
}
