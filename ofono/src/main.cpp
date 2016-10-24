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
#include "HarbourDebug.h"

#include <QDBusConnection>
#include <QDBusPendingReply>
#include <QDBusPendingCallWatcher>

#define OFONO_INTERFACE "org.ofono"
#define OFONOEXT_INTERFACE "org.nemomobile.ofono"
#define MODEM_MANAGER_INTERFACE OFONOEXT_INTERFACE ".ModemManager"

#define SUPER LoggerMain

class OfonoLogger: public SUPER {
    Q_OBJECT

public:
    OfonoLogger(int* aArgc, char** aArgv) :
        SUPER(aArgc, aArgv, "org.ofono", "ofono", "qml/main.qml") {}

private:
    void modemManagerCall(QString aMethod, const char* aSlot);
    void dumpOfonoInfo(QString aPath, QString aService);

protected:
    virtual void saveFilesAtStartup(QString aDir);

private Q_SLOTS:
    void onGetInterfaceVersionFinished(QDBusPendingCallWatcher* aWatcher);
    void onGetAvailableModemsFinished(QDBusPendingCallWatcher* aWatcher);

private:
    QString iSaveDir;
};

void OfonoLogger::dumpOfonoInfo(QString aPath, QString aCall)
{
    QString suffix = QString(aPath).replace("/", "");
    QString dest = QString("--dest=%1").arg(iService);
    QString call = QString(OFONO_INTERFACE ".%1").arg(aCall);
    saveOutput("dbus-send", "--system", "--print-reply", "--type=method_call",
        qPrintable(dest),  qPrintable(aPath), qPrintable(call),
        iSaveDir + "/" + aCall + "." + suffix + ".txt");
}

void OfonoLogger::onGetAvailableModemsFinished(QDBusPendingCallWatcher* aWatcher)
{
    QDBusPendingReply<QList<QDBusObjectPath> > reply(*aWatcher);
    QList<QDBusObjectPath> modems = reply.value();
    for (int i=0; i<modems.count(); i++) {
        QString path(modems.at(i).path());
        HDEBUG(path);
        dumpOfonoInfo(path, "SimManager.GetProperties");
        dumpOfonoInfo(path, "NetworkRegistration.GetProperties");
        dumpOfonoInfo(path, "ConnectionManager.GetProperties");
    }
    aWatcher->deleteLater();
}

void OfonoLogger::onGetInterfaceVersionFinished(QDBusPendingCallWatcher* aWatcher)
{
    QDBusPendingReply<int> reply(*aWatcher);
    QString dest = QString("--dest=%1").arg(iService);
    QString call = QString(MODEM_MANAGER_INTERFACE ".GetAll%1").arg(reply.value());
    QString fname = call.right(call.length() - sizeof(OFONOEXT_INTERFACE));
    HDEBUG(reply.value());
    saveOutput("dbus-send", "--system", "--print-reply", "--type=method_call",
        qPrintable(dest), "/", qPrintable(call),
        iSaveDir + "/" + fname +".txt");
    aWatcher->deleteLater();
}

void OfonoLogger::modemManagerCall(QString aMethod, const char* aSlot)
{
    connect(new QDBusPendingCallWatcher(QDBusConnection::systemBus().
        asyncCall(QDBusMessage::createMethodCall(iService, "/",
        MODEM_MANAGER_INTERFACE, aMethod)), this),
        SIGNAL(finished(QDBusPendingCallWatcher*)), aSlot);
}

void OfonoLogger::saveFilesAtStartup(QString aDir)
{
    iSaveDir = aDir;
    modemManagerCall("GetInterfaceVersion",
        SLOT(onGetInterfaceVersionFinished(QDBusPendingCallWatcher*)));
    modemManagerCall("GetAvailableModems",
        SLOT(onGetAvailableModemsFinished(QDBusPendingCallWatcher*)));
    SUPER::saveFilesAtStartup(aDir);
}

Q_DECL_EXPORT int main(int argc, char* argv[])
{
    OfonoLogger* logger = new OfonoLogger(&argc, argv);
    int ret = logger->run();
    delete logger;
    return ret;
}

#include "main.moc"
