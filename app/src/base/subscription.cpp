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

#include "subscription.h"
#include "dbconnection.h"
#include "dbnotify.h"
#include "json.h"

Subscription::Subscription(QObject *parent) :
    QObject(parent),
    m_downloadEnclosures(false),
    m_sourceType(Url),
    m_status(Idle),
    m_updateInterval(0),
    m_unreadArticles(0),
    m_autoUpdate(false)
{
}

Subscription::Subscription(const QString &id, const QString &description, bool downloadEnclosures,
                           const QString &iconPath, const QDateTime &lastUpdated, const QVariant &source,
                           SourceType sourceType, const QString &title, int updateInterval, const QString &url,
                           int unreadArticles, QObject *parent) :
    QObject(parent),
    m_id(id),
    m_description(description),
    m_downloadEnclosures(downloadEnclosures),
    m_iconPath(iconPath),
    m_lastUpdated(lastUpdated),
    m_source(source),
    m_sourceType(sourceType),
    m_status(Ready),
    m_title(title),
    m_updateInterval(updateInterval),
    m_url(url),
    m_unreadArticles(unreadArticles),
    m_autoUpdate(false)
{
}

QString Subscription::id() const {
    return m_id;
}

void Subscription::setId(const QString &i) {
    if (i != id()) {
        m_id = i;
        emit idChanged();
        emit dataChanged(this);
    }
}

QString Subscription::description() const {
    return m_description;
}

void Subscription::setDescription(const QString &d) {
    if (d != description()) {
        m_description = d;
        emit descriptionChanged();
        emit dataChanged(this);
    }
}

bool Subscription::downloadEnclosures() const {
    return m_downloadEnclosures;
}

void Subscription::setDownloadEnclosures(bool d) {
    if (d != downloadEnclosures()) {
        m_downloadEnclosures = d;
        emit downloadEnclosuresChanged();
        emit dataChanged(this);
    }
}

QString Subscription::errorString() const {
    return m_errorString;
}

void Subscription::setErrorString(const QString &e) {
    m_errorString = e;
}

QString Subscription::iconPath() const {
    return m_iconPath;
}

void Subscription::setIconPath(const QString &p) {
    if (p != iconPath()) {
        m_iconPath = p;
        emit iconPathChanged();
        emit dataChanged(this);
    }
}

QDateTime Subscription::lastUpdated() const {
    return m_lastUpdated;
}

void Subscription::setLastUpdated(const QDateTime &d) {
    if (d != lastUpdated()) {
        m_lastUpdated = d;
        emit lastUpdatedChanged();
        emit dataChanged(this);
    }
}

bool Subscription::isRead() const {
    return unreadArticles() == 0;
}

QVariant Subscription::source() const {
    return m_source;
}

void Subscription::setSource(const QVariant &s) {
    if (s != source()) {
        m_source = s;
        emit sourceChanged();
        emit dataChanged(this);
    }
}

Subscription::SourceType Subscription::sourceType() const {
    return m_sourceType;
}

void Subscription::setSourceType(Subscription::SourceType t) {
    if (t != sourceType()) {
        m_sourceType = t;
        emit sourceTypeChanged();
        emit dataChanged(this);
    }
}

Subscription::Status Subscription::status() const {
    return m_status;
}

void Subscription::setStatus(Subscription::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged();
    }
}

QString Subscription::title() const {
    return m_title;
}

void Subscription::setTitle(const QString &t) {
    if (t != title()) {
        m_title = t;
        emit titleChanged();
        emit dataChanged(this);
    }
}

int Subscription::updateInterval() const {
    return m_updateInterval;
}

void Subscription::setUpdateInterval(int i) {
    if (i != updateInterval()) {
        m_updateInterval = i;
        emit updateIntervalChanged();
        emit dataChanged(this);
    }
}

QString Subscription::url() const {
    return m_url;
}

void Subscription::setUrl(const QString &u) {
    if (u != url()) {
        m_url = u;
        emit urlChanged();
        emit dataChanged(this);
    }
}

int Subscription::unreadArticles() const {
    return m_unreadArticles;
}

