TEMPLATE = lib
CONFIG += static
TARGET = logger
QT += dbus gui
CONFIG += sailfishapp link_pkgconfig
PKGCONFIG += mlite5 gio-2.0 gio-unix-2.0 glib-2.0 libglibutil

WARNINGS = -Wall -Wno-unused-parameter -Wno-deprecated-declarations
EXTRA_CFLAGS = $$WARNINGS -fvisibility=hidden
QMAKE_CXXFLAGS += $$EXTRA_CFLAGS
QMAKE_CFLAGS += $$EXTRA_CFLAGS

HARBOUR_LIB_DIR = $$_PRO_FILE_PWD_/../harbour-lib
HARBOUR_LIB_SRC = $${HARBOUR_LIB_DIR}/src
HARBOUR_LIB_INCLUDE = $${HARBOUR_LIB_DIR}/include

LIBDBUSLOG = src/libdbuslog
LIBDBUSLOG_COMMON = $${LIBDBUSLOG}/common
LIBDBUSLOG_COMMON_SRC = $${LIBDBUSLOG_COMMON}/src
LIBDBUSLOG_COMMON_INCLUDE = $${LIBDBUSLOG_COMMON}/include

LIBDBUSLOG_CLIENT = $${LIBDBUSLOG}/client
LIBDBUSLOG_CLIENT_SRC = $${LIBDBUSLOG_CLIENT}/src
LIBDBUSLOG_CLIENT_INCLUDE = $${LIBDBUSLOG_CLIENT}/include

CONFIG(debug, debug|release) {
    QMAKE_CXXFLAGS_DEBUG *= -O0
    QMAKE_CFLAGS_DEBUG *= -O0
    DEFINES += DEBUG HARBOUR_DEBUG
} else {
    EXTRA_RELEASE_CFLAGS = -fPIC -flto -ffat-lto-objects
    QMAKE_CXXFLAGS += $$EXTRA_RELEASE_CFLAGS
    QMAKE_CFLAGS += $$EXTRA_RELEASE_CFLAGS
}

openrepos {
    DEFINES += OPENREPOS
}

SOURCES += \
    src/LoggerBuffer.cpp \
    src/LoggerCategory.cpp \
    src/LoggerCategoryModel.cpp \
    src/LoggerCategoryFilterModel.cpp \
    src/LoggerEntry.cpp \
    src/LoggerHints.cpp \
    src/LoggerLogModel.cpp \
    src/LoggerLogSaver.cpp \
    src/LoggerMain.cpp \
    src/LoggerSettings.cpp

HEADERS += \
    include/LoggerMain.h \
    src/LoggerBuffer.h \
    src/LoggerCategory.h \
    src/LoggerCategoryModel.h \
    src/LoggerCategoryFilterModel.h \
    src/LoggerEntry.h \
    src/LoggerHints.h \
    src/LoggerLogModel.h \
    src/LoggerLogSaver.h \
    src/LoggerSettings.h

SOURCES += \
  $${HARBOUR_LIB_SRC}/HarbourProcessState.cpp \
  $${HARBOUR_LIB_SRC}/HarbourSigChildHandler.cpp \
  $${HARBOUR_LIB_SRC}/HarbourSystemInfo.cpp \
  $${HARBOUR_LIB_SRC}/HarbourTransferMethodInfo.cpp \
  $${HARBOUR_LIB_SRC}/HarbourTransferMethodsModel.cpp

HEADERS += \
  $${HARBOUR_LIB_INCLUDE}/HarbourProcessState.h \
  $${HARBOUR_LIB_INCLUDE}/HarbourSigChildHandler.h \
  $${HARBOUR_LIB_INCLUDE}/HarbourSystemInfo.h \
  $${HARBOUR_LIB_INCLUDE}/HarbourTransferMethodInfo.h \
  $${HARBOUR_LIB_INCLUDE}/HarbourTransferMethodsModel.h

SOURCES += \
    $${LIBDBUSLOG_COMMON_SRC}/dbuslog_category.c \
    $${LIBDBUSLOG_COMMON_SRC}/dbuslog_message.c

SOURCES += \
    $${LIBDBUSLOG_CLIENT_SRC}/dbuslog_client.c \
    $${LIBDBUSLOG_CLIENT_SRC}/dbuslog_receiver.c

INCLUDEPATH += \
    include \
    $${HARBOUR_LIB_INCLUDE} \
    $${LIBDBUSLOG_COMMON_INCLUDE} \
    $${LIBDBUSLOG_CLIENT_INCLUDE}

OTHER_FILES += \
  qml/pages/*.js \
  qml/pages/*.qml \
  qml/pages/images/*.svg \
  settings/*.qml \
  settings/*.json

DBUS_SPEC_DIR = $$_PRO_FILE_PWD_/src/libdbuslog/spec

OTHER_FILES += \
  $${DBUS_SPEC_DIR}/org.nemomobile.Logger.xml

# org.nemomobile.Logger
DBUSLOGGER_XML = $${DBUS_SPEC_DIR}/org.nemomobile.Logger.xml
DBUSLOGGER_GENERATE = gdbus-codegen --generate-c-code \
  org.nemomobile.Logger $${DBUSLOGGER_XML}
DBUSLOGGER_H = org.nemomobile.Logger.h
org_nemomobile_Logger_h.input = DBUSLOGGER_XML
org_nemomobile_Logger_h.output = $${DBUSLOGGER_H}
org_nemomobile_Logger_h.commands = $${DBUSLOGGER_GENERATE}
org_nemomobile_Logger_h.CONFIG = no_link
QMAKE_EXTRA_COMPILERS += org_nemomobile_Logger_h

DBUSLOGGER_C = org.nemomobile.Logger.c
org_nemomobile_Logger_c.input = DBUSLOGGER_XML
org_nemomobile_Logger_c.output = $${DBUSLOGGER_C}
org_nemomobile_Logger_c.commands = $${DBUSLOGGER_GENERATE}
org_nemomobile_Logger_c.CONFIG = no_link
QMAKE_EXTRA_COMPILERS += org_nemomobile_Logger_c
GENERATED_SOURCES += $${DBUSLOGGER_C}
