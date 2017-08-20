/*
 * Copyright (C) 2016-2017 Jolla Ltd.
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

#include "LoggerCategoryModel.h"
#include "LoggerSettings.h"
#include "HarbourDebug.h"

#include "gutil_strv.h"

#undef signals
#include "dbuslog_client.h"

#define SUPER QAbstractListModel

LoggerCategoryModel::LoggerCategoryModel(LoggerSettings* aSettings,
    DBusLogClient* aClient, QObject* aParent) : SUPER(aParent),
    iSettings(aSettings),
    iClient(dbus_log_client_ref(aClient)),
    iHaveDefaults(false)
{
    memset(iClientSignals, 0, sizeof(iClientSignals));
    iClientSignals[DBusLogClientSignalConnected] =
        dbus_log_client_add_connected_handler(iClient,
            connectedProc, this);
    iClientSignals[DBusLogClientSignalCategoryAdded] =
        dbus_log_client_add_category_added_handler(iClient,
            categoryAddedProc, this);
    iClientSignals[DBusLogClientSignalCategoryRemoved] =
        dbus_log_client_add_category_removed_handler(iClient,
            categoryRemovedProc, this);
    iClientSignals[DBusLogClientSignalCategoryFlags] =
        dbus_log_client_add_category_flags_handler(iClient,
            categoryFlagsProc, this);
    handleConnected();
}

LoggerCategoryModel::~LoggerCategoryModel()
{
    dbus_log_client_remove_handlers(iClient, iClientSignals, G_N_ELEMENTS(iClientSignals));
    dbus_log_client_unref(iClient);
}

int LoggerCategoryModel::count() const
{
    return iClient->connected ? iClient->categories->len : 0;
}

bool LoggerCategoryModel::isConnected() const
{
    return iClient->connected != FALSE;
}

DBusLogCategory* LoggerCategoryModel::categoryAt(int aRow) const
{
    const int count = iClient->categories->len;
    if (aRow >= 0 && aRow < count) {
        return (DBusLogCategory*)g_ptr_array_index(iClient->categories, aRow);
    } else {
        return NULL;
    }
}

QHash<int,QByteArray> LoggerCategoryModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "categoryName";
    roles[EnabledRole] = "categoryEnabled";
    roles[EnabledByDefaultRole] = "categoryEnabledByDefault";
    roles[IdRole] = "categoryId";
    return roles;
}

int LoggerCategoryModel::rowCount(const QModelIndex& aParent) const
{
    return count();
}

QVariant LoggerCategoryModel::data(const QModelIndex& aIndex, int aRole) const
{
    QVariant value;
    LoggerCategory cat(loggerCategoryAt(aIndex.row()));
    if (cat.isValid()) {
        switch (aRole) {
        case NameRole:
            value = cat.name();
            break;
        case EnabledRole:
            value = cat.enabled();
            break;
        case EnabledByDefaultRole:
            value = cat.enabledByDefault();
            break;
        case IdRole:
            value = cat.id();
            break;
        default:
            break;
        }
    }
    return value;
}

void LoggerCategoryModel::enable(uint aId)
{
    LoggerCategory category(iCategories.value(aId));
    HASSERT(category.isValid());
    if (category.isValid() && !category.enabled()) {
        HDEBUG(category.cname());
        dbus_log_client_enable_category(iClient, category.cname(), NULL, NULL);
    }
}

void LoggerCategoryModel::disable(uint aId)
{
    LoggerCategory category(iCategories.value(aId));
    HASSERT(category.isValid());
    if (category.isValid() && category.enabled()) {
        HDEBUG(category.cname());
        dbus_log_client_disable_category(iClient, category.cname(), NULL, NULL);
    }
}

void LoggerCategoryModel::enableAll()
{
    HDEBUG("enabling all categories");
    dbus_log_client_enable_pattern(iClient, "*", NULL, NULL);
}

void LoggerCategoryModel::disableAll()
{
    HDEBUG("disabling all categories");
    dbus_log_client_disable_pattern(iClient, "*", NULL, NULL);
}

void LoggerCategoryModel::resetCategories(QList<LoggerCategory> aCategories)
{
    GStrV* enable = NULL;
    GStrV* disable = NULL;
    const guint n = aCategories.count();
    for (guint i = 0; i < n; i++) {
        LoggerCategory cat = aCategories.at(i);
        if (cat.enabledByDefault()) {
            if (!cat.enabled()) {
                HDEBUG("enable" << cat.cname());
                enable = gutil_strv_add(enable, cat.cname());
            }
        } else {
            if (cat.enabled()) {
                HDEBUG("disable" << cat.cname());
                disable = gutil_strv_add(disable, cat.cname());
            }
        }
    }
    if (enable) {
        dbus_log_client_enable_categories(iClient, enable, NULL, NULL);
        g_strfreev(enable);
    }
    if (disable) {
        dbus_log_client_disable_categories(iClient, disable, NULL, NULL);
        g_strfreev(disable);
    }
}

void LoggerCategoryModel::enableCategories(QList<LoggerCategory> aCategories)
{
    GStrV* enable = NULL;
    const guint n = aCategories.count();
    for (guint i = 0; i < n; i++) {
        LoggerCategory cat = aCategories.at(i);
        if (cat.isValid() && !cat.enabled()) {
            HDEBUG("enable" << cat.cname());
            enable = gutil_strv_add(enable, cat.cname());
        }
    }
    if (enable) {
        dbus_log_client_enable_categories(iClient, enable, NULL, NULL);
        g_strfreev(enable);
    }
}

void LoggerCategoryModel::disableCategories(QList<LoggerCategory> aCategories)
{
    GStrV* disable = NULL;
    const guint n = aCategories.count();
    for (guint i = 0; i < n; i++) {
        LoggerCategory cat = aCategories.at(i);
        if (cat.enabled()) {
            HDEBUG("disable" << cat.cname());
            disable = gutil_strv_add(disable, cat.cname());
        }
    }
    if (disable) {
        dbus_log_client_disable_categories(iClient, disable, NULL, NULL);
        g_strfreev(disable);
    }
}

void LoggerCategoryModel::reset()
{
    GStrV* enable = NULL;
    GStrV* disable = NULL;
    for (guint i = 0; i < iClient->categories->len; i++) {
        const DBusLogCategory* cat = categoryAt(i);
        if (cat->flags & DBUSLOG_CATEGORY_FLAG_ENABLED_BY_DEFAULT) {
            if (!(cat->flags & DBUSLOG_CATEGORY_FLAG_ENABLED)) {
                HDEBUG("enable" << cat->name);
                enable = gutil_strv_add(enable, cat->name);
            }
        } else {
            if (cat->flags & DBUSLOG_CATEGORY_FLAG_ENABLED) {
                HDEBUG("disable" << cat->name);
                disable = gutil_strv_add(disable, cat->name);
            }
        }
    }
    if (enable) {
        dbus_log_client_enable_categories(iClient, enable, NULL, NULL);
        g_strfreev(enable);
    }
    if (disable) {
        dbus_log_client_disable_categories(iClient, disable, NULL, NULL);
        g_strfreev(disable);
    }
}

void LoggerCategoryModel::updateHaveDefaults()
{
    bool haveDefaults = false;
    if (iClient->connected) {
        for (guint i = 0; i < iClient->categories->len; i++) {
            const DBusLogCategory* cat = categoryAt(i);
            if (cat->flags & DBUSLOG_CATEGORY_FLAG_ENABLED_BY_DEFAULT) {
                haveDefaults = true;
                break;
            }
        }
    }
    if (iHaveDefaults != haveDefaults) {
        iHaveDefaults = haveDefaults;
        Q_EMIT haveDefaultsChanged();
    }
}

void LoggerCategoryModel::handleConnected()
{
    if (iClient->connected) {
        HDEBUG("connected");
        beginResetModel();
        iCategories.clear();
        for (guint i = 0; i < iClient->categories->len; i++) {
            DBusLogCategory* cat = categoryAt(i);
            iCategories.insert(cat->id, LoggerCategory(cat));
        }
        endResetModel();
        // Make sure that verbose logging is enabled. The default is often
        // DEBUG which turns off e.g. RIL data dump in ofono
        if (iClient->default_level < DBUSLOG_LEVEL_VERBOSE) {
            dbus_log_client_set_default_level(iClient,
                DBUSLOG_LEVEL_VERBOSE, NULL, NULL);
        }
    } else if (!iCategories.isEmpty()) {
        HDEBUG("disconnected");
        beginResetModel();
        iCategories.clear();
        endResetModel();
    }
    updateHaveDefaults();
    Q_EMIT countChanged();
    Q_EMIT connectedChanged();

    // Automatically enable all categories if necessary
    if (iSettings->autoEnableLogging() == LoggerSettings::AutoEnableAll) {
        enableAll();
    }
}

void LoggerCategoryModel::handleCategoryAdded(DBusLogCategory* aCategory, uint aIndex)
{
    HDEBUG(aCategory->name);
    beginInsertRows(QModelIndex(), aIndex, aIndex);
    iCategories.insert(aCategory->id, LoggerCategory(aCategory));
    endInsertRows();
    updateHaveDefaults();
    Q_EMIT countChanged();
}

void LoggerCategoryModel::handleCategoryRemoved(DBusLogCategory* aCategory, uint aIndex)
{
    HDEBUG(aCategory->name);
    beginRemoveRows(QModelIndex(), aIndex, aIndex);
    iCategories.remove(aCategory->id);
    endRemoveRows();
    updateHaveDefaults();
    Q_EMIT countChanged();
}

void LoggerCategoryModel::handleCategoryFlags(DBusLogCategory* aCategory, uint aIndex)
{
    HDEBUG(aIndex << aCategory->name << aCategory->flags);
    QModelIndex index(createIndex(aIndex, 0));
    QVector<int> roles;
    roles.append(EnabledRole);
    Q_EMIT dataChanged(index, index, roles);
}

void LoggerCategoryModel::connectedProc(DBusLogClient* aClient, gpointer aData)
{
    LoggerCategoryModel* model = (LoggerCategoryModel*)aData;
    QMetaObject::invokeMethod(model, "handleConnected");
}

// Why invokeMethod? See https://bugreports.qt.io/browse/QTBUG-18434
void LoggerCategoryModel::categoryAddedProc(DBusLogClient* aClient,
    DBusLogCategory* aCategory, guint aIndex, gpointer  aData)
{
    QMetaObject::invokeMethod((LoggerCategoryModel*)aData,
        "handleCategoryAdded",
        Q_ARG(DBusLogCategory*,aCategory),
        Q_ARG(uint,aIndex));
}

void LoggerCategoryModel::categoryRemovedProc(DBusLogClient* aClient,
    DBusLogCategory* aCategory, guint aIndex, gpointer  aData)
{
    QMetaObject::invokeMethod((LoggerCategoryModel*)aData,
        "handleCategoryRemoved",
        Q_ARG(DBusLogCategory*,aCategory),
        Q_ARG(uint,aIndex));
}

void LoggerCategoryModel::categoryFlagsProc(DBusLogClient* aClient,
    DBusLogCategory* aCategory, guint aIndex, gpointer  aData)
{
    QMetaObject::invokeMethod((LoggerCategoryModel*)aData,
        "handleCategoryFlags",
        Q_ARG(DBusLogCategory*,aCategory),
        Q_ARG(uint,aIndex));
}
