/*
   Copyright (c) 2017 Marco Martin <mart@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0 as QQC2
import org.kde.kirigami 2.1 as Kirigami


MouseArea {
    id: root
    property alias icon: iconItem.source
    property alias text: label.text
    property string module
    property int iconSize: Kirigami.Units.iconSizes.huge
    Layout.minimumWidth: Kirigami.Units.iconSizes.medium
    Layout.minimumHeight: column.implicitHeight
    cursorShape: Qt.PointingHandCursor
    Layout.fillWidth: true
    Layout.alignment: Qt.AlignTop

    onClicked: systemsettings.loadMostUsed(index);
    ColumnLayout {
        id: column
        width: parent.width
        Kirigami.Icon {
            id: iconItem
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: root.iconSize
            Layout.minimumHeight: Layout.minimumWidth
            height: width
        }
        QQC2.Label {
            id: label
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
    }
    Accessible.role: Accessible.Button
    Accessible.name: label.text
    Accessible.onPressAction: systemsettings.loadMostUsed(index);
}

