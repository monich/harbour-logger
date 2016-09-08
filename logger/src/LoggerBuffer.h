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

#ifndef LOGGER_BUFFER_H
#define LOGGER_BUFFER_H

#include "LoggerEntry.h"

#include "gutil_ring.h"

class LoggerBuffer
{
public:
    LoggerBuffer(int aMaxDepth);
    ~LoggerBuffer();

    int size() const;
    int maxSize() const;
    void setMaxSize(int aMaxSize);
    bool isEmpty() const;
    bool put(LoggerEntry aEntry);
    LoggerEntry get();
    LoggerEntry at(int aIndex) const;
    bool canPut(int aCount = 1) const;
    void drop(int aCount);
    void clear();

private:
    static const LoggerEntry gInvalidEntry;
    static void freeEntry(gpointer aData);

private:
    GUtilRing* iRingBuffer;
};

// Inline methods
inline LoggerBuffer::LoggerBuffer(int aMaxDepth) :
    iRingBuffer(gutil_ring_new_full(0, aMaxDepth, freeEntry)) {}
inline LoggerBuffer::~LoggerBuffer()
    { gutil_ring_unref(iRingBuffer); }
inline int LoggerBuffer::size() const
    { return gutil_ring_size(iRingBuffer); }
inline int LoggerBuffer::maxSize() const
    { return gutil_ring_max_size(iRingBuffer); }
inline void LoggerBuffer::setMaxSize(int aMaxSize)
    { gutil_ring_set_max_size(iRingBuffer, aMaxSize); }
inline bool LoggerBuffer::isEmpty() const
    { return !gutil_ring_size(iRingBuffer); }
inline bool LoggerBuffer::canPut(int aCount) const
    { return gutil_ring_can_put(iRingBuffer, aCount) != FALSE; }
inline void LoggerBuffer::drop(int aCount)
    { gutil_ring_drop(iRingBuffer, aCount); }
inline void LoggerBuffer::clear()
    { gutil_ring_clear(iRingBuffer); }

#endif // LOGGER_BUFFER_H
