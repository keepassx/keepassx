/*
 *  Copyright (C) 2015 Enrico Mariotti <enricomariotti@yahoo.it>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "TestCsvParser.h"
#include <QTest>

//TODO: http://stackoverflow.com/questions/31001398/qt-run-unit-tests-from-multiple-test-classes-and-summarize-the-output-from-all
//useful to show CR/LF cat -v /tmp/keepassXn94do1x.csv

QTEST_GUILESS_MAIN(TestCsvParser)

void TestCsvParser::initTestCase()
{
    parser = new CsvParser();
}

void TestCsvParser::cleanupTestCase()
{
    delete parser;
}

void TestCsvParser::init()
{
    file.setFileName("/tmp/keepassXn94do1x.csv");
    if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
        QFAIL("Cannot open file!");
    parser->setBackslashSyntax(false);
    parser->setComment('#');
    parser->setFieldSeparator(',');
    parser->setTextQualifier(QChar('"'));
}

void TestCsvParser::cleanup()
{
    file.close();
}

/****************** TEST CASES ******************/
void TestCsvParser::testMissingQuote() {
    parser->setTextQualifier(':');
    QTextStream out(&file);
    out << "A,B\n:BM,1";
    QEXPECT_FAIL("", "Bad format", Continue);
    QVERIFY(parser->parse(&file));
    t = parser->getCsvTable();
    QWARN(parser->getStatus().toLatin1());
}

void TestCsvParser::testMalformed() {
    parser->setTextQualifier(':');
    QTextStream out(&file);
    out << "A,B,C\n:BM::,1,:2:";
    QEXPECT_FAIL("", "Bad format", Continue);
    QVERIFY(parser->parse(&file));
    t = parser->getCsvTable();
    QWARN(parser->getStatus().toLatin1());
}

void TestCsvParser::testBackslashSyntax() {
    parser->setBackslashSyntax(true);
    parser->setTextQualifier(QChar('X'));
    QTextStream out(&file);
    //attended result: one"\t\"wo
    out << "Xone\\\"\\\\t\\\\\\\"w\noX\n"
        << "X13X,X2\\X,X,\"\"3\"X\r"
        << "3,X\"4\"X,,\n"
        << "XX\n"
        << "\\";
    QVERIFY(parser->parse(&file));
    t = parser->getCsvTable();
    QVERIFY(t.at(0).at(0) == "one\"\\t\\\"w\no");
    QVERIFY(t.at(1).at(0) == "13");
    QVERIFY(t.at(1).at(1) == "2X,");
    QVERIFY(t.at(1).at(2) == "\"\"3\"X");
    QVERIFY(t.at(2).at(0) == "3");
    QVERIFY(t.at(2).at(1) == "\"4\"");
    QVERIFY(t.at(2).at(2) == "");
    QVERIFY(t.at(2).at(3) == "");
    QVERIFY(t.at(3).at(0) == "\\");
    QVERIFY(t.size() == 4);
}

void TestCsvParser::testQuoted() {
    QTextStream out(&file);
    out << "ro,w,\"end, of \"\"\"\"\"\"row\"\"\"\"\"\n"
        << "2\n";
    QVERIFY(parser->parse(&file));
    t = parser->getCsvTable();
    QVERIFY(t.at(0).at(0) == "ro");
    QVERIFY(t.at(0).at(1) == "w");
    QVERIFY(t.at(0).at(2) == "end, of \"\"\"row\"\"");
    QVERIFY(t.at(1).at(0) == "2");
    QVERIFY(t.size() == 2);
}

void TestCsvParser::testEmptySimple() {
    QTextStream out(&file);
    out <<"";
    QVERIFY(parser->parse(&file));
    t = parser->getCsvTable();
    QVERIFY(t.size() == 0);
}

