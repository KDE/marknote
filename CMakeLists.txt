# SPDX-FileCopyrightText: 2021 Mathis Brüchert <mbb-mail@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

# TODO move to release service
set(RELEASE_SERVICE_VERSION_MAJOR "1")
set(RELEASE_SERVICE_VERSION_MINOR "3")
set(RELEASE_SERVICE_VERSION "1.3.0")

project(marknote VERSION ${RELEASE_SERVICE_VERSION})

include(FeatureSummary)

set(QT_MIN_VERSION 6.6)
set(KF_MIN_VERSION 6.8)
set(KPIM_MIME_VERSION "6.1.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(ECMSetupVersion)
include(KDEClangFormat)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(KDEGitCommitHooks)
include(ECMAddAppIcon)
include(ECMAddTests)
include(ECMDeprecationSettings)
include(ECMQmlModule)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX MARKNOTE
    VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/marknote-version.h
)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Gui Qml QuickControls2 Svg)
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS Kirigami I18n Config CoreAddons IconThemes ColorScheme XmlGui)
if (NOT ANDROID)
    find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Widgets)
    find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS Crash)
endif()
if (BUILD_TESTING)
    find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Test)
endif()

find_package(KF6KirigamiAddons 1.7.0 REQUIRED)
set_package_properties(KF6KirigamiAddons PROPERTIES
    TYPE REQUIRED
    PURPOSE "Required application components and runtime dependency"
)
ecm_set_disabled_deprecation_versions(QT 6.9.0
    KF 6.19.0
)

find_package(KPim6Mime ${KPIM_MIME_VERSION} CONFIG REQUIRED)

find_package(md4c)
set_package_properties(md4c PROPERTIES
    TYPE OPTIONAL
    DESCRIPTION "MD4C stands for \"Markdown for C\" and that's exactly what this project is about."
    URL "https://github.com/mity/md4c"
    PURPOSE "Improve output of markdown to html conversion"
)

add_definitions(-DQT_NO_FOREACH -DQT_NO_KEYWORDS)

add_subdirectory(src)

ki18n_install(po)

install(PROGRAMS org.kde.marknote.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.marknote.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES org.kde.marknote.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)

ecm_qt_install_logging_categories(
    EXPORT MARKNOTE
    FILE marknote.categories
    SORT DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES src/*.cpp src/*.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
