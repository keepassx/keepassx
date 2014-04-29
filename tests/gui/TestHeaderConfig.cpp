#include "TestHeaderConfig.h"
#include "gui/entry/EntryView.h"
#include "core/Config.h"

#include <QTest>
#include <QHeaderView>
#include <QScopedPointer>

TestHeaderConfig::TestHeaderConfig(const QString& configHeaderKeyName)
    : m_headerKeyName(configHeaderKeyName)
{    
}

void TestHeaderConfig::initTestCase()
{
    Config::createTempFileInstance();
}

void TestHeaderConfig::init()
{
    m_entryView = allocTestClass();
}

void TestHeaderConfig::testKeyNameCorrect()
{
    QString actual = m_entryView->getHeaderConfigKeyName();
    QCOMPARE(actual, m_headerKeyName);
}

void TestHeaderConfig::testNoKeyGivesNull()
{
    QVariant actual = config()->get(m_headerKeyName);
    QVERIFY(actual.isNull() );    
}

void TestHeaderConfig::setState()
{
    m_entryView->show();
    QVERIFY(m_entryView->header()->count() > 0);
    m_entryView->header()->resizeSection(0, 100);
    QByteArray state = m_entryView->header()->saveState();
    config()->set(m_headerKeyName, state);
}

void TestHeaderConfig::testRestoreState()
{
    QVariant expected = config()->get(m_headerKeyName);
    m_entryView->show();
    QByteArray actual = m_entryView->header()->saveState();
    QCOMPARE(actual, expected.toByteArray() );
    QCOMPARE(m_entryView->header()->sectionSize(0), 100);
}

void TestHeaderConfig::testChangeSaved()
{
    m_entryView->show();
    QVERIFY(m_entryView->header()->count() > 1);
    m_entryView->header()->resizeSection(1, 75);
    QByteArray expected = m_entryView->header()->saveState();
    QByteArray actualValueInConfig = 
            config()->get(m_headerKeyName).toByteArray();
    QCOMPARE(actualValueInConfig, expected);    
}

void TestHeaderConfig::testDifferentViewHasSameState()
{
    m_entryView->show();
    QByteArray expected = m_entryView->header()->saveState();
    QScopedPointer<EntryView> anotherView(allocTestClass() );

    anotherView->show();
    QCOMPARE(anotherView->header()->sectionSize(1), 75);
    QByteArray actualHeaderState = anotherView->header()->saveState();
    QCOMPARE(actualHeaderState, expected);
}

void TestHeaderConfig::cleanup()
{
    delete m_entryView;
}
