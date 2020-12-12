/*
 * Copyright (C) 2016-2020 Jolla Ltd.
 * Copyright (C) 2016-2020 Slava Monich <slava.monich@jolla.com>
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

import QtQuick 2.0
import Sailfish.Silica 1.0
import "logger.js" as Logger
import "../harbour"

SilicaFlickable {
    id: thisView

    property alias pullDownMenuActive: menu.active

    property var logModel: LogModel
    readonly property string fontFamily: "Monospace"
    clip: true

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
        if (logModel.connected) {
            connected()
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
        id: menu

        // Delay the click because the action may cause custom item to disappear
        property bool customMenuItemClicked

        MenuItem {
            text: customLogMenuItem ? customLogMenuItem.text : ""
            visible: customLogMenuItem ? customLogMenuItem.visible : false
            onClicked: menu.customMenuItemClicked = true
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

        onActiveChanged: {
            if (!active && customMenuItemClicked) {
                customMenuItemClicked = false
                customLogMenuItem.clicked()
            }
        }
    }

    PageHeader {
        id: pageHeader

        //% "Log"
        title: qsTrId("logger-logpage-title")
        visible: y > -height
        y: (list.contentHeight <= thisView.height || !list.atBottom) ? 0 : -height
        Behavior on y { SmoothedAnimation { duration: 200 } }
    }

    SilicaListView {
        id: list

        y: pageHeader.y + pageHeader.height
        width: parent.width
        height: Math.min(contentHeight, thisView.height - y)
        verticalLayoutDirection: ListView.BottomToTop
        model: logModel
        clip: true

        readonly property int rawTextSize: Theme.fontSizeTiny + LogSettings.fontSizeAdjustment
        readonly property int textSize: Math.min(Math.max(Theme.fontSizeTiny, rawTextSize), Theme.fontSizeHuge)
        readonly property real offsetFromBottom: contentHeight + originY - contentY - height
        readonly property bool atBottom: atYEnd || offsetFromBottom <= Theme.paddingLarge/2

        header: Component {
            Item {
                width: 1
                height: Theme.paddingLarge
            }
        }

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
                font.family: thisView.fontFamily
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

        VerticalScrollDecorator {}
    }

    InteractionHintLabel {
        id: selectionHint

        invert: true
        visible: opacity > 0
        opacity: item ? 1.0 : 0.0
        property Item item: null
        Behavior on opacity { FadeAnimation { duration: 1000 } }
    }

    HarbourHintIconButton {
        id: immediateScrollButton

        //: Hint text, displayed on long tap
        //% "Jump to the end of the log to see the most recent messages and follow new messages in real time"
        hint: qsTrId("logger-logpage-hint-jump_to_bottom")
        icon {
            source: "images/down.svg"
            sourceSize: Qt.size(Theme.iconSizeLarge, Theme.iconSizeLarge)
        }
        anchors {
            right: parent.right
            rightMargin: Theme.horizontalPageMargin
            bottom: parent.bottom
            bottomMargin: Theme.paddingLarge
        }
        visible: opacity > 0
        opacity: (!list.moving && !list.atBottom) ? 1.0 : 0.0
        onClicked: {
            cancelHint()
            list.positionViewAtBeginning()
        }
        onShowHint: {
            selectionHint.text = hint
            selectionHint.item = immediateScrollButton
        }
        onHideHint: cancelHint()
        function cancelHint() {
            if (selectionHint.item === immediateScrollButton) {
                selectionHint.item = null
            }
        }
        Behavior on opacity { FadeAnimation { duration: 500 } }
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
