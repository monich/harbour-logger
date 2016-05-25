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

#ifndef LOGGER_CATEGORY_H
#define LOGGER_CATEGORY_H

#include <QString>

#include "dbuslog_category.h"

class LoggerCategory
{
public:
    enum Flags {
        FLAG_ENABLED = 0x01,
        FLAG_ENABLED_BY_DEFAULT = 0x02,
        FLAG_HIDE_NAME = 0x04
    };

    LoggerCategory();
    LoggerCategory(DBusLogCategory* aCategory);
    LoggerCategory(const LoggerCategory& aCategory);
    LoggerCategory& operator=(const LoggerCategory& aCategory);
    ~LoggerCategory();

    bool isValid() const;
    const QString& name() const;
    const char* cname() const;
    guint32 id() const;
    Flags flags() const;
    bool hidden() const;
    bool enabled() const;
    bool enabledByDefault() const;

private:
    class Private;
    Private* iPrivate;
};

// Inline methods
inline bool LoggerCategory::hidden() const
    { return (flags() & FLAG_HIDE_NAME) != 0; }
inline bool LoggerCategory::enabled() const
    { return (flags() & FLAG_ENABLED) != 0; }
inline bool LoggerCategory::enabledByDefault() const
    { return (flags() & FLAG_ENABLED_BY_DEFAULT) != 0; }

#endif // LOGGER_CATEGORY_H
