/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to MediaWiki web-service.
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "mediawikiplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "mediawikiwindow.h"

namespace Digikam
{

MediaWikiPlugin::MediaWikiPlugin(QObject* const parent)
    : DPlugin(parent)
{
}

MediaWikiPlugin::~MediaWikiPlugin()
{
    delete m_toolDlg;
}

QString MediaWikiPlugin::name() const
{
    return i18n("MediaWiki");
}

QString MediaWikiPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon MediaWikiPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("dk-mediawiki"));
}

QString MediaWikiPlugin::description() const
{
    return i18n("A tool to export to MediaWiki web-service");
}

QString MediaWikiPlugin::details() const
{
    return i18n("<p>This tool permit to export items to MediaWiki web-service.</p>"
                "<p>See MediaWiki web site for details: <a href='https://www.mediawiki.org/'>hhttps://www.mediawiki.org/</a></p>");
}

QList<DPluginAuthor> MediaWikiPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2011-2019"))
            << DPluginAuthor(QLatin1String("Alexandre Mendes"),
                             QLatin1String("alex dot mendes1988 at gmail dot com"),
                             QLatin1String("(C) 2011"))
            << DPluginAuthor(QLatin1String("Guillaume Hormiere"),
                             QLatin1String("hormiere dot guillaume at gmail dot com"),
                             QLatin1String("(C) 2011-2012"))
            << DPluginAuthor(QLatin1String("Peter Potrowl"),
                             QLatin1String("peter dot potrowl at gmail dot com"),
                             QLatin1String("(C) 2013"))
            ;
}

void MediaWikiPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to MediaWiki..."));
    ac->setObjectName(QLatin1String("export_MediaWiki"));
    ac->setActionCategory(DPluginAction::GenericExport);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotMediaWiki()));

    addAction(ac);
}

void MediaWikiPlugin::slotMediaWiki()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        delete m_toolDlg;
        m_toolDlg = new MediaWikiWindow(infoIface(sender()), 0);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace Digikam
