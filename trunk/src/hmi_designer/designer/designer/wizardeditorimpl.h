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

#ifndef WIZARDEDITORIMPL_H
#define WIZARDEDITORIMPL_H

class QWizard;
class FormWindow;

#include <qptrlist.h>

#include "wizardeditor.h"
#include "command.h"

class WizardEditor : public WizardEditorBase
{
    Q_OBJECT

public:
    WizardEditor( QWidget *parent, QWizard *wizard, FormWindow *fw );
    ~WizardEditor();

protected slots:
    void okClicked();
    void applyClicked();
    void cancelClicked();
    void helpClicked();

    void addClicked();
    void removeClicked();
    void upClicked();
    void downClicked();

    void itemHighlighted( int );
    void itemSelected( int );

    void itemDragged( QListBoxItem * );
    void itemDropped( QListBoxItem * );

private:
    void updateButtons();
    void fillListBox();

private:
    FormWindow *formwindow;
    QWizard *wizard;
    QPtrList<Command> commands;
    int draggedItem;
};

#endif
