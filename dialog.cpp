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

#include "dialog.h"
#include "ui_dialog.h"
#include "spectrumplot.h"

#include <QtCore>
#include <QtGui>
#include <qwt_plot.h>
#include "sdrcapture.h"

#include "math.h"


#include <stdio.h>

/***************************************************************************
 * Constructor
 * Initialize UI elements
 * Initialize fft engine
 * Connects signals to slots
 ***************************************************************************/
 Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{

    count = 0;
    display_locked_frequency = 0;

    overflow_warning_displayed = false;
    save_next = false;

    plot = new SpectrumPlot(this);
    plot->setGeometry(10, 10, 780, 330);

    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(0);
    ui->label_gainhigh->setVisible(false);
    ui->label_targetfrequency->setVisible(false);

    ui->pushButton_Capture->setVisible(false);      // hide capture pushbutton
    ui->label_counter->setVisible(false);           // hide counter text

#ifdef USE_KISSFFT
    mycfg = kiss_fft_alloc(FFT_LENGTH, 0, NULL, NULL);
#endif

#ifdef USE_AVFFT
    fft_context = av_fft_init(FFT_LENGTH_2N,0);
    fft_data = (FFTComplex*) av_malloc(FFT_LENGTH * sizeof(FFTComplex));
#endif

#ifdef BLACKMANNUTALL_WINDOW
    float a0 = 0.3635819;
    float a1 = 0.4891775;
    float a2 = 0.1365995;
    float a3 = 0.0106411;

    for(int i=0; i<FFT_LENGTH; ++i){
        window[i] = a0 - a1 * cos(2.*M_PI*i/(FFT_LENGTH-1)) + a2 * cos(4.*M_PI*i/(FFT_LENGTH-1)) - a3 * cos(6.*M_PI*i/(FFT_LENGTH-1));
    }
#endif

#ifdef HAMMING_WINDOW
    float alpha = 0.54;
    float beta = 0.46;

    for(int i=0; i<FFT_LENGTH; ++i){
        window[i] = alpha - beta * cos(2*M_PI*i/(FFT_LENGTH-1));
    }
#endif

    dc_i_average = 127;
    dc_q_average = 127;

    gain_correction = 0;
    phase_correction = 0;

    sdrCapture = new SDRCapture();
    if(sdrCapture->isE4000()){
        ui->checkBox_offsettuning->setEnabled(true);
        ui->checkBox_offsettuning->setVisible(true);
    }
    else{
        ui->checkBox_offsettuning->setEnabled(false);
        ui->checkBox_offsettuning->setVisible(false);
    }
    num_gains = sdrCapture->get_num_gains();
    ui->horizontalSlider->setMaximum(num_gains-1);
    sdrThread = new QThread();
    sdrCapture->moveToThread(sdrThread);

    // Connects slots
    sdrCapture->connect(this, SIGNAL(startCapture()), SLOT(threadFunction()));
    this->connect(sdrCapture, SIGNAL(packetCaptured()), SLOT(doneCapture()));
    sdrCapture->connect(this, SIGNAL(setRtlFrequency(unsigned)), SLOT(set_frequency(unsigned)));
    sdrCapture->connect(this, SIGNAL(setRtlGain(unsigned)), SLOT(set_gain(unsigned)));
    sdrCapture->connect(this, SIGNAL(setRtlOffsetTuning(bool)), SLOT(set_offset_tuning(bool)));

    sdr_buffer = sdrCapture->getBuffer();
    sdrThread->start();
    set_target_frequency(DEFAULT_CENTER_FREQUENCY);
    set_target_gain(1);
    emit startCapture();
    if(sdrCapture->isE4000()){
        ui->checkBox_offsettuning->click();
    }
    set_target_gain(num_gains/2);
}

/*****************************************************************************
 * Destructor
 ****************************************************************************/
Dialog::~Dialog()
{
    delete ui;
}

