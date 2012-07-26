#include <qobject.h>
#include <qdialog.h>

class HMI_manager : public QObject
{
    Q_OBJECT

	enum {
		tUnit = 1
	}; // transaction codes

	QString  sample_point_name;
public:
    void setParent( QDialog *parent );
	void setInitialValues();
	
public slots:
    void sendCommand();
	void UpdateTags(); // update tag values
	void UpdateSamplePoint(); // handle updated sample points
	void QueryResponse (QObject *, const QString &, int, QObject*); // handles database responses
	void DoCommand();
	void ReceivedNotify(int, const char *);
private:
    QDialog *p;
};
