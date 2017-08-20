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

#include "LoggerCategory.h"

#include "dbuslog_protocol.h"

// Public enums must match the protocol values
G_STATIC_ASSERT(LoggerCategory::FLAG_ENABLED == DBUSLOG_CATEGORY_FLAG_ENABLED);
G_STATIC_ASSERT(LoggerCategory::FLAG_ENABLED_BY_DEFAULT == DBUSLOG_CATEGORY_FLAG_ENABLED_BY_DEFAULT);
G_STATIC_ASSERT(LoggerCategory::FLAG_HIDE_NAME == DBUSLOG_CATEGORY_FLAG_HIDE_NAME);

// ==========================================================================
// LoggerCategory::Private
// ==========================================================================

class LoggerCategory::Private {
public:
    Private(DBusLogCategory* aCategory);
    ~Private();

public:
    static const QString gEmptyString;
    QAtomicInt iRef;
    DBusLogCategory* iCategory;
    QString iName;
};

const QString LoggerCategory::Private::gEmptyString;

LoggerCategory::Private::Private(DBusLogCategory* aCategory) :
    iRef(1),
    iCategory(dbus_log_category_ref(aCategory)),
    iName(QString::fromUtf8(aCategory->name))
{
}

LoggerCategory::Private::~Private()
{
    dbus_log_category_unref(iCategory);
}

// ==========================================================================
// LoggerCategory
// ==========================================================================

LoggerCategory::LoggerCategory() :
    iPrivate(NULL)
{
}

LoggerCategory::LoggerCategory(DBusLogCategory* aCategory) :
    iPrivate(aCategory ? new Private(aCategory) : NULL)
{
}

LoggerCategory::LoggerCategory(const LoggerCategory& aCategory)
{
    iPrivate = aCategory.iPrivate;
    if (iPrivate) {
        iPrivate->iRef.ref();
    }
}

LoggerCategory::~LoggerCategory()
{
    if (iPrivate && !iPrivate->iRef.deref()) {
        delete iPrivate;
    }
}

LoggerCategory& LoggerCategory::operator=(const LoggerCategory& aCategory)
{
    if (iPrivate != aCategory.iPrivate) {
        if (aCategory.iPrivate) {
            aCategory.iPrivate->iRef.ref();
        }
        if (iPrivate && !iPrivate->iRef.deref()) {
            delete iPrivate;
        }
        iPrivate = aCategory.iPrivate;
    }
    return *this;
}

bool LoggerCategory::isValid() const
{
    return iPrivate != NULL;
}

const QString& LoggerCategory::name() const
{
    return iPrivate ? iPrivate->iName : Private::gEmptyString;
}

const char* LoggerCategory::cname() const
{
    return iPrivate && iPrivate->iCategory ? iPrivate->iCategory->name : NULL;
}

guint32 LoggerCategory::id() const
{
    return iPrivate ? iPrivate->iCategory->id : 0;
}

LoggerCategory::Flags LoggerCategory::flags() const
{
    return (Flags)(iPrivate ? iPrivate->iCategory->flags : 0);
}
