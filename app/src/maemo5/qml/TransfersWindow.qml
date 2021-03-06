/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
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

import QtQuick 1.0
import org.hildon.components 1.0
import cuteNews 1.0

Window {
    id: root
    
    title: qsTr("Downloads")
    objectName: "TransfersWindow"
    menuBar: MenuBar {
        MenuItem {
            action: startAllAction
        }
        
        MenuItem {
            action: pauseAllAction
        }
    }
    
    Action {
        id: startAllAction
        
        text: qsTr("Start all downloads")
        autoRepeat: false
        shortcut: settings.startAllTransfersShortcut
        onTriggered: transfers.start()
    }
    
    Action {
        id: pauseAllAction
        
        text: qsTr("Pause all downloads")
        autoRepeat: false
        shortcut: settings.pauseAllTransfersShortcut
        onTriggered: transfers.pause()
    }
    
    Action {
        id: startAction
        
        text: qsTr("Start")
        autoRepeat: false
        shortcut: settings.startTransferShortcut
        onTriggered: if (view.currentIndex >= 0) transfers.get(view.currentIndex).queue();
    }
    
    Action {
        id: pauseAction
        
        text: qsTr("Pause")
        autoRepeat: false
        shortcut: settings.pauseTransferShortcut
        onTriggered: if (view.currentIndex >= 0) transfers.get(view.currentIndex).pause();
    }

    Action {
        id: commandAction

        text: qsTr("Command")
        autoRepeat: false
        shortcut: settings.transferCommandShortcut
        onTriggered: if (view.currentIndex >= 0) popupManager.open(commandDialog, root);
    }
    
    Action {
        id: categoryAction
        
        text: qsTr("Category")
        autoRepeat: false
        shortcut: settings.transferCategoryShortcut
        onTriggered: if (view.currentIndex >= 0) popupManager.open(categoryDialog, root);
    }
    
    Action {
        id: priorityAction
        
        text: qsTr("Priority")
        autoRepeat: false
        shortcut: settings.transferPriorityShortcut
        onTriggered: if (view.currentIndex >= 0) popupManager.open(priorityDialog, root);
    }
    
    Action {
        id: removeAction
        
        text: qsTr("Remove")
        autoRepeat: false
        shortcut: settings.deleteShortcut
        onTriggered: if (view.currentIndex >= 0) popupManager.open(removeDialog, root);
    }
    
    ListView {
        id: view
        
        anchors.fill: parent
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        model: TransferModel {
            id: transferModel
        }
        delegate: TransferDelegate {
            onClicked: popupManager.open(contextMenu, root)
            onPressAndHold: popupManager.open(contextMenu, root)
        }
    }
    
    Label {
        anchors.centerIn: parent
        font.pointSize: platformStyle.fontSizeLarge
        color: platformStyle.disabledTextColor
        text: qsTr("No downloads")
        visible: transfers.count == 0
    }
    
    Component {
        id: contextMenu
        
        Menu {            
            MenuItem {
                text: transferModel.data(view.currentIndex, "status") >= Transfer.Queued
                ? qsTr("Pause") : qsTr("Start")
                onTriggered: transferModel.data(view.currentIndex, "status") >= Transfer.Queued
                ? transfers.get(view.currentIndex).pause() : transfers.get(view.currentIndex).queue()
            }

            MenuItem {
                action: commandAction
            }
            
            MenuItem {
                action: categoryAction
            }
            
            MenuItem {
                action: priorityAction
            }
            
            MenuItem {
                action: removeAction
            }
        }
    }

    Component {
        id: commandDialog

        CustomCommandDialog {
            command: transferModel.data(view.currentIndex, "customCommand")
            overrideEnabled: transferModel.data(view.currentIndex, "customCommandOverrideEnabled")
            onAccepted: transferModel.setItemData(view.currentIndex,
            {customCommand: command, customCommandOverrideEnabled: overrideEnabled})
        }
    }
    
    Component {
        id: categoryDialog
        
        ListPickSelector {
            title: qsTr("Category")
            model: CategoryNameModel {}
            textRole: "name"
            currentIndex: Math.max(0, model.match(0, "value", transferModel.data(view.currentIndex, "category")))
            onSelected: transferModel.setData(view.currentIndex, text, "category")
        }
    }
    
    Component {
        id: priorityDialog
        
        ListPickSelector {
            title: qsTr("Priority")
            model: TransferPriorityModel {}
            textRole: "name"
            currentIndex: Math.max(0, model.match(0, "value", transferModel.data(view.currentIndex, "priority")))
            onSelected: transferModel.setData(view.currentIndex, model.data(currentIndex, "value"), "priority")
        }
    }
    
    Component {
        id: removeDialog
        
        MessageBox {
            text: qsTr("Do you want to remove") + " '" + transferModel.data(view.currentIndex, "name") + "'?"
            onAccepted: transfers.get(view.currentIndex).cancel()
        }
    }
}
