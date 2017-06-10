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

import QtQuick 2.0
import Sailfish.Silica 1.0
import org.nemomobile.configuration 1.0

Page {
    id: page
    readonly property string rootPath: "/apps/" + appName() + "/"

    // Deduce package name from the path
    function appName() {
        var parts = Qt.resolvedUrl("dummy").split('/')
        if (parts.length > 2) {
            var name = parts[parts.length-3]
            if (name.indexOf("-logger") >= 0) {
                return name
            }
        }
        return "harbour-logger"
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: content.height

        Column {
            id: content
            width: parent.width

            PageHeader {
                //: Page header
                //% "Logger"
                title: qsTrId("logger-settings-page-header")
            }

            SectionHeader {
                //: Section header
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
                minimumValue: 0
                maximumValue: Theme.fontSizeLarge - Theme.fontSizeTiny
                stepSize: 1
                //% "Font size"
                label: qsTrId("logger-settings-fontsize-label")
                valueText: Theme.fontSizeTiny + sliderValue
                onSliderValueChanged: fontSizeAdjustment.value = sliderValue
                Component.onCompleted: value = fontSizeAdjustment.value

                ConfigurationValue {
                    id: fontSizeAdjustment
                    key: rootPath + "fontSizeAdjustment"
                    onValueChanged: fontSizeSlider.value = value
                    defaultValue: 0
                }
            }

            SectionHeader {
                //: Section header
                //% "Logging"
                text: qsTrId("logger-settings-section-header-logging")
            }

            TextSwitch {
                id: autoEnableLoggingSwitch
                automaticCheck: false
                //: Text switch label
                //% "Automatically enable logging"
                text: qsTrId("logger-settings-autoenable_logging-switch-label")
                //: Text switch description
                //% "Enable all logging categories when the application starts."
                description: qsTrId("logger-settings-autoenable_logging-switch-description")
                onClicked: autoEnableLogging.value = checked ? autoEnableLogging.autoEnableNone : autoEnableLogging.autoEnableAll
                Component.onCompleted: updateCheck()
                function updateCheck() { checked = (autoEnableLogging.value !== autoEnableLogging.autoEnableNone)  }

                ConfigurationValue {
                    id: autoEnableLogging
                    key: rootPath + "autoEnableLogging"
                    readonly property int autoEnableNone: 0
                    readonly property int autoEnableAll: 1
                    onValueChanged: autoEnableLoggingSwitch.updateCheck()
                    defaultValue: autoEnableAll
                }
            }

            TextSwitch {
                id: autoResetLoggingSwitch
                automaticCheck: false
                //: Text switch label
                //% "Automatically reset logging on exit"
                text: qsTrId("logger-settings-autoreset_logging-switch-label")
                //: Text switch description
                //% "Reset all logging categories to their default values when the application is exiting. Otherwise logging would continue in the background, wasting the system resources and eating up your battery."
                description: qsTrId("logger-settings-autoreset_logging-switch-description")
                onClicked: autoResetLogging.value = !checked
                Component.onCompleted: checked = autoResetLogging.value

                ConfigurationValue {
                    id: autoResetLogging
                    key: rootPath + "autoResetLogging"
                    onValueChanged: autoResetLoggingSwitch.checked = value
                    defaultValue: true
                }
            }
        }
    }
}
