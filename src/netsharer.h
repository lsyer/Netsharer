#ifndef NETSHARER_H
#define NETSHARER_H
#include <QDialog>
#include <QCloseEvent>
#include <QNetworkInterface>
#include <QMessageBox>
#include <QErrorMessage>
#include <QSystemTrayIcon>
#include <QRegExp>
#include <QMenu>
#include <QProcess>
#include <QSettings>
#include "ui_netsharer.h"
#include "ui_addmapdialog.h"
class netsharer:public QDialog,public Ui::netsharer
{
	Q_OBJECT
public:
	netsharer(QWidget *parent=0);
protected:
	void closeEvent ( QCloseEvent * event );
	void contextMenuEvent(QContextMenuEvent *event);
private:
	int mapnum;
	QErrorMessage * errorMessageDialog;
	QString maparray[20][4];
	void addmap(QString sport,QString dip,QString dport);
	void createActions();
	void createTrayIcon();
	QAction *minimizeAction;
	//QAction *maximizeAction;
	QAction *restoreAction;
	QAction *aboutAction;
	QAction *quitAction;

	QSystemTrayIcon *trayIcon;
	QMenu *trayIconMenu;
private slots:
	void runsharer();
	void stopsharer();
	void delmap();
	void calladdmap();
	void cellchanged(int row,int column);
	void determineip();
	void iconActivated(QSystemTrayIcon::ActivationReason reason);
	void about();
	void quitaction();
};
class addmapdialog:public QDialog,public Ui::addmapdialog
{
	Q_OBJECT
public:
	addmapdialog(QWidget *parent=0);
	QRegExpValidator * validator1;
	QRegExpValidator * validator2;
	QRegExpValidator * validator3;
};
#endif
 
