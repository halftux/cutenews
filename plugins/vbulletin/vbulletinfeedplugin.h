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

#ifndef VBULLETINFEEDPLUGIN_H
#define VBULLETINFEEDPLUGIN_H

#include "feedplugin.h"
#include "vbulletinfeedrequest.h"
#if QT_VERSION < 0x050000
#include <QtPlugin>
#endif

class VbulletinFeedPlugin : public QObject, public FeedPlugin
{
    Q_OBJECT
    Q_INTERFACES(FeedPlugin)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.cutenews.VbulletinFeedPlugin")
#endif

public:
    virtual FeedRequest* feedRequest(QObject *parent = 0) { return new VbulletinFeedRequest(parent); }
};

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(cutenews-vbulletin, VbulletinFeedPlugin)
#endif

#endif // VBULLETINFEEDPLUGIN_H
