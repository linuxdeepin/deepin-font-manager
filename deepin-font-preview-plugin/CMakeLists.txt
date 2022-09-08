# Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

PKG_SEARCH_MODULE(DeepinFileManager REQUIRED dde-file-manager IMPORTED_TARGET)

#libdeepin-font-manager
SET(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libdeepin-font-manager)
FILE(GLOB LIB_SRC_FILES ${LIB_DIR}/*.cpp ${LIB_DIR}/*.h)

FILE(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}*.h)

SET(APP_PLUGIN deepin-font-preview-plugin)
ADD_LIBRARY(${APP_PLUGIN} SHARED "")

TARGET_SOURCES(${APP_PLUGIN} PRIVATE ${SRC_FILES} ${LIB_SRC_FILES})
TARGET_INCLUDE_DIRECTORIES(${APP_PLUGIN} PRIVATE ${LIB_DIR})
TARGET_INCLUDE_DIRECTORIES(${APP_PLUGIN} PRIVATE ${DeepinFileManager_INCLUDE_DIRS})
TARGET_INCLUDE_DIRECTORIES(${APP_PLUGIN} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(${APP_PLUGIN} PRIVATE ${FREETYPE_LIBRARIES} PkgConfig::FontConfig
                                            ${DtkWidget_LIBRARIES}  PkgConfig::DeepinFileManager
                                            Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Sql)

SET(PLUGIN_INSTALL_DIR lib/${CMAKE_LIBRARY_ARCHITECTURE}/dde-file-manager/plugins/previews)
INSTALL(TARGETS ${APP_PLUGIN} LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR})
