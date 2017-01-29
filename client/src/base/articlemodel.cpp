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

#include "articlemodel.h"
#include "article.h"
#include "dbconnection.h"
#include "dbnotify.h"
#include "definitions.h"
#include "json.h"
#include <QFont>
#include <QIcon>

ArticleModel::ArticleModel(QObject *parent) :
    QAbstractListModel(parent),
    m_limit(0),
    m_offset(0),
    m_moreResults(false),
    m_status(Idle),
    m_subscriptionId(ALL_ARTICLES_SUBSCRIPTION_ID)
{
    m_roles[Article::AuthorRole] = "author";
    m_roles[Article::BodyRole] = "body";
    m_roles[Article::CategoriesRole] = "categories";
    m_roles[Article::DateRole] = "date";
    m_roles[Article::DateStringRole] = "dateString";
    m_roles[Article::EnclosuresRole] = "enclosures";
    m_roles[Article::FavouriteRole] = "favourite";
    m_roles[Article::HasEnclosuresRole] = "hasEnclosures";
    m_roles[Article::IdRole] = "id";
    m_roles[Article::ReadRole] = "read";
    m_roles[Article::SubscriptionIdRole] = "subscriptionId";
    m_roles[Article::TitleRole] = "title";
    m_roles[Article::UrlRole] = "url";
#if QT_VERSION <= 0x050000
    setRoleNames(m_roles);
#endif
    connect(DBNotify::instance(), SIGNAL(articleDeleted(QString)), this, SLOT(onArticleDeleted(QString)));
    connect(DBNotify::instance(), SIGNAL(articleFavourited(QString, bool, QVariantMap)),
            this, SLOT(onArticleFavourited(QString, bool, QVariantMap)));
    connect(DBNotify::instance(), SIGNAL(subscriptionDeleted(QString)), this, SLOT(onSubscriptionDeleted(QString)));
}

QString ArticleModel::errorString() const {
    return m_errorString;
}

void ArticleModel::setErrorString(const QString &e) {
    m_errorString = e;
}

int ArticleModel::limit() const {
    return m_limit;
}

void ArticleModel::setLimit(int l) {
    if (l != limit()) {
        m_limit = l;
        emit limitChanged(l);
    }
}

QString ArticleModel::query() const {
    return m_query;
}

ArticleModel::Status ArticleModel::status() const {
    return m_status;
}

void ArticleModel::setStatus(ArticleModel::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

QString ArticleModel::subscriptionId() const {
    return m_subscriptionId;
}

#if QT_VERSION >= 0x050000
QHash<int, QByteArray> ArticleModel::roleNames() const {
    return m_roles;
}
#endif

int ArticleModel::rowCount(const QModelIndex &) const {
    return m_list.size();
}

int ArticleModel::columnCount(const QModelIndex &) const {
    return 3;
}

QVariant ArticleModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole)) {
        switch (section) {
        case 1:
            return tr("Date");
        case 2:
            return tr("Title");
        default:
            break;
        }
    }
    
    return QVariant();
}

bool ArticleModel::canFetchMore(const QModelIndex &) const {
    return (m_moreResults) && (status() == Ready) && (limit() > 0);
}

void ArticleModel::fetchMore(const QModelIndex &) {
    if (canFetchMore()) {
        setStatus(Active);
        DBConnection *connection = DBConnection::connection(this, SLOT(onArticlesFetched(DBConnection*)));
        
        if (!m_query.isEmpty()) {
            connection->searchArticles(m_query, m_offset, m_limit);
        }
        else {
            connection->fetchArticlesForSubscription(m_subscriptionId, m_offset, m_limit);
        }
    }
}

QVariant ArticleModel::data(const QModelIndex &index, int role) const {
    if (const Article *article = get(index.row())) {
        switch (index.column()) {
        case 0:
            switch (role) {
            case Qt::DecorationRole:
                if (article->isFavourite()) {
                    return QIcon::fromTheme("mail-mark-important");
                }
                
                break;
            default:
                break;
            }
            
            break; 
        case 1:
            switch (role) {
            case Qt::DisplayRole:
                return article->dateString();
            case Qt::FontRole:
                if (!article->isRead()) {
                    QFont font;
                    font.setBold(true);
                    return font;
                }
                
                return QFont();
            default:
                break;
            }
        case 2:
            switch (role) {
            case Qt::DisplayRole:
                return article->title();
            case Qt::FontRole:
                if (!article->isRead()) {
                    QFont font;
                    font.setBold(true);
                    return font;
                }
                
                return QFont();
            default:
                break;
            }
        default:
            break;
        }
        
        return article->data(role);
    }
    
    return QVariant();
}

