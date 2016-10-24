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

#ifndef LOGGER_ENTRY_H
#define LOGGER_ENTRY_H

#include <QString>
#include <QDateTime>
#include <QMetaType>

#include "dbuslog_message.h"

class LoggerCategory;
class LoggerEntry
{
public:
    // These must match the protocol values
    enum Level {
        LevelUndefined,
        LevelAlways,
        LevelCritical,
        LevelError,
        LevelWarning,
        LevelNotice,
        LevelInfo,
        LevelDebug,
        LevelVerbose
    };

    // These must match constants in logger.js
    enum Type {
        TypeInvalid,
        TypeLog,
        TypeSkip,
        TypeText,
        TypeEmpty
    };

    ~LoggerEntry();

    LoggerEntry& operator=(const LoggerEntry& aMessage);
    LoggerEntry();
    LoggerEntry(const LoggerEntry& aMessage);
    LoggerEntry(int aSkipCount);
    LoggerEntry(QString aText);
    LoggerEntry(LoggerCategory aCategory, DBusLogMessage* aMessage);

    const QString& text() const;
    const QDateTime& time() const;
    const LoggerCategory& category() const;
    Level level() const;
    Type type() const;
    int skipCount() const;

    void ref();
    void deref();
    bool isValid() const;

private:
    class Private;
    Private* iPrivate;
};

Q_DECLARE_METATYPE(LoggerEntry)

#endif // LOGGER_ENTRY_H
