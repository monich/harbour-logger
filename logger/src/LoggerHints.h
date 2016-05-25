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

#ifndef LOGGER_HINTS_H
#define LOGGER_HINTS_H

#include <QtQml>

class MGConfItem;
class LoggerHints : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString appName READ appName WRITE setAppName NOTIFY appNameChanged)
    Q_PROPERTY(int categoryLeftSwipe READ categoryLeftSwipe WRITE setCategoryLeftSwipe NOTIFY categoryLeftSwipeChanged)
    Q_PROPERTY(int categoryLeftSwipeMax READ categoryLeftSwipeMax CONSTANT)

public:
    explicit LoggerHints(QObject* aParent = NULL);

    QString appName() const;
    void setAppName(QString aAppName);

    int categoryLeftSwipe() const;
    int categoryLeftSwipeMax() const;
    void setCategoryLeftSwipe(int aValue);

Q_SIGNALS:
    void appNameChanged();
    void categoryLeftSwipeChanged();

private:
    QString iAppName;
    MGConfItem* iCategoryLeftSwipe;
};

QML_DECLARE_TYPE(LoggerHints)

inline QString LoggerHints::appName() const
    { return iAppName; }
inline int LoggerHints::categoryLeftSwipeMax() const
    { return 1; }

#endif // LOGGER_HINTS_H
