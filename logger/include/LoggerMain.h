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

#ifndef LOGGER_MAIN_H
#define LOGGER_MAIN_H

#include <QString>
#include <QObject>

class QQuickView;
class QGuiApplication;

class LoggerMain : public QObject {
    Q_OBJECT

public:
    LoggerMain(int* aArgc, char** aArgv, const char* aService,
        QString aPackage, QString aQmlPath);
    virtual ~LoggerMain();

    int run();

    // Quick way to run the app
    static int Run(int aArgc, char** aArgv, const char* aService,
        QString aPackage, QString aQmlPath);

protected:
    bool saveOutput(const char* aExe, const char* const aArgv[],
        QString aOut) const;
    bool saveOutput(const char* aExe, const char* aArg1, const char* aArg2,
        QString aOut) const;
    bool saveOutput(const char* aExe, const char* aArg1, const char* aArg2,
        const char* aArg3, const char* aArg4, const char* aArg5,
        const char* aArg6, QString aOut) const;

    // These are invoked by run()
    virtual void loadTranslations();
    virtual void saveFilesAtStartup(QString aDir);
    virtual void setupView(QQuickView* aView);

protected:
    QGuiApplication* iApp;
    QString iService;
    QString iPackage;
    QString iQmlPath;
    struct dbus_log_client* iClient;
    QString iFullAppName;
    QString iTransDir;
};

#endif // LOGGER_MAIN_H
