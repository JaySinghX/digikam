/* ============================================================
 * Authors: Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date   : 2006-06-11
 * Description : thread safe debugging.
 * See B.K.O #133026: because kdDebug() is not thread-safe
 * we need to use a dedicaced debug statements in threaded 
 * implementation to prevent crash.
 *
 * Copyright 2006 by Marcel Wiesweg
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

// Qt includes.

#include <qmutex.h>

// Local includes.

#include "ddebug.h"

namespace Digikam
{

//static KStaticDeleter<QMutex> deleter;
static QMutex *mutex = 0;

Ddbgstream::Ddbgstream(kdbgstream stream)
          : kdbgstream(stream)
{
    // using a static variable here - we can safely assume that kdDebug
    // is called at least once from the main thread before threads start.
    if (!mutex)
    {
        // leak the mutex object for simplicity
        mutex = new QMutex;
        //deleter.setObject(mutex, new QMutex);
        //KGlobal::unregisterStaticDeleter(&deleter);
    }
    mutex->lock();
}

Ddbgstream::~Ddbgstream()
{
    mutex->unlock();
}

} // namespace Digikam

Digikam::Ddbgstream DDebug(int area)   { return Digikam::Ddbgstream(kdDebug(area));   }
Digikam::Ddbgstream DError(int area)   { return Digikam::Ddbgstream(kdError(area));   }
Digikam::Ddbgstream DWarning(int area) { return Digikam::Ddbgstream(kdWarning(area)); }

