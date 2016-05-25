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

#include "LoggerHints.h"
#include "HarbourDebug.h"

#include <MGConfItem>

#define DCONF_HINTS_DIR             "/apps/%1/hints/"
#define KEY_CATEGORY_LEFT_SWIPE     "categoryLeftSwipt"
#define DEFAULT_CATEGORY_LEFT_SWIPE 0

LoggerHints::LoggerHints(QObject* aParent) :
    QObject(aParent),
    iCategoryLeftSwipe()
{
}

void LoggerHints::setAppName(QString aAppName)
{
    if (iAppName != aAppName) {
        HDEBUG(aAppName);
        delete iCategoryLeftSwipe;
        if (aAppName.isEmpty()) {
            iAppName = QString();
            iCategoryLeftSwipe = NULL;
        } else {
            iAppName = aAppName;
            QString dir(QString(DCONF_HINTS_DIR).arg(aAppName));
            iCategoryLeftSwipe = new MGConfItem(dir + KEY_CATEGORY_LEFT_SWIPE, this);
            connect(iCategoryLeftSwipe, SIGNAL(valueChanged()), SIGNAL(categoryLeftSwipeChanged()));
        }
        Q_EMIT appNameChanged();
        Q_EMIT categoryLeftSwipeChanged();
    }
}

int LoggerHints::categoryLeftSwipe() const
{
    return iCategoryLeftSwipe ?
        iCategoryLeftSwipe->value(DEFAULT_CATEGORY_LEFT_SWIPE).toInt() :
        DEFAULT_CATEGORY_LEFT_SWIPE;
}

void LoggerHints::setCategoryLeftSwipe(int aValue)
{
    if (iCategoryLeftSwipe) {
        HDEBUG(aValue);
        iCategoryLeftSwipe->set(aValue);
    }
}
