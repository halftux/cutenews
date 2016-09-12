/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cutenews.h"
#include "article.h"
#include "articlemodel.h"
#include "cachingnetworkaccessmanagerfactory.h"
#include "categorymodel.h"
#include "categorynamemodel.h"
#include "concurrenttransfersmodel.h"
#include "dbconnection.h"
#include "definitions.h"
#include "logger.h"
#include "loggerverbositymodel.h"
#include "networkproxytypemodel.h"
#include "pluginmanager.h"
#include "settings.h"
#include "subscription.h"
#include "subscriptionmodel.h"
#include "subscriptions.h"
#include "subscriptionsourcetypemodel.h"
#include "transfermodel.h"
#include "transferprioritymodel.h"
#include "transfers.h"
#include "updateintervaltypemodel.h"
#include "urlopenermodel.h"
#include "userinterfacemodel.h"
#include "utils.h"
#include "viewmodemodel.h"
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDeclarativeEngine>
#include <QDeclarativeComponent>
#include <QDeclarativeContext>
#include <qdeclarative.h>
#include <QThread>

const QString CuteNews::WIDGET_FILENAME("/opt/cutenews/qml/Widget.qml");
const QString CuteNews::WINDOW_FILENAME("/opt/cutenews/qml/MainWindow.qml");

CuteNews* CuteNews::self = 0;

CuteNews::CuteNews() :
    QObject(),
    m_connection(0),
    m_engine(0)
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.registerService("org.marxoft.cutenews");
    connection.registerObject("/org/marxoft/cutenews", this, QDBusConnection::ExportScriptableSlots);
}

CuteNews::~CuteNews() {
    self = 0;
    
    if (m_connection) {
        delete m_connection;
        m_connection = 0;
    }
}

CuteNews* CuteNews::instance() {
    return self ? self : self = new CuteNews;
}

bool CuteNews::quit() {
    if (m_connection) {
        m_connection->close();
    }
    
    QThread *dbThread = DBConnection::asynchronousThread();

    if ((dbThread) && (dbThread != QThread::currentThread())) {
        if (dbThread->isRunning()) {
            Logger::log("CuteNews::quit(). Shutting down the database thread", Logger::LowVerbosity);
            connect(dbThread, SIGNAL(finished()), this, SLOT(quit()), Qt::UniqueConnection);
            dbThread->quit();
            return false;
        }
    }
        
    const int expiryDays = Settings::readArticleExpiry();
    
    if (expiryDays > -1) {
        uint expiryDate = QDateTime::currentDateTime().toTime_t();

        if (expiryDays > 0) {
            expiryDate -= expiryDays * 86400;
        }
        
        Logger::log("CuteNews::quit(). Deleting read articles older than "
                    + QDateTime::fromTime_t(expiryDate).toString(Qt::ISODate));
        DBConnection(false).deleteExpiredArticles(expiryDate);
    }
    
    Transfers::instance()->save();
    Logger::log("CuteNews::quit(). Quitting the application", Logger::LowVerbosity);
    QCoreApplication::quit();
    return true;
}

bool CuteNews::showArticle(const QString &articleId) {
    if ((!articleId.isEmpty()) && (showWindow())) {
        emit articleRequested(articleId);
        return true;
    }
    
    return false;
}

bool CuteNews::showWidget() {
    if (m_widget.isNull()) {
        if (QObject *obj = createQmlObject(WIDGET_FILENAME)) {
            m_widget = obj;
            return true;
        }
    }
    
    return false;
}

bool CuteNews::showWindow() {
    if (m_window.isNull()) {
        m_window = createQmlObject(WINDOW_FILENAME);
    }
    
    if (!m_window.isNull()) {
        QMetaObject::invokeMethod(m_window, "activate");
        return true;
    }
    
    return false;
}

