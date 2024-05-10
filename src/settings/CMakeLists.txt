# SPDX-FileCopyrightText: 2024 Gary Wang <opensource@blumia.net>
# SPDX-License-Identifier: BSD-2-Clause

qt_add_library(settings STATIC)
qt_add_qml_module(settings
    URI org.kde.marknote.settings
    QML_FILES
        MarkNoteSettings.qml
        MarkNoteGeneralPage.qml
)
