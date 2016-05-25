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

#include "LoggerBuffer.h"

G_STATIC_ASSERT(sizeof(LoggerEntry) == sizeof(gpointer));

const LoggerEntry LoggerBuffer::gInvalidEntry;

bool LoggerBuffer::put(LoggerEntry aEntry)
{
    // This is a bit tricky. LoggerEntry has the size of the pointer
    // so we store it as a pointer. While we are storing it, we need
    // to ref the shared private data. When we are dropping it (see
    // freeEntry below) we need to do the opposite.
    gpointer data;
    memcpy(&data, &aEntry, sizeof(data));
    if (gutil_ring_put(iRingBuffer, data)) {
        aEntry.ref();
        return true;
    }
    return false;
}

void LoggerBuffer::freeEntry(gpointer aData)
{
    // This is the opposite to what put() does
    ((LoggerEntry*)&aData)->deref();
}

LoggerEntry LoggerBuffer::get()
{
    gpointer data = gutil_ring_get(iRingBuffer);
    if (data) {
        LoggerEntry entry(*((LoggerEntry*)&data));
        entry.deref();
        return entry;
    } else {
        return gInvalidEntry;
    }
}

LoggerEntry LoggerBuffer::at(int aIndex) const
{
    if (aIndex >= 0 && aIndex < gutil_ring_size(iRingBuffer)) {
        gpointer data = gutil_ring_data_at(iRingBuffer, aIndex);
        return LoggerEntry(*((LoggerEntry*)&data));
    } else {
        return gInvalidEntry;
    }
}
