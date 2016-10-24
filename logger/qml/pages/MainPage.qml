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

Page {
    id: mainPage
    allowedOrientations: window.allowedOrientations
    property var customLogMenuItem

    readonly property var pages: [ logPage, categoryPage ]
    readonly property bool swipeHintEnabled: loggerHints.categoryLeftSwipe < loggerHints.categoryLeftSwipeMax

    Component {
        id: logPage
        LogPage {}
    }

    Component {
        id: categoryPage
        CategoryPage {}
    }

    SilicaListView {
        id: scroller
        anchors.fill: parent
        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem
        model: pages
        clip: true
        delegate: Item {
            width: scroller.width
            height: scroller.height
            Loader {
                anchors.fill: parent
                sourceComponent: modelData
            }
        }
        onWidthChanged: updateCurrentIndex()
        onContentXChanged: updateCurrentIndex()
        function updateCurrentIndex() {
            currentIndex = indexAt(contentX + width/2, contentY + height/2)
        }
        onCurrentIndexChanged: {
            if (currentIndex && swipeHintEnabled) {
                // Use has seen the second page
                loggerHints.categoryLeftSwipe++
                if (swipeHint.item) {
                    swipeHint.item.stop()
                }
            }
        }
    }

    Component.onCompleted: swipeHint.showIfNeeded()
    onSwipeHintEnabledChanged: swipeHint.showIfNeeded()

    // Swipe hint for the first time users

    Connections {
        target: LogModel
        onConnectedChanged: swipeHint.showIfNeeded()
    }

    Loader {
        id: swipeHint
        anchors.fill: scroller
        active: false
        property bool hintShown
        function showIfNeeded() {
            if (swipeHintEnabled && !hintShown && LogModel.connected) {
                active = true
                hintShown = true
            }
        }
        sourceComponent: Component {
            Item {
                function stop() { touchInteractionHint.stop() }
                InteractionHintLabel {
                    //% "Swipe left to configure log categories"
                    text: qsTrId("logger-mainpage-swipe-left-hint")
                    anchors.fill: parent
                    opacity: touchInteractionHint.running ? 1.0 : 0.0
                    Behavior on opacity { FadeAnimation { duration: 1000 } }
                }
                TouchInteractionHint {
                    id: touchInteractionHint
                    direction: TouchInteraction.Left
                    anchors.centerIn: parent
                    Component.onCompleted: start()
                }
            }
        }
    }
}
