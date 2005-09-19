/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2004-11-17
 * Description :
 *
 * Copyright 2004-2005 by Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

// C++ include.

#include <cstring>
#include <cmath>
#include <cstdlib>

// Qt Includes.
 
#include <qlayout.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qgroupbox.h>
#include <qhbuttongroup.h> 
#include <qpushbutton.h>
#include <qtooltip.h>

// KDE includes.

#include <kstandarddirs.h>
#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>

// Local includes.

#include "imagehistogram.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"
#include "albumsettings.h"
#include "thumbnailjob.h"
#include "imagepropertieshistogram.h"

ImagePropertiesHistogram::ImagePropertiesHistogram(QWidget* page,
                                                   QRect* selectionArea)
{
    m_selectionArea = selectionArea;
   
    QVBoxLayout *topLayout = new QVBoxLayout( page, 0, 5);
    QHBoxLayout *hlay = new QHBoxLayout(topLayout);
    
    m_labelThumb = new QLabel( page );
    m_labelThumb->setFixedHeight( 48 );
    hlay->addWidget(m_labelThumb);
    
    QGridLayout *grid = new QGridLayout(hlay, 2, 4);
    
    QLabel *label1 = new QLabel(i18n("Channel:"), page);
    label1->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_channelCB = new QComboBox( false, page );
    m_channelCB->insertItem( i18n("Luminosity") );
    m_channelCB->insertItem( i18n("Red") );
    m_channelCB->insertItem( i18n("Green") );
    m_channelCB->insertItem( i18n("Blue") );
    m_channelCB->insertItem( i18n("Alpha") );
    m_channelCB->insertItem( i18n("Colors") );
    QWhatsThis::add( m_channelCB, i18n("<p>Select here the histogram channel to display:<p>"
                                       "<b>Luminosity</b>: drawing the image luminosity values.<p>"
                                       "<b>Red</b>: drawing the red image channel values.<p>"
                                       "<b>Green</b>: drawing the green image channel values.<p>"
                                       "<b>Blue</b>: drawing the blue image channel values.<p>"
                                       "<b>Alpha</b>: drawing the alpha image channel values. " 
                                       "This channel corresponding to the transparency value and "
                                       "is supported by some image formats such as PNG or GIF.<p>"
                                       "<b>Colors</b>: drawing all color channels values at the same time."));
    
    m_scaleBG = new QHButtonGroup(page);
    m_scaleBG->setExclusive(true);
    m_scaleBG->setFrameShape(QFrame::NoFrame);
    m_scaleBG->setInsideMargin( 0 );
    QWhatsThis::add( m_scaleBG, i18n("<p>Select here the histogram scale.<p>"
                                     "If the image's maximal counts are small, you can use the linear scale.<p>"
                                     "Logarithmic scale can be used when the maximal counts are big; "
                                     "if it is used, all values (small and large) will be visible on the graph."));
    
    QPushButton *linHistoButton = new QPushButton( m_scaleBG );
    QToolTip::add( linHistoButton, i18n( "<p>Linear" ) );
    m_scaleBG->insert(linHistoButton, Digikam::HistogramWidget::LinScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-lin", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("histogram-lin", "histogram-lin.png");
    linHistoButton->setPixmap( QPixmap( directory + "histogram-lin.png" ) );
    linHistoButton->setToggleButton(true);
    
    QPushButton *logHistoButton = new QPushButton( m_scaleBG );
    QToolTip::add( logHistoButton, i18n( "<p>Logarithmic" ) );
    m_scaleBG->insert(logHistoButton, Digikam::HistogramWidget::LogScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-log", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("histogram-log", "histogram-log.png");
    logHistoButton->setPixmap( QPixmap( directory + "histogram-log.png" ) );
    logHistoButton->setToggleButton(true);       
    
    QLabel *label10 = new QLabel(i18n("Colors:"), page);
    label10->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_colorsCB = new QComboBox( false, page );
    m_colorsCB->insertItem( i18n("Red") );
    m_colorsCB->insertItem( i18n("Green") );
    m_colorsCB->insertItem( i18n("Blue") );
    m_colorsCB->setEnabled( false );
    QWhatsThis::add( m_colorsCB, i18n("<p>Select here the main color displayed with Colors Channel mode:<p>"
                                       "<b>Red</b>: drawing the red image channel on the foreground.<p>"
                                       "<b>Green</b>: drawing the green image channel on the foreground.<p>"
                                       "<b>Blue</b>: drawing the blue image channel on the foreground.<p>"));
                                       
    m_regionBG = new QHButtonGroup(page);
    m_regionBG->setExclusive(true);
    m_regionBG->setFrameShape(QFrame::NoFrame);
    m_regionBG->setInsideMargin( 0 );
    m_regionBG->hide();
    QWhatsThis::add( m_regionBG, i18n("<p>Select here the histogram region computation:<p>"
                                      "<b>Full Image</b>: drawing histogram using the full image.<p>"
                                      "<b>Selection</b>: drawing histogram using the current image selection."));
    
    QPushButton *fullImageButton = new QPushButton( m_regionBG );
    QToolTip::add( fullImageButton, i18n( "<p>Full Image" ) );
    m_regionBG->insert(fullImageButton, Digikam::HistogramWidget::FullImageHistogram);
    KGlobal::dirs()->addResourceType("image-full", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("image-full", "image-full.png");
    fullImageButton->setPixmap( QPixmap( directory + "image-full.png" ) );
    fullImageButton->setToggleButton(true);
    
    QPushButton *SelectionImageButton = new QPushButton( m_regionBG );
    QToolTip::add( SelectionImageButton, i18n( "<p>Selection" ) );
    m_regionBG->insert(SelectionImageButton, Digikam::HistogramWidget::ImageSelectionHistogram);
    KGlobal::dirs()->addResourceType("image-selection", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("image-selection", "image-selection.png");
    SelectionImageButton->setPixmap( QPixmap( directory + "image-selection.png" ) );
    SelectionImageButton->setToggleButton(true);       
                                                                                
    grid->addWidget(label1, 0, 0);
    grid->addWidget(m_channelCB, 0, 1);
    grid->addWidget(m_scaleBG, 0, 2);
    grid->addWidget(label10, 1, 0);
    grid->addWidget(m_colorsCB, 1, 1);
    grid->addWidget(m_regionBG, 1, 2);
                                                                            
    // -------------------------------------------------------------
    
    m_histogramWidget = new Digikam::HistogramWidget(256, 140, page);
    QWhatsThis::add( m_histogramWidget, i18n("<p>This is the histogram drawing of the "
                                             "selected image channel"));
    
    m_hGradient = new Digikam::ColorGradientWidget( Digikam::ColorGradientWidget::Horizontal, 10, page );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
    
    topLayout->addWidget(m_histogramWidget, 4);
    topLayout->addWidget(m_hGradient, 0);

    // -------------------------------------------------------------

    QHBoxLayout *hlay2 = new QHBoxLayout(topLayout);
    QLabel *label3 = new QLabel(i18n("Intensity range:"), page);
    label3->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    m_minInterv = new QSpinBox(0, 255, 1, page);
    m_minInterv->setValue(0);
    QWhatsThis::add( m_minInterv, i18n("<p>Select here the minimal intensity "
                                       "value of the histogram selection."));    
    m_maxInterv = new QSpinBox(0, 255, 1, page);
    m_maxInterv->setValue(255);
    QWhatsThis::add( m_minInterv, i18n("<p>Select here the maximal intensity value "
                                       "of the histogram selection."));
    hlay2->addWidget(label3);
    hlay2->addWidget(m_minInterv);
    hlay2->addWidget(m_maxInterv);
    
    // -------------------------------------------------------------
    
    QGroupBox *gbox = new QGroupBox(4, Qt::Horizontal, i18n("Statistics"), page);
    QWhatsThis::add( gbox, i18n("<p>You can see here the statistic results calculated with the "
                                "selected histogram part. These values are available for all "
                                "channels."));
                                
    QLabel *label4 = new QLabel(i18n("Mean:"), gbox);
    label4->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_labelMeanValue = new QLabel(gbox);
    m_labelMeanValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    QLabel *label5 = new QLabel(i18n("Pixels:"), gbox);
    label5->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_labelPixelsValue = new QLabel(gbox);
    m_labelPixelsValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    QLabel *label6 = new QLabel(i18n("Standard deviation:"), gbox);
    label6->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_labelStdDevValue = new QLabel(gbox);
    m_labelStdDevValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *label7 = new QLabel(i18n("Count:"), gbox);
    label7->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_labelCountValue = new QLabel(gbox);
    m_labelCountValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    QLabel *label8 = new QLabel(i18n("Median:"), gbox);
    label8->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_labelMedianValue = new QLabel(gbox);
    m_labelMedianValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    QLabel *label9 = new QLabel(i18n("Percentile:"), gbox);
    label9->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_labelPercentileValue = new QLabel(gbox);
    m_labelPercentileValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    topLayout->addWidget(gbox);
    topLayout->addStretch();

    // -------------------------------------------------------------
    
    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));
    
    connect(m_scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));
    
    connect(m_colorsCB, SIGNAL(activated(int)),
            this, SLOT(slotColorsChanged(int)));     
                   
    connect(m_regionBG, SIGNAL(released(int)),
            this, SLOT(slotRenderingChanged(int))); ;       
             
    connect(m_histogramWidget, SIGNAL(signalMousePressed( int )),
            this, SLOT(slotUpdateMinInterv(int)));
       
    connect(m_histogramWidget, SIGNAL(signalMouseReleased( int )),
            this, SLOT(slotUpdateMaxInterv(int)));

    connect(m_histogramWidget, SIGNAL(signalHistogramComputationDone()),
            this, SLOT(slotRefreshOptions()));
            
    connect(m_minInterv, SIGNAL(valueChanged (int)),
            m_histogramWidget, SLOT(slotMinValueChanged(int)));

    connect(m_minInterv, SIGNAL(valueChanged (int)),
            this, SLOT(slotIntervChanged(int)));

    connect(m_maxInterv, SIGNAL(valueChanged (int)),
            m_histogramWidget, SLOT(slotMaxValueChanged(int)));

    connect(m_maxInterv, SIGNAL(valueChanged (int)),
            this, SLOT(slotIntervChanged(int)));

    // -- read config ---------------------------------------------------------

    KConfig* config = kapp->config();
    config->setGroup("Image Properties Dialog");
    m_channelCB->setCurrentItem(config->readNumEntry("Histogram Channel", 0));    // Luminosity.
    m_scaleBG->setButton(config->readNumEntry("Histogram Scale", Digikam::HistogramWidget::LinScaleHistogram)); 
    m_colorsCB->setCurrentItem(config->readNumEntry("Histogram Color", 0));       // Red.
    m_regionBG->setButton(config->readNumEntry("Histogram Rendering", Digikam::HistogramWidget::FullImageHistogram));
}

ImagePropertiesHistogram::~ImagePropertiesHistogram()
{
    // If there is a currently histogram computation when dialog is closed,
    // stop it before the m_image data are deleted automaticly!
    m_histogramWidget->stopHistogramComputation();

    KConfig* config = kapp->config();
    config->setGroup("Image Properties Dialog");
    config->writeEntry("Histogram Channel", m_channelCB->currentItem());
    config->writeEntry("Histogram Scale", m_scaleBG->selectedId());
    config->writeEntry("Histogram Color", m_colorsCB->currentItem());
    config->writeEntry("Histogram Rendering", m_regionBG->selectedId());
    
    if (!m_thumbJob.isNull())
        m_thumbJob->kill();

    if ( m_histogramWidget )
       delete m_histogramWidget;
    
    if ( m_hGradient )        
       delete m_hGradient;

}

void ImagePropertiesHistogram::setData(const KURL& url, uint* imageData, int imageWidth, int imageHeight)
{
    if (!m_thumbJob.isNull())
    {
        m_thumbJob->kill();
        m_thumbJob = 0;
    }

    m_thumbJob = new ThumbnailJob(url, 48, true, AlbumSettings::instance()->getExifRotate());
    
    connect(m_thumbJob, SIGNAL(signalThumbnail(const KURL&,
                                               const QPixmap&)),
            SLOT(slotGotThumbnail(const KURL&,
                                  const QPixmap&)));
   
    connect(m_thumbJob, SIGNAL(signalFailed(const KURL&)),
            SLOT(slotFailedThumbnail(const KURL&)));     

    // ----------------------------------------------------------------

    // This is necessary to stop computation because m_image.bits() is currently used by
    // threaded histogram algorithm.
    
    m_histogramWidget->stopHistogramComputation();
        
    if (!imageData && !imageWidth && !imageHeight)
    {
        if ( m_image.load(url.path()) )
        {
            if(m_image.depth() < 32)                 // we works always with 32bpp.
                m_image = m_image.convertDepth(32);
        
            // If a selection area is done in Image Editor and if the current image is the same 
            // in Image Editor, then compute too the histogram for this selection.
            
            if (m_selectionArea)
            {
                m_imageSelection = m_image.copy(*m_selectionArea);
                m_histogramWidget->updateData((uint *)m_image.bits(), m_image.width(), m_image.height(),
                                              (uint *)m_imageSelection.bits(), m_imageSelection.width(),
                                              m_imageSelection.height());
                m_regionBG->show();                                         
            }
            else 
            {
                m_histogramWidget->updateData((uint *)m_image.bits(), m_image.width(), m_image.height());
                m_regionBG->hide();
            }
        }
        else 
        {
            m_imageSelection.reset();
            m_image.reset();
            m_histogramWidget->updateData(0L, 0, 0);
        }
    }
    else 
    {
    if ( m_image.create(imageWidth, imageHeight, 32) )
        {
            memcpy( m_image.bits(), imageData, m_image.numBytes());
            
            // If a selection area is done in Image Editor and if the current image is the same 
            // in Image Editor, then compute too the histogram for this selection.
            
            if (m_selectionArea)
            {
                m_imageSelection = m_image.copy(*m_selectionArea);
                m_histogramWidget->updateData((uint *)m_image.bits(), m_image.width(), m_image.height(),
                                              (uint *)m_imageSelection.bits(), m_imageSelection.width(),
                                              m_imageSelection.height());
                m_regionBG->show();                                         
            }
            else 
            {
                m_histogramWidget->updateData((uint *)m_image.bits(), m_image.width(), m_image.height());
                m_regionBG->hide();
            }
        }
        else 
        {
            m_imageSelection.reset();
            m_image.reset();
            m_histogramWidget->updateData(0L, 0, 0);
        }}
}

void ImagePropertiesHistogram::slotRefreshOptions()
{
    slotChannelChanged(m_channelCB->currentItem());
    slotScaleChanged(m_scaleBG->selectedId());
    slotColorsChanged(m_colorsCB->currentItem());
    
    if (m_selectionArea)
       slotRenderingChanged(m_regionBG->selectedId());
}

void ImagePropertiesHistogram::slotChannelChanged(int channel)
{
    switch(channel)
    {
    case 1:           // Red.
        m_histogramWidget->m_channelType = Digikam::HistogramWidget::RedChannelHistogram;
        m_hGradient->setColors( QColor( "black" ), QColor( "red" ) );
        m_colorsCB->setEnabled(false);
        break;
       
    case 2:           // Green.
        m_histogramWidget->m_channelType = Digikam::HistogramWidget::GreenChannelHistogram;
        m_hGradient->setColors( QColor( "black" ), QColor( "green" ) );
        m_colorsCB->setEnabled(false);
        break;
          
    case 3:           // Blue.
        m_histogramWidget->m_channelType = Digikam::HistogramWidget::BlueChannelHistogram;
        m_hGradient->setColors( QColor( "black" ), QColor( "blue" ) );
        m_colorsCB->setEnabled(false);
        break;

    case 4:           // Alpha.
        m_histogramWidget->m_channelType = Digikam::HistogramWidget::AlphaChannelHistogram;
        m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
        m_colorsCB->setEnabled(false);
        break;
          
    case 5:           // All color channels.
        m_histogramWidget->m_channelType = Digikam::HistogramWidget::ColorChannelsHistogram;
        m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
        m_colorsCB->setEnabled(true);
        break;
                              
    default:          // Luminosity.
        m_histogramWidget->m_channelType = Digikam::HistogramWidget::ValueHistogram;
        m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
        m_colorsCB->setEnabled(false);
        break;
    }
   
    m_histogramWidget->repaint(false);
    updateInformation();
}

void ImagePropertiesHistogram::slotScaleChanged(int scale)
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint(false);
}

