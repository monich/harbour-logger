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

import QtQuick 2.0
import Sailfish.Silica 1.0

import "logger.js" as Logger
import "../harbour"

CoverBackground {
    id: cover

    readonly property bool _privileged: ProcessState.privileged

    Label {
        id: title
        horizontalAlignment: Text.AlignHCenter
        //% "Log"
        text: qsTrId("logger-cover-title")
        font.bold: true
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            topMargin: Theme.paddingMedium
            leftMargin: Theme.paddingMedium
            rightMargin: Theme.paddingMedium
        }
    }
    ListView {
        id: list

        visible: _privileged
        anchors {
            top: title.bottom
            left: parent.left
            right: parent.right
            leftMargin: Theme.paddingMedium
            rightMargin: Theme.paddingSmall
        }
        height: Math.min(contentHeight, parent.height - y - Theme.itemSizeSmall/cover.parent.scale)
        verticalLayoutDirection: ListView.BottomToTop
        model: LogModel
        clip: true
        delegate: Label {
            text: messageText
            width: parent.width
            color: Logger.textColor(messageType, messageLevel)
            truncationMode: TruncationMode.Fade
            font.pixelSize: Theme.fontSizeTiny
        }
    }

    OpacityRampEffect {
        enabled: list.contentHeight > list.height
        sourceItem: list
        parent: cover
        direction: OpacityRamp.BottomToTop
        slope: 3
        offset: 1 - 1 / slope
    }

    HarbourHighlightIcon {
        visible: !_privileged
        x: Theme.paddingLarge
        anchors.verticalCenter: parent.verticalCenter
        source: "images/shrug.svg"
        sourceSize.width: cover.width - 2 * x
        highlightColor: Theme.highlightColor
        smooth: true
    }

    CoverActionList {
        enabled: _privileged
        CoverAction {
            iconSource: "image://theme/icon-cover-cancel"
            onTriggered: list.model.clear()
        }
    }
}
