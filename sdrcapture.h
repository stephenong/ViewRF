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

#ifndef SDRCAPTURE_H
#define SDRCAPTURE_H

#include <QObject>
#include "rtl-sdr.h"

//#define USE_KISSFFT
#define USE_AVFFT                               //AVFFT is a touch faster

#define HAMMING_WINDOW
//#define BLACKMANNUTALL_WINDOW

#define DEFAULT_CENTER_FREQUENCY 952000000      // In Hz
#define DEFAULT_SAMPLE_RATE      2400000        // 2.4 MS/s gives good result

//----------------------------
//BLOCK_LENGTH is divided into overlapping FFT_LENGTH blocks
//number of edge-to-edge blocks n = BLOCK_LENGTH/FFT_LENGTH
//number of 50% overlapping blocks is n + (n-1)

#define FFT_LENGTH_2N   (10)                    //2^10 = 1024
#define FFT_LENGTH      (1<<FFT_LENGTH_2N)      //2^(FFT_LENGTH_2N)
#define FFT_SLIDE       (FFT_LENGTH/2)          //50% overlap

#define BLOCK_LENGTH    (20*FFT_LENGTH)
#define FFT_NUM_BLOCKS  ((BLOCK_LENGTH/FFT_LENGTH) + (BLOCK_LENGTH/FFT_LENGTH) - 1)

#define RESULT_LENGTH   (FFT_LENGTH)

//--------------------------------
class SDRCapture : public QObject
{
    Q_OBJECT
public:
    explicit SDRCapture(QObject *parent = 0);
    bool isE4000();
    unsigned get_num_gains();
    uint8_t* getBuffer();
    u_int32_t locked_frequency;
    bool is_locked;
    
private:
    rtlsdr_dev_t *dev;
    int out_block_size;
    uint8_t *buffer;
    int gain_table[50];
    unsigned num_gains;

signals:
    void packetCaptured();

public slots:
    void threadFunction();
    void set_frequency(unsigned freq);
    void set_gain(unsigned gain_index);
    void set_offset_tuning(bool on);
};

#endif // SDRCAPTURE_H
