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

#include "LoggerSettings.h"
#include "HarbourDebug.h"

#include <MGConfItem>

#define DCONF_KEY(app,key)              QString("/apps/%1/%2").arg(app).arg(key)
#define DCONF_LOG_SIZE_LIMIT(app)       DCONF_KEY(app,"logSizeLimit")
#define DCONF_FONT_SIZE_ADJUSTMENT(app) DCONF_KEY(app,"fontSizeAdjustment")

const int LoggerSettings::DEFAULT_LOG_SIZE_LIMIT = 1000;
const int LoggerSettings::DEFAULT_FONT_SIZE_ADJUSTMENT = 0;

LoggerSettings::LoggerSettings(QString aApp, QObject* aParent) :
    QObject(aParent),
    iLogSizeLimit(new MGConfItem(DCONF_LOG_SIZE_LIMIT(aApp), this)),
    iFontSizeAdjustment(new MGConfItem(DCONF_FONT_SIZE_ADJUSTMENT(aApp), this))
{
    connect(iLogSizeLimit, SIGNAL(valueChanged()), SIGNAL(logSizeLimitChanged()));
    connect(iFontSizeAdjustment, SIGNAL(valueChanged()), SIGNAL(fontSizeAdjustmentChanged()));
}

int LoggerSettings::logSizeLimit() const
{
    QVariant value = iLogSizeLimit->value();
    if (value.isValid()) {
        bool ok = false;
        int ival = value.toInt(&ok);
        if (ok) {
            return ival;
        }
    }
    return DEFAULT_LOG_SIZE_LIMIT;
}

int LoggerSettings::fontSizeAdjustment() const
{
    return iFontSizeAdjustment->value(DEFAULT_FONT_SIZE_ADJUSTMENT).toInt();
}
