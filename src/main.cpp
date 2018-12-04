#include <QApplication>
#include <QTextCodec>
#include "netsharer.h"
int main(int argc,char **argv)
{
	QApplication app(argc,argv);
	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
	if (!QSystemTrayIcon::isSystemTrayAvailable()) {
		QMessageBox::critical(0, QObject::tr("Systray"),
		QObject::tr("I couldn't detect any system tray on this system."));
		return 1;
	}
	netsharer *dialog=new netsharer;
	dialog->show();
	return app.exec();
}
 
