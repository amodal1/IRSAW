/*
 * Copyright (c) 2016 Intel Corporation.  All rights reserved.
 * See the bottom of this file for the license terms.
 *
 * Modified for Joule demo project - 
 */

import QtQuick 2.0

Rectangle {
    id: rectangle1
    width: 500
    height: 600

    Header {
        id: header
        height: 55
        color: "#0055ff"
        radius: 3
        border.color: "#4ad4af"
        anchors.top: parent.top
        headerText: "Characteristics list"
    }

    Dialog {
        id: info
        anchors.verticalCenterOffset: -212
        anchors.horizontalCenterOffset: 1
        anchors.centerIn: parent
        visible: true
        dialogText: "Scanning for characteristics...";
    }

    Connections {
        target: device
        onCharacteristicsUpdated: {
            menu.menuText = "Back"
            if (characteristicview.count === 0) {
                info.dialogText = "No characteristic found"
                info.busyImage = false
            } else {
                info.visible = false
                info.busyImage = true
            }
        }

        onDisconnected: {
            pageLoader.source = "main.qml"
        }
    }

    ListView {
        id: characteristicview
        width: parent.width
        clip: true

        anchors.top: header.bottom
        anchors.bottom: menu.top
        model: device.characteristicList

        delegate: Rectangle {
            id: characteristicbox
            height:100
            width: parent.width
            color: "lightsteelblue"
            border.width: 1
            border.color: "black"
            radius: 5

            Label {
                id: characteristicName
                textContent: modelData.characteristicName
                anchors.top: parent.top
                anchors.topMargin: 5
            }

            Label {
                id: characteristicUuid
                font.pointSize: characteristicName.font.pointSize*0.7
                textContent: modelData.characteristicUuid
                anchors.top: characteristicName.bottom
                anchors.topMargin: 5
            }

            Label {
                id: characteristicValue
                font.pointSize: characteristicName.font.pointSize*0.7
                textContent: ("Value: " + modelData.characteristicValue)
                anchors.bottom: characteristicHandle.top
                horizontalAlignment: Text.AlignHCenter
                anchors.topMargin: 5
            }

            Label {
                id: characteristicHandle
                font.pointSize: characteristicName.font.pointSize*0.7
                textContent: ("Handlers: " + modelData.characteristicHandle)
                anchors.bottom: characteristicPermission.top
                anchors.topMargin: 5
            }

            Label {
                id: characteristicPermission
                font.pointSize: characteristicName.font.pointSize*0.7
                textContent: modelData.characteristicPermission
                anchors.bottom: parent.bottom
                anchors.topMargin: 5
                anchors.bottomMargin: 5
            }
        }
    }

    Menu {
        id: menu
        anchors.bottom: parent.bottom
        menuWidth: parent.width
        menuText: device.update
        menuHeight: (parent.height/10)
        onButtonClick: {
            pageLoader.source = "Services.qml"
            device.update = "Back"
        }
    }
////////////////////////////////////////////////////////////////////////////////////////////////////////////
    MouseArea {
        id: mouseArea1
        y: 435
        height: 100
        anchors.top: mouseArea2.top
        anchors.left: parent.left
        anchors.leftMargin: 5
        anchors.right: mouseArea2.left
        anchors.rightMargin: 4
        anchors.bottom: mouseArea2.bottom
        anchors.bottomMargin: 0

        Text {
            id: text1
            x: 68
            y: 44
            color: "#ff0000"
            text: qsTr("Up")
            scale: 1.5
            font.pixelSize: 12
        }
        onClicked: {
            console.count();
            console.log("Volume Up");
            device.writeData(1);
        }
    }
////////////////////////////////////////////////////////////////////////////////////////////////////////////
    MouseArea {
        id: mouseArea2
        y: 435
        height: 100
        anchors.right: parent.right
        anchors.rightMargin: 176
        anchors.left: parent.left
        anchors.leftMargin: 161
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 65

        Text {
            id: text2
            x: 64
            y: 44
            color: "#0000ff"
            text: qsTr("Down")
            font.pixelSize: 12
            scale: 1.5
        }
        onClicked: {
            console.count();
            console.log("Volume Down");
            device.writeData(2);
        }
    }
////////////////////////////////////////////////////////////////////////////////////////////////////////////
    MouseArea {
        id: mouseArea3
        x: 129
        y: 434
        width: 163
        height: 100
        anchors.rightMargin: -169
        anchors.leftMargin: 169
        anchors.right: mouseArea2.right
        anchors.left: mouseArea2.left
        anchors.bottom: menu.top
        anchors.bottomMargin: 6

        Text {
            id: text3
            x: 41
            y: 44
            color: "#000000"
            text: qsTr("Light Switch")
            font.pixelSize: 12
            scale: 1.5
        }
        onClicked: {
            console.count();
            console.log("Light Switch");
            device.writeData(3);
        }
    }
////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/*
   Copyright (c) 2016 Intel Corporation.  All rights reserved.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/***************************************************************************
**
** Copyright (C) 2013 BlackBerry Limited. All rights reserved.
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
