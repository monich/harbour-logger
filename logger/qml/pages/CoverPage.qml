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

CoverBackground {
    id: cover
    allowResize: true

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
        anchors {
            top: title.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            leftMargin: Theme.paddingMedium
            rightMargin: Theme.paddingSmall
            bottomMargin: Theme.itemSizeSmall
        }
        model: LogModel
        onCountChanged: positioner.start()
        onHeightChanged: scrollToEnd()
        delegate: Label {
            text: messageText
            width: parent.width
            color: Logger.textColor(messageType, messageLevel)
            truncationMode: TruncationMode.Fade
            font.pixelSize: Theme.fontSizeTiny
        }
        function scrollToEnd() {
            if (count > 1) {
                // FsIoLog model has one extra item at the end
                positionViewAtIndex(count - 2, ListView.End)
            }
        }
        Timer {
            // positionViewAtXxx() doesn't work directly from onCountChanged
            id: positioner
            interval: 0
            onTriggered: list.scrollToEnd()
        }
    }

    OpacityRampEffect {
        enabled: !list.atYBeginning
        sourceItem: list
        parent: cover
        direction: OpacityRamp.BottomToTop
        slope: 3
        offset: 1 - 1 / slope
    }

    CoverActionList {
        CoverAction {
            iconSource: "image://theme/icon-cover-cancel"
            onTriggered: list.model.clear()
        }
    }
}
