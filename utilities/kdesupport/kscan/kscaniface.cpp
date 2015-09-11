/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-15
 * Description : KScan interface
 *
 * Copyright (C) 2010-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "kscaniface.h"

// Qt includes

#include <QAction>
#include <QString>
#include <QApplication>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// LibKSane includes

#include <ksane.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

KScanAction::KScanAction(QObject* const parent)
    : QAction(parent)
{
    m_saneWidget = 0;

    setText(i18n("Import from Scanner..."));
    setIcon(QIcon::fromTheme(QLatin1String("scanner")));

    connect(this, SIGNAL(triggered(bool)),
            this, SLOT(slotActivate()));
}

KScanAction::~KScanAction()
{
}

void KScanAction::slotActivate()
{
    if (!m_saneWidget)
    {
        m_saneWidget = new KSaneIface::KSaneWidget(0);
    }

    if (m_saneWidget)
    {
        QString dev = m_saneWidget->selectDevice(0);

        if (dev.isEmpty())
        {
            return;
        }

        if (!m_saneWidget->openDevice(dev))
        {
            // could not open a scanner
            QMessageBox::warning(0, qApp->applicationName(), i18n("Cannot open scanner device."));
            return;
        }

        m_saneWidget->show();
    }
}

} // namespace Digikam
