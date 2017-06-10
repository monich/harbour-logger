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

#include "LoggerSettings.h"
#include "HarbourDebug.h"

#include <MGConfItem>

#define DCONF_KEY(app,key)              QString("/apps/%1/%2").arg(app).arg(key)
#define DCONF_LOG_SIZE_LIMIT(app)       DCONF_KEY(app,"logSizeLimit")
#define DCONF_FONT_SIZE_ADJUSTMENT(app) DCONF_KEY(app,"fontSizeAdjustment")
#define DCONF_AUTO_ENABLE_LOGGING(app)  DCONF_KEY(app,"autoEnableLogging")
#define DCONF_AUTO_RESET_LOGGING(app)   DCONF_KEY(app,"autoResetLogging")

//===========================================================================
// LoggerSettings::Private
//===========================================================================

class LoggerSettings::Private : public QObject
{
    Q_OBJECT
public:
    Private(QString aApp, QObject* aParent);

    int logSizeLimit() const;
    int fontSizeAdjustment() const;
    AutoEnable autoEnableLogging() const;
    bool autoResetLogging() const;

public:
    static const int DEFAULT_LOG_SIZE_LIMIT = 1000;
    static const int DEFAULT_FONT_SIZE_ADJUSTMENT = 0;
    static const int DEFAULT_AUTO_ENABLE_LOGGING = AutoEnableAll;
    static const bool DEFAULT_AUTO_RESET_LOGGING = true;

    MGConfItem* iLogSizeLimit;
    MGConfItem* iFontSizeAdjustment;
    MGConfItem* iAutoEnableLogging;
    MGConfItem* iAutoResetLogging;
};

LoggerSettings::Private::Private(QString aApp, QObject* aParent) :
    QObject(aParent),
    iLogSizeLimit(new MGConfItem(DCONF_LOG_SIZE_LIMIT(aApp), this)),
    iFontSizeAdjustment(new MGConfItem(DCONF_FONT_SIZE_ADJUSTMENT(aApp), this)),
    iAutoEnableLogging(new MGConfItem(DCONF_AUTO_ENABLE_LOGGING(aApp), this)),
    iAutoResetLogging(new MGConfItem(DCONF_AUTO_RESET_LOGGING(aApp), this))
{
}

inline int LoggerSettings::Private::logSizeLimit() const
{
    return iLogSizeLimit->value(DEFAULT_LOG_SIZE_LIMIT).toInt();
}

inline int LoggerSettings::Private::fontSizeAdjustment() const
{
    return iFontSizeAdjustment->value(DEFAULT_FONT_SIZE_ADJUSTMENT).toInt();
}

inline LoggerSettings::AutoEnable LoggerSettings::Private::autoEnableLogging() const
{
    int value = iAutoEnableLogging->value(DEFAULT_AUTO_ENABLE_LOGGING).toInt();
    switch(value) {
    case AutoEnableNone:
    case AutoEnableAll:
        return (AutoEnable)value;
    }
    return (AutoEnable)DEFAULT_AUTO_ENABLE_LOGGING;
}

inline bool LoggerSettings::Private::autoResetLogging() const
{
    return iAutoResetLogging->value(DEFAULT_AUTO_RESET_LOGGING).toBool();
}

//===========================================================================
// LoggerSettings::Private
//===========================================================================

LoggerSettings::LoggerSettings(QString aApp, QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(aApp, this))
{
    connect(iPrivate->iLogSizeLimit, SIGNAL(valueChanged()),
        this, SIGNAL(logSizeLimitChanged()));
    connect(iPrivate->iFontSizeAdjustment, SIGNAL(valueChanged()),
        this, SIGNAL(fontSizeAdjustmentChanged()));
    connect(iPrivate->iAutoEnableLogging, SIGNAL(valueChanged()),
        this, SIGNAL(autoEnableLoggingChanged()));
    connect(iPrivate->iAutoResetLogging, SIGNAL(valueChanged()),
        this, SIGNAL(autoResetLoggingChanged()));
}

int LoggerSettings::defaultLogSizeLimit()
{
    return Private::DEFAULT_LOG_SIZE_LIMIT;
}

int LoggerSettings::logSizeLimit() const
{
    return iPrivate->logSizeLimit();
}

int LoggerSettings::fontSizeAdjustment() const
{
    return iPrivate->fontSizeAdjustment();
}

LoggerSettings::AutoEnable LoggerSettings::autoEnableLogging() const
{
    return iPrivate->autoEnableLogging();
}

bool LoggerSettings::autoResetLogging() const
{
    return iPrivate->autoResetLogging();
}

#include "LoggerSettings.moc"
