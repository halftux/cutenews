/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 1.1
import com.nokia.symbian 1.1
import cuteNews 1.0

MyPage {
    id: root

    title: qsTr("Downloads")
    tools: ToolBarLayout {

        BackToolButton {}

        MyToolButton {
            id: startButton
            
            iconSource: "toolbar-mediacontrol-play"
            toolTip: qsTr("Start")
            enabled: transfers.count > 0
            onClicked: transfers.start()
        }

        MyToolButton {
            id: pauseButton
            
            iconSource: "toolbar-mediacontrol-pause"
            toolTip: qsTr("Pause")
            enabled: transfers.count > 0
            onClicked: transfers.pause()
        }
        
        MyToolButton {
            id: reloadButton
            
            iconSource: "toolbar-refresh"
            toolTip: qsTr("Reload")
            onClicked: transfers.load()
        }
    }

    MyListView {
        id: view

        anchors.fill: parent
        model: transfers
        delegate: TransferDelegate {
            onClicked: popups.open(contextMenu, root)
            onPressAndHold: popups.open(contextMenu, root)
        }
    }

    ScrollDecorator {
        flickableItem: view
    }

    Label {
        id: label
        
        anchors {
            fill: parent
            margins: platformStyle.paddingLarge
        }
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
        font.pixelSize: 32
        color: platformStyle.colorNormalMid
        text: qsTr("No downloads")
        visible: false
    }
    
    Component {
        id: contextMenu
        
        MyContextMenu {
            id: contextMenu

            focusItem: view

            MenuLayout {

                MenuItem {
                    text: qsTr("Start")
                    enabled: transfers.data(view.currentIndex, "status") <= Transfer.Queued
                    onClicked: transfers.get(view.currentIndex).queue()
                }

                MenuItem {
                    text: qsTr("Pause")
                    enabled: transfers.data(view.currentIndex, "status") >= Transfer.Downloading
                    onClicked: transfers.get(view.currentIndex).pause()
                }

                MenuItem {
                    text: qsTr("Priority")
                    platformSubItemIndicator: true
                    onClicked: popups.open(priorityDialog, root)
                }

                MenuItem {
                    text: qsTr("Category")
                    platformSubItemIndicator: true
                    onClicked: popups.open(categoryDialog, root)
                }

                MenuItem {
                    text: qsTr("Remove")
                    onClicked: popups.open(removeDialog, root)
                }
            }
        }
    }

    Component {
        id: priorityDialog

        ValueDialog {
            focusItem: view
            titleText: qsTr("Priority")
            model: TransferPriorityModel {}
            value: transfers.data(view.currentIndex, "priority")
            onAccepted: transfers.setData(view.currentIndex, value, "priority")
        }
    }

    Component {
        id: categoryDialog

        ValueDialog {
            focusItem: view
            titleText: qsTr("Category")
            model: CategoryNameModel {}
            value: transfers.data(view.currentIndex, "category")
            onAccepted: transfers.setData(view.currentIndex, value, "category")
        }
    }
    
    Component {
        id: removeDialog
        
        MyQueryDialog {
            titleText: qsTr("Remove?")
            message: qsTr("Do you want to remove") + " '" + transfers.data(view.currentIndex, "name")+ "'?"
            onAccepted: transfers.get(view.currentIndex).cancel()
        }
    }
    
    Connections {
        target: transfers
        onStatusChanged: {
            switch (transfers.status) {
            case TransferModel.Active: {
                root.showProgressIndicator = true;
                reloadButton.enabled = false;
                label.visible = false;
                break;
            }
            default: {
                root.showProgressIndicator = false;
                reloadButton.enabled = true;
                label.visible = (transfers.count == 0);
                break;
            }
            }
        }
    }
    
    Component.onCompleted: transfers.load()
}
