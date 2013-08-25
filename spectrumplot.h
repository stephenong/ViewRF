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

#ifndef SPECTRUMPLOT_H
#define SPECTRUMPLOT_H

#include <stdint.h>

#include <qwt_plot.h>
#include <qwt_plot_directpainter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include <qwt_curve_fitter.h>
#include <qwt_painter.h>

#include "sdrcapture.h"

class SpectrumPlot : public QwtPlot
{
    Q_OBJECT
public:
    explicit SpectrumPlot(QWidget *parent = 0);
    void SetData(double data[]);
    void SetXRange(double xStart, double xStop);
signals:
    
public slots:

private:
    QwtPlotDirectPainter *d_directPainter;
    QwtPlotCurve *d_curve;
    double d_x[RESULT_LENGTH];
};

#endif // SPECTRUMPLOT_H
