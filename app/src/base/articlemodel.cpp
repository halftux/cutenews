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
#ifdef WIDGETS_UI
#include <QFont>
#include <QIcon>
#endif

ArticleModel::ArticleModel(QObject *parent) :
    QAbstractListModel(parent),
    m_limit(-1),
    m_offset(0),
    m_insert(false),
    m_moreResults(false),
    m_status(Idle),
    m_subscriptionId(ALL_ARTICLES_SUBSCRIPTION_ID)
{
    m_roles[IdRole] = "id";
    m_roles[AuthorRole] = "author";
    m_roles[BodyRole] = "body";
    m_roles[CategoriesRole] = "categories";
    m_roles[DateRole] = "date";
    m_roles[EnclosuresRole] = "enclosures";
    m_roles[FavouriteRole] = "favourite";
    m_roles[HasEnclosuresRole] = "hasEnclosures";
    m_roles[ReadRole] = "read";
    m_roles[SubscriptionIdRole] = "subscriptionId";
    m_roles[TitleRole] = "title";
    m_roles[UrlRole] = "url";
#if QT_VERSION <= 0x050000
    setRoleNames(m_roles);
#endif
    connect(DBNotify::instance(), SIGNAL(articlesAdded(QStringList, QString)),
            this, SLOT(onArticlesAdded(QStringList, QString)));
    connect(DBNotify::instance(), SIGNAL(articlesDeleted(QStringList, QString)),
            this, SLOT(onArticlesDeleted(QStringList, QString)));
    connect(DBNotify::instance(), SIGNAL(articleFavourited(QString, bool)),
            this, SLOT(onArticleFavourited(QString, bool)));
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

ArticleModel::Status ArticleModel::status() const {
    return m_status;
}

void ArticleModel::setStatus(ArticleModel::Status s) {
    if (s != status()) {
        m_status = s;
        emit statusChanged(s);
    }
}

#if QT_VERSION >= 0x050000
QHash<int, QByteArray> ArticleModel::roleNames() const {
    return m_roles;
}
#endif

int ArticleModel::rowCount(const QModelIndex &) const {
    return m_list.size();
}

#ifdef WIDGETS_UI
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
#endif

bool ArticleModel::canFetchMore(const QModelIndex &) const {
    return (m_moreResults) && (status() == Ready) && (limit() > 0);
}

void ArticleModel::fetchMore(const QModelIndex &) {
    if (canFetchMore()) {
        if (!m_query.isEmpty()) {
            fetchArticles(m_query);
        }
        else {
            if (m_subscriptionId == ALL_ARTICLES_SUBSCRIPTION_ID) {
                fetchArticles();
            }
            else if (m_subscriptionId == FAVOURITES_SUBSCRIPTION_ID) {
                fetchArticles("WHERE isFavourite = 1");
            }
            else {
                fetchArticles(QString("WHERE subscriptionId = '%1'").arg(m_subscriptionId));
            }
        }
    }
}

QVariant ArticleModel::data(const QModelIndex &index, int role) const {
    if (const Article *article = get(index.row())) {
#ifdef WIDGETS_UI
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
                return article->date().toString("dd/MM/yyyy HH:mm");
            case Qt::FontRole:
                if (!article->isRead()) {
                    QFont font;
                    font.setBold(true);
                    return font;
                }
                
                return QFont();
            case SortRole:
                return article->date();
            default:
                break;
            }
        case 2:
            switch (role) {
            case Qt::DisplayRole:
            case SortRole:
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
#endif
        return article->property(m_roles.value(role));
    }
    
    return QVariant();
}

QVariant ArticleModel::data(int row, const QByteArray &role) const {
    if (const Article *article = get(row)) {
        return article->property(role);
    }
    
    return QVariant();
}

QMap<int, QVariant> ArticleModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;
        
    if (const Article *article = get(index.row())) {
        QHashIterator<int, QByteArray> iterator(m_roles);
        
        while (iterator.hasNext()) {
            iterator.next();
            map[iterator.key()] = article->property(iterator.value());
        }
    }
    
    return map;
}

QVariantMap ArticleModel::itemData(int row) const {
    QVariantMap map;
        
    if (const Article *article = get(row)) {
        foreach (const QByteArray &role, m_roles.values()) {
            map[role] = article->property(role);
        }
    }
    
    return map;
}

Article* ArticleModel::get(int row) const {
    if ((row >= 0) && (row < m_list.size())) {
        return m_list.at(row);
    }
    
    return 0;
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
        setStatus(Active);
        beginResetModel();
        qDeleteAll(m_list);
        m_list.clear();
        endResetModel();
        emit countChanged(0);
        m_offset = 0;
        m_moreResults = false;
        setStatus(Idle);
    }
}

