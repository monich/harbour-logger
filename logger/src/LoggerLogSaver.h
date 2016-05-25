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

#ifndef LOGGER_LOG_SAVER_H
#define LOGGER_LOG_SAVER_H

#include "LoggerBuffer.h"

#include <QObject>
#include <QMutex>
#include <QFile>
#include <QTemporaryDir>
#include <QThreadPool>

class LoggerLogSaver : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool packing READ isPacking NOTIFY packingChanged)
    Q_PROPERTY(bool saving READ isSaving NOTIFY savingChanged)
    Q_PROPERTY(QString archivePath READ archivePath NOTIFY archivePathChanged)
    Q_PROPERTY(QString archiveFile READ archiveFile CONSTANT)
    Q_PROPERTY(QString archiveType READ archiveType CONSTANT)

public:
    LoggerLogSaver(QString aName, QObject* aParent = NULL);
    ~LoggerLogSaver();

    bool isPacking() const;
    bool isSaving() const;
    QString archivePath() const;
    QString archiveFile() const;
    QString archiveType() const;

    QString dirName() const;

public Q_SLOTS:
    void pack();
    void save();
    void addEntry(LoggerEntry entry);
    void onProcessDied(int aPid, int aStatus);
    void onSaveTaskDone(bool aSuccess);

Q_SIGNALS:
    void saveFinished(bool success);
    void packingChanged();
    void savingChanged();
    void archivePathChanged();

private:
    void doWrite();

private:
    class WriteTask;
    class SaveTask;
    friend class WriteTask;
    bool iWriteTaskRunning;
    QMutex iMutex;
    LoggerBuffer iBuffer;
    QThreadPool* iThreadPool;
    QString iArchivePath;
    QString iArchiveType;
    QString iArchiveName;
    QString iArchiveFile;
    QTemporaryDir iTempDir;
    QString iRootDir;
    QFile iLogFile;
    SaveTask* iSaveTask;
    int iPid;
};

inline QString LoggerLogSaver::dirName() const
    { return iRootDir; }
inline bool LoggerLogSaver::isPacking() const
    { return iPid > 0; }
inline bool LoggerLogSaver::isSaving() const
    { return iSaveTask != NULL; }
inline QString LoggerLogSaver::archivePath() const
    { return iArchivePath; }
inline QString LoggerLogSaver::archiveFile() const
    { return iArchiveFile; }
inline QString LoggerLogSaver::archiveType() const
    { return iArchiveType; }

#endif // LOGGER_LOG_SAVER_H
