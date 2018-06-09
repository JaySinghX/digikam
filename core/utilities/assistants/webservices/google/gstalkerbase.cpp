/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-21
 * Description : a tool to export items to Google web services
 *
 * Copyright (C) 2015      by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 * Copyright (C) 2015-2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
 * Copyright (C) 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "gstalkerbase.h"

// Qt includes

#include <QByteArray>
#include <QtAlgorithms>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QList>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QPair>
#include <QFileInfo>
#include <QDebug>
#include <QApplication>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrlQuery>
#include <QSettings>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Local includes

#include "gdmpform.h"
#include "digikam_debug.h"

// O2 includes

#include "o0globals.h"
#include "o0settingsstore.h"
#include "wstoolutils.h"

namespace Digikam
{

class GSTalkerBase::Private
{
public:

    enum AuthState
    {
        GS_ACCESSTOKEN = 0,
        GS_REFRESHTOKEN
    };

public:

    explicit Private()
    {
        parent       = 0;
        netMngr      = 0;
        
        redirectUri  = QString::fromLatin1("urn:ietf:wg:oauth:2.0:oob");
        responseType = QString::fromLatin1("code");
        apikey       = QString::fromLatin1("258540448336-hgdegpohibcjasvk1p595fpvjor15pbc.apps.googleusercontent.com");
        clientSecret = QString::fromLatin1("iiIKTNM4ggBXiTdquAzbs2xw");     
        /* Old api key and secret below only work for gdrive, not gphoto
         * Switch to new api key and secret above
         * apikey       = QString::fromLatin1("735222197981-mrcgtaqf05914buqjkts7mk79blsquas.apps.googleusercontent.com");
         * clientSecret = QString::fromLatin1("4MJOS0u1-_AUEKJ0ObA-j22U");
         */
        
        authUrl      = QLatin1String("https://accounts.google.com/o/oauth2/auth");
        tokenUrl     = QLatin1String("https://accounts.google.com/o/oauth2/token");
        refreshUrl   = QLatin1String("https://accounts.google.com/o/oauth2/token");
        
        code         = QString::fromLatin1("0");
        continuePos  = 0;
        authState    = GS_ACCESSTOKEN;
        window       = 0;
    }

    int                    continuePos;

    QWidget*               parent;

    AuthState              authState;
    
    QString                authUrl;
    QString                tokenUrl;
    QString                refreshUrl;
    
    QString                apikey;
    QString                clientSecret;
    
    QString                responseType;
    QString                redirectUri;
    QString                code;

    QDialog*               window;

    QNetworkAccessManager* netMngr;
    O2*                    o2;
    QSettings*             settings;
};

GSTalkerBase::GSTalkerBase(QWidget* const parent, const QString & scope, const QString& serviceName)
    : d(new Private)
{
    m_reply         = 0;
    m_scope         = scope;
    m_serviceName   = serviceName;
    d->parent       = parent;
    d->netMngr      = new QNetworkAccessManager(this);

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotAuthFinished(QNetworkReply*)));
    
    //Ported to O2
    d->o2 = new O2(this);
    d->o2->setClientId(d->apikey);
    d->o2->setClientSecret(d->clientSecret);
    
    // OAuth2 flow control 
    d->o2->setRequestUrl(d->authUrl);
    d->o2->setTokenUrl(d->tokenUrl);
    d->o2->setRefreshTokenUrl(d->refreshUrl);
    d->o2->setLocalPort(8000);
    d->o2->setGrantFlow(O2::GrantFlow::GrantFlowAuthorizationCode);
    d->o2->setScope(m_scope);
    
    // OAuth configuration saved to between dk sessions
    d->settings                  = WSToolUtils::getOauthSettings(this);
    O0SettingsStore* const store = new O0SettingsStore(d->settings, QLatin1String(O2_ENCRYPTION_KEY), this);
    store->setGroupKey(m_serviceName);
    d->o2->setStore(store);
    
    // Refresh token permission when offline
    QMap<QString, QVariant> extraParams;
    extraParams.insert("access_type", "offline");
    d->o2->setExtraRequestParams(extraParams);
    
    connect(d->o2, SIGNAL(linkingSucceeded()),
            this, SLOT(slotLinkingSucceeded()));
    connect(this, SIGNAL(signalLinkingSucceeded()),
            this, SLOT(slotLinkingSucceeded()));
    connect(d->o2, SIGNAL(linkingFailed()),
            this, SLOT(slotLinkingFailed()));
    connect(d->o2, SIGNAL(openBrowser(QUrl)),
            this, SLOT(slotOpenBrowser(QUrl)));

}

