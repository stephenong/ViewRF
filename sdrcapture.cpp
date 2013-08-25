/*
ViewRF - RTL-SDR Spectrum Analyzer for the BeagleBone
Copyright (C) 2013 Stephen Ong <http://robotics.ong.id.au>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "sdrcapture.h"
#include <QtCore>

/*****************************************************************************
 * Initializes hardware
 ****************************************************************************/
SDRCapture::SDRCapture(QObject *parent) :
    QObject(parent)
{
    qDebug() << Q_FUNC_INFO << QThread::currentThreadId() << QThread::currentThread();

    char vendor[256], product[256], serial[256];
    dev = NULL;
    out_block_size = 2*BLOCK_LENGTH;
    buffer = (uint8_t *) malloc(out_block_size * sizeof(uint8_t));

    int device_count = rtlsdr_get_device_count();
    qDebug() << device_count;
    for (int i = 0; i < device_count; i++) {
        rtlsdr_get_device_usb_strings(i, vendor, product, serial);
        qDebug() << i << ", " << vendor <<", "<< product << ", " << serial;

    }
    int r = rtlsdr_open(&dev, 0);
    if (r < 0) qDebug() << "Fail rtlsdr_open";
    r = rtlsdr_set_sample_rate(dev, DEFAULT_SAMPLE_RATE);
    if (r < 0) qDebug() << "Fail rtlsdr_set_sample_rate";

    set_frequency(DEFAULT_CENTER_FREQUENCY);


    //Set Manual gain
    r = rtlsdr_set_tuner_gain_mode(dev, 1);
    if (r < 0) qDebug() << "WARNING: Failed to enable manual gain.\n";

    //Use offset tuning
    set_offset_tuning(false);


    //rtlsdr_set_testmode(dev, 1);  //Test mode for detecting lost samples

    /* Reset endpoint before we start reading from it (mandatory) */
    r = rtlsdr_reset_buffer(dev);
    if (r < 0) qDebug() << "WARNING: Failed to reset buffers.\n";

    is_locked = FALSE;

    num_gains = rtlsdr_get_tuner_gains(dev, gain_table);
}

/*****************************************************************************
 * Is the tuner Elonics E4000.
 ****************************************************************************/
bool SDRCapture::isE4000()
{
    if(rtlsdr_get_tuner_type(dev)==RTLSDR_TUNER_E4000) return TRUE;
    return FALSE;

}

/*****************************************************************************
 * How many gain settings are available for this tuner
 ****************************************************************************/
unsigned SDRCapture::get_num_gains()
{
    return (unsigned) num_gains;
}

/*****************************************************************************
 * Get IQ data buffer
 ****************************************************************************/
uint8_t *SDRCapture::getBuffer()
{
    return buffer;
}

/*****************************************************************************
 * Event handler for gathering IQ block
 ****************************************************************************/
void SDRCapture::threadFunction()
{
    int n_read;
    int r;

    r = rtlsdr_reset_buffer(dev);
    if (r < 0) qDebug() << "rtlsdr_reset_buffer1 fail\n";

    r = rtlsdr_read_sync(dev, buffer, out_block_size, &n_read);
    if (r < 0) qDebug() << "rtlsdr_read_sync fail\n";

    emit packetCaptured();
}

/*****************************************************************************
 * Sets frequency
 ****************************************************************************/
void SDRCapture::set_frequency(unsigned freq)
{
    int r = rtlsdr_set_center_freq(dev, freq);
    if (r < 0) {
        qDebug() << "Fail rtlsdr_set_center_freq";
        is_locked = FALSE;
    }
    else{
        locked_frequency = freq;
        is_locked = TRUE;
    }
}

/*****************************************************************************
 * Set LNA gain
 ****************************************************************************/
void SDRCapture::set_gain(unsigned gain_index)
{
    if(gain_index >= num_gains){
        gain_index = num_gains - 1;
    }

    int r = rtlsdr_set_tuner_gain(dev, gain_table[gain_index]);
    if (r < 0) qDebug() << "Fail rtlsdr_set_tuner_gain";
}

/*****************************************************************************
 * Enable offset tuning for E4000
 ****************************************************************************/
void SDRCapture::set_offset_tuning(bool on)
{
    int r;
    if(on){
        r = rtlsdr_set_offset_tuning(dev, 1);
    }
    else{
        r = rtlsdr_set_offset_tuning(dev, 0);
    }
    if (r < 0) qDebug() << "WARNING: Failed to use offset tuning.\n";
}

// End of sdrcapture.cpp
