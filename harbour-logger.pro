TEMPLATE = subdirs
SUBDIRS = logger nfc ofono
logger.target = logger-common
nfc.target = logger-nfc
nfc.depends = logger-common
ofono.target = logger-ofono
ofono.depends = logger-common
OTHER_FILES += LICENSE rpm/*.spec