void ArticleModel::load(const QString &subscriptionId) {
    if (status() == Active) {
        return;
    }
    
    m_subscriptionId = subscriptionId;
    m_query = QString();
    clear();
    
    if (subscriptionId == ALL_ARTICLES_SUBSCRIPTION_ID) {
        fetchArticles();
    }
    else if (subscriptionId == FAVOURITES_SUBSCRIPTION_ID) {
        fetchArticles("WHERE isFavourite = 1");
    }
    else {
        fetchArticles(QString("WHERE subscriptionId = '%1'").arg(subscriptionId));
    }
}

void ArticleModel::search(const QString &query) {
    if (status() == Active) {
        return;
    }
    
    m_query = query;
    m_subscriptionId = QString();
    clear();
    fetchArticles(query);
}

void ArticleModel::reload() {
    if (!m_query.isEmpty()) {
        search(m_query);
    }
    else {
        load(m_subscriptionId);
    }
}

void ArticleModel::fetchArticles(const QString &query) {
    m_insert = false;
    setStatus(Active);
    DBConnection::connection(this, SLOT(onArticlesFetched(DBConnection*)))->fetchArticles(QString("%1 %2%3")
                            .arg(query).arg(QString("ORDER BY date DESC")).arg(limit() > 0 ? QString(" LIMIT %1, %2")
                            .arg(m_offset).arg(limit()) : QString()));
}

void ArticleModel::onArticleChanged(Article *article) {
    const int i = m_list.indexOf(article);
    
    if (i >= 0) {
#ifdef WIDGETS_UI
        emit dataChanged(index(i, 0), index(i, 2));
#else
        const QModelIndex idx = index(i);
        emit dataChanged(idx, idx);
#endif
    }
}

void ArticleModel::onArticlesAdded(const QStringList &articleIds, const QString &subscriptionId) {
    if ((status() == Ready) && ((subscriptionId == m_subscriptionId)
                                || (m_subscriptionId == ALL_ARTICLES_SUBSCRIPTION_ID))) {
        m_insert = true;
        setStatus(Active);
        DBConnection::connection(this, SLOT(onArticlesFetched(DBConnection*)))->fetchArticles(articleIds);
    }
}

void ArticleModel::onArticlesDeleted(const QStringList &articleIds, const QString &subscriptionId) {
    if ((subscriptionId == m_subscriptionId) || (m_subscriptionId == ALL_ARTICLES_SUBSCRIPTION_ID)
        || (m_subscriptionId == FAVOURITES_SUBSCRIPTION_ID)) {

        for (int i = 0; i < articleIds.size(); i++) {
            for (int j = 0; j < m_list.size(); j++) {
                if (m_list.at(j)->id() == articleIds.at(i)) {
                    beginRemoveRows(QModelIndex(), j, j);
                    m_list.takeAt(j)->deleteLater();
                    endRemoveRows();
                    emit countChanged(rowCount());
                    m_offset--;
                    break;
                }
            }
        }
    }
}

void ArticleModel::onArticleFavourited(const QString &articleId, bool isFavourite) {
    if ((m_subscriptionId == FAVOURITES_SUBSCRIPTION_ID) && (status() == Ready)) {
        if (isFavourite) {
            m_insert = true;
            setStatus(Active);
            DBConnection::connection(this, SLOT(onArticlesFetched(DBConnection*)))->fetchArticles(QStringList()
                                                                                                  << articleId);
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
        
        if (m_insert) {
            while (connection->nextRecord()) {
                beginInsertRows(QModelIndex(), 0, 0);
                Article *article = new Article(connection->value(0).toString(), connection->value(1).toString(),
                                               connection->value(2).toString(),
                                               connection->value(3).toString().split(", ", QString::SkipEmptyParts),
                                               QDateTime::fromTime_t(connection->value(4).toInt()),
                                               QtJson::Json::parse(connection->value(5).toString()).toList(),
                                               connection->value(6).toBool(), connection->value(7).toBool(),
                                               connection->value(8).toString(), connection->value(9).toString(),
                                               connection->value(10).toString(), this);
                article->setAutoUpdate(true);
                connect(article, SIGNAL(dataChanged(Article*)), this, SLOT(onArticleChanged(Article*)));
                m_list.prepend(article);
                endInsertRows();
            }
        }
        else {
            while (connection->nextRecord()) {
                beginInsertRows(QModelIndex(), rowCount(), rowCount());
                Article *article = new Article(connection->value(0).toString(), connection->value(1).toString(),
                                               connection->value(2).toString(),
                                               connection->value(3).toString().split(", ", QString::SkipEmptyParts),
                                               QDateTime::fromTime_t(connection->value(4).toInt()),
                                               QtJson::Json::parse(connection->value(5).toString()).toList(),
                                               connection->value(6).toBool(), connection->value(7).toBool(),
                                               connection->value(8).toString(), connection->value(9).toString(),
                                               connection->value(10).toString(), this);
                article->setAutoUpdate(true);
                connect(article, SIGNAL(dataChanged(Article*)), this, SLOT(onArticleChanged(Article*)));
                m_list << article;
                endInsertRows();
            }
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