QVariant ArticleModel::data(int row, const QByteArray &role) const {
    return data(index(row), m_roles.key(role));
}

bool ArticleModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (Article *article = get(index.row())) {
        return article->setData(role, value);
    }

    return false;
}

bool ArticleModel::setData(int row, const QVariant &value, const QByteArray &role) {
    return setData(index(row), value, m_roles.key(role));
}

QMap<int, QVariant> ArticleModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;
        
    if (const Article *article = get(index.row())) {
        for (int i = Article::AuthorRole; i <= Article::UrlRole; i++) {
            map[i] = article->data(i);
        }
    }
    
    return map;
}

QVariantMap ArticleModel::itemData(int row) const {
    QVariantMap map;
        
    if (const Article *article = get(row)) {
        for (int i = Article::AuthorRole; i <= Article::UrlRole; i++) {
            map[m_roles.value(i)] = article->data(i);
        }
    }
    
    return map;
}

bool ArticleModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles) {
    QMapIterator<int, QVariant> iterator(roles);

    while (iterator.hasNext()) {
        iterator.next();

        if (!setData(index, iterator.value(), iterator.key())) {
            return false;
        }
    }

    return true;
}

bool ArticleModel::setItemData(int row, const QVariantMap &roles) {
    QMapIterator<QString, QVariant> iterator(roles);

    while (iterator.hasNext()) {
        iterator.next();

        if (!setData(row, iterator.value(), iterator.key().toUtf8())) {
            return false;
        }
    }

    return true;
}

Article* ArticleModel::get(int row) const {
    if ((row >= 0) && (row < m_list.size())) {
        return m_list.at(row);
    }
    
    return 0;
}

bool ArticleModel::remove(int row) {
    if (Article *article = get(row)) {
        article->remove();
        return true;
    }

    return false;
}

QModelIndexList ArticleModel::match(const QModelIndex &start, int role, const QVariant &value, int hits,
                                    Qt::MatchFlags flags) const {
    return QAbstractListModel::match(start, role, value, hits, flags);
}

int ArticleModel::match(int start, const QByteArray &role, const QVariant &value, int flags) const {
    const QModelIndexList indexes = match(index(start), m_roles.key(role), value, Qt::MatchFlags(flags));
    return indexes.isEmpty() ? -1 : indexes.first().row();
}

void ArticleModel::clear() {
    if (!m_list.isEmpty()) {
        beginResetModel();
        qDeleteAll(m_list);
        m_list.clear();
        endResetModel();
        emit countChanged(0);
        m_offset = 0;
        m_moreResults = false;
    }

    setStatus(Idle);
}

void ArticleModel::load(const QString &subscriptionId) {
    if (status() == Active) {
        return;
    }
    
    m_subscriptionId = subscriptionId;
    m_query = QString();
    emit queryChanged(m_query);
    emit subscriptionIdChanged(m_subscriptionId);
    clear();
    DBConnection::connection(this,
    SLOT(onArticlesFetched(DBConnection*)))->fetchArticlesForSubscription(m_subscriptionId, 0, m_limit);
}

void ArticleModel::search(const QString &query) {
    if (status() == Active) {
        return;
    }
    
    m_query = query;
    m_subscriptionId = ALL_ARTICLES_SUBSCRIPTION_ID;
    emit queryChanged(m_query);
    emit subscriptionIdChanged(m_subscriptionId);
    clear();
    DBConnection::connection(this, SLOT(onArticlesFetched(DBConnection*)))->searchArticles(m_query, 0, m_limit);
}

void ArticleModel::reload() {
    if (!m_query.isEmpty()) {
        search(m_query);
    }
    else {
        load(m_subscriptionId);
    }
}

