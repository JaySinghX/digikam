/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-09
 * Description : Thread actions task for metadata synchronizer.
 *
 * Copyright (C) 2013-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef METADATA_TASK_H
#define METADATA_TASK_H

// Qt includes

#include <QThread>

// Libkdcraw includes

#include <KDCRAW/RActionJob>

// Local includes

#include "imageinfo.h"
#include "metadatasynchronizer.h"

using namespace KDcrawIface;

namespace Digikam
{

class MetadataTask : public RActionJob
{
    Q_OBJECT

public:

    MetadataTask();
    ~MetadataTask();

    void setItem(const ImageInfo& item, MetadataSynchronizer::SyncDirection dir);
    void setTagsOnly(bool value);

Q_SIGNALS:

    void signalFinished(const QImage&);

public Q_SLOTS:

    void slotCancel();

protected:

    void run();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* METADATA_TASK_H */