void CuteNews::initEngine() {
    if (m_engine) {
        return;
    }
    
    qRegisterMetaType< QList<QVariantList> >("QList<QVariantList>");
    qmlRegisterType<Article>("cuteNews", 1, 0, "Article");
    qmlRegisterType<ArticleModel>("cuteNews", 1, 0, "ArticleModel");
    qmlRegisterType<CategoryModel>("cuteNews", 1, 0, "CategoryModel");
    qmlRegisterType<CategoryNameModel>("cuteNews", 1, 0, "CategoryNameModel");
    qmlRegisterType<ConcurrentTransfersModel>("cuteNews", 1, 0, "ConcurrentTransfersModel");
    qmlRegisterType<DBConnection>("cuteNews", 1, 0, "DBConnection");
    qmlRegisterType<LoggerVerbosityModel>("cuteNews", 1, 0, "LoggerVerbosityModel");
    qmlRegisterType<NetworkProxyTypeModel>("cuteNews", 1, 0, "NetworkProxyTypeModel");
    qmlRegisterType<SelectionModel>("cuteNews", 1, 0, "SelectionModel");
    qmlRegisterType<Subscription>("cuteNews", 1, 0, "Subscription");
    qmlRegisterType<SubscriptionModel>("cuteNews", 1, 0, "SubscriptionModel");
    qmlRegisterType<SubscriptionSourceTypeModel>("cuteNews", 1, 0, "SubscriptionSourceTypeModel");
    qmlRegisterType<TransferModel>("cuteNews", 1, 0, "TransferModel");
    qmlRegisterType<TransferPriorityModel>("cuteNews", 1, 0, "TransferPriorityModel");
    qmlRegisterType<UpdateIntervalTypeModel>("cuteNews", 1, 0, "UpdateIntervalTypeModel");
    qmlRegisterType<UserInterfaceModel>("cuteNews", 1, 0, "UserInterfaceModel");
    qmlRegisterType<ViewModeModel>("cuteNews", 1, 0, "ViewModeModel");
    
    qmlRegisterUncreatableType<FeedPluginConfig>("cuteNews", 1, 0, "FeedPluginConfig", "");
    qmlRegisterUncreatableType<Subscriptions>("cuteNews", 1, 0, "Subscriptions", "");
    qmlRegisterUncreatableType<Transfer>("cuteNews", 1, 0, "Transfer", "");
    
    m_engine = new QDeclarativeEngine(this);
    m_engine->setNetworkAccessManagerFactory(new CachingNetworkAccessManagerFactory);
    
    Logger *logger = new Logger(this);
    
    QDeclarativeContext *context = m_engine->rootContext();
    context->setContextProperty("cutenews", this);
    context->setContextProperty("database", m_connection ? m_connection : m_connection = new DBConnection(true));
    context->setContextProperty("logger", logger);
    context->setContextProperty("plugins", PluginManager::instance());
    context->setContextProperty("settings", Settings::instance());
    context->setContextProperty("subscriptions", Subscriptions::instance());
    context->setContextProperty("transfers", Transfers::instance());
    context->setContextProperty("urlopener", new UrlOpenerModel(this));
    context->setContextProperty("utils", new Utils(this));
    context->setContextProperty("ALL_ARTICLES_SUBSCRIPTION_ID", ALL_ARTICLES_SUBSCRIPTION_ID);
    context->setContextProperty("FAVOURITES_SUBSCRIPTION_ID", FAVOURITES_SUBSCRIPTION_ID);
    context->setContextProperty("VERSION_NUMBER", VERSION_NUMBER);
    
    connect(Settings::instance(), SIGNAL(loggerFileNameChanged(QString)), logger, SLOT(setFileName(QString)));
    connect(Settings::instance(), SIGNAL(loggerVerbosityChanged(int)), logger, SLOT(setVerbosity(int)));
}

QObject* CuteNews::createQmlObject(const QString &fileName) {
    initEngine();
    QDeclarativeContext *context = new QDeclarativeContext(m_engine->rootContext());
    QDeclarativeComponent *component = new QDeclarativeComponent(m_engine, fileName, this);
    
    if (QObject *obj = component->create(context)) {
        context->setParent(obj);
        return obj;
    }

    if (component->isError()) {
        foreach (QDeclarativeError error, component->errors()) {
            Logger::log("CuteNews::createQmlObject(). Error: " + error.toString());
        }        
    }
    
    delete component;
    delete context;
    return 0;
}
