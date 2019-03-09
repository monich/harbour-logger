TEMPLATE = subdirs
SUBDIRS = logger ofono
ofono.depends = logger
OTHER_FILES += LICENSE rpm/*.spec
