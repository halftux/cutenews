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

#ifndef URLOPENERMODEL_H
#define URLOPENERMODEL_H

#include "selectionmodel.h"

class EnclosureRequest;

class UrlOpenerModel : public SelectionModel
{
    Q_OBJECT
    
public:
    ~UrlOpenerModel();

    static UrlOpenerModel* instance();
    
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    Q_INVOKABLE void append(const QString &regExp, const QVariant &command);
    Q_INVOKABLE void insert(int row, const QString &regExp, const QVariant &command);
        
public Q_SLOTS:    
    bool open(const QString &url);
    bool openWithPlugin(const QString &url);
    
    void load();
    void save();

private Q_SLOTS:
    void onEnclosureRequestFinished(EnclosureRequest *request);

private:
    UrlOpenerModel();

    static UrlOpenerModel *self;
};

#endif // URLOPENERMODEL_H
