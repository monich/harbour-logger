/*
 * Copyright (C) 2019-2021 Jolla Ltd.
 * Copyright (C) 2019-2021 Slava Monich <slava.monich@jolla.com>
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

#include <QQuickView>

#define SUPER LoggerMain

class NfcLogger: public SUPER
{
    Q_OBJECT

public:
    NfcLogger(int* aArgc, char** aArgv, QStringList aPackages);

protected:
    void setupView(QQuickView* aView) Q_DECL_OVERRIDE;
};

NfcLogger::NfcLogger(int* aArgc, char** aArgv, QStringList aPackages) :
    SUPER(aArgc, aArgv, "org.sailfishos.nfc.daemon", aPackages, "nfc",
    "qml/main.qml")
{
}

void NfcLogger::setupView(QQuickView* aView)
{
    //: Settings page title (app name)
    //% "NFC Logger"
    aView->setTitle(qtTrId("openrepos-logger-nfc-app_name"));
    SUPER::setupView(aView);
}

Q_DECL_EXPORT int main(int argc, char* argv[])
{
    QStringList packages;
    packages.append("libgbinder");
    packages.append("libncicore");
    packages.append("libnciplugin");
    packages.append("libglibutil");
    packages.append("nfcd");
    packages.append("nfcd-binder-plugin");
    packages.append("pn54x-binder-plugin");
    return NfcLogger(&argc, argv, packages).run();
}

#include "main.moc"
