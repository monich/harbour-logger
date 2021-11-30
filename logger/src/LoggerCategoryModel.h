/*
 * Copyright (C) 2016-2021 Jolla Ltd.
 * Copyright (C) 2016-2021 Slava Monich <slava.monich@jolla.com>
 *
 * You may use this file under the terms of BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. Neither the names of the copyright holders nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef LOGGER_CATEGORY_MODEL_H
#define LOGGER_CATEGORY_MODEL_H

#include "LoggerCategory.h"
#include "LoggerBuffer.h"

#include "dbuslog_client_types.h"

#include <QAbstractListModel>

class LoggerSettings;
class LoggerCategoryModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(bool haveDefaults READ haveDefaults NOTIFY haveDefaultsChanged)

public:
    enum Role {
        NameRole = Qt::UserRole + 1,
        EnabledRole,
        EnabledByDefaultRole,
        IdRole
    };

    LoggerCategoryModel(LoggerSettings* aSettings, DBusLogClient* aClient,
        QObject* aParent);
    ~LoggerCategoryModel();

    virtual QHash<int,QByteArray> roleNames() const;
    virtual int rowCount(const QModelIndex& aParent) const;
    virtual QVariant data(const QModelIndex& aIndex, int aRole) const;

    int count() const;
    bool isConnected() const;
    bool haveDefaults() const;
    void restoreLogLevel() const;

    LoggerCategory loggerCategoryAt(int aRow) const;
    void resetCategories(QList<LoggerCategory> aCategories);
    void enableCategories(QList<LoggerCategory> aCategories);
    void disableCategories(QList<LoggerCategory> aCategories);

    Q_INVOKABLE void reset();
    Q_INVOKABLE void enable(uint id);
    Q_INVOKABLE void disable(uint id);
    Q_INVOKABLE void enableAll();
    Q_INVOKABLE void disableAll();

Q_SIGNALS:
    void countChanged();
    void connectedChanged();
    void haveDefaultsChanged();

private Q_SLOTS:
    void handleConnected();
    void handleCategoryAdded(DBusLogCategory* aCategory, uint aIndex);
    void handleCategoryRemoved(DBusLogCategory* aCategory, uint aIndex);
    void handleCategoryFlags(DBusLogCategory* aCategory, uint aIndex);

private:
    static void connectedProc(DBusLogClient* aClient, gpointer aData);
    static void categoryAddedProc(DBusLogClient* aClient,
        DBusLogCategory* aCategory, guint aIndex, gpointer  aData);
    static void categoryRemovedProc(DBusLogClient* aClient,
        DBusLogCategory* aCategory, guint aIndex, gpointer  aData);
    static void categoryFlagsProc(DBusLogClient* aClient,
        DBusLogCategory* aCategory, guint aIndex, gpointer  aData);

    void updateHaveDefaults();
    DBusLogCategory* categoryAt(int aRow) const;

private:
    enum DBusLogClientSignals {
        DBusLogClientSignalConnected,
        DBusLogClientSignalCategoryAdded,
        DBusLogClientSignalCategoryRemoved,
        DBusLogClientSignalCategoryFlags,
        DBusLogClientSignalCount
    };

    LoggerSettings* iSettings;
    DBusLogClient* iClient;
    gulong iClientSignals[DBusLogClientSignalCount];
    QHash<guint32,LoggerCategory> iCategories;
    DBUSLOG_LEVEL iDefaultLevel;
    bool iHaveDefaults;
};

inline LoggerCategory LoggerCategoryModel::loggerCategoryAt(int aRow) const
    { return LoggerCategory(categoryAt(aRow)); }
inline bool LoggerCategoryModel::haveDefaults() const
    { return iHaveDefaults; }

#endif // LOGGER_CATEGORY_MODEL_H
