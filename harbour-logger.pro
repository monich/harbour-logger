TEMPLATE = subdirs
SUBDIRS = harbour-lib logger ofono
ofono.depends = harbour-lib logger
OTHER_FILES += rpm/*.spec
