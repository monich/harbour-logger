TEMPLATE = subdirs
SUBDIRS = logger nfc ofono
logger.target = logger-target
nfc.depends = logger-target
ofono.depends = logger-target
OTHER_FILES += LICENSE rpm/*.spec
