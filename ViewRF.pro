#-------------------------------------------------
#
# Project created by QtCreator 2013-06-15T14:58:14
#
#-------------------------------------------------

QT       += core gui


include (/opt/qwt-6.1.0/features/qwt.prf)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ViewRF
    target.files = ViewRF
    target.path = /home/root

INSTALLS += target

TEMPLATE = app



SOURCES += main.cpp\
        dialog.cpp \
    spectrumplot.cpp \
    sdrcapture.cpp \
    kiss_fft_bfly2_neon.S \
    kiss_fft_bfly4_neon.S \
    kiss_fft.c

HEADERS  += dialog.h \
    spectrumplot.h \
    sdrcapture.h \
    kiss_fft.h \
    _kiss_fft_guts.h

FORMS    += dialog.ui

INCLUDEPATH += /opt/rtl-sdr/include
LIBS        += -L/opt/rtl-sdr/lib -lrtlsdr -L/opt/libusb-1.0/lib -lusb-1.0 -L/opt/libav/lib -lavdevice -lavfilter -lavformat -lavresample -lavcodec -lswscale -lavutil

INCLUDEPATH += /opt/libav/include

OTHER_FILES +=

QMAKE_CXXFLAGS += -Wno-psabi
