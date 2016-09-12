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

Dialog {
    id: root
    
    title: qsTr("Categories")
    height: 360
    
    ListView {
        id: view
        
        anchors {
            left: parent.left
            right: button.left
            rightMargin: platformStyle.paddingMedium
            top: parent.top
            bottom: parent.bottom
        }
        model: CategoryModel {
            id: categoryModel
        }
        delegate: CategoryDelegate {
            onClicked: {
                var dialog = popupLoader.load(categoryDialog, root);
                dialog.name = name;
                dialog.path = path;
                dialog.open();
            }
            onPressAndHold: contextMenu.open()
        }
    }
    
    Label {
        anchors.centerIn: view
        font.pointSize: platformStyle.fontSizeLarge
        color: platformStyle.disabledTextColor
        text: qsTr("No categories")
        visible: categoryModel.count == 0
    }
    
    Menu {
        id: contextMenu
        
        MenuItem {
            text: qsTr("Edit")
            onTriggered: {
                var dialog = popupLoader.load(categoryDialog, root);
                dialog.name = categoryModel.data(view.currentIndex, "name");
                dialog.path = categoryModel.data(view.currentIndex, "path");
                dialog.open();
            }
        }
        
        MenuItem {
            text: qsTr("Remove")
            onTriggered: categoryModel.removeCategory(view.currentIndex)
        }
    }
    
    Button {
        id: button
        
        anchors {
            right: parent.right
            bottom: parent.bottom
        }
        style: DialogButtonStyle {}
        text: qsTr("New")
        onClicked: popupLoader.open(categoryDialog, root)
    }
    
    PopupLoader {
        id: popupLoader
    }
    
    Component {
        id: categoryDialog
        
        CategoryDialog {
            onAccepted: categoryModel.addCategory(name, path)
        }
    }
}
