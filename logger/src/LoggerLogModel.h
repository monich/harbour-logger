/*
 * Copyright (C) 2016 Jolla Ltd.
 * Contact: Slava Monich <slava.monich@jolla.com>
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
 *   3. Neither the name of Jolla Ltd nor the names of its contributors may
 *      be used to endorse or promote products derived from this software
 *      without specific prior written permission.
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

#ifndef LOGGER_LOG_MODEL_H
#define LOGGER_LOG_MODEL_H

#include "LoggerCategory.h"
#include "LoggerBuffer.h"

#include "dbuslog_client_types.h"
#include "dbuslog_message.h"

#include <QAbstractListModel>

class LoggerLogModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool empty READ isEmpty NOTIFY emptyChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)

public:
    enum Role {
        TypeRole = Qt::UserRole + 1,
        CategoryRole,
        LevelRole,
        TimeRole,
        TextRole
    };

    LoggerLogModel(DBusLogClient* aClient, QObject* aParent);
    ~LoggerLogModel();

    virtual QHash<int,QByteArray> roleNames() const;
    virtual int rowCount(const QModelIndex& aParent) const;
    virtual QVariant data(const QModelIndex& aIndex, int aRole) const;

    bool isEmpty() const;
    bool isConnected() const;

    Q_INVOKABLE void add(QString aText);
    Q_INVOKABLE void clear();

Q_SIGNALS:
    void connected();
    void logCleared();
    void emptyChanged();
    void connectedChanged();
    void entryAdded(LoggerEntry entry);

private:
    static void connectedProc(DBusLogClient* aClient, gpointer aData);
    static void messageProc(DBusLogClient* aClient, DBusLogCategory* aCategory,
        DBusLogMessage* aMessage, gpointer aData);
    static void skipProc(DBusLogClient* aClient, guint aCount, gpointer aData);

    void handleConnected();
    void handleMessage(DBusLogCategory* aCategory, DBusLogMessage* aMessage);
    void handleSkip(guint aCount);
    void addEntry(LoggerEntry aEntry);

private:
    enum DBusLogClientSignals {
        DBusLogClientSignalConnected,
        DBusLogClientSignalMessage,
        DBusLogClientSignalSkip,
        DBusLogClientSignalCount
    };

    DBusLogClient* iClient;
    gulong iClientSignals[DBusLogClientSignalCount];
    QHash<guint32,LoggerCategory> iCategories;
    LoggerBuffer iBuffer;
};

#endif // LOGGER_LOG_MODEL_H
