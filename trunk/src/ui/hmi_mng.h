/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2012 Enscada 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#include <qobject.h>
#include <qdialog.h>

class HMI_manager : public QObject
{
    Q_OBJECT

	enum {
		tUnit = 1, tTagLimits
	}; // transaction codes

	QString sample_point_name;
	QString value_for_command;

public:
    void setParent( QDialog *parent );
	void setInitialValuesAndLimits();
	
public slots:
    void sendCommand();
	void pSwitchToggledSendCommand();
	void RightClicked(QString &class_name, QString &widget_name);
	void UpdateTags(); // update tag values
	void UpdateSamplePoint(); // handle updated sample points
	void QueryResponse (QObject *, const QString &, int, QObject*); // handles database responses
	void DoButtonCommand();
	void Do_pSwitchCommand();
	void ReceivedNotify(int, const char *);
private:
    QDialog *dialog_parent;
};
