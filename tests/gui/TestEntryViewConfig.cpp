
#include "TestEntryViewConfig.h"
#include "gui/entry/EntryView.h"

#include <QTest>

TestEntryViewConfig::TestEntryViewConfig() 
    : TestHeaderConfig(EntryView::m_HEADER_CONFIG_KEY_NAME)
{
}

EntryView* TestEntryViewConfig::allocTestClass()
{
    return new EntryView();
}


QTEST_MAIN(TestEntryViewConfig)

