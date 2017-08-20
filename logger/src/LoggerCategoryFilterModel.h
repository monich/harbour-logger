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

#ifndef LOGGER_CATEGORY_FILTER_MODEL_H
#define LOGGER_CATEGORY_FILTER_MODEL_H

#include "LoggerCategory.h"

#include <QList>
#include <QSortFilterProxyModel>

class LoggerCategoryModel;
class LoggerCategoryFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool haveDefaults READ haveDefaults NOTIFY haveDefaultsChanged)

public:
    LoggerCategoryFilterModel(LoggerCategoryModel* aSourceModel);

    int count() const;
    bool haveDefaults() const;

    Q_INVOKABLE void reset();
    Q_INVOKABLE void enableAll();
    Q_INVOKABLE void disableAll();

Q_SIGNALS:
    void countChanged();
    void haveDefaultsChanged();

private:
    LoggerCategoryModel* categoryModel();
    const LoggerCategoryModel* categoryModel() const;
    QList<LoggerCategory> selectedCategories() const;
    int sourceRow(int aRow) const;

private Q_SLOTS:
    void updateHaveDefaults();

private:
    bool iHaveDefaults;
};

inline bool LoggerCategoryFilterModel::haveDefaults() const
    { return iHaveDefaults; }

#endif // LOGGER_CATEGORY_FILTER_MODEL_H
