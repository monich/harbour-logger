TARGET = harbour-logger-ofono
CONFIG += sailfishapp link_pkgconfig
PKGCONFIG += sailfishapp mlite5 gio-2.0 gio-unix-2.0 glib-2.0
#QT += dbus

WARNINGS = -Wall -Wno-unused-parameter -Wno-deprecated-declarations
QMAKE_CXXFLAGS += $$WARNINGS -Wno-psabi
QMAKE_CFLAGS += $$WARNINGS

CONFIG(debug, debug|release) {
  QMAKE_CXXFLAGS_DEBUG *= -O0
  QMAKE_CFLAGS_DEBUG *= -O0
}

CONFIG(debug, debug|release) {
  DEFINES += HARBOUR_DEBUG=1
}

# Directories
HARBOUR_LIB_DIR = $$_PRO_FILE_PWD_/../harbour-lib
LOGGER_LIB_DIR = $$_PRO_FILE_PWD_/../logger

# Libraries
HARBOUR_LIB = $$OUT_PWD/../harbour-lib/libharbour-lib.a
LOGGER_LIB = $$OUT_PWD/../logger/liblogger.a

PRE_TARGETDEPS += \
  $$LOGGER_LIB \
  $$HARBOUR_LIB

LIBS += \
  $$LOGGER_LIB \
  $$HARBOUR_LIB

OTHER_FILES += \
  icons/harbour-logger-ofono.svg \
  harbour-logger-ofono.desktop \
  qml/*.qml \
  translations/*.ts

TARGET_DATA_DIR = /usr/share/harbour-logger-ofono
TARGET_ICON_ROOT = /usr/share/icons/hicolor

qml_pages.files = $${LOGGER_LIB_DIR}/qml/*
qml_pages.path = $${TARGET_DATA_DIR}/qml
INSTALLS += qml_pages

icon86.files = icons/86x86/$${TARGET}.png
icon86.path = $${TARGET_ICON_ROOT}/86x86/apps
INSTALLS += icon86

icon108.files = icons/108x108/$${TARGET}.png
icon108.path = $${TARGET_ICON_ROOT}/108x108/apps
INSTALLS += icon108

icon128.files = icons/128x128/$${TARGET}.png
icon128.path = $${TARGET_ICON_ROOT}/128x128/apps
INSTALLS += icon128

icon256.files = icons/256x256/$${TARGET}.png
icon256.path = $${TARGET_ICON_ROOT}/256x256/apps
INSTALLS += icon256

INCLUDEPATH += \
  src \
  $$LOGGER_LIB_DIR/include

SOURCES += \
  src/main.cpp

TRANSLATIONS += \
  translations/harbour-logger-ofono.ts \
  translations/harbour-logger-ofono-ru.ts

# Translations
TS_FILE = \"$${_PRO_FILE_PWD_}/translations/$${TARGET}.ts\"
HAVE_TRANSLATIONS = 0

# Translation source directories
TRANSLATION_SOURCE_CANDIDATES = $${_PRO_FILE_PWD_}/src $${_PRO_FILE_PWD_}/qml $${LOGGER_LIB_DIR}/qml
for(dir, TRANSLATION_SOURCE_CANDIDATES) {
    exists($$dir) {
        TRANSLATION_SOURCES += \"$$dir\"
    }
}

# prefix all TRANSLATIONS with the src dir
# the qm files are generated from the ts files copied to out dir
for(t, TRANSLATIONS) {
    TRANSLATIONS_IN  += \"$${_PRO_FILE_PWD_}/$$t\"
    TRANSLATIONS_OUT += \"$${OUT_PWD}/$$t\"
    HAVE_TRANSLATIONS = 1
}

qm.files = $$replace(TRANSLATIONS_OUT, \.ts, .qm)
qm.path = /usr/share/$${TARGET}/translations
qm.CONFIG += no_check_exist

# update the ts files in the src dir and then copy them to the out dir
TRANSLATE_OBSOLETE = -noobsolete
qm.commands += lupdate $${TRANSLATE_OBSOLETE} $${TRANSLATION_SOURCES} -ts $${TS_FILE} $$TRANSLATIONS_IN && \
    mkdir -p translations && \
    [ \"$${OUT_PWD}\" != \"$${_PRO_FILE_PWD_}\" -a $$HAVE_TRANSLATIONS -eq 1 ] && \
    cp -af $${TRANSLATIONS_IN} \"$${OUT_PWD}/translations\" || :

TRANSLATE_UNFINISHED = -nounfinished
TRANSLATE_IDBASED = -idbased

# create the qm files
qm.commands +=  [ $$HAVE_TRANSLATIONS -eq 1 ] && lrelease $${TRANSLATE_IDBASED} $${TRANSLATE_UNFINISHED} $${TRANSLATIONS_OUT} || :

INSTALLS += qm