void TestCsvParser::testEmptyQuoted() {
    QTextStream out(&file);
    out <<"\"\"";
    QVERIFY(parser->parse(&file));
    t = parser->getCsvTable();
    QVERIFY(t.size() == 0);
}

void TestCsvParser::testEmptyNewline() {
    QTextStream out(&file);
    out <<"\"\n\"";
    QVERIFY(parser->parse(&file));
    t = parser->getCsvTable();
    QVERIFY(t.size() == 0);
}

void TestCsvParser::testEmptyFile()
{
    QVERIFY(parser->parse(&file));
    t = parser->getCsvTable();
    QVERIFY(t.size() == 0);
}

void TestCsvParser::testNewline()
{
    QTextStream out(&file);
    out << "1,2\n\n\n";
    QVERIFY(parser->parse(&file));
    t = parser->getCsvTable();
    QVERIFY(t.size() == 1);
    QVERIFY(t.at(0).at(0) == "1");
    QVERIFY(t.at(0).at(1) == "2");
}

void TestCsvParser::testCR()
{
    QTextStream out(&file);
    out << "1,2\r3,4";
    QVERIFY(parser->parse(&file));
    t = parser->getCsvTable();
    QVERIFY(t.size() == 2);
    QVERIFY(t.at(0).at(0) == "1");
    QVERIFY(t.at(0).at(1) == "2");
    QVERIFY(t.at(1).at(0) == "3");
    QVERIFY(t.at(1).at(1) == "4");
}

void TestCsvParser::testLF()
{
    QTextStream out(&file);
    out << "1,2\n3,4";
    QVERIFY(parser->parse(&file));
    t = parser->getCsvTable();
    QVERIFY(t.size() == 2);
    QVERIFY(t.at(0).at(0) == "1");
    QVERIFY(t.at(0).at(1) == "2");
    QVERIFY(t.at(1).at(0) == "3");
    QVERIFY(t.at(1).at(1) == "4");
}

void TestCsvParser::testCRLF()
{
    QTextStream out(&file);
    out << "1,2\r\n3,4";
    QVERIFY(parser->parse(&file));
    t = parser->getCsvTable();
    QVERIFY(t.size() == 2);
    QVERIFY(t.at(0).at(0) == "1");
    QVERIFY(t.at(0).at(1) == "2");
    QVERIFY(t.at(1).at(0) == "3");
    QVERIFY(t.at(1).at(1) == "4");
}

void TestCsvParser::testComments()
{
    QTextStream out(&file);
    out << "  #one\n"
        << " \t  # two, three \r\n"
        << " #, sing\t with\r"
        << " #\t  me!\n"
        << "useful,text #1!";
    QVERIFY(parser->parse(&file));
    t = parser->getCsvTable();
    QVERIFY(t.size() == 1);
    QVERIFY(t.at(0).at(0) == "useful");
    QVERIFY(t.at(0).at(1) == "text #1!");
}

void TestCsvParser::testColumns() {
    QTextStream out(&file);
    out << "1,2\n"
        << ",,,,,,,,,a\n"
        << "a,b,c,d\n";
    QVERIFY(parser->parse(&file));
    t = parser->getCsvTable();
    QVERIFY(parser->getCsvCols() == 10);
}

void TestCsvParser::testSimple() {
    QTextStream out(&file);
    out << ",,2\r,2,3\n"
        << "A,,B\"\n"
        << " ,,\n";
    QVERIFY(parser->parse(&file));
    t = parser->getCsvTable();
    QVERIFY(t.size() == 4);
    QVERIFY(t.at(0).at(0) == "");
    QVERIFY(t.at(0).at(1) == "");
    QVERIFY(t.at(0).at(2) == "2");
    QVERIFY(t.at(1).at(0) == "");
    QVERIFY(t.at(1).at(1) == "2");
    QVERIFY(t.at(1).at(2) == "3");
    QVERIFY(t.at(2).at(0) == "A");
    QVERIFY(t.at(2).at(1) == "");
    QVERIFY(t.at(2).at(2) == "B\"");
    QVERIFY(t.at(3).at(0) == " ");
    QVERIFY(t.at(3).at(1) == "");
    QVERIFY(t.at(3).at(2) == "");
}