/*****************************************************************************
 * Function is called after a block of IQ data has been captured.
 * Calculates PSD
 * retriggers the collection of another block of IQ data
 ****************************************************************************/
void Dialog::doneCapture()
{
    count++;                        //Counter for working out FFT update rate
    QString text;
    text.sprintf("%d", count);
    ui->label_counter->setText(text);

    uint8_t re,im;
    float ref, imf;
    float ptr, pti;
    bool overflow = false;

    int i,j;
    for (i = 0; i < FFT_LENGTH; i++) {
        data_result[i] = 0;                 //Initialize PSD output
        re = sdr_buffer[2*i];               //Check for overflow
        im = sdr_buffer[2*i+1];
        if(re<5 || re>250 || im<5 || im>250) overflow = true;
    }

    //calc block DC average
    ptr = pti = 0;
    for (i=0; i<BLOCK_LENGTH; ++i){
        ptr += sdr_buffer[2*i];
        pti += sdr_buffer[2*i+1];
    }
    dc_i_average += 0.1f*((ptr/BLOCK_LENGTH)-dc_i_average);
    dc_q_average += 0.1f*((pti/BLOCK_LENGTH)-dc_q_average);

    //Calculate PSD
    int slide_length = 0;
    for(j=0; j< FFT_NUM_BLOCKS; ++j){
        for (i = 0; i < FFT_LENGTH; i++) {
            ref = sdr_buffer[slide_length+2*i] - dc_i_average;
            imf = sdr_buffer[slide_length+2*i+1] - dc_q_average;

            ptr = (ref) * (1.0f + gain_correction)* window[i];
            pti = (imf + ref*phase_correction) * window[i];

#ifdef USE_AVFFT
            fft_data[i].re = ptr;
            fft_data[i].im = pti;
#endif
#ifdef USE_KISSFFT
            fin[i].r = ptr;
            fin[i].i = pti;
#endif
        }

#ifdef USE_AVFFT
        av_fft_permute(fft_context, fft_data);
        av_fft_calc(fft_context, fft_data);
#endif
#ifdef USE_KISSFFT
        kiss_fft(mycfg, fin, fout);
#endif

        for (i = 0; i < FFT_LENGTH; i++) {
#ifdef USE_AVFFT
            data_result[i] += fft_data[i].re*fft_data[i].re+fft_data[i].im*fft_data[i].im;
#endif
#ifdef USE_KISSFFT
            data_result[i] += fout[i].r*fout[i].r+fout[i].i*fout[i].i;
#endif
        }
        slide_length += FFT_SLIDE;
    }

    //Convert to dB
    for (i = 0; i < FFT_LENGTH; i++) {
        data_result[i] = 10. * log10(data_result[i]/FFT_NUM_BLOCKS)-100;
    }


    //Display overflow - gain too high warning
    if(overflow != overflow_warning_displayed){
        if(overflow){
            ui->label_gainhigh->setVisible(true);
        }
        else{
            ui->label_gainhigh->setVisible(false);
        }
        overflow_warning_displayed = overflow;
    }

    //Update graph display range and title if frequency changes
    QString title;
    if(sdrCapture->is_locked){
         if(display_locked_frequency!= sdrCapture->locked_frequency){
             double freq_MHz = sdrCapture->locked_frequency/1000000.0;
             title.sprintf("Center %4.1f MHz", freq_MHz);
             plot->setTitle(title);
             plot->SetXRange(freq_MHz-1.2, freq_MHz+1.2);
             display_locked_frequency = sdrCapture->locked_frequency;
         }
    }
    else{
        title.sprintf("Frequency Not Locked [%4.1f MHz]", target_frequency/1000000.);
        plot->setTitle(title);
    }

    plot->SetData(data_result);

    //Save IQ and FFT block if requested
    if(save_next){
        FILE* fp = fopen("sample_dump.txt", "w");
        for(int i=0; i<BLOCK_LENGTH; ++i){
            fprintf(fp, "%d %d\n", sdr_buffer[2*i], sdr_buffer[2*i+1]);
        }
        fclose(fp);

        fp = fopen("fft_dump.txt", "w");
        for(int i=0; i<FFT_LENGTH; ++i){
#ifdef USE_KISSFFT
            ptr = fout[i].r;
            pti = fout[i].i;
#endif
#ifdef USE_AVFFT
            ptr = fft_data[i].re;
            pti = fft_data[i].im;
#endif
            fprintf(fp, "%f + %fi\n", ptr, pti);

        }
        fclose(fp);
        save_next = false;
    }

    //Change frequency if +0.1MHz or -0.1MHz button is pressed
    if(ui->pushButton_Inc01->isDown()) update_target_frequency(100000);
    if(ui->pushButton_Dec01->isDown()) update_target_frequency(-100000);

    emit startCapture();
}


