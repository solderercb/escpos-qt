TEMPLATE = lib
TARGET = EscPosQt
CONFIG += shared

QT  += core printsupport
equals(QT_MAJOR_VERSION, 6){
  QT += Core5Compat
}


DEFINES += EscPosQt5_EXPORTS

SOURCES += \
  escposprinter.cpp

HEADERS  += \
  escposexports.h \
  escposprinter.h

if(exists(../../3rdparty-common.pri)){
    include(../../3rdparty-common.pri)
} else {
    isEmpty(EXPORT_LIBS) {
        EXPORT_LIBS = $${OUT_PWD}
    }
}

!isEmpty(EXPORT_LIBS) {
    TARGET_DIR = $${PWD}
    EXPORT_LIBS  ~= s,/,\\,g
    TARGET_DIR   ~= s,/,\\,g

    QMAKE_POST_LINK += $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += chcp 65001 >nul 2>&1 $$escape_expand(\\n\\t)
    # Замена загловочных файлов
    for(FILE, HEADERS) {
        FILE  ~= s,/,\\,g
        QMAKE_POST_LINK += $$QMAKE_COPY \"$${TARGET_DIR}\\$${FILE}\" \"$${EXPORT_LIBS}\\include\\$${FILE}\" >nul 2>&1 $$escape_expand(\\n\\t)
    }
}

DESTDIR = $${EXPORT_LIBS}
message($$DESTDIR)

LIBS += -lwinspool -lKernel32
