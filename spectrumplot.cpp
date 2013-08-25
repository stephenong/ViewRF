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

#include "spectrumplot.h"

#include <qevent.h>

SpectrumPlot::SpectrumPlot(QWidget *parent) :
    QwtPlot(parent)
{
    d_directPainter = new QwtPlotDirectPainter();

    setAutoReplot( false );
    setCanvas( new QwtPlotCanvas() );

    plotLayout()->setAlignCanvasToScales( true );

    setTitle("Spectrum Analyzer - Stephen Ong");
    setAxisTitle( QwtPlot::xBottom, "Frequency (MHz)" );
    setAxisTitle( QwtPlot::yLeft, "Amplitude (dB)" );
    setAxisScale( QwtPlot::xBottom, 0, RESULT_LENGTH );
    setAxisScale( QwtPlot::yLeft, -90, -10);

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setPen( Qt::gray, 0.0, Qt::DotLine );
    grid->enableX( true );
    grid->enableXMin( true );
    grid->enableY( true );
    grid->enableYMin( false );
    grid->attach( this );


    d_curve = new QwtPlotCurve();
    d_curve->setStyle( QwtPlotCurve::Lines );
    d_curve->setPen(Qt::darkGray);
    //d_curve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
    d_curve->setPaintAttribute( QwtPlotCurve::ClipPolygons, false );
    d_curve->attach( this );

    for(int i=0; i<RESULT_LENGTH; ++i) d_x[i] = i;

}

void SpectrumPlot::SetData(double data[])
{
    // Do Frequency wrap here
    // Original FFT result
    // Bin (0) ..... Bin (N/2-1),   Bin (N/2), ....., Bin(N-1)
    // 0, ...........(Fs/2)-(Fs/N), Fs/2, .........., Fs-(Fs/N)
    // 0, ...........(Fs/2)-(Fs/N), -Fs/2,.........., -Fs/N
    // After this wrap
    // Bin (N/2), ....., Bin(N-1), Bin (0) ..... Bin (N/2-1)
    // -Fs/2,     ...., Fs-(Fs/N), 0       ..... (Fs/2)-(Fs/N)

    double temp;
    for (int i = 0; i <  RESULT_LENGTH/2; i++) {
        temp = data[RESULT_LENGTH/2 + i];
         data[RESULT_LENGTH/2 + i] = data[i];
         data[i] = temp;
    }
    d_curve->setSamples(d_x, data, RESULT_LENGTH);
    replot();

}

void SpectrumPlot::SetXRange(double xStart, double xStop)
{
    setAxisScale( QwtPlot::xBottom, xStart, xStop );
    int num_points = RESULT_LENGTH;
    for(int i=0; i<num_points; ++i) {
        d_x[i] = xStart + (i/(num_points-1.0))* (xStop-xStart);
    }
}

