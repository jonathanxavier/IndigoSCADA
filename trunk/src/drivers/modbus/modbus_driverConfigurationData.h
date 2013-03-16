/****************************************************************************
** Form interface generated from reading ui file 'modbus_driver_configuration.ui'
**
** Created: Sat Mar 16 15:33:40 2013
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.2.1   edited May 19 14:22 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef FORM1_H
#define FORM1_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QLabel;
class QLineEdit;
class QSpinBox;
class QButtonGroup;
class QRadioButton;
class QPushButton;

class Modbus_driverConfigurationData : public QDialog
{
    Q_OBJECT

public:
    Modbus_driverConfigurationData( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~Modbus_driverConfigurationData();

    QLabel* textLabel1;
    QLabel* textLabel5;
    QLabel* textLabel4;
    QLabel* textLabel7;
    QLabel* textLabel8;
    QLabel* textLabel9;
    QLabel* textLabel10;
    QLabel* textLabel11;
    QLabel* textLabel6;
    QLineEdit* Parity;
    QLineEdit* SerialDevice;
    QLineEdit* MODBUSServerIPPortText;
    QLineEdit* MODBUSServerIPAddressText;
    QLineEdit* ServerID;
    QLineEdit* Name;
    QLabel* textLabel2;
    QSpinBox* PollInterval;
    QSpinBox* Baud;
    QSpinBox* DataBits;
    QSpinBox* StopBit;
    QSpinBox* NItems;
    QButtonGroup* buttonGroup1;
    QRadioButton* TCPButton;
    QRadioButton* RTUButton;
    QLabel* textLabel3;
    QPushButton* HelpButton;
    QPushButton* CancelButton;
    QPushButton* OkButton;

public slots:
    virtual void Help();
    virtual void OkClicked();
    virtual void RTUContextActive(bool);
    virtual void TCPContextActive(bool);

protected:

protected slots:
    virtual void languageChange();

};

#endif // FORM1_H
