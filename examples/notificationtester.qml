/*
   SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick 2.0

import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.11

import org.kde.notification 1.0

ApplicationWindow {

    width: 600
    height: 400
    visible: true

    Component {
        id: notificationComponent
        Notification {
            componentName: Qt.platform.os === "android" ? "android_defaults" : "plasma_workspace"
            eventId: "notification"
            text: "Temporary notification we can create new instances of."
            autoDelete: true
        }
    }

    ColumnLayout {
        anchors.fill: parent

        Notification {
            id: basicNotification
            componentName: Qt.platform.os === "android" ? "android_defaults" : "plasma_workspace"
            eventId: "notification"
            title: titleField.text
            text: textField.text
            iconName: "kde"
            defaultAction: "Default Action"
            actions: ["Action 1", "Action 2", "Action 3"]
            flags: (persistentFlag.checked ? Notification.Persistent : Notification.CloseOnTimeout) | (skipGroupingFlag ? Notification.SkipGrouping : 0)
            urgency: urgencyCombo.currentValue

            onClosed: log.append("Notification closed.")
            onDefaultActivated: log.append("Default action activated.")
            onAction1Activated: log.append("Action 1 activated.")
            onAction2Activated: log.append("Action 2 activated.")
            onAction3Activated: log.append("Action 3 activated.")
            onIgnored: log.append("Notification ignored.")
        }

        RowLayout {
            Label {
                text: "Title"
            }
            TextField {
                id: titleField
                text: "Test"
            }
        }
        RowLayout {
            Label {
                text: "Text"
            }
            TextField {
                id: textField
                text: "Lorem ipsum dolor sit"
            }
        }

        CheckBox {
            id: persistentFlag
            text: "Persistent"
        }
        CheckBox {
            id: skipGroupingFlag
            text: "Skip Grouping"
        }

        ListModel {
            id: urgencyModel
            ListElement { name: "Default"; value: Notification.DefaultUrgency }
            ListElement { name: "Low"; value: Notification.LowUrgency }
            ListElement { name: "Normal"; value: Notification.NormalUrgency }
            ListElement { name: "High"; value: Notification.HighUrgency }
            ListElement { name: "Critical"; value: Notification.CriticalUrgency }
        }
        ComboBox {
            id: urgencyCombo
            model: urgencyModel
            textRole: "name"
            valueRole: "value"
        }

        RowLayout {
            Button {
                text: "Show"
                onClicked: basicNotification.sendEvent()
            }
            Button {
                text: "Close"
                onClicked: basicNotification.close()
            }
        }

        Notification {
            id: replyNotification
            componentName: Qt.platform.os === "android" ? "android_defaults" : "plasma_workspace"
            eventId: "notification"
            title: titleField.text
            text: textField.text
            urgency: urgencyCombo.currentValue
            replyAction {
                label: "Reply"
                placeholderText: "Reply to chat group..."
                submitButtonText: "Send Reply"
                submitButtonIconName: "mail-reply-all"
                onReplied: log.append("Reply: " + text)
                onActivated: log.append("Reply action activated")
            }

            onClosed: log.append("Reply notification closed.")
            onDefaultActivated: log.append("Reply default action activated.")
            onIgnored: log.append("Reply notification ignored.")
        }
        Button {
            text: "Inline Reply"
            onClicked: replyNotification.sendEvent()
        }

        Button {
            text: "Create New"
            property int count: 1
            onClicked: {
                var n = notificationComponent.createObject(parent, { title = "New Notification " + count });
                n.sendEvent()
                ++count;
            }
        }

        TextArea {
            Layout.fillWidth: true
            Layout.fillHeight: true
            id: log
        }
    }
}
