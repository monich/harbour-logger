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

int Logger::Main(int aArgc, char* aArgv[], const char* aService,
    QString aPackage, QString aQmlPath)
{
    QGuiApplication* app = SailfishApp::application(aArgc, aArgv);

    QString pluginPrefix(QString("harbour.logger.") + aPackage);
    register_types(qPrintable(pluginPrefix));
    HarbourTransferMethodInfo::registerTypes();

    // Load translations
    QLocale locale;
    QTranslator* translator = new QTranslator(app);
#ifdef OPENREPOS
    QString fullAppName(QString("openrepos-logger-") + aPackage);
    QString transDir("/usr/share/translations");
#else
    QString fullAppName(QString("harbour-logger-") + aPackage);
    QString transDir = SailfishApp::pathTo("translations").toLocalFile();
#endif
    if (translator->load(locale, fullAppName, "-", transDir) ||
        translator->load(fullAppName, transDir)) {
        app->installTranslator(translator);
    } else {
        HWARN("Failed to load" << qPrintable(fullAppName) << "translations for" << locale);
        HDEBUG("Translation directory" << transDir);
        HDEBUG("App name" << fullAppName);
        delete translator;
    }

    translator = new QTranslator(app);
    if (HarbourTransferMethodsModel::loadTranslations(translator, locale) ||
        HarbourTransferMethodsModel::loadTranslations(translator, QLocale("en_GB"))) {
        app->installTranslator(translator);
    } else {
        delete translator;
    }

    // Signal handler
    HarbourSigChildHandler* sigChild = HarbourSigChildHandler::install(app);

    // Log client and models
    DBusLogClient* client = dbus_log_client_new(G_BUS_TYPE_SYSTEM,
        aService, "/", DBUSLOG_CLIENT_FLAG_AUTOSTART);
    LoggerSettings* logSettings = new LoggerSettings(fullAppName, app);
    LoggerLogModel* logModel = new LoggerLogModel(logSettings, client, app);
    LoggerCategoryListModel* categoryModel = new LoggerCategoryListModel(client, app);
    LoggerLogSaver* logSaver = new LoggerLogSaver(aPackage, app);
    logSaver->connect(logModel, SIGNAL(entryAdded(LoggerEntry)), SLOT(addEntry(LoggerEntry)));
    logSaver->connect(sigChild, SIGNAL(processDied(int,int)), SLOT(onProcessDied(int,int)));

    // Copy OS version
    QString dir(logSaver->dirName());
    QFile::copy("/etc/sailfish-release", dir + "/sailfish-release");

    // And the package version
    QString pkgVersionFile(dir + "/" + aPackage + "-version");
    int pkgVersionFd = open(qPrintable(pkgVersionFile), O_WRONLY | O_CREAT, 0644);
    if (pkgVersionFd >= 0) {
        if (fork() == 0) {
            dup2(pkgVersionFd, STDOUT_FILENO);
            dup2(pkgVersionFd, STDERR_FILENO);
            execlp("rpm", "rpm", "-q", qPrintable(aPackage), NULL);
        }
        close(pkgVersionFd);
    }

    // Create and show the view
    QQuickView* view = SailfishApp::createView();
    QQmlContext* context = view->rootContext();
    context->setContextProperty("LogSettings", logSettings);
    context->setContextProperty("LogModel", logModel);
    context->setContextProperty("LogSaver", logSaver);
    context->setContextProperty("CategoryModel", categoryModel);
    context->setContextProperty("AppName", QString(fullAppName));
    view->setSource(SailfishApp::pathTo(aQmlPath));
    view->showFullScreen();

    int ret = app->exec();

    dbus_log_client_unref(client);
    delete view;
    delete app;
    return ret;
}
