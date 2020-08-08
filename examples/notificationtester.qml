/*
   SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>

   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick 2.0

import QtQuick.Window 2.14
import QtQuick.Controls 2.10
import QtQuick.Layouts 1.11

import org.kde.knotifications.tester 1.0

Window {

    width: 600
    height: 400
    visible: true

    ColumnLayout {

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

        Button {
            text: "Send"
            onClicked: Tester.sendNotification(titleField.text, textField.text)
        }
    }
}