void ArticleModel::onArticleChanged(Article *article, int role) {
    const int row = m_list.indexOf(article);

    if (row == -1) {
        return;
    }

    int column = 0;
    
    switch (role) {
    case Article::ReadRole:
        emit dataChanged(index(row, column), index(row, 2));
        return;
    case Article::DateRole:
        column = 1;
        break;
    case Article::TitleRole:
        column = 2;
        break;
    default:
        break;
    }

    const QModelIndex idx = index(row, column);
    emit dataChanged(idx, idx);
}

void ArticleModel::onArticleDeleted(const QString &articleId) {
    for (int i = 0; i < m_list.size(); i++) {
        if (m_list.at(i)->id() == articleId) {
            beginRemoveRows(QModelIndex(), i, i);
            m_list.at(i)->deleteLater();
            endRemoveRows();
            emit countChanged(rowCount());
            m_offset--;
            return;
        }
    }
}

void ArticleModel::onArticleFavourited(const QString &articleId, bool isFavourite, const QVariantMap &properties) {
    if ((m_subscriptionId == FAVOURITES_SUBSCRIPTION_ID) && (status() == Ready)) {
        if (isFavourite) {
            beginInsertRows(QModelIndex(), 0, 0);
            Article *article = new Article(properties.value("id").toString(), properties.value("author").toString(),
                                           properties.value("body").toString(),
                                           properties.value("categories").toStringList(),
                                           QDateTime::fromTime_t(properties.value("date").toInt()),
                                           properties.value("enclosures").toList(),
                                           properties.value("favourite").toBool(),
                                           properties.value("read").toBool(),
                                           properties.value("subscriptionId").toString(),
                                           properties.value("title").toString(),
                                           properties.value("url").toString(), this);
            article->setAutoUpdate(true);
            connect(article, SIGNAL(dataChanged(Article*, int)), this, SLOT(onArticleChanged(Article*, int)));
            m_list.prepend(article);
            endInsertRows();
        }
        else {
            for (int i = 0; i < m_list.size(); i++) {
                if (m_list.at(i)->id() == articleId) {
                    beginRemoveRows(QModelIndex(), i, i);
                    m_list.takeAt(i)->deleteLater();
                    endRemoveRows();
                    emit countChanged(rowCount());
                    m_offset--;
                    return;
                }
            }
        }
    }
}

void ArticleModel::onArticlesFetched(DBConnection *connection) {
    if (connection->status() == DBConnection::Ready) {
        const int oldCount = rowCount();
        
        foreach (const QVariant &v, connection->result().toList()) {
            const QVariantMap a = v.toMap();
            beginInsertRows(QModelIndex(), rowCount(), rowCount());
            Article *article = new Article(a.value("id").toString(), a.value("author").toString(),
                                           a.value("body").toString(),
                                           a.value("categories").toStringList(),
                                           QDateTime::fromTime_t(a.value("date").toInt()),
                                           a.value("enclosures").toList(),
                                           a.value("favourite").toBool(),
                                           a.value("read").toBool(),
                                           a.value("subscriptionId").toString(),
                                           a.value("title").toString(),
                                           a.value("url").toString(), this);
            article->setAutoUpdate(true);
            connect(article, SIGNAL(dataChanged(Article*, int)), this, SLOT(onArticleChanged(Article*, int)));
            m_list << article;
            endInsertRows();
        }
        
        const int newCount = rowCount();
        m_offset = newCount;
        m_moreResults = ((newCount - oldCount) >= limit());
        
        emit countChanged(newCount);
        setErrorString(QString());
        setStatus(Ready);
    }
    else {
        setErrorString(connection->errorString());
        setStatus(Error);
    }
    
    connection->deleteLater();
}

void ArticleModel::onSubscriptionDeleted(const QString &id) {
    if (m_list.isEmpty()) {
        return;
    }
    
    if (id == m_subscriptionId) {
        clear();
    }
    else if ((m_subscriptionId == ALL_ARTICLES_SUBSCRIPTION_ID) || (m_subscriptionId == FAVOURITES_SUBSCRIPTION_ID)) {
        for (int i = m_list.size() - 1; i >= 0; i--) {
            if (m_list.at(i)->subscriptionId() == id) {
                beginRemoveRows(QModelIndex(), i, i);
                m_list.takeAt(i)->deleteLater();
                m_offset--;
                endRemoveRows();
            }
        }
        
        emit countChanged(rowCount());
    }
}