void Subscription::setUnreadArticles(int u) {
    if ((u != unreadArticles()) && (u >= 0)) {
        m_unreadArticles = u;
        emit unreadArticlesChanged();
        emit dataChanged(this);
    }
}

bool Subscription::autoUpdate() const {
    return m_autoUpdate;
}

void Subscription::setAutoUpdate(bool enabled) {
    if (enabled != autoUpdate()) {
        m_autoUpdate = enabled;
        emit autoUpdateChanged();

        if (enabled) {
            connect(DBNotify::instance(), SIGNAL(articlesAdded(QStringList, QString)),
                    this, SLOT(onArticlesAdded(QStringList, QString)));
            connect(DBNotify::instance(), SIGNAL(articlesDeleted(QStringList, QString)),
                    this, SLOT(onArticlesDeleted(QStringList, QString)));
            connect(DBNotify::instance(), SIGNAL(articleRead(QString, QString, bool)),
                    this, SLOT(onArticleRead(QString, QString, bool)));
            connect(DBNotify::instance(), SIGNAL(allArticlesRead()), this, SLOT(onAllArticlesRead()));
            connect(DBNotify::instance(), SIGNAL(subscriptionRead(QString, bool)),
                    this, SLOT(onSubscriptionRead(QString, bool)));
            connect(DBNotify::instance(), SIGNAL(subscriptionUpdated(QString)),
                    this, SLOT(onSubscriptionUpdated(QString)));
        }
        else {
            disconnect(DBNotify::instance(), 0, this, 0);
        }
    }
}

void Subscription::load(const QString &id) {
    setId(id);
    setStatus(Active);
    DBConnection::connection(this, SLOT(onSubscriptionFetched(DBConnection*)))->fetchSubscription(id);
}

void Subscription::onArticlesAdded(const QStringList &articleIds, const QString &subscriptionId) {
    if (subscriptionId == id()) {
        setUnreadArticles(unreadArticles() + articleIds.size());
    }
}

void Subscription::onArticlesDeleted(const QStringList &, const QString &subscriptionId) {
    if (subscriptionId == id()) {
        setStatus(Active);
        DBConnection::connection(this, SLOT(onSubscriptionFetched(DBConnection*)))->fetchSubscription(subscriptionId);
    }
}

void Subscription::onArticleRead(const QString &, const QString &subscriptionId, bool isRead) {
    if (subscriptionId == id()) {
        setUnreadArticles(isRead ? unreadArticles() - 1 : unreadArticles() + 1);
    }
}

void Subscription::onAllArticlesRead() {
    setUnreadArticles(0);
}

void Subscription::onSubscriptionFetched(DBConnection *connection) {
    if (connection->status() == DBConnection::Ready) {        
        setDescription(connection->value(1).toString());
        setDownloadEnclosures(connection->value(2).toBool());
        setIconPath(connection->value(3).toString());
        setLastUpdated(QDateTime::fromTime_t(connection->value(4).toInt()));
        setSourceType(SourceType(connection->value(6).toInt()));
        setSource(sourceType() == Plugin ? QtJson::Json::parse(connection->value(5).toString())
                                         : connection->value(5).toString());
        setTitle(connection->value(7).toString());
        setUpdateInterval(connection->value(8).toInt());
        setUrl(connection->value(9).toString());
        setUnreadArticles(connection->value(10).toInt());
        setErrorString(QString());
        setStatus(Ready);
    }
    else {
        setErrorString(connection->errorString());
        setStatus(Error);
    }
    
    connection->deleteLater();
    emit finished(this);
}

void Subscription::onSubscriptionRead(const QString &subscriptionId, bool isRead) {
    if (subscriptionId == id()) {
        if (isRead) {
            setUnreadArticles(0);
        }
        else {
            setStatus(Active);
            DBConnection::connection(this, SLOT(onSubscriptionFetched(DBConnection*)))->fetchSubscription(subscriptionId);
        }
    }
}

void Subscription::onSubscriptionUpdated(const QString &subscriptionId) {
    if (subscriptionId == id()) {
        setStatus(Active);
        DBConnection::connection(this, SLOT(onSubscriptionFetched(DBConnection*)))->fetchSubscription(subscriptionId);
    }
}
