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

#include "LoggerLogSaver.h"
#include "LoggerCategory.h"
#include "HarbourDebug.h"

#include <QFileInfo>
#include <QDir>

#include <unistd.h>

// ==========================================================================
// LoggerLogSaver::WriteTask
// ==========================================================================

class LoggerLogSaver::WriteTask: public QRunnable
{
public:
    WriteTask(LoggerLogSaver* aOwner) : iOwner(aOwner)
        { setAutoDelete(true); }

protected:
    virtual void run();

private:
    LoggerLogSaver* iOwner;
};

void LoggerLogSaver::WriteTask::run()
{
    iOwner->doWrite();
}

// ==========================================================================
// LoggerLogSaver::SaveTask
// ==========================================================================

class LoggerLogSaver::SaveTask: public QObject, public QRunnable
{
    Q_OBJECT

public:
    SaveTask(QString aSrc, QString aDest, QObject* aParent = NULL) :
        QObject(aParent), iSrc(aSrc), iDest(aDest) {
        setAutoDelete(false);
    }

protected:
    virtual void run();

Q_SIGNALS:
    void done(bool aSuccess);

private:
    QString iSrc;
    QString iDest;
};

void LoggerLogSaver::SaveTask::run()
{
    QFile::remove(iDest);
    bool ok = QFile::copy(iSrc, iDest);
    if (ok) {
        HDEBUG("Copied" << qPrintable(iSrc) << "->" << qPrintable(iDest));
    } else {
        HDEBUG("Failed to copy" << qPrintable(iSrc) << "->" << qPrintable(iDest));
    }
    Q_EMIT done(ok);
}

// ==========================================================================
// LoggerLogSaver
// ==========================================================================

LoggerLogSaver::LoggerLogSaver(QString aName, QObject* aParent) :
    QObject(aParent),
    iWriteTaskRunning(false),
    iBuffer(-1),
    iThreadPool(new QThreadPool),
    iArchiveType("application/x-gzip"),
    iArchiveName(aName + QDateTime::currentDateTime().toString("_yyyy-MM-dd_hhmmss")),
    iArchiveFile(iArchiveName + ".tar.gz"),
    iTempDir(QString("/tmp/") + aName + "_XXXXXX"),
    iRootDir(iTempDir.path() + "/" + iArchiveName),
    iLogFile(iRootDir + "/" + aName + ".log"),
    iSaveTask(NULL),
    iPid(0)
{
    iThreadPool->setMaxThreadCount(1);
    iTempDir.setAutoRemove(true);
    HDEBUG("Temporary directory" << iTempDir.path());
    if (!QDir(iRootDir).mkpath(iRootDir)) {
        HDEBUG("Failed to create " << qPrintable(iRootDir));
    }
    if (!iLogFile.open(QFile::Text | QFile::ReadWrite)) {
        HDEBUG("Failed to open " << qPrintable(iLogFile.fileName()));
    }
}

LoggerLogSaver::~LoggerLogSaver()
{
    iThreadPool->waitForDone();
    HASSERT(!iWriteTaskRunning);
    if (!iArchivePath.isEmpty()) unlink(qPrintable(iArchivePath));
    delete iThreadPool;
}

void LoggerLogSaver::pack()
{
    iArchivePath = iTempDir.path() + "/" + iArchiveFile;
    HDEBUG("Creating" << iArchivePath);
    if (iPid > 0) kill(iPid, SIGKILL);
    iPid = fork();
    if (iPid > 0) {
        // Parent
        Q_EMIT packingChanged();
    } else if (iPid == 0) {
        // Child
        execlp("tar", "tar", "-czf", qPrintable(iArchivePath), "-C",
            qPrintable(iTempDir.path()), qPrintable(iArchiveName), NULL);
    }
    Q_EMIT archivePathChanged();
}

void LoggerLogSaver::onProcessDied(int aPid, int aStatus)
{
    if (iPid > 0 && iPid == aPid) {
        HDEBUG("Tar done, pid" << aPid << "status" << aStatus);
        iPid = -1;
        Q_EMIT packingChanged();
    }
}

void LoggerLogSaver::save()
{
    HDEBUG(qPrintable(iArchivePath));
    if (!iArchivePath.isEmpty() && !iSaveTask) {
        QString fileName = QFileInfo(iArchivePath).fileName();
        QString destPath = QDir::homePath() + "/Documents/" + fileName;
        iSaveTask = new SaveTask(iArchivePath, destPath, this);
        connect(iSaveTask, SIGNAL(done(bool)), SLOT(onSaveTaskDone(bool)),
            Qt::QueuedConnection);
        HDEBUG(">" << qPrintable(destPath));
        iThreadPool->start(iSaveTask);
        Q_EMIT savingChanged();
    }
}

void LoggerLogSaver::onSaveTaskDone(bool aSuccess)
{
    HDEBUG((aSuccess ? "OK" : "ERROR"));
    if (iSaveTask) {
        delete iSaveTask;
        iSaveTask = NULL;
        Q_EMIT saveFinished(aSuccess);
        Q_EMIT savingChanged();
    }
}

// Invoked on UI thread
void LoggerLogSaver::addEntry(LoggerEntry aEntry)
{
    iMutex.lock();
    iBuffer.put(aEntry);
    if (!iWriteTaskRunning) {
        iWriteTaskRunning = true;
        iThreadPool->start(new WriteTask(this));
    }
    iMutex.unlock();
}

// Invoked on the worker thread
void LoggerLogSaver::doWrite()
{
    iMutex.lock();
    HASSERT(iWriteTaskRunning);
    while (!iBuffer.isEmpty()) {
        LoggerEntry entry(iBuffer.get());
        iMutex.unlock();
        // Actually save it
        LoggerEntry::Type type(entry.type());
        if (type == LoggerEntry::TypeLog) {
            LoggerCategory category(entry.category());
            QString categoryName(category.name());
            QString timestamp(entry.time().toString("yyyy-MM-dd hh:mm:ss.zzz "));
            QString line;
            if (category.isValid() &&
                !category.name().isEmpty() &&
                !category.hidden()) {
                line = timestamp + categoryName + ": " + entry.text();
            } else {
                line = timestamp + entry.text();
            }
            iLogFile.write(line.toUtf8());
            iLogFile.write("\n");
        } else if (type == LoggerEntry::TypeSkip) {
            iLogFile.write(QString("... skipped %1 entries\n").
                arg(entry.skipCount()).toUtf8());
        }
        iMutex.lock();
    }
    iWriteTaskRunning = false;
    iMutex.unlock();
    iLogFile.flush();
}

#include "LoggerLogSaver.moc"
