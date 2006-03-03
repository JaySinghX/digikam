/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-20
 * Description : a widget to display non standard Exif metadata
 *               used by camera makers
 *
 * Copyright 2006 by Gilles Caulier
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

// C++ includes.

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <string>

// Qt includes.

#include <qmap.h>
#include <qfile.h>

// KDE includes.

#include <kdebug.h>
#include <klocale.h>

// LibExiv2 includes.

#include <exiv2/exif.hpp>
#include <exiv2/tags.hpp>
#include <exiv2/ifd.hpp>

// Local includes.

#include "dmetadata.h"
#include "makernotewidget.h"

namespace Digikam
{

static char* ExifHumanList[] =
{
     "Make",
     "Model",
     "DateTime",
     "ImageDescription",
     "Copyright",
     "ShutterSpeedValue",
     "ApertureValue",
     "ExposureProgram",
     "ExposureMode",
     "ExposureBiasValue",
     "ExposureTime",
     "WhiteBalance",
     "ISOSpeedRatings",
     "FocalLength",
     "SubjectDistance",
     "MeteringMode",
     "Contrast",
     "Saturation",
     "Sharpness",
     "LightSource",
     "Flash",
     "-1"
};

static char* StandardExifEntryList[] =
{
     "Image",
     "Photo",
     "GPSInfo",
     "Iop",
     "Thumbnail",
     "-1"
};

MakerNoteWidget::MakerNoteWidget(QWidget* parent, const char* name)
               : MetadataWidget(parent, name)
{
    for (int i=0 ; QString(StandardExifEntryList[i]) != QString("-1") ; i++)
        m_keysFilter << StandardExifEntryList[i];

    for (int i=0 ; QString(ExifHumanList[i]) != QString("-1") ; i++)
        m_tagsfilter << ExifHumanList[i];
}

MakerNoteWidget::~MakerNoteWidget()
{
}

QString MakerNoteWidget::getMetadataTitle(void)
{
    return i18n("Maker Note Exif Tags");
}

bool MakerNoteWidget::loadFromURL(const KURL& url)
{
    setFileName(url.path());

    if (url.isEmpty())
    {
        setMetadata();
        return false;
    }
    else
    {    
        DMetadata metadata(url.path());
        QByteArray exifData = metadata.getExif();

        if (exifData.isEmpty())
        {
            setMetadata();
            return false;
        }
        else
            setMetadata(exifData);
    }

    return true;
}

bool MakerNoteWidget::decodeMetadata()
{
    try
    {
        Exiv2::ExifData exifData;
        if (exifData.load((Exiv2::byte*)getMetadata().data(), getMetadata().size()) != 0)
        {
            kdDebug() << "Cannot parse MAKERNOTE metadata using Exiv2" << endl;
            return false;
        }

        exifData.sortByKey();
        
        QString ifDItemName;
        MetaDataMap metaDataMap;

        for (Exiv2::ExifData::iterator md = exifData.begin(); md != exifData.end(); ++md)
        {
            QString key = QString::fromLocal8Bit(md->key().c_str());
            
            // Decode the tag value with a user friendly output.
            std::ostringstream os;
            os << *md;
            QString value = QString::fromLocal8Bit(os.str().c_str());

            // We apply a filter to get only standard Exif tags, not maker notes.
            if (!m_keysFilter.contains(key.section(".", 1, 1)))
                metaDataMap.insert(key, value);
        }
        
        // Update all metadata contents.
        setMetadataMap(metaDataMap);

        return true;
    }
    catch (Exiv2::Error& e)
    {
        kdDebug() << "Cannot parse MAKERNOTE metadata using Exiv2 ("
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
        return false;
    }
}

void MakerNoteWidget::buildView(void)
{
    if (getMode() == SIMPLE)
    {
        setIfdList(getMetadataMap(), m_tagsfilter);
    }
    else
    {
        setIfdList(getMetadataMap());
    }
}

QString MakerNoteWidget::getTagTitle(const QString& key)
{
    try 
    {
        std::string exifkey(key.ascii());
        Exiv2::ExifKey ek(exifkey); 
        return QString::fromLocal8Bit( Exiv2::ExifTags::tagTitle(ek.tag(), ek.ifdId()) );
    }
    catch (Exiv2::Error& e) 
    {
        kdDebug() << "Cannot get metadata tag title using Exiv2 ("
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
        return i18n("Unknow");
    }
}

QString MakerNoteWidget::getTagDescription(const QString& key)
{
    try 
    {
        std::string exifkey(key.ascii());
        Exiv2::ExifKey ek(exifkey); 
        return QString::fromLocal8Bit( Exiv2::ExifTags::tagDesc(ek.tag(), ek.ifdId()) );
    }
    catch (Exiv2::Error& e) 
    {
        kdDebug() << "Cannot get metadata tag description using Exiv2 ("
                  << QString::fromLocal8Bit(e.what().c_str())
                  << ")" << endl;
        return i18n("No description available");
    }
}

}  // namespace Digikam

#include "makernotewidget.moc"
