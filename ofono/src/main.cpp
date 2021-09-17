/*
 * Copyright (C) 2016-2021 Jolla Ltd.
 * Copyright (C) 2016-2021 Slava Monich <slava.monich@jolla.com>
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
 *   3. Neither the names of the copyright holders nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
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
#include "networktechnology.h"

#include <QQuickView>
#include <QQmlContext>

#define OFONO_INTERFACE "org.ofono"
#define OFONOEXT_INTERFACE "org.nemomobile.ofono"
#define MODEM_MANAGER_INTERFACE OFONOEXT_INTERFACE ".ModemManager"
#define CELLULAR_TECHNOLOGY_PATH "/net/connman/technology/cellular"

#define SUPER LoggerMain

class OfonoLogger: public SUPER
{
    Q_OBJECT
    Q_PROPERTY(bool mobileDataBroken READ mobileDataBroken NOTIFY mobileDataBrokenChanged)
    Q_PROPERTY(bool mobileDataDisabled READ mobileDataDisabled NOTIFY mobileDataDisabledChanged)

    static const QString AUTO;

public:
    OfonoLogger(int* aArgc, char** aArgv, QStringList aPackages);
    bool mobileDataBroken() const;
    bool mobileDataDisabled() const;
    Q_INVOKABLE void fixMobileData();
    Q_INVOKABLE void enableMobileData();

private:
    void maybeSaveFiles();
    void saveFiles() const;
    void dumpOfonoInfo(QString aPath, QString aService) const;
    void dumpConnmanInfo(QString aPath, QString aService) const;

protected:
    void saveFilesAtStartup(QString aDir) Q_DECL_OVERRIDE;
    void setupView(QQuickView* aView) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void updateModemManagerState();
    void updateNetworkTechnologyState();
    void onNetworkTechnologyReady();

Q_SIGNALS:
    void mobileDataBrokenChanged();
    void mobileDataEnabledChanged();
    void mobileDataDisabledChanged();

private:
    QString iSaveDir;
    QSharedPointer<QOfonoExtModemManager> iModemManager;
    NetworkTechnology* iNetworkTechnology;
    bool iNetworkTechnologyReady;
    bool iFilesSaved;
    bool iMobileDataBroken;
    bool iMobileDataDisabled;
};

const QString OfonoLogger::AUTO("auto");

OfonoLogger::OfonoLogger(int* aArgc, char** aArgv, QStringList aPackages) :
    SUPER(aArgc, aArgv, "org.ofono", aPackages, "ofono", "qml/main.qml"),
    iModemManager(QOfonoExtModemManager::instance()),
    iNetworkTechnology(new NetworkTechnology(this)),
    iFilesSaved(false),
    iMobileDataBroken(false),
    iMobileDataDisabled(false)
{
    iNetworkTechnology->setPath(CELLULAR_TECHNOLOGY_PATH);
    connect(iNetworkTechnology,
        SIGNAL(poweredChanged(bool)),
        SLOT(updateNetworkTechnologyState()));
    connect(iNetworkTechnology,
        SIGNAL(propertiesReady()),
        SLOT(onNetworkTechnologyReady()));
    connect(iModemManager.data(),
        SIGNAL(validChanged(bool)),
        SLOT(updateModemManagerState()));
    connect(iModemManager.data(),
        SIGNAL(defaultDataSimChanged(QString)),
        SLOT(updateModemManagerState()));
    updateModemManagerState();
}

inline bool OfonoLogger::mobileDataBroken() const
{
    return iMobileDataBroken;
}

inline bool OfonoLogger::mobileDataDisabled() const
{
    return iMobileDataDisabled;
}

void OfonoLogger::fixMobileData()
{
    HDEBUG("ok");
    iModemManager->setDefaultDataSim(AUTO);
}

void OfonoLogger::enableMobileData()
{
    HDEBUG("ok");
    iNetworkTechnology->setPowered(true);
}

void OfonoLogger::updateModemManagerState()
{
    maybeSaveFiles();
    // On a single-SIM phone default data sim should be always set to "auto"
    // because UI doesn't provide any way to change it.
    const bool wasBroken = iMobileDataBroken;
    iMobileDataBroken = iModemManager->valid() &&
        iModemManager->availableModems().count() == 1 &&
        iModemManager->defaultDataSim() != AUTO;
    if (iMobileDataBroken != wasBroken) {
        HDEBUG("default data SIM" << (iMobileDataBroken ? "broken" : "ok"));
        Q_EMIT mobileDataBrokenChanged();
    }
}

void OfonoLogger::updateNetworkTechnologyState()
{
    if (iNetworkTechnologyReady) {
        const bool wasDisabled = iMobileDataDisabled;
        iMobileDataDisabled = !iNetworkTechnology->powered();
        if (iMobileDataDisabled != wasDisabled) {
            HDEBUG("data" << (iMobileDataDisabled ? "disabled" : "enabled"));
            Q_EMIT mobileDataDisabledChanged();
        }
    }
}

void OfonoLogger::onNetworkTechnologyReady()
{
    HDEBUG("NetworkTechnology ready");
    iNetworkTechnologyReady = true;
    updateNetworkTechnologyState();
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
        dumpOfonoInfo(path, "Modem.GetProperties");
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

void OfonoLogger::dumpConnmanInfo(QString aPath, QString aCall) const
{
    QString call = QString("net.connman.%1").arg(aCall);
    QString outFile = iSaveDir + "/connman." + aCall;
    const int lastSlash = aPath.lastIndexOf('/');
    if (lastSlash > 0 && (lastSlash + 1) < aPath.length()) {
        outFile += ".";
        outFile += aPath.right(aPath.length() - lastSlash - 1);
    }
    outFile += ".txt";
    saveOutput("dbus-send", "--system", "--print-reply", "--type=method_call",
        "--dest=net.connman",  qPrintable(aPath), qPrintable(call), outFile);
}

void OfonoLogger::saveFilesAtStartup(QString aDir)
{
    iSaveDir = aDir;
    dumpConnmanInfo("/net/connman/technology/cellular",
        "Technology.GetProperties");
    maybeSaveFiles();
    SUPER::saveFilesAtStartup(aDir);
}

void OfonoLogger::setupView(QQuickView* aView)
{
    QQmlContext* context = aView->rootContext();
    //: Settings page title (app name)
    //% "Ofono Log"
    aView->setTitle(qtTrId("openrepos-logger-ofono-app_name"));
    context->setContextProperty("OfonoLogger", this);
    SUPER::setupView(aView);
}

Q_DECL_EXPORT int main(int argc, char* argv[])
{
    QStringList packages;
    packages.append("libgbinder");
    packages.append("libgbinder-radio");
    packages.append("libglibutil");
    packages.append("libgrilio");
    packages.append("libgrilio-binder");
    packages.append("ofono");
    packages.append("ofono-ril-binder-plugin");
    return OfonoLogger(&argc, argv, packages).run();
}

#include "main.moc"
