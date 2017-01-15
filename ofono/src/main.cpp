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

#include "LoggerMain.h"
#include "HarbourDebug.h"

#include "qofonoextmodemmanager.h"

#include <QQuickView>
#include <QQmlContext>

#define OFONO_INTERFACE "org.ofono"
#define OFONOEXT_INTERFACE "org.nemomobile.ofono"
#define MODEM_MANAGER_INTERFACE OFONOEXT_INTERFACE ".ModemManager"

#define SUPER LoggerMain

class OfonoLogger: public SUPER {
    Q_OBJECT
    Q_PROPERTY(bool mobileDataBroken READ mobileDataBroken NOTIFY mobileDataBrokenChanged)
    static const QString AUTO;
public:
    OfonoLogger(int* aArgc, char** aArgv);
    bool mobileDataBroken() const;
    Q_INVOKABLE void fixMobileData();

private:
    void maybeSaveFiles();
    void saveFiles() const;
    void dumpOfonoInfo(QString aPath, QString aService) const;

protected:
    virtual void saveFilesAtStartup(QString aDir);
    virtual void setupView(QQuickView* aView);

private Q_SLOTS:
    void updateState();

Q_SIGNALS:
    void mobileDataBrokenChanged();

private:
    QString iSaveDir;
    QOfonoExtModemManager* iModemManager;
    bool iFilesSaved;
    bool iMobileDataBroken;
};

const QString OfonoLogger::AUTO("auto");

OfonoLogger::OfonoLogger(int* aArgc, char** aArgv) :
    SUPER(aArgc, aArgv, "org.ofono", "ofono", "qml/main.qml"),
    iModemManager(new QOfonoExtModemManager(this)),
    iFilesSaved(false),
    iMobileDataBroken(false)
{
    connect(iModemManager, SIGNAL(validChanged(bool)), SLOT(updateState()));
    connect(iModemManager, SIGNAL(defaultDataSimChanged(QString)), SLOT(updateState()));
    updateState();
}

inline bool OfonoLogger::mobileDataBroken() const
{
    return iMobileDataBroken;
}

void OfonoLogger::fixMobileData()
{
    HDEBUG("ok");
    iModemManager->setDefaultDataSim(AUTO);
}

void OfonoLogger::updateState()
{
    maybeSaveFiles();
    // On a single-SIM phone default data sim should be always set to "auto"
    // because UI doesn't provide any way to change it.
    bool broken = iModemManager->valid() &&
        iModemManager->availableModems().count() == 1 &&
        iModemManager->defaultDataSim() != AUTO;
    if (iMobileDataBroken != broken) {
        iMobileDataBroken = broken;
        HDEBUG((broken ? "broken" : "ok"));
        Q_EMIT mobileDataBrokenChanged();
    }
}

void OfonoLogger::maybeSaveFiles()
{
    if (!iFilesSaved && !iSaveDir.isEmpty() && iModemManager->valid()) {
        iFilesSaved = true;
        saveFiles();
    }
}

void OfonoLogger::saveFiles() const
{
    QString rilerror("/var/lib/ofono/rilerror");
    if (QFile::exists(rilerror)) {
        QFile::copy(rilerror, iSaveDir + "/rilerror");
    }

    // ModemManager.GetAll
    const int ver = iModemManager->interfaceVersion();
    QString dest = QString("--dest=%1").arg(iService);
    QString call = QString(MODEM_MANAGER_INTERFACE ".GetAll%1").arg(ver);
    QString fname = call.right(call.length() - sizeof(OFONOEXT_INTERFACE));
    saveOutput("dbus-send", "--system", "--print-reply", "--type=method_call",
        qPrintable(dest), "/", qPrintable(call),
        iSaveDir + "/" + fname +".txt");

    // Ofono information for each modem
    QStringList modems = iModemManager->availableModems();
    for (int i=0; i<modems.count(); i++) {
        QString path(modems.at(i));
        HDEBUG(path);
        dumpOfonoInfo(path, "SimManager.GetProperties");
        dumpOfonoInfo(path, "NetworkRegistration.GetProperties");
        dumpOfonoInfo(path, "ConnectionManager.GetProperties");
        dumpOfonoInfo(path, "ConnectionManager.GetContexts");
    }
}

void OfonoLogger::dumpOfonoInfo(QString aPath, QString aCall) const
{
    QString suffix = QString(aPath).replace("/", "");
    QString dest = QString("--dest=%1").arg(iService);
    QString call = QString(OFONO_INTERFACE ".%1").arg(aCall);
    saveOutput("dbus-send", "--system", "--print-reply", "--type=method_call",
        qPrintable(dest),  qPrintable(aPath), qPrintable(call),
        iSaveDir + "/" + aCall + "." + suffix + ".txt");
}

void OfonoLogger::saveFilesAtStartup(QString aDir)
{
    iSaveDir = aDir;
    maybeSaveFiles();
    SUPER::saveFilesAtStartup(aDir);
}

void OfonoLogger::setupView(QQuickView* aView)
{
    QQmlContext* context = aView->rootContext();
    context->setContextProperty("OfonoLogger", this);
    SUPER::setupView(aView);
}

Q_DECL_EXPORT int main(int argc, char* argv[])
{
    OfonoLogger* logger = new OfonoLogger(&argc, argv);
    int ret = logger->run();
    delete logger;
    return ret;
}

#include "main.moc"