GSTalkerBase::~GSTalkerBase()
{  
    if (m_reply)
        m_reply->abort();

    delete d;
}

void GSTalkerBase::link()
{
    emit signalBusy(true);
    d->o2->link();                
}

void GSTalkerBase::unlink()
{
    emit signalBusy(true);
    
    d->o2->unlink();
    
    d->settings->beginGroup(m_serviceName);
    d->settings->remove("");
    d->settings->endGroup();
}

void GSTalkerBase::slotLinkingSucceeded()
{
    if (!d->o2->linked())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "UNLINK to " << m_serviceName << " ok";
        emit signalBusy(false);
        return;
    }
    
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to " << m_serviceName << " ok";
    
    m_accessToken = d->o2->token();
    m_refreshToken = d->o2->refreshToken();
    m_bearerAccessToken = QString::fromLatin1("Bearer ") + m_accessToken;
    
    emit signalAccessTokenObtained();
}

void GSTalkerBase::slotLinkingFailed()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to " << m_serviceName << " fail";
    
    emit signalBusy(false);
}

void GSTalkerBase::slotOpenBrowser(const QUrl& url)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Open browser...";
    QDesktopServices::openUrl(url);;
}

bool GSTalkerBase::authenticated() const
{
    return d->o2->linked();
}
        //TODO: Port to O2
        /**
        * Starts authentication by opening the browser
        */
        void GSTalkerBase::doOAuth()
        {            
//             QUrl url(QString::fromLatin1("https://accounts.google.com/o/oauth2/auth"));
//             QUrlQuery urlQuery;
//             urlQuery.addQueryItem(QString::fromLatin1("scope"),         m_scope);
//             urlQuery.addQueryItem(QString::fromLatin1("redirect_uri"),  d->redirectUri);
//             urlQuery.addQueryItem(QString::fromLatin1("response_type"), d->responseType);
//             urlQuery.addQueryItem(QString::fromLatin1("client_id"),     d->apikey);
//             urlQuery.addQueryItem(QString::fromLatin1("access_type"),   QString::fromLatin1("offline"));
//             url.setQuery(urlQuery);
//             qCDebug(DIGIKAM_WEBSERVICES_LOG) << "OAuth URL: " << url;
//             QDesktopServices::openUrl(url);
// 
//             emit signalBusy(false);
// 
//             d->window = new QDialog(QApplication::activeWindow(),0);
//             d->window->setModal(true);
//             d->window->setWindowTitle(i18n("Google Drive Authorization"));
// 
//             QDialogButtonBox* const buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
//             QPushButton* const okButton       = buttonBox->button(QDialogButtonBox::Ok);
//             okButton->setDefault(true);
// 
//             d->window->connect(buttonBox, SIGNAL(accepted()),
//                             this, SLOT(slotAccept()));
// 
//             d->window->connect(buttonBox, SIGNAL(rejected()),
//                             this, SLOT(slotReject()));
// 
//             QLineEdit* const textbox      = new QLineEdit();
//             QPlainTextEdit* const infobox = new QPlainTextEdit(i18n("Please follow the instructions in the browser. "
//                                                                     "After logging in and authorizing the application, "
//                                                                     "copy the code from the browser, paste it in the "
//                                                                     "textbox below, and click OK."));
//             QVBoxLayout* const layout = new QVBoxLayout;
//             d->window->setLayout(layout);
//             infobox->setReadOnly(true);
//             layout->addWidget(infobox);
//             layout->addWidget(textbox);
//             layout->addWidget(buttonBox);
// 
//             d->window->exec();
// 
//             if (d->window->result() == QDialog::Accepted && !(textbox->text().isEmpty()))
//             {
//                 qCDebug(DIGIKAM_WEBSERVICES_LOG) << "1";
//                 d->code = textbox->text();
//             }
// 
//             if (textbox->text().isEmpty())
//             {
//                 qCDebug(DIGIKAM_WEBSERVICES_LOG) << "3";
//                 emit signalTextBoxEmpty();
//             }
// 
//             if (d->code != QString::fromLatin1("0"))
//             {
//                 getAccessToken();
//             }
            
            // for test
//             unlink();
            
            int sessionExpires = d->o2->expires();
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "current time " << QDateTime::currentMSecsSinceEpoch() / 1000;
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "expires at : " << sessionExpires;
            
            // If user has not logined yet (sessionExpires == 0), link
            // If access token has expired yet, refresh
            // Otherwise, provoke slotLinkingSucceeded
            if(sessionExpires == 0)
            {
                link();
            }
            else if(sessionExpires <= QDateTime::currentMSecsSinceEpoch() / 1000)
            {
                d->o2->refresh();
            }
            else 
            {
                emit signalLinkingSucceeded();
            }
        }

