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

SET(APP_BIN deepin-font-manager)
ADD_EXECUTABLE(${APP_BIN} "")

#编译翻译文件
SET(TRANSLATE_SHELL ${CMAKE_CURRENT_LIST_DIR}/../deepin-font-manager-assets/translate_generation.sh)
SET(TRANSLATION_DIR ${CMAKE_CURRENT_LIST_DIR}/../translations)
EXECUTE_PROCESS(COMMAND ${TRANSLATE_SHELL} WORKING_DIRECTORY ${TRANSLATION_DIR})
#安装翻译文件
FILE(GLOB TRANSLATION_FILES ${CMAKE_CURRENT_LIST_DIR}/../translations/*.qm)
INSTALL(FILES ${TRANSLATION_FILES} DESTINATION share/${APP_BIN}/translations)

#libdeepin-font-manager
SET(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libdeepin-font-manager)
FILE(GLOB LIB_SRC_FILES ${LIB_DIR}/*.cpp ${LIB_DIR}/*.h)
#INSTALL(FILES ${LIB_DIR}/schemas/com.deepin.font-manager.gschema.xml
#        DESTINATION /usr/share/glib-2.0/schemas)

#可执行文件
FILE(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/views/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/views/*.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/*.h)

SET(QRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../deepin-font-manager-assets/deepin-font-manager.qrc)

TARGET_SOURCES(${APP_BIN} PRIVATE ${SRC_FILES} ${QRC_FILES} ${LIB_SRC_FILES})
TARGET_INCLUDE_DIRECTORIES(${APP_BIN} PRIVATE ${LIB_DIR})
TARGET_INCLUDE_DIRECTORIES(${APP_BIN} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_INCLUDE_DIRECTORIES(${APP_BIN} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/views)
TARGET_INCLUDE_DIRECTORIES(${APP_BIN} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/interfaces)

TARGET_LINK_LIBRARIES(${APP_BIN} PUBLIC Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Sql Qt5::Xml Qt5::Svg Qt5::DBus Qt5::Network
                                        ${DtkCore_LIBRARIES} ${DtkWidget_LIBRARIES} ${FREETYPE_LIBRARIES}
                                        ${FREETYPE_LIBRARIES} PkgConfig::FontConfig)

INSTALL(TARGETS ${APP_BIN} RUNTIME DESTINATION bin)

SET(DESKTOP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../deepin-font-manager-assets/deepin-font-manager.desktop)
INSTALL(FILES ${DESKTOP_FILE} DESTINATION share/applications)

SET(ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../deepin-font-manager-assets/deepin-font-manager.svg)
INSTALL(FILES ${ICON_FILE} DESTINATION share/icons/hicolor/scalable/apps)

SET(PREVIEW_CONTENT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../deepin-font-manager-assets/contents.txt)
INSTALL(FILES ${PREVIEW_CONTENT_FILE} DESTINATION share/${APP_BIN}/)
