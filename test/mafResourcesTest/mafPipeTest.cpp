/*
 *  mafPipeTest.cpp
 *  mafResourcesTest
 *
 *  Created by Paolo Quadrani - Daniele Giunchi on 22/09/09.
 *  Copyright 2011 SCS-B3C. All rights reserved.
 *
 *  See License at: http://tiny.cc/QXJ4D
 *
 */

#include "mafResourcesTestList.h"

using namespace mafCore;
using namespace mafResources;

//------------------------------------------------------------------------------------------
/**
 Class name: testPipeCustom
 This class implements the data pipe to be tested.
 */
class  testPipeCustom : public  mafPipe {
    Q_OBJECT
    Q_PROPERTY(QString param READ param WRITE setParam)

    /// typedef macro.
    mafSuperclassMacro(mafResources::mafPipe);

public:
    /// Object constructor.
    testPipeCustom(const QString code_location = "");

    /// Return the string variable initialized and updated from the data pipe.
    QString pipeline() {return m_PipeLine;}

    /// Return the value of the parameter.
    QString param() const {return m_Parameter;};

    /// Assign the value to the custom parameter.
    void setParam(QString s) {m_Parameter = s;};

public Q_SLOTS:
    /// Allow to execute and update the pipeline when something change
    /*virtual*/ void updatePipe(double t = -1);

private:
    QString m_PipeLine; ///< Test Var.
    QString m_Parameter; ///< Test parameter.
};

testPipeCustom::testPipeCustom(const QString code_location) : mafPipe(code_location), m_PipeLine("Created") {
}

void testPipeCustom::updatePipe(double t) {
    m_PipeLine = "Updated";
    m_PipeLine.append(QString::number(t));
}
//------------------------------------------------------------------------------------------

void mafPipeTest::initTestCase() {
    mafMessageHandler::instance()->installMessageHandler();
    mafResourcesRegistration::registerResourcesObjects();
    m_Pipe = mafNEW(testPipeCustom);
}

void mafPipeTest::cleanupTestCase() {
    mafDEL(m_Pipe);
    mafMessageHandler::instance()->shutdown();
}

void mafPipeTest::mafPipeAllocationTest() {
    QVERIFY(m_Pipe != NULL);
}

void mafPipeTest::mafPipeCreationAndUpdateTest() {
    QString res("Created");
    QCOMPARE(m_Pipe->pipeline(), res);

    res = "Updated1";
    m_Pipe->updatePipe(1);
    QCOMPARE(m_Pipe->pipeline(), res);
}

void mafPipeTest::inputManagementTest() {
    mafDataSet *data1 = mafNEW(mafResources::mafDataSet);
    mafDataSet *data2 = mafNEW(mafResources::mafDataSet);

    mafVME *vme1 = mafNEW(mafResources::mafVME);
    vme1->dataSetCollection()->insertItem(data1);

    mafVME *vme2 = mafNEW(mafResources::mafVME);
    vme2->dataSetCollection()->insertItem(data2);

    m_Pipe->setInput(vme1);

    int num = m_Pipe->inputList()->length();
    QVERIFY(num == 1);

    // Get the vme1
    // Check if vme has been added
    mafVME *vme = m_Pipe->inputList()->at(0);
    QCOMPARE(vme, vme1);

    // remove the item at index 0
    m_Pipe->removeInput(vme1);
    num = m_Pipe->inputList()->length();
    QVERIFY(num == 0);

    m_Pipe->setInput(vme1);
    // substitute the vme1 with the vme2
    m_Pipe->setInput(vme2);

    // Get the vme2
    // Check if vme has been added.
    vme = m_Pipe->inputList()->at(0);
    QCOMPARE(vme, vme2);

    // try to remove a vme not present in the list
    mafMessageHandler::instance()->testSuiteLogMode(true);
    m_Pipe->removeInput(vme1);
    mafMessageHandler::instance()->testSuiteLogMode(false);
    num = m_Pipe->inputList()->length();
    QVERIFY(num == 1);

    // destroy the VME => will be removed also from the input list of the pipe.
    mafDEL(data1);
    mafDEL(data2);
    mafDEL(vme1);
    mafDEL(vme2);
}

void mafPipeTest::setParameterHashTest() {
    QVariantHash hash;
    QVariant val("test");
    hash.insert("param", val);
    m_Pipe->setParametersHash(hash);

    QString res(m_Pipe->param());
    QVERIFY(res == val.toString());
}

#include "mafPipeTest.moc"
