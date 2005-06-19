////////////////////////////////////////////////////////////////////////////////
//
//    Copyright (C) 2005 Tom Albers <tomalbers@kde.nl>
//    Copyright (C) 2005 Renchi Raju <renchi@pooh.tam.uiuc.edu>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//////////////////////////////////////////////////////////////////////////////

/** @file searchwidgets.cpp */

#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qgroupbox.h>
#include <qvgroupbox.h>
#include <qlayout.h>
#include <qdatetime.h>

#include <klocale.h>
#include <kdateedit.h>
#include <kdebug.h>
#include <kurl.h>
#include <kdialog.h>

#include <album.h>
#include <albuminfo.h>
#include <albummanager.h>
#include <searchwidgets.h>

static struct
{
    QString keyText;
    QString key;
    SearchAdvancedRule::valueWidgetTypes cat;
}
RuleKeyTable[] =
{
    { "Album",            "album",           SearchAdvancedRule::ALBUMS },
    { "Album Name",       "albumname",       SearchAdvancedRule::LINEEDIT },
    { "Album Caption",    "albumcaption",    SearchAdvancedRule::LINEEDIT },
    { "Album Collection", "albumcollection", SearchAdvancedRule::LINEEDIT },
    { "Tag",              "tag",             SearchAdvancedRule::TAGS },
    { "Tag Name",         "tagname",         SearchAdvancedRule::LINEEDIT },
    { "Image Name",       "imagename",       SearchAdvancedRule::LINEEDIT },
    { "Image Date",       "imagedate",       SearchAdvancedRule::DATE     },
    { "Image Caption",    "imagecaption",    SearchAdvancedRule::LINEEDIT },
    { "Keyword",          "keyword",         SearchAdvancedRule::LINEEDIT },
};
static const int RuleKeyTableCount = 10;

static struct
{
    QString keyText;
    QString key;
    SearchAdvancedRule::valueWidgetTypes cat;
}

RuleOpTable[] =
{
    { "contains",           "LIKE",         SearchAdvancedRule::LINEEDIT },
    { "does not contain",   "NLIKE",        SearchAdvancedRule::LINEEDIT },
    { "equals",             "EQ",           SearchAdvancedRule::LINEEDIT },
    { "does not equal",     "NE",           SearchAdvancedRule::LINEEDIT },
    { "equals",             "EQ",           SearchAdvancedRule::ALBUMS },
    { "does not equal",     "NE",           SearchAdvancedRule::ALBUMS },
    { "equals",             "EQ",           SearchAdvancedRule::TAGS },
    { "does not equal",     "NE",           SearchAdvancedRule::TAGS },
    { ">",                  "GT",           SearchAdvancedRule::DATE },
    { "<",                  "LT",           SearchAdvancedRule::DATE },
    { "=",                  "EQ",           SearchAdvancedRule::DATE },
};
static const int RuleOpTableCount = 11;



SearchAdvancedRule::SearchAdvancedRule(QWidget* parent,
                                       SearchAdvancedRule::Option option)
    : SearchAdvancedBase(SearchAdvancedBase::RULE)
{
    m_box = new QVBox(parent);
    m_box->layout()->setSpacing( KDialog::spacingHint() );
    m_box->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );

    m_optionsBox   = 0;
    m_option       = option;
    if (option != NONE)
    {
        m_optionsBox  = new QHBox( m_box );
        QLabel* label = new QLabel( i18n(option == AND ? "As well as" : "Or"),
                                    m_optionsBox);
        QFrame* hline = new QFrame( m_optionsBox );
        hline->setFrameStyle( QFrame::HLine|QFrame::Sunken );
        label->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
        hline->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
    }

    m_hbox = new QWidget( m_box );
    m_hbox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );

    m_key = new QComboBox( m_hbox, "key" );
    for (int i=0; i< RuleKeyTableCount; i++)
        m_key->insertItem( RuleKeyTable[i].keyText, i );

    m_operator = new QComboBox( m_hbox );
    m_operator->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );

    m_valueBox = new QHBox( m_hbox );
    m_widgetType = NOWIDGET;

    slotKeyChanged( 0 );
    m_check = new QCheckBox( m_hbox );

    QHBoxLayout* m_hboxLayout = new QHBoxLayout( m_hbox );

    m_hboxLayout->setSpacing( KDialog::spacingHint() );
    m_hboxLayout->addWidget( m_key );
    m_hboxLayout->addWidget( m_operator );
    m_hboxLayout->addWidget( m_valueBox );
    m_hboxLayout->addStretch( 5 );
    m_hboxLayout->addWidget( m_check );

    m_box->show();

    connect( m_key, SIGNAL( activated(int) ),
             this, SLOT(slotKeyChanged(int)));
    connect( m_key, SIGNAL( activated(int) ),
             this, SIGNAL( signalPropertyChanged() ));
    connect( m_operator, SIGNAL( activated(int) ),
             this, SIGNAL( signalPropertyChanged() ));
    connect( m_check, SIGNAL( toggled( bool ) ),
             this, SIGNAL( signalBaseItemToggled() ));
}

