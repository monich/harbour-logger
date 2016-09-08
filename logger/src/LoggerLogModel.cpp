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

#include "LoggerLogModel.h"
#include "LoggerEntry.h"
#include "LoggerSettings.h"
#include "HarbourDebug.h"

#undef signals
#include "dbuslog_client.h"

#define SUPER QAbstractListModel

#define LOG_REMOVE_COUNT_MAX        (100)
#define LOG_SIZE_LIMIT_MIN          (100)
#define LOG_SIZE_LIMIT_NONE         (0)

LoggerLogModel::LoggerLogModel(LoggerSettings* aSettings, DBusLogClient* aClient,
    QObject* aParent) : SUPER(aParent),
    iSettings(aSettings),
    iClient(dbus_log_client_ref(aClient)),
    iBuffer(LoggerSettings::DEFAULT_LOG_SIZE_LIMIT)
{
    // updateLogSizeLimit() will initialize iLogSizeLimit and iLogRemoveCount:
    updateLogSizeLimit();
    connect(iSettings, SIGNAL(logSizeLimitChanged()), SLOT(updateLogSizeLimit()));
    memset(iClientSignals, 0, sizeof(iClientSignals));
    iClientSignals[DBusLogClientSignalConnected] =
        dbus_log_client_add_connected_handler(iClient, connectedProc, this);
    iClientSignals[DBusLogClientSignalMessage] =
        dbus_log_client_add_message_handler(iClient, messageProc, this);
    iClientSignals[DBusLogClientSignalSkip] =
        dbus_log_client_add_skip_handler(iClient, skipProc, this);
    handleConnected();
}

LoggerLogModel::~LoggerLogModel()
{
    dbus_log_client_remove_handlers(iClient, iClientSignals, G_N_ELEMENTS(iClientSignals));
    dbus_log_client_unref(iClient);
}

void LoggerLogModel::updateLogSizeLimit()
{
    iLogSizeLimit = LoggerSettings::DEFAULT_LOG_SIZE_LIMIT;
    int value = iSettings->logSizeLimit();
    if (value <= 0) {
        iLogSizeLimit = LOG_SIZE_LIMIT_NONE;
    } else if (value < LOG_SIZE_LIMIT_MIN) {
        iLogSizeLimit = LOG_SIZE_LIMIT_MIN;
    } else {
        iLogSizeLimit = value;
    }
    if (iLogSizeLimit > 0) {
        // Truncate the buffer if necessary
        const int bufsize = iBuffer.size();
        if (bufsize > iLogSizeLimit) {
            const int dropCount = (bufsize - iLogSizeLimit);
            beginRemoveRows(QModelIndex(), 0, dropCount-1);
            iBuffer.drop(dropCount);
            endRemoveRows();
        }
        HDEBUG("log size limit" << iLogSizeLimit);
        iBuffer.setMaxSize(iLogSizeLimit);
        iLogRemoveCount = iLogSizeLimit/50;
        if (iLogRemoveCount > LOG_REMOVE_COUNT_MAX) {
            iLogRemoveCount = LOG_REMOVE_COUNT_MAX;
        }
    } else {
        HDEBUG("log size unlimited");
        iLogRemoveCount = LOG_REMOVE_COUNT_MAX;
        iBuffer.setMaxSize(GUTIL_RING_UNLIMITED_SIZE);
    }
}

void LoggerLogModel::handleConnected()
{
    iCategories.clear();
    if (iClient->connected) {
        HDEBUG("connected");
        for (guint i = 0; i < iClient->categories->len; i++) {
            DBusLogCategory* cat = (DBusLogCategory*)g_ptr_array_index(iClient->categories, i);
            iCategories.insert(cat->id, LoggerCategory(cat));
        }
        Q_EMIT connected();
    }
    Q_EMIT connectedChanged();
}

void LoggerLogModel::handleMessage(DBusLogCategory* aCategory, DBusLogMessage* aMessage)
{
    LoggerCategory cat;
    if (aCategory) {
        HASSERT(iCategories.contains(aCategory->id));
        cat = iCategories.value(aCategory->id);
    }
    addEntry(LoggerEntry(cat, aMessage));
}

void LoggerLogModel::handleSkip(guint aCount)
{
    addEntry(LoggerEntry(aCount));
}

void LoggerLogModel::addEntry(LoggerEntry aEntry)
{
    const bool wasEmpty = isEmpty();
    if (!iBuffer.canPut(1)) {
        beginRemoveRows(QModelIndex(), 0, iLogRemoveCount-1);
        iBuffer.drop(iLogRemoveCount);
        endRemoveRows();
    }
    const int count = iBuffer.size();
    beginInsertRows(QModelIndex(), count, count);
    HVERIFY(iBuffer.put(aEntry));
    endInsertRows();
    if (wasEmpty) {
        Q_EMIT emptyChanged();
    }
    Q_EMIT entryAdded(aEntry);
}

void LoggerLogModel::add(QString aText)
{
    addEntry(LoggerEntry(aText));
}

void LoggerLogModel::clear()
{
    const int count = iBuffer.size();
    if (count > 0) {
        HDEBUG("Clearing the model");
        beginResetModel();
        iBuffer.clear();
        endResetModel();
        Q_EMIT emptyChanged();
        Q_EMIT logCleared();
    }
}

bool LoggerLogModel::isEmpty() const
{
    return iBuffer.isEmpty();
}

bool LoggerLogModel::isConnected() const
{
    return iClient->connected != FALSE;
}

QHash<int,QByteArray> LoggerLogModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[CategoryRole] = "messageCategory";
    roles[LevelRole] = "messageLevel";
    roles[TimeRole] = "messageTime";
    roles[TextRole] = "messageText";
    roles[TypeRole] = "messageType";
    return roles;
}

int LoggerLogModel::rowCount(const QModelIndex& aParent) const
{
    // Leave the last row empty
    return iBuffer.size() + 1;
}

QVariant LoggerLogModel::data(const QModelIndex& aIndex, int aRole) const
{
    QVariant value;
    const int row = aIndex.row();
    if (row >= 0) {
        const int count = iBuffer.size();
        if (row < count) {
            LoggerEntry entry(iBuffer.at(row));
            switch (aRole) {
            case TypeRole:
                value = entry.type();
                break;
            case LevelRole:
                value = entry.level();
                break;
            case TimeRole:
                value = entry.time().isValid() ?
                    entry.time().toString("hh:mm:ss.zzz") :
                    QString();
                break;
            case CategoryRole:
                value = entry.category().hidden() ?
                    QString() : entry.category().name();
                break;
            case TextRole:
                value = entry.text();
                break;
            default:
                break;
            }
        } else if (row == count) {
            // Last row is empty
            switch (aRole) {
            case TypeRole:
                value = LoggerEntry::TypeEmpty;
                break;
            case TextRole:
                value = QString();
                break;
            default:
                break;
            }
        }
    }
    return value;
}

void LoggerLogModel::connectedProc(DBusLogClient* aClient, gpointer aData)
{
    ((LoggerLogModel*)aData)->handleConnected();
}

void LoggerLogModel::messageProc(DBusLogClient* aClient, DBusLogCategory* aCategory,
    DBusLogMessage* aMessage, gpointer aData)
{
    ((LoggerLogModel*)aData)->handleMessage(aCategory, aMessage);
}

void LoggerLogModel::skipProc(DBusLogClient* aClient, guint aCount, gpointer aData)
{
    ((LoggerLogModel*)aData)->handleSkip(aCount);
}
