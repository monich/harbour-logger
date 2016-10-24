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

import QtQuick 2.0
import Sailfish.Silica 1.0
import "logger.js" as Logger

SilicaFlickable {
    id: page
    property var logModel: LogModel
    readonly property string fontFamily: "Monospace"

    function packAndShare() {
        LogSaver.pack()
        pageStack.push(sharePageComponent)
    }

    Component {
        id: sharePageComponent
        SharePage {}
    }

    function connected() {
        connectTimer.stop()
        //% "Connected."
        logModel.add(qsTrId("logger-logpage-msg-connected"))
    }

    Component.onCompleted: {
        // Dynamically bind custom menu item
        if (customLogMenuItem) {
            customMenuItem.clicked.connect(customLogMenuItem.clicked)
            customLogMenuItemTextComponent.createObject(page)
            customLogMenuItemVisibleComponent.createObject(page)
            if (customLogMenuItem.active !== undefined) {
                customLogMenuItemActiveComponent.createObject(page)
            }
        }
        if (logModel.connected) {
            connected()
        }
    }

    Component {
        id: customLogMenuItemTextComponent
        Binding {
            target: customMenuItem
            property: "text"
            value: customLogMenuItem.text
        }
    }

    Component {
        id: customLogMenuItemVisibleComponent
        Binding {
            target: customMenuItem
            property: "visible"
            value: customLogMenuItem.visible
        }
    }

    Component {
        id: customLogMenuItemActiveComponent
        Binding {
            target: customLogMenuItem
            property: "active"
            value: pullDownMenu.active
        }
    }

    Connections {
        target: logModel
        //% "Log cleared"
        onLogCleared: logModel.add(qsTrId("logger-logpage-msg-log-cleared"))
        onConnectedChanged: {
            if (logModel.connected) {
                connected()
            } else {
                //% "Connection lost."
                logModel.add(qsTrId("logger-logpage-msg-disconnected"))
            }
        }
    }

    PullDownMenu {
        id: pullDownMenu
        MenuItem {
            id: customMenuItem
            visible: false
        }
        MenuItem {
            //% "Clear log"
            text: qsTrId("logger-logpage-pm-clear-log")
            onClicked: logModel.clear()
            visible: !logModel.empty
        }
        MenuItem {
            //% "Pack and send"
            text: qsTrId("logger-logpage-pm-pack-and-send")
            onClicked: packAndShare()
        }
    }

    SilicaListView {
        id: list
        model: logModel
        anchors.fill: parent
        clip: true

        readonly property int rawTextSize: Theme.fontSizeTiny + LogSettings.fontSizeAdjustment
        readonly property int textSize: Math.min(Math.max(Theme.fontSizeTiny, rawTextSize), Theme.fontSizeHuge)

        //% "Log"
        header: PageHeader { title: qsTrId("logger-logpage-title") }
        delegate: Item {
            width: parent.width
            height: textLabel.height
            readonly property color textColor: Logger.textColor(messageType, messageLevel)
            Label {
                id: timeLabel
                text: (messageType === Logger.TypeLog) ? messageTime : ""
                font.pixelSize: list.textSize
                color: textColor
                anchors {
                    top: parent.top
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                    rightMargin: Theme.horizontalPageMargin
                }
            }
            Label {
                id: textLabel
                text: messageCategory ? messageCategory + ": " + messageText : messageText
                font.pixelSize: list.textSize
                font.family: page.fontFamily
                wrapMode: Text.WordWrap
                color: textColor
                anchors {
                    top: parent.top
                    left: timeLabel.right
                    right: parent.right
                    leftMargin: Theme.horizontalPageMargin
                    rightMargin: Theme.horizontalPageMargin
                }
            }
        }

        onCountChanged: {
            if (atYEnd && !dragging && !flicking) {
                positioner.restart()
            }
        }

        onHeightChanged: if (atYEnd) positioner.restart()

        VerticalScrollDecorator {}
    }

    Timer {
        id: positioner
        interval: 100
        onTriggered: {
            if (!pullDownMenu.active) {
                list.cancelFlick()
                list.positionViewAtEnd()
            }
        }
    }

    Timer {
        id: connectTimer
        interval: 5000
        running: true
        onTriggered: {
            if (!logModel.connected) {
                //% "We are having trouble connecting to the service."
                logModel.add(qsTrId("logger-logpage-msg-connect-error"))
            }
        }
    }
}