void SearchAdvancedRule::setValues(const KURL& url)
{
    if (url.isEmpty())
        return;

    // set the key widget
    for (int i=0; i< RuleKeyTableCount; i++)
        if (RuleKeyTable[i].key == url.queryItem("1.key"))
        {
            m_key->setCurrentText( RuleKeyTable[i].keyText );
        }

    // set the operator and the last widget
    slotKeyChanged( m_key->currentItem() );
    for (int i=0; i< RuleOpTableCount; i++)
        if ( RuleOpTable[i].key == url.queryItem("1.op") &&
             RuleOpTable[i].cat == m_widgetType )
        {
            m_operator->setCurrentText( RuleOpTable[i].keyText );
        }

    // Set the value for the last widget.
    QString value = url.queryItem("1.val");
    if (m_widgetType == LINEEDIT)
        m_lineEdit->setText( value );

    if (m_widgetType == DATE)
        m_dateEdit->setDate( QDate::fromString( value, Qt::ISODate) );

    if (m_widgetType == TAGS || m_widgetType == ALBUMS)
    {
        bool ok;
        int  num = value.toInt(&ok);
        if (ok)
        {
            QMapIterator<int,int> it;
            for (it = m_itemsIndexIDMap.begin() ; it != m_itemsIndexIDMap.end(); ++it)
                if (it.data() == num)
                    m_valueCombo->setCurrentItem( it.key() );
        }
    }


}


SearchAdvancedRule::~SearchAdvancedRule()
{
    delete m_box;
}

void SearchAdvancedRule::slotKeyChanged(int id)
{
    QString currentOperator = m_operator->currentText();
    valueWidgetTypes currentType = m_widgetType;

    m_operator->clear();
    m_widgetType = RuleKeyTable[id].cat;

    for (int i=0; i< RuleOpTableCount; i++)
        if ( RuleOpTable[i].cat == m_widgetType )
    {
        m_operator->insertItem( RuleOpTable[i].keyText );

        if ( currentOperator == RuleOpTable[i].key )
            m_operator->setCurrentText( currentOperator );
    }
    m_operator->adjustSize();
    setValueWidget( currentType, m_widgetType );
}

void SearchAdvancedRule::setValueWidget(
        valueWidgetTypes oldType,
        valueWidgetTypes newType)
{
    if (oldType == newType)
        return;

    if (m_lineEdit && oldType == LINEEDIT)
        delete m_lineEdit;

    if (m_dateEdit && oldType == DATE)
        delete m_dateEdit;

    if (m_valueCombo && (oldType == ALBUMS || oldType == TAGS))
        delete m_valueCombo;

    if (newType == DATE)
    {
        m_dateEdit = new KDateEdit( m_valueBox,"datepicker");
        m_dateEdit->show();

        connect( m_dateEdit, SIGNAL( dateChanged(const QDate& ) ),
                 this, SIGNAL(signalPropertyChanged()));
    }
    else if (newType == LINEEDIT)
    {
        m_lineEdit = new QLineEdit( m_valueBox, "lineedit" );
        m_lineEdit->show();

        connect( m_lineEdit, SIGNAL ( textChanged(const QString&) ),
                 this, SIGNAL(signalPropertyChanged()));

    }
    else if (newType == ALBUMS)
    {
        m_valueCombo = new QComboBox( m_valueBox, "albumscombo" );

        AlbumManager* aManager = AlbumManager::instance();
        AlbumList aList = aManager->allPAlbums();

        m_itemsIndexIDMap.clear();
        int index = 0;
        for ( AlbumList::Iterator it = aList.begin();
              it != aList.end(); ++it )
        {
            PAlbum *album = (PAlbum*)(*it);
            if ( !album->isRoot() )
            {
                m_valueCombo->insertItem( album->url(), index );
                m_itemsIndexIDMap.insert(index, album->id());
                index++;
            }
        }

        m_valueCombo->show();

        connect( m_valueCombo, SIGNAL( activated(int) ),
                 this, SIGNAL( signalPropertyChanged() ));
    }
    else if (newType == TAGS)
    {
        m_valueCombo = new QComboBox( m_valueBox, "tagscombo" );
        m_valueCombo->setMaxCount(10000);

        AlbumManager* aManager = AlbumManager::instance();
        AlbumList tList = aManager->allTAlbums();

        m_itemsIndexIDMap.clear();
        int index = 0;
        for ( AlbumList::Iterator it = tList.begin();
              it != tList.end(); ++it )
        {
            TAlbum *album = (TAlbum*)(*it);
            if ( !album->isRoot() )
            {
                m_valueCombo->insertItem( album->url(), index );
                m_itemsIndexIDMap.insert( index, album->id() );
                ++index;
            }
        }

        m_valueCombo->show();

        connect( m_valueCombo, SIGNAL( activated(int) ),
                 this, SIGNAL( signalPropertyChanged() ));
    }
}

