TEMPLATE = subdirs

system(echo BUILD_DIR = $$OUT_PWD > $$PWD/../outpwd.tmp)

SUBDIRS += \
  src/escposprinter.pro