void TestCsvParser::testSeparator() {
    parser->setFieldSeparator('\t');
    QTextStream out(&file);
    out << "\t\t2\r\t2\t3\n"
        << "A\t\tB\"\n"
        << " \t\t\n";
    QVERIFY(parser->parse(&file));
    t = parser->getCsvTable();
    QVERIFY(t.size() == 4);
    QVERIFY(t.at(0).at(0) == "");
    QVERIFY(t.at(0).at(1) == "");
    QVERIFY(t.at(0).at(2) == "2");
    QVERIFY(t.at(1).at(0) == "");
    QVERIFY(t.at(1).at(1) == "2");
    QVERIFY(t.at(1).at(2) == "3");
    QVERIFY(t.at(2).at(0) == "A");
    QVERIFY(t.at(2).at(1) == "");
    QVERIFY(t.at(2).at(2) == "B\"");
    QVERIFY(t.at(3).at(0) == " ");
    QVERIFY(t.at(3).at(1) == "");
    QVERIFY(t.at(3).at(2) == "");
}

void TestCsvParser::testMultiline()
{
    parser->setTextQualifier(QChar(':'));
    QTextStream out(&file);
    out << ":1\r\n2a::b:,:3\r4:\n"
        << "2\n";
    QVERIFY(parser->parse(&file));
    t = parser->getCsvTable();
    QVERIFY(t.at(0).at(0) == "1\n2a:b");
    QVERIFY(t.at(0).at(1) == "3\n4");
    QVERIFY(t.at(1).at(0) == "2");
    QVERIFY(t.size() == 2);
}

void TestCsvParser::testEmptyReparsing()
{
    parser->parse(nullptr);
    QVERIFY(parser->reparse());
    t = parser->getCsvTable();
    QVERIFY(t.size() == 0);
}

void TestCsvParser::testReparsing()
{
    QTextStream out(&file);
    out << ":te\r\nxt1:,:te\rxt2:,:end of \"this\n string\":\n"
        << "2\n";
    QVERIFY(parser->parse(&file));
    t = parser->getCsvTable();

    QEXPECT_FAIL("", "Wrong qualifier", Continue);
    QVERIFY(t.at(0).at(0) == "te\nxt1");

    parser->setTextQualifier(QChar(':'));

    QVERIFY(parser->reparse());
    t = parser->getCsvTable();
    QVERIFY(t.at(0).at(0) == "te\nxt1");
    QVERIFY(t.at(0).at(1) == "te\nxt2");
    QVERIFY(t.at(0).at(2) == "end of \"this\n string\"");
    QVERIFY(t.at(1).at(0) == "2");
    QVERIFY(t.size() == 2);
}

void TestCsvParser::testQualifier() {
    parser->setTextQualifier(QChar('X'));
    QTextStream out(&file);
    out << "X1X,X2XX,X,\"\"3\"\"\"X\r"
        << "3,X\"4\"X,,\n";
    QVERIFY(parser->parse(&file));
    t = parser->getCsvTable();
    QVERIFY(t.size() == 2);
    QVERIFY(t.at(0).at(0) == "1");
    QVERIFY(t.at(0).at(1) == "2X,");
    QVERIFY(t.at(0).at(2) == "\"\"3\"\"\"X");
    QVERIFY(t.at(1).at(0) == "3");
    QVERIFY(t.at(1).at(1) == "\"4\"");
    QVERIFY(t.at(1).at(2) == "");
    QVERIFY(t.at(1).at(3) == "");
}

