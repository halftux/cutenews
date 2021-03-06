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

#include "clientsettingspage.h"
#include "settings.h"
#include <QCheckBox>
#include <QFormLayout>
#include <QLineEdit>

ClientSettingsPage::ClientSettingsPage(QWidget *parent) :
    SettingsPage(parent),
    m_authCheckBox(new QCheckBox(tr("Enable server &authentication"), this)),
    m_javascriptCheckBox(new QCheckBox(tr("Enable &JavaScript in browser"), this)),
    m_addressEdit(new QLineEdit(this)),
    m_usernameEdit(new QLineEdit(this)),
    m_passwordEdit(new QLineEdit(this)),
    m_layout(new QFormLayout(this))
{
    setWindowTitle(tr("Client"));
    
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    
    m_layout->addRow(tr("Server &address:"), m_addressEdit);
    m_layout->addRow(m_authCheckBox);
    m_layout->addRow(tr("Server &username:"), m_usernameEdit);
    m_layout->addRow(tr("Server &password:"), m_passwordEdit);
    m_layout->addRow(m_javascriptCheckBox);
    
    restore();
}

void ClientSettingsPage::restore() {
    m_authCheckBox->setChecked(Settings::serverAuthenticationEnabled());
    m_addressEdit->setText(Settings::serverAddress());
    m_usernameEdit->setText(Settings::serverUsername());
    m_passwordEdit->setText(Settings::serverPassword());
    m_javascriptCheckBox->setChecked(Settings::enableJavaScriptInBrowser());
}

void ClientSettingsPage::save() {
    Settings::setServerUsername(m_usernameEdit->text());
    Settings::setServerPassword(m_passwordEdit->text());
    Settings::setServerAuthenticationEnabled(m_authCheckBox->isChecked());
    Settings::setServerAddress(m_addressEdit->text());
    Settings::setEnableJavaScriptInBrowser(m_javascriptCheckBox->isChecked());
}
