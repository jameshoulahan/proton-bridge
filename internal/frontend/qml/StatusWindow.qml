// Copyright (c) 2021 Proton Technologies AG
//
// This file is part of ProtonMail Bridge.
//
// ProtonMail Bridge is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ProtonMail Bridge is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ProtonMail Bridge.  If not, see <https://www.gnu.org/licenses/>.

import QtQml 2.12
import QtQuick 2.13
import QtQuick.Window 2.13
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.13

import Proton 4.0
import Notifications 1.0

Window {
    id: root
    title: "ProtonMail Bridge"

    height: contentLayout.implicitHeight
    width: contentLayout.implicitWidth

    flags: Qt.FramelessWindowHint | Qt.NoDropShadowWindowHint | Qt.WindowStaysOnTopHint

    property ColorScheme colorScheme: ProtonStyle.currentStyle

    property var backend
    property var notifications

    color: "transparent"

    signal showMainWindow()
    signal showHelp()
    signal showSettings()
    signal showSignIn(string username)
    signal quit()

    ColumnLayout {
        id: contentLayout

        Layout.minimumHeight: 201

        anchors.fill: parent
        spacing: 0

        ColumnLayout {
            Layout.minimumWidth: 448
            Layout.fillWidth: true
            spacing: 0

            Item {
                implicitHeight: 12
                Layout.fillWidth: true
                clip: true
                Rectangle {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: parent.height * 2
                    radius: 10

                    color: {
                        if (!statusItem.activeNotification) {
                            return root.colorScheme.signal_success
                        }

                        switch (statusItem.activeNotification.type) {
                            case Notification.NotificationType.Danger:
                            return root.colorScheme.signal_danger
                            case Notification.NotificationType.Warning:
                            return root.colorScheme.signal_warning
                            case Notification.NotificationType.Success:
                            return root.colorScheme.signal_success
                            case Notification.NotificationType.Info:
                            return root.colorScheme.signal_info
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true

                implicitHeight: children[0].implicitHeight + children[0].anchors.topMargin + children[0].anchors.bottomMargin
                implicitWidth: children[0].implicitWidth + children[0].anchors.leftMargin + children[0].anchors.rightMargin

                color: colorScheme.background_norm

                RowLayout {
                    anchors.fill: parent

                    anchors.topMargin: 8
                    anchors.bottomMargin: 8
                    anchors.leftMargin: 24
                    anchors.rightMargin: 24

                    spacing: 8

                    Status {
                        id: statusItem

                        Layout.fillHeight: true
                        Layout.fillWidth: true

                        Layout.topMargin: 20
                        Layout.bottomMargin: 20

                        colorScheme: root.colorScheme
                        backend: root.backend
                        notifications: root.notifications

                        notificationWhitelist: Notifications.Group.Connection | Notifications.Group.Update | Notifications.Group.Configuration
                    }

                    Button {
                        colorScheme: root.colorScheme
                        secondary: true

                        visible: (statusItem.activeNotification && statusItem.activeNotification.action) ? true : false
                        action: statusItem.activeNotification && statusItem.activeNotification.action.length > 0 ? statusItem.activeNotification.action[0] : null
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: root.colorScheme.background_norm

                Rectangle {
                    anchors.fill: parent
                    anchors.leftMargin: 24
                    anchors.rightMargin: 24
                    color: root.colorScheme.border_norm
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Layout.maximumHeight: accountListView.count ?
            accountListView.contentHeight / accountListView.count * 3 + accountListView.anchors.topMargin + accountListView.anchors.bottomMargin :
            Number.POSITIVE_INFINITY

            color: root.colorScheme.background_norm
            clip: true

            implicitHeight: children[0].contentHeight + children[0].anchors.topMargin + children[0].anchors.bottomMargin
            implicitWidth: children[0].contentWidth + children[0].anchors.leftMargin + children[0].anchors.rightMargin

            ListView {
                id: accountListView

                model: root.backend.users
                anchors.fill: parent

                anchors.topMargin: 8
                anchors.bottomMargin: 8
                anchors.leftMargin: 24
                anchors.rightMargin: 24

                interactive: contentHeight > parent.height
                snapMode: ListView.SnapToItem
                boundsBehavior: Flickable.StopAtBounds

                delegate: Item {
                    id: viewItem
                    width: ListView.view.width

                    implicitHeight: children[0].implicitHeight
                    implicitWidth: children[0].implicitWidth

                    property var user: root.backend.users.get(index)

                    RowLayout {
                        spacing: 0
                        anchors.fill: parent

                        AccountDelegate {
                            Layout.fillWidth: true

                            Layout.margins: 12

                            user: viewItem.user
                            colorScheme: root.colorScheme
                        }

                        Button {
                            Layout.margins: 12
                            colorScheme: root.colorScheme
                            visible: viewItem.user ? !viewItem.user.loggedIn : false
                            text: qsTr("Sign in")
                            onClicked: {
                                root.showSignIn(viewItem.username)
                                root.close()
                            }
                        }
                    }
                }
            }
        }

        Item {
            Layout.fillWidth: true

            implicitHeight: children[1].implicitHeight + children[1].anchors.topMargin + children[1].anchors.bottomMargin
            implicitWidth: children[1].implicitWidth + children[1].anchors.leftMargin + children[1].anchors.rightMargin

            // background:
            clip: true
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height: parent.height * 2
                radius: 10

                color: root.colorScheme.background_weak
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 0

                Button {
                    colorScheme: root.colorScheme
                    secondary: true
                    text: qsTr("Open ProtonBridge")

                    borderless: true
                    labelType: Label.LabelType.Caption_semibold

                    onClicked: {
                        root.showMainWindow()
                        root.close()
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    colorScheme: root.colorScheme
                    secondary: true
                    icon.source: "./icons/ic-three-dots-vertical.svg"
                    borderless: true
                    checkable: true

                    onClicked: {
                        menu.open()
                    }

                    Menu {
                        id: menu
                        colorScheme: root.colorScheme
                        modal: true

                        y: 0 - height

                        MenuItem {
                            colorScheme: root.colorScheme
                            text: qsTr("Help")
                            onClicked: {
                                root.showHelp()
                                root.close()
                            }
                        }
                        MenuItem {
                            colorScheme: root.colorScheme
                            text: qsTr("Settings")
                            onClicked: {
                                root.showSettings()
                                root.close()
                            }
                        }
                        MenuItem {
                            colorScheme: root.colorScheme
                            text: qsTr("Quit ProtonBridge")
                            onClicked: {
                                root.quit()
                                root.close()
                            }
                        }

                        onClosed: {
                            parent.checked = false
                        }
                        onOpened: {
                            parent.checked = true
                        }
                    }
                }
            }
        }
    }

    onActiveChanged: {
        if (!active) root.close()
    }

    function showAndRise() {
        root.show()
        root.raise()
        if (!root.active) {
            root.requestActivate()
        }
    }
}
