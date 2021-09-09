#/*****************************************************************************
#* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
#*
#* Author:     Xiao Zhiguo <xiaozhiguo@uniontech.com>
#* Date:       2020-09-11
#*
#* Maintainer: Xiao Zhiguo <xiaozhiguo@uniontech.com>
#*
#* This program is free software: you can redistribute it and/or modify
#* it under the terms of the GNU General Public License as published by
#* the Free Software Foundation, either version 3 of the License, or
#* any later version.
#*
#* This program is distributed in the hope that it will be useful,
#* but WITHOUT ANY WARRANTY; without even the implied warranty of
#* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#* GNU General Public License for more details.
#*
#* You should have received a copy of the GNU General Public License
#* along with this program.  If not, see <http://www.gnu.org/licenses/>.
#*
#*****************************************************************************/

PKG_SEARCH_MODULE(DeepinFileManager REQUIRED dde-file-manager IMPORTED_TARGET)

#libdeepin-font-manager
SET(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libdeepin-font-manager)
FILE(GLOB LIB_SRC_FILES ${LIB_DIR}/*.cpp ${LIB_DIR}/*.h)

FILE(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

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
