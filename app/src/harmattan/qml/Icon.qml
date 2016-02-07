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

import QtQuick 1.1
import cuteNews 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

MaskedItem {
    id: root

    property alias source: icon.source
    property alias sourceSize: icon.sourceSize
    property alias fillMode: icon.fillMode
    property alias status: icon.status

    mask: Image {
        width: root.width
        height: root.height
        source: "images/icon-mask.png"
        fillMode: Image.Stretch
        smooth: true
    }

    Image {
        id: frame

        anchors.fill: parent
        source: "images/icon-frame.png"
        sourceSize.width: width
        sourceSize.height: height
        smooth: true
        fillMode: Image.Stretch
        visible: icon.status == Image.Ready

        Image {
            id: icon

            z: -1
            anchors.fill: parent
            sourceSize.width: width
            sourceSize.height: height
            smooth: true
            fillMode: Image.PreserveAspectCrop
            clip: true
            onStatusChanged: if (status == Image.Error) source = "images/icon.png";
        }
    }
}

