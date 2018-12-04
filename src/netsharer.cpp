#include "netsharer.h"
netsharer::netsharer(QWidget *parent)
	:QDialog(parent)
{
	setupUi(this);
	mapnum=0;
    errorMessageDialog = new QErrorMessage(this);
    errorMessageDialog->setWindowTitle(tr("Net Sharer"));
	iinterface->clear();
	einterface->clear();
	foreach(QNetworkInterface interfaceX,QNetworkInterface::allInterfaces())
		if(interfaceX.name()!="lo"){
			iinterface->addItem(interfaceX.name());
			einterface->addItem(interfaceX.name());
		}
	externalip->clear();
	foreach(QHostAddress address,QNetworkInterface::allAddresses())
		if(address.toString()!="127.0.0.1")
			externalip->addItem(address.toString());
	portmaptab->setColumnWidth(1,50);
	connect(runbutton,SIGNAL(clicked()),this,SLOT(runsharer()));
	connect(stopbutton,SIGNAL(clicked()),this,SLOT(stopsharer()));
	connect(addmapbutton,SIGNAL(clicked()),this,SLOT(calladdmap()));
	connect(delmapbutton,SIGNAL(clicked()),this,SLOT(delmap()));
	connect(portmaptab,SIGNAL(cellChanged ( int, int )),this,SLOT(cellchanged( int, int )));
	connect(einterface,SIGNAL(currentIndexChanged(int)),this,SLOT(determineip()));
	connect(iinterface,SIGNAL(currentIndexChanged(int)),this,SLOT(determineip()));
	connect(externalip,SIGNAL(currentIndexChanged(int)),this,SLOT(determineip()));

	createActions();
	createTrayIcon();
	connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
	QIcon icon = QIcon(":/images/icon.png");
	trayIcon->setIcon(icon);
	setWindowIcon(icon);
	trayIcon->setToolTip("Net Sharer 0.4.8");
	trayIcon->show();
	
	QSettings settings("lsyer", "netsharer");
	move(settings.value("pos", QPoint(100, 60)).toPoint());
	einterface->setCurrentIndex(settings.value("einterface",0).toInt());
	externalip->setCurrentIndex(settings.value("externalip",0).toInt());
	iinterface->setCurrentIndex(settings.value("iinterface",0).toInt());
	internalip->setText(settings.value("internalip","192.168.1.1").toString());
	internalnet->setText(settings.value("internalnet","192.168.1.1/24").toString());
}
addmapdialog::addmapdialog(QWidget*parent)
	:QDialog(parent)
{
	setupUi(this);
	QRegExp regexp("[0-9]{1,5}");
	validator1=new QRegExpValidator(regexp,sport);
	sport->setValidator(validator1);
	validator2=new QRegExpValidator(regexp,dport);
	dport->setValidator(validator2);
	regexp.setPattern("[0-9]{1,3}[.][0-9]{1,3}[.][0-9]{1,3}[.][0-9]{1,3}");
	validator3=new QRegExpValidator(regexp,dip);
	dip->setValidator(validator3);
}
void netsharer::closeEvent ( QCloseEvent * event )
{
    //errorMessageDialog->showMessage(tr("The program will continue run hiding in systray.If you want to quit,please right click systray icon and select \"Quit\"."));
    hide();
    event->ignore();
}
void netsharer::runsharer()
{
	bool isok=true;
    QProcess myProcess;
    myProcess.setStandardOutputFile("/proc/sys/net/ipv4/ip_forward");
    myProcess.start("echo 1");
	myProcess.waitForFinished();
	
    myProcess.setStandardOutputFile("/etc/iptables.tmp");
    myProcess.start("iptables-save");
	myProcess.waitForFinished();
	if(myProcess.exitCode()!=0) isok=false;
	
    myProcess.setStandardOutputFile("/dev/stdout");
    myProcess.start("iptables -F");
	myProcess.waitForFinished();
	if(myProcess.exitCode()!=0) isok=false;
	
    myProcess.start("iptables -F -t nat");
	myProcess.waitForFinished();
	if(myProcess.exitCode()!=0) isok=false;
		
	if(einterface->currentText()==iinterface->currentText()){
		
	    myProcess.start(QString("ifconfig %1:0 %2").arg(einterface->currentText()).arg(internalip->text()));
		myProcess.waitForFinished();
		if(myProcess.exitCode()!=0) isok=false;
	}
	
	myProcess.start("iptables -t nat -A POSTROUTING -s "+internalnet->text()+" -j SNAT --to "+externalip->currentText());
	myProcess.waitForFinished();
	if(myProcess.exitCode()!=0) isok=false;
		
	int i;
	for(i=0;i<mapnum;i++){
	
    	myProcess.start("iptables -t nat -A PREROUTING -p tcp -d "+externalip->currentText()+" --dport "+maparray[i][0]+" -j DNAT --to "+maparray[i][2]+":"+maparray[i][3]);
		myProcess.waitForFinished();
		if(myProcess.exitCode()!=0) isok=false;
	
    	myProcess.start("iptables -t nat -A POSTROUTING -p tcp -d "+maparray[i][2]+" --dport "+maparray[i][3]+" -j SNAT --to-source "+externalip->currentText());
		myProcess.waitForFinished();
		if(myProcess.exitCode()!=0) isok=false;
	}
	runbutton->setDisabled(TRUE);
	stopbutton->setEnabled(TRUE);
	einterface->setDisabled(TRUE);
	iinterface->setDisabled(TRUE);
	externalip->setDisabled(TRUE);
	internalip->setDisabled(TRUE);
	internalnet->setDisabled(TRUE);
	addmapbutton->setDisabled(TRUE);
	delmapbutton->setDisabled(TRUE);
	if(!isok){		
        QMessageBox::critical(this, tr("Net Sharer"),
                                 tr("Error,the soft cann't run,please communicate with the author!"));
        stopsharer();
	}
}
void netsharer::stopsharer()
{
    QProcess myProcess;
    myProcess.start("iptables-restore /etc/iptables.tmp");
	myProcess.waitForFinished();
	
    myProcess.start("rm -rf /etc/iptables.tmp");
	myProcess.waitForFinished();
	
	if(einterface->currentText()==iinterface->currentText()){	
    	myProcess.start("ifconfig "+einterface->currentText()+":0 del "+internalip->text());
		myProcess.waitForFinished();	
	}
	runbutton->setDisabled(FALSE);
	stopbutton->setEnabled(FALSE);
	einterface->setDisabled(FALSE);
	iinterface->setDisabled(FALSE);
	externalip->setDisabled(FALSE);
	internalip->setDisabled(FALSE);
	internalnet->setDisabled(FALSE);
	addmapbutton->setDisabled(FALSE);
	delmapbutton->setDisabled(FALSE);
}
void netsharer::addmap(QString sport,QString dip,QString dport)
{
	//maparray[mapnum][0]=sport;
	//maparray[mapnum][1]=dip;
	//maparray[mapnum][2]=dport;
	//they will be modified by cellchanged(int,int);
	int row=portmaptab->rowCount();
	portmaptab->insertRow(row);
	QTableWidgetItem *item0=new QTableWidgetItem;
	QTableWidgetItem *item1=new QTableWidgetItem;
	QTableWidgetItem *item2=new QTableWidgetItem;
	QTableWidgetItem *item3=new QTableWidgetItem;
	item0->setText(sport);
	item1->setText("<->");
	item1->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
	item1->setFlags(Qt::ItemIsEnabled);
	item2->setText(dip);
	item3->setText(dport);
	portmaptab->setItem(row,0,item0);
	portmaptab->setItem(row,1,item1);
	portmaptab->setItem(row,2,item2);
	portmaptab->setItem(row,3,item3);
	//portmaptab->item(row,0)->setText();
	mapnum+=1;
	if (mapnum>=20)
		addmapbutton->setDisabled(TRUE);
}
void netsharer::delmap()
{
	int i;
	for(i=0;i<mapnum;i++){
	delete portmaptab->item(mapnum,0);
	delete portmaptab->item(mapnum,1);
	delete portmaptab->item(mapnum,2);
	delete portmaptab->item(mapnum,3);
	}
	mapnum=0;
	portmaptab->setRowCount(0);
	addmapbutton->setDisabled(FALSE);
}
void netsharer::cellchanged(int row,int column)
{
	//printf("row:%d  column:%d\n",row,column);
	maparray[row][column]=portmaptab->item(row,column)->text();
	//printf("%s\n",(const char*)maparray[row][column].toLocal8Bit());
}
void netsharer::calladdmap()
{
	addmapdialog * adddialog=new addmapdialog(this);
	if(adddialog->exec()&&adddialog->sport->text()!=""&&adddialog->dport->text()!=""&&adddialog->dip->text()!=""){
		addmap(adddialog->sport->text(),adddialog->dip->text(),adddialog->dport->text());
	}
	delete adddialog;
}
void netsharer::determineip()
{
	if(einterface->currentText()!=iinterface->currentText()){
		foreach(QHostAddress address,QNetworkInterface::allAddresses())
			if(address.toString()!="127.0.0.1"&&address.toString()!=externalip->currentText()){
				internalip->setText(address.toString());
				internalnet->setText(address.toString()+"/24");
				break;
			}
	}
}
void netsharer::createActions()
{
    minimizeAction = new QAction(tr("(&H)Hide"), this);
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

    //maximizeAction = new QAction(tr("(&M)最大化"), this);
    //connect(maximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));

    restoreAction = new QAction(tr("(&S)Show"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(show()));

    aboutAction = new QAction(tr("(&A)About"), this);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

    quitAction = new QAction(tr("(&Q)Quit"), this);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(quitaction()));
}

