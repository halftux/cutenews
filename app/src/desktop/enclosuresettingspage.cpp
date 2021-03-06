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

#include "enclosuresettingspage.h"
#include "pluginconfigmodel.h"
#include "pluginsettingspage.h"
#include <QLabel>
#include <QListView>
#include <QScrollArea>
#include <QSplitter>
#include <QVBoxLayout>

EnclosureSettingsPage::EnclosureSettingsPage(QWidget *parent) :
    SettingsPage(parent),
    m_model(new PluginConfigModel(this)),
    m_view(new QListView(this)),
    m_scrollArea(new QScrollArea(this)),
    m_splitter(new QSplitter(Qt::Horizontal, this)),
    m_layout(new QVBoxLayout(this))
{
    setWindowTitle(tr("Enclosures"));

    m_view->setModel(m_model);
    m_view->setUniformItemSizes(true);

    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setWidget(new QLabel(tr("No plugin selected"), m_scrollArea));

    m_splitter->addWidget(m_view);
    m_splitter->addWidget(m_scrollArea);
    m_splitter->setStretchFactor(1, 1);

    m_layout->addWidget(m_splitter);

    connect(m_view, SIGNAL(clicked(QModelIndex)), this, SLOT(setCurrentPlugin(QModelIndex)));
}

void EnclosureSettingsPage::save() {
    if (PluginSettingsPage *page = qobject_cast<PluginSettingsPage*>(m_scrollArea->widget())) {
        page->save();
    }
}

void EnclosureSettingsPage::setCurrentPlugin(const QModelIndex &index) {
    save();
    
    if (!index.isValid()) {
        m_scrollArea->setWidget(new QLabel(tr("No plugin selected"), m_scrollArea));
        return;
    }

    const bool supportsEnclosures = index.data(PluginConfigModel::SupportsEnclosuresRole).toBool();

    if (supportsEnclosures) {
        const QString id = index.data(PluginConfigModel::IdRole).toString();
        const QVariantList settings = index.data(PluginConfigModel::EnclosureSettingsRole).toList();
        
        if ((!id.isEmpty()) && (!settings.isEmpty())) {
            m_scrollArea->setWidget(new PluginSettingsPage(id, settings, m_scrollArea));
            return;
        }
    }
    
    m_scrollArea->setWidget(new QLabel(tr("No settings for this plugin"), m_scrollArea));
}
