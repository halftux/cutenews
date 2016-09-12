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

#ifndef JAVASCRIPTENCLOSUREREQUEST_H
#define JAVASCRIPTENCLOSUREREQUEST_H

#include "enclosurerequest.h"
#include "javascriptenclosurerequestglobalobject.h"
#include <QNetworkRequest>

class QScriptEngine;

class JavaScriptEnclosureRequest : public EnclosureRequest
{
    Q_OBJECT

    Q_PROPERTY(QString fileName READ fileName)
    Q_PROPERTY(QString id READ id)

public:
    explicit JavaScriptEnclosureRequest(const QString &id, const QString &fileName, QObject *parent = 0);

    QString fileName() const;

    QString id() const;

    virtual QString errorString() const;

    virtual Enclosure result() const;

    virtual Status status() const;

public Q_SLOTS:
    virtual bool cancel();
    virtual bool getEnclosure(const QString &url, const QVariantMap &settings);

private Q_SLOTS:
    void onRequestError(const QString &errorString);
    void onRequestFinished(const QVariantMap &result);

private:
    void setErrorString(const QString &e);
    
    void setResult(const Enclosure &r);
    
    void setStatus(Status s);
    
    void initEngine();
    
    JavaScriptEnclosureRequestGlobalObject *m_global;
    QScriptEngine *m_engine;
    
    QString m_fileName;
    QString m_id;

    QString m_errorString;

    Enclosure m_result;

    Status m_status;
    
    bool m_evaluated;
};

#endif // JAVASCRIPTENCLOSUREPLUGIN_H
