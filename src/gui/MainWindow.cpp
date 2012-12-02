#include <QtGui>
#include "MainWindow.h"
#include "GuiRegistry.h"
#include "AppRegistry.h"
#include "dialogs/ConnectionsDialog.h"
#include "settings/SettingsManager.h"
#include "QMessageBox"
#include "widgets/LogWidget.h"
#include "widgets/explorer/ExplorerWidget.h"
#include "domain/MongoServer.h"
#include "mongodb/MongoException.h"
#include "Dispatcher.h"
#include "widgets/workarea/WorkAreaWidget.h"
#include "domain/App.h"

using namespace Robomongo;

MainWindow::MainWindow() : QMainWindow(),
    _app(AppRegistry::instance().app()),
    _settingsManager(AppRegistry::instance().settingsManager()),
    _dispatcher(AppRegistry::instance().dispatcher())
{
    _dispatcher.subscribe(this, ConnectionFailedEvent::EventType);

    // Exit action
    QAction *exitAction = new QAction("&Exit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

    // Connect action
    QAction *connectAction = new QAction("&Connect", this);
    connectAction->setShortcut(QKeySequence::Open);
    connectAction->setIcon(GuiRegistry::instance().serverIcon());
    connectAction->setIconText("Connect");
    connectAction->setToolTip("Connect to MongoDB");
    connect(connectAction, SIGNAL(triggered()), this, SLOT(manageConnections()));

    // Refresh action
    QAction *refreshAction = new QAction("Refresh", this);
    refreshAction->setIcon(qApp->style()->standardIcon(QStyle::SP_BrowserReload));
    connect(refreshAction, SIGNAL(triggered()), this, SLOT(refreshConnections()));

    // File menu
    QMenu *fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction(connectAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    // Toolbar
    QToolBar *toolBar = new QToolBar("Toolbar", this);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolBar->addAction(connectAction);
    toolBar->addAction(refreshAction);
    toolBar->setShortcutEnabled(1, true);
    toolBar->setMovable(false);
    addToolBar(toolBar);

    _status = new QLabel;
    QPushButton *log = new QPushButton("Logs", this);
    log->setCheckable(true);
    connect(log, SIGNAL(toggled(bool)), this, SLOT(toggleLogs(bool)));
    statusBar()->insertWidget(0, log);
    statusBar()->addPermanentWidget(_status);

    createTabs();
    createDatabaseExplorer();

    setWindowTitle("Robomongo 0.3");
    setWindowIcon(GuiRegistry::instance().databaseIcon());

    //connect(_viewModel, SIGNAL(statusMessageUpdated(QString)), SLOT(vm_statusMessageUpdated(QString)));
}

bool MainWindow::event(QEvent *event)
{
    R_HANDLE(event)
    R_EVENT(ConnectionFailedEvent)
    else return QMainWindow::event(event);
}

void MainWindow::manageConnections()
{
    ConnectionsDialog dialog(&_settingsManager);
    int result = dialog.exec();

    // save settings
    _settingsManager.save();

    if (result == QDialog::Accepted)
    {
        ConnectionRecordPtr selected = dialog.selectedConnection();

        try
        {
            _app.openServer(selected);
        }
        catch(MongoException &ex)
        {
            QString message = QString("Cannot connect to MongoDB (%1)").arg(selected->getFullAddress());
            QMessageBox::information(this, "Error", message);
        }
    }


    // on linux focus is lost - we need to activate main window back
    activateWindow();
}

void MainWindow::refreshConnections()
{
    QToolTip::showText(QPoint(0,0),QString
                       ("Refresh not working yet... : <br/>  <b>Ctrl+D</b> : push Button"));
}

void MainWindow::toggleLogs(bool show)
{
    _logDock->setVisible(show);
}

void MainWindow::handle(ConnectionFailedEvent *event)
{
    ConnectionRecordPtr connection = event->server->connectionRecord();
    QString message = QString("Cannot connect to MongoDB (%1)").arg(connection->getFullAddress());
    QMessageBox::information(this, "Error", message);
}

void MainWindow::createDatabaseExplorer()
{
    /*
    ** Explorer
    */
    _explorer = new ExplorerWidget(this);
    QDockWidget *explorerDock = new QDockWidget(tr(" Database Explorer"));
    explorerDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    explorerDock->setWidget(_explorer);

    QWidget *titleWidget = new QWidget(this);         // this lines simply removes
    explorerDock->setTitleBarWidget(titleWidget);     // title bar widget.

    addDockWidget(Qt::LeftDockWidgetArea, explorerDock);

    _log = new LogWidget(this);
    _logDock = new QDockWidget(tr(" Log"));
    _logDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    _logDock->setWidget(_log);
    _logDock->setVisible(false);
    addDockWidget(Qt::BottomDockWidgetArea, _logDock);
}

void MainWindow::createTabs()
{
    _workArea = new WorkAreaWidget(this);
    setCentralWidget(_workArea);

    //QLabel *label = new QLabel("muahahah");
    //setCentralWidget(label);
}