QString SearchAdvancedRule::urlKey() const
{
    return RuleKeyTable[m_key->currentItem()].key;
}

QString SearchAdvancedRule::urlOperator() const
{
    QString string;

    int countItems = 0;
    for (int i=0; i< RuleOpTableCount; i++)
        if ( RuleOpTable[i].cat == m_widgetType )
        {
            if ( countItems == m_operator->currentItem() )
                string = RuleOpTable[i].key;
            ++countItems;
        }

    return string;
}

QString SearchAdvancedRule::urlValue() const
{
    QString string;

    if (m_widgetType == LINEEDIT)
        string = m_lineEdit->text() ;

    else if (m_widgetType == DATE)
        string = m_dateEdit->date().toString(Qt::ISODate) ;

    else if (m_widgetType == TAGS || m_widgetType == ALBUMS)
        string = QString::number(m_itemsIndexIDMap[ m_valueCombo->currentItem() ]);

    return string;
}

QWidget* SearchAdvancedRule::widget() const
{
    return m_box;
}

bool SearchAdvancedRule::isChecked() const
{
    return (m_check && m_check->isChecked());    
}

void SearchAdvancedRule::addOption(Option option)
{
    if (option == NONE)
    {
        removeOption();
        return;
    }

    m_box->layout()->remove(m_hbox);

    m_optionsBox = new QHBox(m_box);
    new QLabel(i18n( option == AND ? "As well as" : "Or" ),
               m_optionsBox);
    QFrame* hline = new QFrame(m_optionsBox);
    hline->setFrameStyle(QFrame::HLine|QFrame::Sunken);
    hline->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_optionsBox->show();

    m_box->layout()->add(m_hbox);

    m_option =  option;
}

void SearchAdvancedRule::removeOption()
{
    m_option = NONE;
    delete m_optionsBox;
    m_optionsBox = 0;
}

void SearchAdvancedRule::addCheck()
{
    m_check = new QCheckBox(m_hbox);
    m_check->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_check->show();

    connect( m_check, SIGNAL( toggled( bool ) ),
             this, SIGNAL( signalBaseItemToggled() ));
}

void SearchAdvancedRule::removeCheck()
{
    delete m_check;
    m_check = 0;
}

SearchAdvancedGroup::SearchAdvancedGroup(QWidget* parent)
    : SearchAdvancedBase(SearchAdvancedBase::GROUP)
{
    m_box      = new QHBox(parent);
    m_box->layout()->setSpacing(KDialog::spacingHint());
    m_groupbox = new QVGroupBox(m_box);
    m_check    = new QCheckBox(m_box);
    m_option   = SearchAdvancedRule::NONE;
    m_box->show();

    connect( m_check, SIGNAL( toggled( bool ) ),
             this, SIGNAL( signalBaseItemToggled() ));
}

SearchAdvancedGroup::~SearchAdvancedGroup()
{
    delete m_box;
}

QWidget* SearchAdvancedGroup::widget() const
{
    return m_box;
}

bool SearchAdvancedGroup::isChecked() const
{
    return m_check->isChecked();
}

void SearchAdvancedGroup::addRule(SearchAdvancedRule* rule)
{
    if (m_childRules.isEmpty() &&
        rule->option() != SearchAdvancedRule::NONE)
    {
        // this is the first rule being inserted in this group.
        // get its option and remove ite option
        addOption(rule->option());
        rule->removeOption();
    }

    rule->removeCheck();
    
    m_childRules.append(rule);
    rule->widget()->reparent(m_groupbox, QPoint(0,0));
    rule->widget()->show();
}

void SearchAdvancedGroup::removeRules()
{
    typedef QValueList<SearchAdvancedRule*> RuleList;

    for (RuleList::iterator it = m_childRules.begin();
         it != m_childRules.end(); ++it)
    {
        SearchAdvancedRule* rule = (SearchAdvancedRule*)(*it);
        if (it == m_childRules.begin())
        {
            rule->addOption(m_option);
        }
        rule->addCheck();

        rule->widget()->reparent((QWidget*)m_box->parent(), QPoint(0,0));
        rule->widget()->show();
    }

    m_childRules.clear();
    removeOption();
}

QValueList<SearchAdvancedRule*> SearchAdvancedGroup::childRules() const
{
    return m_childRules;
}

void SearchAdvancedGroup::addOption(Option option)
{
    m_option = option;
    m_groupbox->setTitle(
            i18n(m_option == SearchAdvancedRule::AND ? "As well as" : "Or"));
}

void SearchAdvancedGroup::removeOption()
{
    m_option = NONE;
    m_groupbox->setTitle("");
}

#include "searchwidgets.moc"
