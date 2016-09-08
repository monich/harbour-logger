/*
  Copyright (C) 2015-2016 Jolla Ltd.
  Contact: Slava Monich <slava.monich@jolla.com>

  You may use this file under the terms of BSD license as follows:

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Jolla Ltd nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
  THE POSSIBILITY OF SUCH DAMAGE.
*/

import QtQuick 2.0
import Sailfish.Silica 1.0
import org.nemomobile.configuration 1.0

Page {
    id: page
    readonly property string rootPath: "/apps/harbour-logger-conf/"

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: content.height

        Column {
            id: content
            width: parent.width

            PageHeader {
                //% "Logger"
                title: qsTrId("logger-settings-page-header")
            }

            SectionHeader {
                //% "Display"
                text: qsTrId("logger-settings-section-header-display")
            }

            ComboBox {
                id: logSizeLimitComboBox
                //% "Screen buffer size"
                label: qsTrId("logger-settings-logsizelimit")
                //% "Don't worry, everything will be written to the log file regardless of the screen buffer size."
                description: qsTrId("logger-settings-logsizelimit-description")
                value: currentItem ? currentItem.text : ""
                menu: logSizeLimitMenu
                ContextMenu {
                    id: logSizeLimitMenu
                    readonly property int defaultIndex: 1
                    MenuItem {
                        text: maxLines
                        readonly property int maxLines: 100
                    }
                    MenuItem {
                        text: maxLines
                        readonly property int maxLines: 1000
                    }
                    MenuItem {
                        text: maxLines
                        readonly property int maxLines: 10000
                    }
                    MenuItem {
                        //% "Unlimited"
                        text: qsTrId("logger-settings-logsizelimit-unlimited")
                        readonly property int maxLines: 0
                    }
                }
                onCurrentIndexChanged: if (currentItem) logSizeLimit.value = currentItem.maxLines
                Component.onCompleted: updateSelection(logSizeLimit.value)
                function updateSelection(value) {
                    if (value === undefined) {
                        currentIndex = logSizeLimitMenu.defaultIndex
                    } else {
                        var n = logSizeLimitMenu.children.length
                        for (var i=0; i<n; i++) {
                            if (value === logSizeLimitMenu.children[i].maxLines) {
                                currentIndex = i
                                return
                            }
                        }
                        currentItem = null // Non-standard value
                    }
                }

                ConfigurationValue {
                    id: logSizeLimit
                    key: rootPath + "logSizeLimit"
                    onValueChanged: logSizeLimitComboBox.updateSelection(value)
                    defaultValue: 1000
                }
            }

            Slider {
                id: fontSizeSlider
                width: parent.width
                minimumValue: Theme.fontSizeTiny
                maximumValue: Theme.fontSizeLarge
                stepSize: 1
                //% "Font size"
                label: qsTrId("logger-settings-fontsize-label")
                valueText: minimumValue + sliderValue
                onSliderValueChanged: fontSizeAdjustment.value = sliderValue - minimumValue
                Component.onCompleted: value = minimumValue + fontSizeAdjustment.value

                ConfigurationValue {
                    id: fontSizeAdjustment
                    key: rootPath + "fontSizeAdjustment"
                    onValueChanged: fontSizeSlider.value = fontSizeSlider.minimumValue + value
                    defaultValue: 0
                }
            }
        }
    }
}
