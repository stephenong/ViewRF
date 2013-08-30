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

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include "spectrumplot.h"
#include "sdrcapture.h"



#ifdef USE_KISSFFT
#include "kiss_fft.h"
#endif

#ifdef USE_AVFFT
extern "C" {
#include "libavcodec/avfft.h"
#include "libavutil/mem.h"
}
#endif

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
    
private slots:

    void doneCapture();
    void on_pushButton_Inc100_clicked();
    void on_pushButton_Dec100_clicked();
    void on_pushButton_Inc10_clicked();
    void on_pushButton_Dec10_clicked();
    void on_pushButton_Inc1_clicked();
    void on_pushButton_Dec1_clicked();
    void on_pushButton_Inc01_clicked();
    void on_pushButton_Dec01_clicked();
    void on_pushButton_GainInc_2_clicked();
    void on_pushButton_GainDec_clicked();
    void on_checkBox_offsettuning_clicked(bool checked);
    void on_pushButton_Capture_clicked();
    void on_pushButton_Exit_clicked();

private:
    Ui::Dialog *ui;

    QTimer *timer;

    SDRCapture *sdrCapture;
    QThread *sdrThread;
    SpectrumPlot *plot;
    uint8_t* sdr_buffer;
    int count;

    float window[FFT_LENGTH];

    double data_result[RESULT_LENGTH];

    int data_counter;
    u_int32_t target_frequency;
    unsigned target_gain;

    void set_target_frequency(u_int32_t frequency);
    void update_target_frequency(int32_t delta);

    void set_target_gain(unsigned gain_index);
    u_int32_t display_locked_frequency;
    bool display_is_locked;

    bool overflow_warning_displayed;

    bool save_next;

    float dc_i_average;
    float dc_q_average;

    float gain_correction;
    float phase_correction;

    unsigned num_gains;
#ifdef USE_KISSFFT
    kiss_fft_cfg mycfg;
    kiss_fft_cpx fin[FFT_LENGTH];
    kiss_fft_cpx fout[FFT_LENGTH];
#endif

#ifdef USE_AVFFT
    FFTContext* fft_context;
    FFTComplex* fft_data;
#endif

signals:
    void startCapture();
    void setRtlFrequency(unsigned freq);
    void setRtlGain(unsigned gain_index);
    void setRtlOffsetTuning(bool on);

};

#endif // DIALOG_H