/*****************************************************************************
 * Set target frequency
 ****************************************************************************/
void Dialog::set_target_frequency(u_int32_t frequency)
{
    target_frequency = frequency;
    QString text;
    text.sprintf("Target: %4.1f MHz", target_frequency/1000000.);
    ui->label_targetfrequency->setText(text);
    emit setRtlFrequency(frequency);
}

/*****************************************************************************
 * Change target frequency by the amount delta
 ****************************************************************************/
void Dialog::update_target_frequency(int32_t delta)
{
    u_int32_t new_frequency = target_frequency + delta;
    if((20000000u<new_frequency) && (new_frequency<2210000000u)){
        set_target_frequency(new_frequency);
    }
}

/*****************************************************************************
 * Sets LNA gain
 ****************************************************************************/
void Dialog::set_target_gain(unsigned gain_index)
{
    target_gain = gain_index;
    if(target_gain>num_gains-1) target_gain = num_gains-1;
    ui->horizontalSlider->setValue(target_gain);
    emit setRtlGain(target_gain);
}


/*****************************************************************************
 * Event handlers for changing frequency.
 ****************************************************************************/
void Dialog::on_pushButton_Inc100_clicked()
{
    update_target_frequency(100000000);
}

void Dialog::on_pushButton_Dec100_clicked()
{
    update_target_frequency(-100000000);
}

void Dialog::on_pushButton_Inc10_clicked()
{
    update_target_frequency(10000000);
}

void Dialog::on_pushButton_Dec10_clicked()
{
    update_target_frequency(-10000000);
}

void Dialog::on_pushButton_Inc1_clicked()
{
    update_target_frequency(1000000);
}

void Dialog::on_pushButton_Dec1_clicked()
{
    update_target_frequency(-1000000);
}

void Dialog::on_pushButton_Inc01_clicked()
{
    update_target_frequency(100000);
}

void Dialog::on_pushButton_Dec01_clicked()
{
    update_target_frequency(-100000);
}

/*****************************************************************************
 * Event handlers for increasing or decreasing gain
 ****************************************************************************/
void Dialog::on_pushButton_GainInc_2_clicked()
{
    set_target_gain(target_gain+1);
}

void Dialog::on_pushButton_GainDec_clicked()
{
    if(target_gain>0) set_target_gain(target_gain-1);
}

/*****************************************************************************
 * Event handler for enable/disable offset tuning for the E4000 tuner
 ****************************************************************************/
void Dialog::on_checkBox_offsettuning_clicked(bool checked)
{
    emit setRtlOffsetTuning(checked);
}

/*****************************************************************************
 * Event handler for capture button, triggers the saving of next IQ block to
 * file.
 ****************************************************************************/
void Dialog::on_pushButton_Capture_clicked()
{
    save_next = true;
}

/*****************************************************************************
 * Exit
 ****************************************************************************/
void Dialog::on_pushButton_Exit_clicked()
{
    this->close();
}

// End of dialog.cpp
