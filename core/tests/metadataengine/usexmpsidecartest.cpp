/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-10-30
 * Description : An unit-test to read/write metadata from XMP sidecar with DMetadata.
 *
 * Copyright (C) 2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "usexmpsidecartest.h"

// Qt includes

#include <QDebug>
#include <QTest>
#include <QFile>

// Local includes

#include "dmetadata.h"
#include "wstoolutils.h"

QTEST_MAIN(UseXmpSidecarTest)

const QString originalImageFolder(QFINDTESTDATA("data/"));

void UseXmpSidecarTest::initTestCase()
{
    MetaEngine::initializeExiv2();
    qDebug() << "Using Exiv2 Version:" << MetaEngine::Exiv2Version();
}

void UseXmpSidecarTest::testUseXmpSidecar()
{
    MetaEngineSettingsContainer settings;

    settings.useXMPSidecar4Reading = true;
    settings.metadataWritingMode = DMetadata::WRITETOSIDECARONLY;
    useXmpSidecar(originalImageFolder + QLatin1String("_27A1417.CR2"), settings);

    settings.useXMPSidecar4Reading = true;
    settings.metadataWritingMode = DMetadata::WRITETOSIDECARONLY4READONLYFILES;
    useXmpSidecar(originalImageFolder + QLatin1String("_27A1417.CR2"), settings);

    settings.useXMPSidecar4Reading = false;
    settings.metadataWritingMode = DMetadata::WRITETOIMAGEONLY;
    useXmpSidecar(originalImageFolder + QLatin1String("_27A1434.JPG"), settings);

    settings.useXMPSidecar4Reading = true;
    settings.metadataWritingMode = DMetadata::WRITETOSIDECARONLY;
    useXmpSidecar(originalImageFolder + QLatin1String("_27A1434.JPG"), settings);

    settings.useXMPSidecar4Reading = true;
    settings.metadataWritingMode = DMetadata::WRITETOSIDECARANDIMAGE;
    useXmpSidecar(originalImageFolder + QLatin1String("_27A1434.JPG"), settings);
}

void UseXmpSidecarTest::cleanupTestCase()
{
    MetaEngine::cleanupExiv2();
}

void UseXmpSidecarTest::useXmpSidecar(const QString& file,
                                      const MetaEngineSettingsContainer& settings)
{
    qDebug() << "File to process:          " << file;
    bool ret        = false;

    QString path    = WSToolUtils::makeTemporaryDir("usexmpsidecartest")
                      .filePath(QFileInfo(file).fileName().trimmed());

    QString pathXmp = path + QLatin1String(".xmp");


    qDebug() << "Temporary target file:    " << path;

    ret = !path.isNull();
    QVERIFY(ret);

    QFile::remove(path);
    QFile target(file);
    ret = target.copy(path);
    QVERIFY(ret);

    qDebug() << "Temporary XMP target file:" << pathXmp;

    ret = !pathXmp.isNull();
    QVERIFY(ret);

    QFile::remove(pathXmp);
    QFile xmpTarget(file + QLatin1String(".xmp"));
    ret = xmpTarget.copy(pathXmp);

    DMetadata meta;
    meta.setSettings(settings);
    ret = meta.load(path);
    QVERIFY(ret);

    meta.setImageRating(3);
    meta.setImagePickLabel(2);
    meta.setImageColorLabel(1);

    QStringList tags = QStringList() << QString::fromUtf8("City/Paris/Eiffel Tower")
                                     << QString::fromUtf8("Animals/Dog/Illka")
                                     << QString::fromUtf8("People/Family/Agnès");
    meta.setImageTagsPath(tags);
    ret = meta.applyChanges(true);
    QVERIFY(ret);

    DMetadata meta2;
    meta2.setSettings(settings);
    ret = meta2.load(path);
    QVERIFY(ret);

    int val = meta2.getImageRating();
    QCOMPARE(val, 3);

    val = meta2.getImagePickLabel();
    QCOMPARE(val, 2);

    val = meta2.getImageColorLabel();
    QCOMPARE(val, 1);

    QStringList newTags;
    ret = meta2.getImageTagsPath(newTags);
    QVERIFY(ret);

    int count = tags.count();

    foreach (const QString& tag, newTags)
    {
        if (tags.contains(tag))
        {
            --count;
        }
    }

    QCOMPARE(count, 0);

    WSToolUtils::removeTemporaryDir("usexmpsidecartest");
}