void GSTalkerBase::slotAccept()
{
    d->window->close();
    d->window->setResult(QDialog::Accepted);
}

void GSTalkerBase::slotReject()
{
    d->window->close();
    d->window->setResult(QDialog::Rejected);
}
        //TODO: Port to O2
        /**
        * Gets access token from googledrive after authentication by user
        */
        void GSTalkerBase::getAccessToken()
        {
            QUrl url(QString::fromLatin1("https://accounts.google.com/o/oauth2/token?"));
            QUrlQuery urlQuery;
            urlQuery.addQueryItem(QString::fromLatin1("scope"),         m_scope);
            urlQuery.addQueryItem(QString::fromLatin1("response_type"), d->responseType);
            urlQuery.addQueryItem(QString::fromLatin1("token_uri"),     d->tokenUrl);
            url.setQuery(urlQuery);
            QByteArray postData;
            postData = "code=";
            postData += d->code.toLatin1();
            postData += "&client_id=";
            postData += d->apikey.toLatin1();
            postData += "&client_secret=";
            postData += d->clientSecret.toLatin1();
            postData += "&redirect_uri=";
            postData += d->redirectUri.toLatin1();
            postData += "&grant_type=authorization_code";

            QNetworkRequest netRequest(url);
            netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

            m_reply = d->netMngr->post(netRequest, postData);

            d->authState = Private::GS_ACCESSTOKEN;
            m_buffer.resize(0);
            emit signalBusy(true);
        }

        /**
        * Gets access token from refresh token for handling login of user across digikam sessions
        */
        void GSTalkerBase::getAccessTokenFromRefreshToken(const QString& msg)
        {
            QUrl url(QString::fromLatin1("https://accounts.google.com/o/oauth2/token"));

            QByteArray postData;
            postData = "&client_id=";
            postData += d->apikey.toLatin1();
            postData += "&client_secret=";
            postData += d->clientSecret.toLatin1();
            postData += "&refresh_token=";
            postData += msg.toLatin1();
            postData += "&grant_type=refresh_token";

            QNetworkRequest netRequest(url);
            netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

            m_reply = d->netMngr->post(netRequest, postData);

            d->authState = Private::GS_REFRESHTOKEN;
            m_buffer.resize(0);
            emit signalBusy(true);
        }

void GSTalkerBase::slotAuthFinished(QNetworkReply* reply)
{
    if (reply != m_reply)
    {
        return;
    }

    m_reply = 0;

    if (reply->error() != QNetworkReply::NoError)
    {
        if (d->authState == Private::GS_ACCESSTOKEN)
        {
            emit signalBusy(false);
            emit signalAccessTokenFailed(reply->error(), reply->errorString());
        }
        else
        {
            emit signalBusy(false);
            QMessageBox::critical(QApplication::activeWindow(),
                                  i18n("Error"), reply->errorString());
        }

        reply->deleteLater();
        return;
    }

    m_buffer.append(reply->readAll());

    switch(d->authState)
    {
        case (Private::GS_ACCESSTOKEN):
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In Private::GS_ACCESSTOKEN";// << m_buffer;
            parseResponseAccessToken(m_buffer);
            break;
        case (Private::GS_REFRESHTOKEN):
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In Private::GS_REFRESHTOKEN" << m_buffer;
            parseResponseRefreshToken(m_buffer);
            break;
        default:
            break;
    }

    reply->deleteLater();
}

