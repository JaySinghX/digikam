/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-25-02
 * Description : Levels image filter
 *
 * Copyright (C) 2010-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "levelsfilter.h"

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "imagelevels.h"

namespace Digikam
{

LevelsFilter::LevelsFilter(QObject* parent)
    : DImgThreadedFilter(parent)
{
    initFilter();
}

LevelsFilter::LevelsFilter(DImg* orgImage, QObject* parent, const LevelsContainer& settings)
    : DImgThreadedFilter(orgImage, parent, "LevelsFilter")
{
    m_settings = settings;
    initFilter();
}

LevelsFilter::LevelsFilter(const LevelsContainer& settings, DImgThreadedFilter* master,
        const DImg& orgImage, DImg& destImage, int progressBegin, int progressEnd)
    : DImgThreadedFilter(master, orgImage, destImage, progressBegin, progressEnd, "LevelsFilter")
{
    m_settings = settings;
    initFilter();
    destImage = m_destImage;
}

LevelsFilter::~LevelsFilter()
{
    cancelFilter();
}

void LevelsFilter::filterImage()
{
    ImageLevels levels(m_orgImage.sixteenBit());

    for (int i = 0 ; i < 5 ; ++i)
    {
        postProgress(i * 10);
        levels.setLevelLowInputValue(i,   m_settings.lInput[i]);
        levels.setLevelHighInputValue(i,  m_settings.hInput[i]);
        levels.setLevelLowOutputValue(i,  m_settings.lOutput[i]);
        levels.setLevelHighOutputValue(i, m_settings.hOutput[i]);
        levels.setLevelGammaValue(i,      m_settings.gamma[i]);
    }

    postProgress(50);

    m_destImage = DImg(m_orgImage.width(), m_orgImage.height(), m_orgImage.sixteenBit(), m_orgImage.hasAlpha());
    postProgress(60);

    levels.levelsCalculateTransfers();
    postProgress(70);

    // Process all channels Levels
    levels.levelsLutSetup(AlphaChannel);
    postProgress(80);

    levels.levelsLutProcess(m_orgImage.bits(), m_destImage.bits(), m_orgImage.width(), m_orgImage.height());
    postProgress(90);
}

FilterAction LevelsFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    for (int i = 0; i < 5; ++i)
    {
        action.addParameter(QString("gamma[%1]").arg(i),   m_settings.gamma[i]);
        action.addParameter(QString("hInput[%1]").arg(i),  m_settings.hInput[i]);
        action.addParameter(QString("hOutput[%1]").arg(i), m_settings.hOutput[i]);
        action.addParameter(QString("lInput[%1]").arg(i),  m_settings.lInput[i]);
        action.addParameter(QString("lOutput[%1]").arg(i), m_settings.lOutput[i]);
    }

    return action;
}

void LevelsFilter::readParameters(const Digikam::FilterAction& action)
{
    for (int i = 0; i < 5; ++i)
    {
        m_settings.gamma[i]   = action.parameter(QString("gamma[%1]").arg(i)).toDouble();
        m_settings.hInput[i]  = action.parameter(QString("hInput[%1]").arg(i)).toInt();
        m_settings.hOutput[i] = action.parameter(QString("hOutput[%1]").arg(i)).toInt();
        m_settings.lInput[i]  = action.parameter(QString("lInput[%1]").arg(i)).toInt();
        m_settings.lOutput[i] = action.parameter(QString("lOutput[%1]").arg(i)).toInt();
    }
}

}  // namespace Digikam
