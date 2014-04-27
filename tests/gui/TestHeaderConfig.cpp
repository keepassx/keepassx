#include "TestHeaderConfig.h"
#include "gui/entry/EntryView.h"
#include "core/Config.h"

#include <QTest>
#include <QHeaderView>

void TestHeaderConfig::initTestCase()
{
    Config::createTempFileInstance();
}

void TestHeaderConfig::init()
{
    m_entryView = new EntryView();
}

void TestHeaderConfig::testKeyNameCorrect()
{
    QString actual = m_entryView->getHeaderConfigKeyName();
    QCOMPARE(actual, EntryView::m_HEADER_CONFIG_KEY_NAME);
}

void TestHeaderConfig::testNoKeyGivesNull()
{
    QVariant actual = config()->get(EntryView::m_HEADER_CONFIG_KEY_NAME);
    QVERIFY(actual.isNull() );    
}

void TestHeaderConfig::setState()
{
    m_entryView->show();
    QVERIFY(m_entryView->header()->count() > 0);
    m_entryView->header()->resizeSection(0, 100);
    QByteArray state = m_entryView->header()->saveState();
    config()->set(EntryView::m_HEADER_CONFIG_KEY_NAME, state);
}

void TestHeaderConfig::testRestoreState()
{
    QVariant expected = config()->get(EntryView::m_HEADER_CONFIG_KEY_NAME);
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
            config()->get(EntryView::m_HEADER_CONFIG_KEY_NAME).toByteArray();
    QCOMPARE(actualValueInConfig, expected);    
}

void TestHeaderConfig::testDifferentViewHasSameState()
{
    m_entryView->show();
    QByteArray expected = m_entryView->header()->saveState();
    EntryView anotherView;
    
    anotherView.show();
    QCOMPARE(anotherView.header()->sectionSize(1), 75);
    QByteArray actualHeaderState = anotherView.header()->saveState();
    QCOMPARE(actualHeaderState, expected);
}

void TestHeaderConfig::cleanup()
{
    delete m_entryView;
}

QTEST_MAIN(TestHeaderConfig)
