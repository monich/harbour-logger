/*
 * Copyright (C) 2017 Jolla Ltd.
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

#include "LoggerCategoryFilterModel.h"
#include "LoggerCategoryModel.h"
#include "HarbourDebug.h"

LoggerCategoryFilterModel::LoggerCategoryFilterModel(LoggerCategoryModel* aSource) :
    QSortFilterProxyModel(aSource),
    iHaveDefaults(aSource->haveDefaults())
{
    setSourceModel(aSource);
    setDynamicSortFilter(true);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setFilterRole(LoggerCategoryModel::NameRole);

    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)), SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)), SIGNAL(countChanged()));
    connect(this, SIGNAL(modelReset()), SIGNAL(countChanged()));

    connect(this, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), SLOT(updateHaveDefaults()));
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(updateHaveDefaults()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(updateHaveDefaults()));
    connect(this, SIGNAL(modelReset()), SLOT(updateHaveDefaults()));
}

int LoggerCategoryFilterModel::count() const
{
    return rowCount();
}

int LoggerCategoryFilterModel::sourceRow(int aProxyRow) const
{
    QModelIndex proxyIndex = index(aProxyRow, 0);
    QModelIndex sourceIndex = mapToSource(proxyIndex);
    return sourceIndex.isValid() ? sourceIndex.row() : -1;
}

QList<LoggerCategory> LoggerCategoryFilterModel::selectedCategories() const
{
    QList<LoggerCategory> list;
    const LoggerCategoryModel* fullModel = categoryModel();
    if (fullModel) {
        const int n = rowCount();
        list.reserve(n);
        for (int i=0; i<n; i++) {
            LoggerCategory cat = fullModel->loggerCategoryAt(sourceRow(i));
            if (cat.isValid()) {
                list.append(cat);
            }
        }
    }
    return list;
}

void LoggerCategoryFilterModel::reset()
{
    LoggerCategoryModel* fullModel = categoryModel();
    if (fullModel) {
        if (fullModel->count() == count()) {
            // This is more efficient
            fullModel->reset();
        } else {
            HDEBUG("resetting all selected categories");
            fullModel->resetCategories(selectedCategories());
        }
    }
}

void LoggerCategoryFilterModel::enableAll()
{
    LoggerCategoryModel* fullModel = categoryModel();
    if (fullModel) {
        if (fullModel->count() == count()) {
            // This is more efficient
            fullModel->enableAll();
        } else {
            HDEBUG("enabling all selected categories");
            fullModel->enableCategories(selectedCategories());
        }
    }
}

void LoggerCategoryFilterModel::disableAll()
{
    LoggerCategoryModel* fullModel = categoryModel();
    if (fullModel) {
        if (fullModel->count() == count()) {
            // This is more efficient
            fullModel->disableAll();
        } else {
            HDEBUG("disabling all selected categories");
            fullModel->disableCategories(selectedCategories());
        }
    }
}

void LoggerCategoryFilterModel::updateHaveDefaults()
{
    bool haveDefaults = false;
    LoggerCategoryModel* fullModel = categoryModel();
    if (fullModel && fullModel->isConnected()) {
        const int n = rowCount();
        if (fullModel->count() == n) {
            // Same as the source model
            haveDefaults = fullModel->haveDefaults();
        } else {
            for (int i=0; i<n; i++) {
                LoggerCategory cat = fullModel->loggerCategoryAt(sourceRow(i));
                if (cat.enabledByDefault()) {
                    haveDefaults = true;
                    break;
                }
            }
        }
    }
    if (iHaveDefaults != haveDefaults) {
        iHaveDefaults = haveDefaults;
        Q_EMIT haveDefaultsChanged();
    }
}

const LoggerCategoryModel* LoggerCategoryFilterModel::categoryModel() const
{
    return qobject_cast<LoggerCategoryModel*>(parent());
}


LoggerCategoryModel* LoggerCategoryFilterModel::categoryModel()
{
    return qobject_cast<LoggerCategoryModel*>(parent());
}
