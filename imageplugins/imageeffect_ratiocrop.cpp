/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-06
 * Description : Ratio crop tool for ImageEditor
 * 
 * Copyright 2004 by Gilles Caulier
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

// Imlib2 include.

#define X_DISPLAY_MISSING 1
#include <Imlib2.h>

// C++ include.

#include <cstring>

// Qt includes.

#include <qlayout.h>
#include <qframe.h>
#include <qrect.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qtimer.h>
#include <qcombobox.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <knuminput.h>

// Digikam includes.

#include <imageiface.h>
#include <imageselectionwidget.h>

// Local includes.

#include "imageeffect_ratiocrop.h"

ImageEffect_RatioCrop::ImageEffect_RatioCrop(QWidget* parent)
                     : KDialogBase(Plain, i18n("Aspect Ratio Crop"),
                                   Help|User1|Ok|Cancel, Ok,
                                   parent, 0, true, true, 
                                   i18n("&Reset Values")),
                       m_parent(parent)
{
    setHelp("ratiocroptool.anchor", "digikam");
    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    QVGroupBox *gbox = new QVGroupBox(i18n("Aspect Ratio Crop Preview"), plainPage());
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_imageSelectionWidget = new Digikam::ImageSelectionWidget(480, 320, frame);
    l->addWidget(m_imageSelectionWidget, 0, Qt::AlignCenter);
    topLayout->addWidget(gbox);
 
    QHBoxLayout *hlay = new QHBoxLayout(topLayout);
    QLabel *label = new QLabel(i18n("Aspect Ratio:"), plainPage());
    m_ratioCB = new QComboBox( false, plainPage() );
    m_ratioCB->insertItem( "1:1" );
    m_ratioCB->insertItem( "2:3" );
    m_ratioCB->insertItem( "3:4" );
    m_ratioCB->insertItem( "4:5" );
    m_ratioCB->insertItem( "5:7" );
    m_ratioCB->insertItem( "7:10" );
    QWhatsThis::add( m_ratioCB, i18n("<p>Select here your aspect ratio for cropping."));
    
    QLabel *label2 = new QLabel(i18n("Orientation:"), plainPage());
    m_orientCB = new QComboBox( false, plainPage() );
    m_orientCB->insertItem( i18n("Landscape") );
    m_orientCB->insertItem( i18n("Portrait") );
    
    hlay->addWidget(label, 1);
    hlay->addWidget(m_ratioCB, 2);
    hlay->addWidget(label2, 1);
    hlay->addWidget(m_orientCB, 2);

    QHBoxLayout *hlay2 = new QHBoxLayout(topLayout);
    QLabel *label3 = new QLabel(i18n("Width:"), plainPage());
    m_widthInput = new KIntNumInput(plainPage());
    m_widthInput->setRange(10, m_imageSelectionWidget->getOriginalImageWidth(), 1, true);
    hlay2->addWidget(label3, 1);
    hlay2->addWidget(m_widthInput, 5);
    
    QHBoxLayout *hlay3 = new QHBoxLayout(topLayout);
    QLabel *label4 = new QLabel(i18n("Height:"), plainPage());
    m_heightInput = new KIntNumInput(plainPage());
    m_heightInput->setRange(10, m_imageSelectionWidget->getOriginalImageHeight(), 1, true);
    hlay3->addWidget(label4, 1);
    hlay3->addWidget(m_heightInput, 5);
    
    connect(m_ratioCB, SIGNAL(activated(int)),
            this, SLOT(slotRatioChanged(int)));
    
    connect(m_orientCB, SIGNAL(activated(int)),
            this, SLOT(slotOrientChanged(int)));
        
    connect(m_widthInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotWidthChanged(int)));

    connect(m_heightInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotHeightChanged(int)));
    
    connect(m_imageSelectionWidget, SIGNAL(signalSelectionChanged(QRect)),
            this, SLOT(slotSelectionChanged(QRect)));                                            
            
    QTimer::singleShot(0, this, SLOT(slotUser1()));
    adjustSize();
    disableResize();                  
}

ImageEffect_RatioCrop::~ImageEffect_RatioCrop()
{
}

void ImageEffect_RatioCrop::slotUser1()
{
    m_imageSelectionWidget->resetSelection();
/*    m_imageSelectionWidget->setSelectionX(0);
    m_imageSelectionWidget->setSelectionY(0);
    m_widthInput->setValue((int)(m_imageSelectionWidget->getOriginalImageWidth() / 2.0));
    m_imageSelectionWidget->setCenterSelection();*/
} 

void ImageEffect_RatioCrop::slotSelectionChanged(QRect rect)
{
    m_widthInput->blockSignals(true);
    m_heightInput->blockSignals(true);
    m_widthInput->setValue(rect.width());
    m_heightInput->setValue(rect.height());
    m_widthInput->blockSignals(false);
    m_heightInput->blockSignals(false);
}

void ImageEffect_RatioCrop::slotWidthChanged(int w)
{
    m_imageSelectionWidget->setSelectionWidth(w);
}

void ImageEffect_RatioCrop::slotHeightChanged(int h)
{
    m_imageSelectionWidget->setSelectionHeight(h);
}

void ImageEffect_RatioCrop::slotOrientChanged(int o)
{
    m_imageSelectionWidget->setSelectionOrientation(o);
}

void ImageEffect_RatioCrop::slotRatioChanged(int a)
{
    m_imageSelectionWidget->setSelectionAspectRatio(a);
}

void ImageEffect_RatioCrop::slotOk()
{
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface iface(0, 0);
    
    uint* data       = iface.getOriginalData();
    int w            = iface.originalWidth();
    int h            = iface.originalHeight();
    QRect currentPos = m_imageSelectionWidget->getRegionSelection();

    Imlib_Context context = imlib_context_new();
    imlib_context_push(context);

    Imlib_Image imOrg = imlib_create_image_using_copied_data(w, h, data);
    imlib_context_set_image(imOrg);
    
    Imlib_Image imDest = imlib_create_cropped_image(currentPos.left(), currentPos.top(),
                                                    currentPos.width(), currentPos.height());
    imlib_context_set_image(imDest);    
    
    uint* ptr  = imlib_image_get_data_for_reading_only();
    int   newW = imlib_image_get_width();
    int   newH = imlib_image_get_height();

    iface.putOriginalData(ptr, newW, newH);   
    
    imlib_context_set_image(imOrg);
    imlib_free_image_and_decache();

    imlib_context_set_image(imDest);
    imlib_free_image_and_decache();
    
    imlib_context_pop();
    imlib_context_free(context);       
    delete [] data;
    m_parent->setCursor( KCursor::arrowCursor() );
    accept();
}

#include "imageeffect_ratiocrop.moc"
