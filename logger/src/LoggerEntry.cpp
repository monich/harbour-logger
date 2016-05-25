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

#include "LoggerEntry.h"
#include "LoggerCategory.h"

#include "dbuslog_category.h"

// Public enums must match the protocol values
G_STATIC_ASSERT((int)LoggerEntry::LevelUndefined == (int)DBUSLOG_LEVEL_UNDEFINED);
G_STATIC_ASSERT((int)LoggerEntry::LevelAlways == (int)DBUSLOG_LEVEL_ALWAYS);
G_STATIC_ASSERT((int)LoggerEntry::LevelCritical == (int)DBUSLOG_LEVEL_CRITICAL);
G_STATIC_ASSERT((int)LoggerEntry::LevelError == (int)DBUSLOG_LEVEL_ERROR);
G_STATIC_ASSERT((int)LoggerEntry::LevelWarning == (int)DBUSLOG_LEVEL_WARNING);
G_STATIC_ASSERT((int)LoggerEntry::LevelNotice == (int)DBUSLOG_LEVEL_NOTICE);
G_STATIC_ASSERT((int)LoggerEntry::LevelInfo == (int)DBUSLOG_LEVEL_INFO);
G_STATIC_ASSERT((int)LoggerEntry::LevelDebug == (int)DBUSLOG_LEVEL_DEBUG);
G_STATIC_ASSERT((int)LoggerEntry::LevelVerbose == (int)DBUSLOG_LEVEL_VERBOSE);

// ==========================================================================
// LoggerEntry::Private
// ==========================================================================

class LoggerEntry::Private {
public:
    Private(int aSkipCount);
    Private(QString aText);
    Private(DBusLogMessage* aMessage, LoggerCategory aCategory);
    ~Private();

public:
    static const LoggerCategory gDummyCategory;
    static const QDateTime gDummyTime;
    static const QString gDummyString;

    QAtomicInt ref;
    Type iType;
    int iSkipCount;
    DBusLogMessage* iMessage;
    LoggerCategory iCategory;
    QString iText;
    QDateTime iTime;
};

const LoggerCategory LoggerEntry::Private::gDummyCategory;
const QDateTime LoggerEntry::Private::gDummyTime;
const QString LoggerEntry::Private::gDummyString;

LoggerEntry::Private::Private(int aSkipCount) :
    ref(1),
    iType(TypeSkip),
    iSkipCount(aSkipCount),
    iMessage(NULL)
{
}

LoggerEntry::Private::Private(QString aText) :
    ref(1),
    iType(TypeText),
    iSkipCount(0),
    iMessage(NULL),
    iText(aText),
    iTime(QDateTime::currentDateTime())
{
}

LoggerEntry::Private::Private(DBusLogMessage* aMessage,
    LoggerCategory aCategory) :
    ref(1),
    iType(TypeLog),
    iSkipCount(0),
    iMessage(dbus_log_message_ref(aMessage)),
    iCategory(aCategory),
    iText(QString::fromUtf8(aMessage->string, aMessage->length)),
    iTime(QDateTime::fromMSecsSinceEpoch(aMessage->timestamp/1000))
{
}

LoggerEntry::Private::~Private()
{
    dbus_log_message_unref(iMessage);
}

// ==========================================================================
// LoggerEntry
// ==========================================================================

LoggerEntry::LoggerEntry() :
    iPrivate(NULL)
{
}

LoggerEntry::LoggerEntry(int aSkipCount) :
    iPrivate(new Private(aSkipCount))
{
}

LoggerEntry::LoggerEntry(QString aText) :
    iPrivate(new Private(aText))
{
}

LoggerEntry::LoggerEntry(LoggerCategory aCategory, DBusLogMessage* aMessage) :
    iPrivate(new Private(aMessage, aCategory))
{
}

LoggerEntry::LoggerEntry(const LoggerEntry& aMessage) :
    iPrivate(aMessage.iPrivate)
{
    ref();
}

LoggerEntry::~LoggerEntry()
{
    deref();
}

LoggerEntry& LoggerEntry::operator=(const LoggerEntry& aMessage)
{
    if (iPrivate != aMessage.iPrivate) {
        if (aMessage.iPrivate) {
            aMessage.iPrivate->ref.ref();
        }
        deref();
        iPrivate = aMessage.iPrivate;
    }
    return *this;
}

const QString& LoggerEntry::text() const
{
    return iPrivate ? iPrivate->iText : Private::gDummyString;
}

const QDateTime& LoggerEntry::time() const
{
    return iPrivate ? iPrivate->iTime : Private::gDummyTime;
}

const LoggerCategory& LoggerEntry::category() const
{
    return iPrivate ? iPrivate->iCategory : Private::gDummyCategory;
}

LoggerEntry::Level LoggerEntry::level() const
{
    return (iPrivate && iPrivate->iMessage) ?
        (Level)iPrivate->iMessage->level :
        LevelUndefined;
}

LoggerEntry::Type LoggerEntry::type() const
{
    return iPrivate ? iPrivate->iType : TypeInvalid;
}

int LoggerEntry::skipCount() const
{
    return iPrivate ? iPrivate->iSkipCount : 0;
}

void LoggerEntry::ref()
{
    if (iPrivate) {
        iPrivate->ref.ref();
    }
}

void LoggerEntry::deref()
{
    if (iPrivate && !iPrivate->ref.deref()) {
        delete iPrivate;
    }
}

bool LoggerEntry::isValid() const
{
    return iPrivate != NULL;
}
