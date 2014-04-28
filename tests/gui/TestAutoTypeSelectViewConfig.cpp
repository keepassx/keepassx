
#include "TestAutoTypeSelectViewConfig.h"
#include "autotype/AutoTypeSelectView.h"

#include <QTest>

TestAutoTypeSelectViewConfig::TestAutoTypeSelectViewConfig() 
    : TestHeaderConfig(AutoTypeSelectView::m_HEADER_CONFIG_KEY_NAME)
{
}

EntryView* TestAutoTypeSelectViewConfig::allocTestClass()
{
    return new AutoTypeSelectView();
}


QTEST_MAIN(TestAutoTypeSelectViewConfig)