void netsharer::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(minimizeAction);
    //trayIconMenu->addAction(maximizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addAction(aboutAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
}
void netsharer::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    //case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
	setVisible(isHidden());
        break;
    //case QSystemTrayIcon::MiddleClick:
        //showMessage();
        //break;
    default:
        ;
    }
}
void netsharer::about()
{
	show();
   QMessageBox::about(this, tr("NetSharer"),
            tr("<p align=center>NetSharer 0.4.8 </p><p align=right> Design by <a href=http://lishao378.blog.sohu.com target=_blank>lsyer</a>(木子)<br><a href=mailto:lisho378@sohu.com>lishao378@sohu.com</a><br>2007-10-28&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</p>"));
	return ;
}
void netsharer::quitaction()
{
	QSettings settings("lsyer", "netsharer");
	settings.setValue("pos", pos());
	settings.setValue("einterface", einterface->currentIndex());
	settings.setValue("externalip", externalip->currentIndex());
	settings.setValue("iinterface", iinterface->currentIndex());
	settings.setValue("internalip", internalip->text());
	settings.setValue("internalnet", internalnet->text());
	if (runbutton->isEnabled())
	{
		qApp->quit();
	}
	int r=QMessageBox::warning(this,tr("NetSharer 0.4.6"),tr("the server is running,\nif quit now()?"),QMessageBox::Yes|QMessageBox::Default,QMessageBox::Cancel|QMessageBox::Escape);
	if (r==QMessageBox::Yes)
	{
		stopsharer();
		qApp->quit();
	}
}
void netsharer::contextMenuEvent(QContextMenuEvent *event)
{
    //QMenu menu(this);
   // menu.addAction(cutAct);
    //menu.addAction(copyAct);
    //menu.addAction(pasteAct);
    trayIconMenu->exec(event->globalPos());
	return ;
}
