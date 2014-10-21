/*
 *  mafQAManagerTest.cpp
 *  mafQATest
 *
 *  Created by Paolo Quadrani - Daniele Giunchi on 22/09/09.
 *  Copyright 2009 SCS-B3C. All rights reserved.
 *
 *  See Licence at: http://tiny.cc/QXJ4D
 *
 */

#include "mafQATestList.h"

#define TEST_SCRIPT_NAME "python"
#define TEST_LIBRARY_NAME "mafPluginTest.mafplugin"

using namespace mafCore;
using namespace mafQA;

void mafQAManagerTest::initTestCase() {
    m_QAManager = mafQAManager::instance();
}

void mafQAManagerTest::cleanupTestCase() {
    m_QAManager->shutdown();
}

void mafQAManagerTest::mafQAManagerAllocationTest() {
    QVERIFY(m_QAManager != NULL);
}

long int calcToProfile(long int v) {
    return (v + 5) * 32 - 15;
}

void mafQAManagerTest::profilerTest() {
    // Initialize the profiler
    m_QAManager->profilerInit();
    // Start the profiler with a string comment.
    m_QAManager->profilerStart("Profile First Run (longer)");

    // snippet of code to be monitored.
    long int m = 0;
    int i = 0;
    for(; i < 500000000; ++i) {
        m = calcToProfile(m);
    }

    // Stop the profiler to get the timing information
    m_QAManager->profilerStop();

    m_QAManager->profilerStart("Profile Second Run (shorter)");

    // snippet of code to be monitored.
    m = 0;
    i = 0;
    for(; i < 100000000; ++i) {
        m = calcToProfile(m);
    }

    // Stop the profiler to get the timing information
    m_QAManager->profilerStop();

    // Show the results on console.
    m_QAManager->profilerViewResultsOnConsole();
    // Save the results on file with the possibility to open it using default viewer.
    QString tmp = QDir::tempPath();
    tmp.append("/maf3Logs");
    QDir log_dir(tmp);
    if(!log_dir.exists()) {
        log_dir.mkpath(tmp);
    }
    tmp.append("/profilerLog.txt");

    // Save the result to the file and if uncommented the boolean value
    // open the file with the default viewer.
    m_QAManager->profilerViewResultsOnFile(tmp/*, true*/);

    // Shutdown the profiler.
    m_QAManager->profilerShutdown();
}

void mafQAManagerTest::pollUrlTest() {
    //m_QAManager->openPollUrl("http://www.biomedtown.org/biomed_town/MAF/MAF3%20Floor/Reception/");
}

#ifndef valgrind_ENABLE
void mafQAManagerTest::runPythonScriptTest() {
    //create temporary script
    QFile file("temporaryPythonScript.py");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    QTextStream out(&file);
    out << "import os" << "\n";
    out << "print \"Test Python Script\"" << "\n";

    file.close();

    qDebug() << "Asynchronous:";
    QStringList argList;
    int res = m_QAManager->runPythonScript("temporaryPythonScript.py",argList);
    QVERIFY(res == 0);

    //launch sync
    qDebug() << "Synchronous:";
    res = m_QAManager->runPythonScript("temporaryPythonScript.py",argList, true);
    QVERIFY(res == 0);

    QFile::remove("temporaryPythonScript.py");
}

void mafQAManagerTest::runScriptTest() {
    //create temporary script
    QFile file("temporaryPythonScript.py");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    QTextStream out(&file);
    out << "import os" << "\n";
    out << "print \"Test Python Script\"" << "\n";

    file.close();

    //TODO: these tests don't work
    qDebug() << "Asynchronous:";
    QStringList argList;
    argList.append("temporaryPythonScript.py");
    int res = m_QAManager->runScript(TEST_SCRIPT_NAME,argList, false);
    QCOMPARE(res, 0);

    //launch sync
    qDebug() << "Synchronous:";
    res = m_QAManager->runScript(TEST_SCRIPT_NAME,argList, true);
    QCOMPARE(res, 0);
}
#endif //valgrind_ENABLE

void mafQAManagerTest::memoryMonitorTest() {
    // Print all monitor result
    m_QAManager->enableHardDiskMonitor(true);
    m_QAManager->enableRAMMonitor(true);
    m_QAManager->printMemoryMonitorResultOnConsole();

    // Print only RAM monitor result
    m_QAManager->enableHardDiskMonitor(false);
    m_QAManager->printMemoryMonitorResultOnConsole();

    // Don't print anything
    m_QAManager->enableHardDiskMonitor(false);
    m_QAManager->enableRAMMonitor(false);
    m_QAManager->printMemoryMonitorResultOnConsole();
}

/*void mafQAManagerTest::pluginValidateTest() {
    bool res = m_QAManager->pluginValidate(TEST_LIBRARY_NAME);
    QVERIFY(res);

    res = m_QAManager->pluginValidate("FakePlugin");
    QVERIFY(!res);
  }*/


#include "mafQAManagerTest.moc"
