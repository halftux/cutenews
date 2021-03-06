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

#include "pluginconfigmodel.h"

PluginConfigModel::PluginConfigModel(QObject *parent) :
    QAbstractListModel(parent)
{
    m_roles[DisplayNameRole] = "displayName";
    m_roles[ArticleRegExpRole] = "articleRegExp";
    m_roles[ArticleSettingsRole] = "articleSettings";
    m_roles[EnclosureRegExpRole] = "enclosureRegExp";
    m_roles[EnclosureSettingsRole] = "enclosureSettings";
    m_roles[FeedSettingsRole] = "feedSettings";
    m_roles[IdRole] = "id";
    m_roles[PluginTypeRole] = "pluginType";
    m_roles[SupportsArticlesRole] = "supportsArticles";
    m_roles[SupportsEnclosuresRole] = "supportsEnclosures";
    m_roles[SupportsFeedsRole] = "supportsFeeds";
    m_roles[VersionRole] = "version";
#if QT_VERSION < 0x050000
    setRoleNames(m_roles);
#endif
    reload();
}

#if QT_VERSION >= 0x050000
QHash<int, QByteArray> PluginConfigModel::roleNames() const {
    return m_roles;
}
#endif

int PluginConfigModel::rowCount(const QModelIndex &) const {
    return m_items.size();
}

int PluginConfigModel::columnCount(const QModelIndex &) const {
    return 3;
}

QVariant PluginConfigModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ((orientation != Qt::Horizontal) || (role != Qt::DisplayRole)) {
        return QVariant();
    }
    
    switch (section) {
    case 0:
        return tr("Name");
    case 1:
        return tr("Type");
    case 2:
        return tr("Version");
    default:
        return QVariant();
    }
}

QVariant PluginConfigModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    const FeedPluginConfig *config = m_items.at(index.row());

    if (!config) {
        return QVariant();
    }
    
    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0:
            return config->displayName();
        case 1:
            return config->pluginType();
        case 2:
            return config->version();
        default:
            break;
        }
        
        return QVariant();
    case ArticleRegExpRole:
        return config->articleRegExp();
    case ArticleSettingsRole:
        return config->articleSettings();
    case EnclosureRegExpRole:
        return config->enclosureRegExp();
    case EnclosureSettingsRole:
        return config->enclosureSettings();
    case FeedSettingsRole:
        return config->feedSettings();
    case IdRole:
        return config->id();
    case PluginTypeRole:
        return config->pluginType();
    case SupportsArticlesRole:
        return config->supportsArticles();
    case SupportsEnclosuresRole:
        return config->supportsEnclosures();
    case SupportsFeedsRole:
        return config->supportsFeeds();
    case VersionRole:
        return config->version();
    default:
        return QVariant();
    }
}

QMap<int, QVariant> PluginConfigModel::itemData(const QModelIndex &index) const {
    QMap<int, QVariant> map;

    foreach (const int &role, m_roles.keys()) {
        map[role] = data(index, role);
    }
    
    return map;
}

QVariant PluginConfigModel::data(int row, const QByteArray &role) const {
    return data(index(row), m_roles.key(role));
}

QVariantMap PluginConfigModel::itemData(int row) const {
    QVariantMap map;

    foreach (const QByteArray &role, m_roles.values()) {
        map[QString::fromUtf8(role)] = data(row, role);
    }
    
    return map;
}

QModelIndexList PluginConfigModel::match(const QModelIndex &start, int role, const QVariant &value, int hits,
                                                Qt::MatchFlags flags) const {
    return QAbstractListModel::match(start, role, value, hits, flags);
}

int PluginConfigModel::match(int start, const QByteArray &role, const QVariant &value, int flags) const {
    const QModelIndexList idxs = match(index(start), m_roles.key(role), value, 1, Qt::MatchFlags(flags));
    return idxs.isEmpty() ? -1 : idxs.first().row();
}

void PluginConfigModel::clear() {
    if (!m_items.isEmpty()) {
        beginResetModel();
        m_items.clear();
        endResetModel();
        emit countChanged(0);
    }
}

void PluginConfigModel::reload() {
    clear();
    beginResetModel();
    m_items = PluginManager::instance()->plugins();
    endResetModel();
    emit countChanged(rowCount());
}
