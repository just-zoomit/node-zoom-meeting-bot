/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Quick Controls 2 module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

import QtQuick 2.12
import QtQuick.Templates 2.12 as T
import QtQuick.Controls.Imagine 2.12
import QtQuick.Controls.Imagine.impl 2.12

T.Popup {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    topPadding: background ? background.topPadding : undefined
    leftPadding: background ? background.leftPadding : undefined
    rightPadding: background ? background.rightPadding : undefined
    bottomPadding: background ? background.bottomPadding : undefined

    topInset: background ? -background.topInset || 0 : 0
    leftInset: background ? -background.leftInset || 0 : 0
    rightInset: background ? -background.rightInset || 0 : 0
    bottomInset: background ? -background.bottomInset || 0 : 0

    background: NinePatchImage {
        source: Imagine.url + "popup-background"
        NinePatchImageSelector on source {
            states: [
                {"modal": control.modal},
                {"dim": control.dim}
            ]
        }
    }

    T.Overlay.modal: NinePatchImage {
        source: Imagine.url + "popup-overlay"
        NinePatchImageSelector on source {
            states: [
                {"modal": true}
            ]
        }
    }

    T.Overlay.modeless: NinePatchImage {
        source: Imagine.url + "popup-overlay"
        NinePatchImageSelector on source {
            states: [
                {"modal": false}
            ]
        }
    }
}