void GSTalkerBase::parseResponseAccessToken(const QByteArray& data)
{
    m_accessToken  = getValue(QString::fromUtf8(data), QString::fromLatin1("access_token"));
    m_refreshToken = getValue(QString::fromUtf8(data), QString::fromLatin1("refresh_token"));

    if (getValue(QString::fromUtf8(data), QString::fromLatin1("error")) == QString::fromLatin1("invalid_request") ||
        getValue(QString::fromUtf8(data), QString::fromLatin1("error")) == QString::fromLatin1("invalid_grant"))
    {
        doOAuth();
        return;
    }

    m_bearerAccessToken = QString::fromLatin1("Bearer ") + m_accessToken;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In parse Private::GS_ACCESSTOKEN"
                                     << m_bearerAccessToken
                                     << "  " << data;
    //emit signalAccessTokenObtained();
    emit signalRefreshTokenObtained(m_refreshToken);
}

void GSTalkerBase::parseResponseRefreshToken(const QByteArray& data)
{
    m_accessToken = getValue(QString::fromUtf8(data), QString::fromLatin1("access_token"));

    if (getValue(QString::fromUtf8(data), QString::fromLatin1("error")) == QString::fromLatin1("invalid_request") ||
        getValue(QString::fromUtf8(data), QString::fromLatin1("error")) == QString::fromLatin1("invalid_grant"))
    {
        doOAuth();
        return;
    }

    m_bearerAccessToken = QString::fromLatin1("Bearer ") + m_accessToken;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In parse Private::GS_ACCESSTOKEN"
                                     << m_bearerAccessToken
                                     << "  " << data;
    emit signalAccessTokenObtained();
}

QString GSTalkerBase::getValue(const QString& jsonStr, const QString& key) const
{
    QString token(getToken(jsonStr, key, QString::fromLatin1(",")));

    token.remove(QRegExp(QString::fromLatin1("[\"}]")));

    QStringList tokenValues(token.split(QString::fromLatin1(": ")));
    QString value;

    if (tokenValues.count() == 2)
        value = tokenValues[1].trimmed();

    return value;
}

QStringList GSTalkerBase::getParams(const QString& jsonStr, const QStringList& pathValues, const QString& key) const
{
    if (pathValues.count() == 0)
        return QStringList();

    QString token(getToken(jsonStr, pathValues[0], QString::fromLatin1("]")));

    for (int i = 1; i < pathValues.count(); ++i)
    {
        token = getToken(token, pathValues[i], QString::fromLatin1("]"));
    }

    QStringList tokens;
    QString nextToken;

    d->continuePos = 0;

    while (!(nextToken = getValue(token, key)).isEmpty())
    {
        token = token.mid(d->continuePos);
        tokens << nextToken;
    }

    return tokens;
}

QString GSTalkerBase::getToken(const QString& object, const QString& key, const QString& endDivider) const
{
    QString searchToken(QString::fromLatin1("\"") + key + QString::fromLatin1("\""));

    int beginPos(object.indexOf(searchToken));

    if (beginPos == -1)
        return QString();

    int endPos;

    if (endDivider == QString::fromLatin1(","))
        endPos = object.indexOf(endDivider, beginPos);
    else
        endPos = getTokenEnd(object, beginPos);

    int strLength = endPos - beginPos;
    QString token(object.mid(beginPos, strLength));

    if (endPos != -1)
        d->continuePos = endPos;
    else
        d->continuePos = beginPos + token.length();

    return token;
}

int GSTalkerBase::getTokenEnd(const QString& object, int beginPos) const
{
    int beginDividerPos(object.indexOf(QString::fromLatin1("["), beginPos));
    int endDividerPos(object.indexOf(QString::fromLatin1("]"),   beginPos + 1));

    while ((beginDividerPos < endDividerPos) && beginDividerPos != -1)
    {
        beginDividerPos = object.indexOf(QString::fromLatin1("["), endDividerPos);
        endDividerPos   = object.indexOf(QString::fromLatin1("]"), endDividerPos + 1);
    }

    return endDividerPos + 1;
}

} // namespace Digikam
