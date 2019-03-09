TEMPLATE = subdirs
SUBDIRS = logger ofono
logger.target = logger-target
ofono.depends = logger-target
OTHER_FILES += LICENSE rpm/*.spec