void TestCsvParser::testUnicode() {
    //QString m("Texte en fran\u00e7ais");
    //CORRECT QString g("\u20AC");
    //CORRECT QChar g(0x20AC);
    //ERROR QChar g("\u20AC");
    parser->setFieldSeparator(QChar('A'));
    QTextStream out(&file);
    out << QString("€1A2śA\"3śAż\"Ażac");

    QVERIFY(parser->parse(&file));
    t = parser->getCsvTable();
    QVERIFY(t.size() == 1);
    QVERIFY(t.at(0).at(0) == "€1");
    QVERIFY(t.at(0).at(1) == "2ś");
    QVERIFY(t.at(0).at(2) == "3śAż");
    QVERIFY(t.at(0).at(3) == "żac");
}

void TestCsvParser::testKeepass() {
    file.close();
    file.setFileName("../../res/keepass.csv");
    if (!file.open(QIODevice::ReadWrite))
         QFAIL("Cannot open file!");
    parser->setBackslashSyntax(true);
    QVERIFY(parser->parse(&file));
    t = parser->getCsvTable();
    QVERIFY(t.size() == 3);
    QVERIFY(t.at(1).at(1) == "2\"");
    QVERIFY(t.at(1).at(4) == "some notes...\n\nksjdkj@jdjd.com\n");
    QVERIFY(t.at(2).at(1) == "€èéç");
}

void TestCsvParser::dumpRow(csvtable, int) {}
/*
void TestCsvParser::dumpRow(csvtable table, int row) {
    if ( (row < 0) || (row >= table.size())) {
        qDebug() << QString("Error, nonexistent row %1").arg(row);
        return;
    }
    csvrow::const_iterator it = table.at(row).constBegin();
    qDebug() <<"@row" <<row <<" ";
    for (; it != table.at(row).constEnd(); ++it)
        qDebug() <<"|" <<*it;
}
*/

/*

void TestCsvParser::testExport()
{
    Group* groupRoot = m_db->rootGroup();
    Group* group= new Group();
    group->setName("Test Group Name");
    group->setParent(groupRoot);
    Entry* entry = new Entry();
    entry->setGroup(group);
    entry->setTitle("Test Entry Title");
    entry->setUsername("Test Username");
    entry->setPassword("Test Password");
    entry->setUrl("http://test.url");
    entry->setNotes("Test Notes");

    QBuffer buffer;
    QVERIFY(buffer.open(QIODevice::ReadWrite));
    m_csvExporter->exportDatabase(&buffer, m_db);

    QString expectedResult = QString().append(ExpectedHeaderLine).append("\"Test Group Name\",\"Test Entry Title\",\"Test Username\",\"Test Password\",\"http://test.url\",\"Test Notes\"\n");

    QCOMPARE(QString::fromUtf8(buffer.buffer().constData()), expectedResult);
}

void TestCsvParser::testEmptyDatabase()
{
    QBuffer buffer;
    QVERIFY(buffer.open(QIODevice::ReadWrite));
    m_csvExporter->exportDatabase(&buffer, m_db);

    QCOMPARE(QString::fromUtf8(buffer.buffer().constData()), ExpectedHeaderLine);
}

void TestCsvParser::testNestedGroups()
{
    Group* groupRoot = m_db->rootGroup();
    Group* group= new Group();
    group->setName("Test Group Name");
    group->setParent(groupRoot);
    Group* childGroup= new Group();
    childGroup->setName("Test Sub Group Name");
    childGroup->setParent(group);
    Entry* entry = new Entry();
    entry->setGroup(childGroup);
    entry->setTitle("Test Entry Title");

    QBuffer buffer;
    QVERIFY(buffer.open(QIODevice::ReadWrite));
    m_csvExporter->exportDatabase(&buffer, m_db);

    QCOMPARE(QString::fromUtf8(buffer.buffer().constData()), QString().append(ExpectedHeaderLine).append("\"Test Group Name/Test Sub Group Name\",\"Test Entry Title\",\"\",\"\",\"\",\"\"\n"));
}
*/
