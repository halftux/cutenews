/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
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

#ifndef SUBSCRIPTIONSOURCETYPEMODEL_H
#define SUBSCRIPTIONSOURCETYPEMODEL_H

#include "selectionmodel.h"
#include "pluginmanager.h"
#include "subscription.h"

class SubscriptionSourceTypeModel : public SelectionModel
{
    Q_OBJECT
    
public:
    explicit SubscriptionSourceTypeModel(QObject *parent = 0) :
        SelectionModel(parent)
    {
        append(tr("URL"), Subscription::Url);
        append(tr("Local file"), Subscription::LocalFile);
        append(tr("Command"), Subscription::Command);
        
        const FeedPluginList plugins = PluginManager::instance()->plugins();
        
        for (int i = 0; i < plugins.size(); i++) {
            const FeedPluginConfig *config = plugins.at(i).config;

            if (config->supportsFeeds()) {
                append(config->displayName(), config->id());
            }
        }
    }
};

#endif // SUBSCRIPTIONSOURCETYPEMODEL_H
