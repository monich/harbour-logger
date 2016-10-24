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

#include "LoggerMain.h"
#include "LoggerHints.h"
#include "LoggerLogModel.h"
#include "LoggerLogSaver.h"
#include "LoggerSettings.h"
#include "LoggerCategoryListModel.h"

#include "HarbourDebug.h"
#include "HarbourSigChildHandler.h"
#include "HarbourTransferMethodsModel.h"

#include <sailfishapp.h>
#include <QFile>
#include <QGuiApplication>
#include <QtQuick>
#include <QtQml>

#include <fcntl.h>
#include <unistd.h>

#undef signals
#include "dbuslog_client.h"

static void register_types(const char* uri, int v1 = 1, int v2 = 0)
{
    qmlRegisterType<LoggerHints>(uri, v1, v2, "LoggerHints");
    qmlRegisterType<HarbourTransferMethodsModel>(uri, v1, v2, "TransferMethodsModel");
}

LoggerMain::LoggerMain(int* aArgc, char** aArgv, const char* aService,
    QString aPackage, QString aQmlPath) :
    iApp(SailfishApp::application(*aArgc, aArgv)),
    iService(aService),
    iPackage(aPackage),
    iQmlPath(aQmlPath),
    iClient(dbus_log_client_new(G_BUS_TYPE_SYSTEM, aService, "/",
        DBUSLOG_CLIENT_FLAG_AUTOSTART)),
#ifdef OPENREPOS
    iFullAppName(QString("openrepos-logger-") + aPackage),
    iTransDir("/usr/share/translations")
#else
    iFullAppName(QString("harbour-logger-") + aPackage),
    iTransDir(SailfishApp::pathTo("translations").toLocalFile())
#endif
{
    QString pluginPrefix(QString("harbour.logger.") + aPackage);
    register_types(qPrintable(pluginPrefix));
    HarbourTransferMethodInfo::registerTypes();
}

LoggerMain::~LoggerMain()
{
    dbus_log_client_unref(iClient);
    delete iApp;
}

void LoggerMain::loadTranslations()
{
    QLocale locale;
    QTranslator* translator = new QTranslator(iApp);
    if (translator->load(locale, iFullAppName, "-", iTransDir) ||
        translator->load(iFullAppName, iTransDir)) {
        iApp->installTranslator(translator);
    } else {
        HWARN("Failed to load" << qPrintable(iFullAppName) << "translations for" << locale);
        HDEBUG("Translation directory" << iTransDir);
        HDEBUG("App name" << iFullAppName);
        delete translator;
    }

    translator = new QTranslator(iApp);
    if (HarbourTransferMethodsModel::loadTranslations(translator, locale) ||
        HarbourTransferMethodsModel::loadTranslations(translator, QLocale("en_GB"))) {
        iApp->installTranslator(translator);
    } else {
        delete translator;
    }
}

bool LoggerMain::saveOutput(const char* aExe, const char* const aArgv[],
    QString aOut)
{
    int fd = open(qPrintable(aOut), O_WRONLY | O_CREAT, 0644);
    if (fd >= 0) {
        if (fork() == 0) {
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            execvp(aExe, (char**)aArgv);
        }
        close(fd);
        return true;
    }
    return false;
}

bool LoggerMain::saveOutput(const char* aExe, const char* aArg1,
    const char* aArg2, QString aOut)
{
    const char* argv[4];
    argv[0] = aExe;
    argv[1] = aArg1;
    argv[2] = aArg2;
    argv[3] = NULL;
    return saveOutput(aExe, argv, aOut);
}

bool LoggerMain::saveOutput(const char* aExe, const char* aArg1,
    const char* aArg2, const char* aArg3, const char* aArg4,
    const char* aArg5, const char* aArg6, QString aOut)
{
    const char* argv[8];
    argv[0] = aExe;
    argv[1] = aArg1;
    argv[2] = aArg2;
    argv[3] = aArg3;
    argv[4] = aArg4;
    argv[5] = aArg5;
    argv[6] = aArg6;
    argv[7] = NULL;
    return saveOutput(aExe, argv, aOut);
}

void LoggerMain::saveFilesAtStartup(QString aDir)
{
    // Copy OS version
    QFile::copy("/etc/sailfish-release", aDir + "/sailfish-release");

    // And the package version
    saveOutput("rpm", "-q", qPrintable(iPackage),
        aDir + "/" + iPackage + "-version");
}

int LoggerMain::run()
{
    loadTranslations();

    // Signal handler
    HarbourSigChildHandler* sigChild = HarbourSigChildHandler::install(iApp);

    // Models and stuff
    LoggerSettings* logSettings = new LoggerSettings(iFullAppName, iApp);
    LoggerLogModel* logModel = new LoggerLogModel(logSettings, iClient, iApp);
    LoggerCategoryListModel* categoryModel = new LoggerCategoryListModel(iClient, iApp);
    LoggerLogSaver* logSaver = new LoggerLogSaver(iPackage, iApp);
    logSaver->connect(logModel, SIGNAL(entryAdded(LoggerEntry)), SLOT(addEntry(LoggerEntry)));
    logSaver->connect(sigChild, SIGNAL(processDied(int,int)), SLOT(onProcessDied(int,int)));

    // Save some files
    saveFilesAtStartup(logSaver->dirName());

    // Create and show the view
    QQuickView* view = SailfishApp::createView();
    QQmlContext* context = view->rootContext();
    context->setContextProperty("LogSettings", logSettings);
    context->setContextProperty("LogModel", logModel);
    context->setContextProperty("LogSaver", logSaver);
    context->setContextProperty("CategoryModel", categoryModel);
    context->setContextProperty("AppName", iFullAppName);
    view->setSource(SailfishApp::pathTo(iQmlPath));
    view->showFullScreen();

    int ret = iApp->exec();

    delete view;
    return ret;
}

int LoggerMain::Run(int aArgc, char* aArgv[], const char* aService,
    QString aPackage, QString aQmlPath)
{
    LoggerMain* main = new LoggerMain(&aArgc, aArgv, aService, aPackage, aQmlPath);
    int ret = main->run();
    delete main;
    return ret;
}
