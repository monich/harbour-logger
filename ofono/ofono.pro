openrepos {
    PREFIX = openrepos
    DEFINES += OPENREPOS
} else {
    PREFIX = harbour
}

NAME = logger-ofono
TARGET = $${PREFIX}-$${NAME}

app_settings {
    # This path is hardcoded in jolla-settings
    TRANSLATIONS_PATH = /usr/share/translations
} else {
    TRANSLATIONS_PATH = /usr/share/$${TARGET}/translations
}

CONFIG += sailfishapp link_pkgconfig
PKGCONFIG += sailfishapp mlite5 gio-2.0 gio-unix-2.0 glib-2.0
QT += dbus

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
QOFONOEXT_LIB_DIR = $$_PRO_FILE_PWD_/src/libqofonoext
QCONNMAN_LIB_DIR = $$_PRO_FILE_PWD_/../libconnman-qt

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
  icons/harbour-$${NAME}.svg \
  harbour-$${NAME}.desktop \
  qml/*.qml \
  privileges/* \
  translations/*.ts

TARGET_DATA_DIR = /usr/share/$${TARGET}
TARGET_ICON_ROOT = /usr/share/icons/hicolor

qml_pages.files = $${LOGGER_LIB_DIR}/qml/*
qml_pages.path = $${TARGET_DATA_DIR}/qml
INSTALLS += qml_pages

INCLUDEPATH += \
  src \
  $${LOGGER_LIB_DIR}/include \
  $${HARBOUR_LIB_DIR}/include \
  $${QOFONOEXT_LIB_DIR}/src \
  $${QCONNMAN_LIB_DIR}/libconnman-qt

HEADERS += \
  $${QOFONOEXT_LIB_DIR}/src/qofonoextmodemmanager.h \
  $${QCONNMAN_LIB_DIR}/libconnman-qt/networktechnology.h

SOURCES += \
  src/main.cpp \
  $${QOFONOEXT_LIB_DIR}/src/qofonoext.cpp \
  $${QOFONOEXT_LIB_DIR}/src/qofonoextmodemmanager.cpp \
  $${QCONNMAN_LIB_DIR}/libconnman-qt/networktechnology.cpp

# D-Bus interfaces
DBUS_INTERFACES += net_connman_technology
net_connman_technology.files = $${QCONNMAN_LIB_DIR}/libconnman-qt/connman_technology.xml

OTHER_FILES += \
    $${net_connman_technology.files}

# Settings
app_settings {
    settings_json.files = $${LOGGER_LIB_DIR}/settings/$${TARGET}.json
    settings_json.path = /usr/share/jolla-settings/entries/
    settings_json.extra = sed s/harbour-logger/$${TARGET}/g $${LOGGER_LIB_DIR}/settings/harbour-logger.json > $$eval(settings_json.files)
    settings_json.CONFIG += no_check_exist
    settings_qml.files = $${LOGGER_LIB_DIR}/settings/settings.qml
    settings_qml.path = /usr/share/$${TARGET}/settings/
    settings_qml.extra = sed -i s/harbour-logger-conf/$${TARGET}/g $$eval(settings_qml.files)
    INSTALLS += settings_qml settings_json
}

# Priveleges
privileges.files = privileges/$${PREFIX}-logger-ofono
privileges.path = /usr/share/mapplauncherd/privileges.d/
INSTALLS += privileges

# Desktop file
openrepos {
    desktop.extra = sed s/harbour/openrepos/g harbour-$${NAME}.desktop > $${TARGET}.desktop
    desktop.CONFIG += no_check_exist
}

# Icons
ICON_SIZES = 86 108 128 256
for(s, ICON_SIZES) {
    icon_target = icon$${s}
    icon_dir = icons/$${s}x$${s}
    $${icon_target}.files = $${icon_dir}/$${TARGET}.png
    $${icon_target}.path = /usr/share/icons/hicolor/$${s}x$${s}/apps
    openrepos {
        $${icon_target}.extra = cp $${icon_dir}/harbour-$${NAME}.png $$eval($${icon_target}.files)
        $${icon_target}.CONFIG += no_check_exist
    }
    INSTALLS += $${icon_target}
}

# Translations
TRANSLATION_SOURCES = \
  $${_PRO_FILE_PWD_}/qml \
  $${LOGGER_LIB_DIR}/qml \
  $${LOGGER_LIB_DIR}/settings

TRANSLATION_FILES = \
  $${NAME} \
  $${NAME}-nl \
  $${NAME}-pl \
  $${NAME}-ru \
  $${NAME}-sv

for(t, TRANSLATION_FILES) {
    suffix = $$replace(t,-,_)
    in = $${_PRO_FILE_PWD_}/translations/harbour-$${t}
    out = $${OUT_PWD}/translations/$${PREFIX}-$${t}

    lupdate_target = lupdate_$$suffix
    lrelease_target = lrelease_$$suffix

    $${lupdate_target}.commands = lupdate -noobsolete -extensions qml $${TRANSLATION_SOURCES} -ts \"$${in}.ts\" && \
        mkdir -p \"$${OUT_PWD}/translations\" &&  [ \"$${in}.ts\" != \"$${out}.ts\" ] && \
        cp -af \"$${in}.ts\" \"$${out}.ts\" || :

    $${lrelease_target}.target = $${out}.qm
    $${lrelease_target}.depends = $${lupdate_target}
    $${lrelease_target}.commands = lrelease -idbased \"$${out}.ts\"

    QMAKE_EXTRA_TARGETS += $${lrelease_target} $${lupdate_target}
    PRE_TARGETDEPS += \"$${out}.qm\"
    qm.files += \"$${out}.qm\"
}

qm.path = $$TRANSLATIONS_PATH
qm.CONFIG += no_check_exist
INSTALLS += qm