void ImagePropertiesHistogram::slotColorsChanged(int color)
{
    switch(color)
    {
    case 1:           // Green.
        m_histogramWidget->m_colorType = Digikam::HistogramWidget::GreenColor;
        break;
       
    case 2:           // Blue.
        m_histogramWidget->m_colorType = Digikam::HistogramWidget::BlueColor;
        break;

    default:          // Red.
        m_histogramWidget->m_colorType = Digikam::HistogramWidget::RedColor;
        break;
    }

    m_histogramWidget->repaint(false);
    updateInformation();
}

void ImagePropertiesHistogram::slotRenderingChanged(int rendering)
{
    m_histogramWidget->m_renderingType = rendering;
    m_histogramWidget->repaint(false);
    updateInformation();
}

void ImagePropertiesHistogram::slotIntervChanged(int)
{
    m_maxInterv->setMinValue(m_minInterv->value());
    m_minInterv->setMaxValue(m_maxInterv->value());
    updateInformation();
}

void ImagePropertiesHistogram::slotUpdateMinInterv(int min)
{
    m_minInterv->setValue(min);
}

void ImagePropertiesHistogram::slotUpdateMaxInterv(int max)
{
    m_maxInterv->setValue(max);
    updateInformation();
}

void ImagePropertiesHistogram::updateInformation()
{
    QString value;
    int min = m_minInterv->value();
    int max = m_maxInterv->value();
    int channel = m_channelCB->currentItem();

    if ( channel == Digikam::HistogramWidget::ColorChannelsHistogram )
        channel = m_colorsCB->currentItem()+1;
               
    double mean = m_histogramWidget->m_imageHistogram->getMean(channel, min, max);
    m_labelMeanValue->setText(value.setNum(mean, 'f', 1));
    
    double pixels = m_histogramWidget->m_imageHistogram->getPixels();
    m_labelPixelsValue->setText(value.setNum((float)pixels, 'f', 0));
    
    double stddev = m_histogramWidget->m_imageHistogram->getStdDev(channel, min, max);
    m_labelStdDevValue->setText(value.setNum(stddev, 'f', 1));
      
    double counts = m_histogramWidget->m_imageHistogram->getCount(channel, min, max);
    m_labelCountValue->setText(value.setNum((float)counts, 'f', 0));
    
    double median = m_histogramWidget->m_imageHistogram->getMedian(channel, min, max);
    m_labelMedianValue->setText(value.setNum(median, 'f', 1)); 

    double percentile = (pixels > 0 ? (100.0 * counts / pixels) : 0.0);
    m_labelPercentileValue->setText(value.setNum(percentile, 'f', 1));
}

void ImagePropertiesHistogram::slotGotThumbnail(const KURL&,
                                                const QPixmap& pix)
{
    if (!pix.isNull())
        m_labelThumb->setPixmap(pix);
    else
        m_labelThumb->clear();
}

void ImagePropertiesHistogram::slotFailedThumbnail(const KURL&)
{
    m_labelThumb->clear();
}

#include "imagepropertieshistogram.moc"
