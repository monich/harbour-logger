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

SilicaFlickable {
    id: categoryPage
    property var categoryModel: CategoryModel

    PullDownMenu {
        visible: categoryModel.connected
        MenuItem {
            //% "Enable all"
            text: qsTrId("logger-categories-pm-enable-all")
            onClicked: categoryModel.enableAll()
            visible: categoryModel.count > 0
        }
        MenuItem {
            //% "Disable all"
            text: qsTrId("logger-categories-pm-disable-all")
            onClicked: categoryModel.disableAll()
            visible: categoryModel.count > 0
        }
        MenuItem {
            //% "Reset to default"
            text: qsTrId("logger-categories-pm-default")
            onClicked: categoryModel.reset()
            visible: categoryModel.haveDefaults
        }
    }

    SilicaListView {
        id: list
        model: categoryModel
        clip: true
        anchors.fill: parent

        //% "Log categories"
        header: PageHeader { title: qsTrId("logger-categories-title") }
        delegate: Item {
            width: parent.width
            height: checkBox.height
            TextSwitch {
                id: checkBox
                text: categoryName
                checked: categoryEnabled
                automaticCheck: false
                onClicked: {
                    if (checked) {
                        categoryModel.disable(categoryId)
                    } else {
                        categoryModel.enable(categoryId)
                    }
                }
                anchors {
                    left: parent.left
                    right: parent.right
                    leftMargin: Theme.horizontalPageMargin
                    rightMargin: Theme.horizontalPageMargin
                }
            }
        }

        VerticalScrollDecorator {}
    }

    ViewPlaceholder {
        enabled: !categoryModel.connected
        visible: opacity > 0
        Behavior on opacity { FadeAnimation {} }
        //% "Not connected"
        text: qsTrId("logger-categories-not-connected")
    }
}
