/*
 *  mafVMETest.cpp
 *  mafResourcesTest
 *
 *  Created by Paolo Quadrani - Daniele Giunchi on 14/07/14.
 *  Copyright 2011 SCS-B3C. All rights reserved.
 *
 *  See License at: http://tiny.cc/QXJ4D
 *
 */

#include "mafResourcesTestList.h"

using namespace mafCore;
using namespace mafEventBus;
using namespace mafResources;

//------------------------------------------------------------------------------------------
/**
 Class name: testVMEPipeDataCustom
 This class implements the data pipe to be tested.
 */
class testVMEPipeDataCustom : public  mafPipeData {
    Q_OBJECT
    /// typedef macro.
    mafSuperclassMacro(mafResources::mafPipeData);

public:
    /// Object constructor.
    testVMEPipeDataCustom(const QString code_location = "");

    /// Return the string variable initialized and updated from the data pipe.
    QString pipeline() {return m_PipeLine;}

public Q_SLOTS:
    /// Allow to execute and update the pipeline when something change
    /*virtual*/ void updatePipe(double t = -1);

private:
    QString m_PipeLine; ///< Test Var.
};

testVMEPipeDataCustom::testVMEPipeDataCustom(const QString code_location) : mafPipeData(code_location), m_PipeLine("Created") {
}

void testVMEPipeDataCustom::updatePipe(double t) {
    m_PipeLine = "Updated";
    m_PipeLine.append(QString::number(t));

    Superclass::updatePipe(t);
}
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------

class testVMEConcurrentAccess : public QObject {
    Q_OBJECT
    
public:
    /// Constructor
    testVMEConcurrentAccess(mafVME *vme);
    
Q_SIGNALS:
    /// Finish the threaded elaboration.
    void finished();
    
public Q_SLOTS:
    /// Start the VME access.
    void startElaboration();
    
private:
    mafVME *m_VME; ///< input vme.
};

testVMEConcurrentAccess::testVMEConcurrentAccess(mafVME *vme) : QObject(NULL), m_VME(vme) {
    
}

void testVMEConcurrentAccess::startElaboration() {
    mafPipeData *pipe = m_VME->dataPipe();
    if (pipe == NULL) {
        m_VME->setDataPipe("testVMEPipeDataCustom");
    }
    
    QTime dieTime = QTime::currentTime().addSecs(1);
    while(QTime::currentTime() < dieTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
    }

    QVariantList b;
    int n = 0;
    while (n < 100) {
        for (int i = n; i < n+6; ++i) {
            b.append(32.5 * i);
        }
        b.clear();
        ++n;
    }
    
    QTime dieTime2 = QTime::currentTime().addSecs(1);
    while(QTime::currentTime() < dieTime2) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
    }

    m_VME->setDataPipe(NULL);

    QTime dieTime3 = QTime::currentTime().addSecs(1);
    while(QTime::currentTime() < dieTime3) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
    }

    mafMemento *m = m_VME->createMemento();
    if (m == NULL) {
        qDebug() << mafTr("Problem creating VME memento");
    }
    mafDEL(m);
    
    Q_EMIT finished();
}

//------------------------------------------------------------------------------------------


void mafVMETest::initTestCase() {
    mafMessageHandler::instance()->installMessageHandler();
    mafResourcesRegistration::registerResourcesObjects();
    
    mafRegisterObject(testVMEPipeDataCustom);
    //! <snippet>
    m_VME = mafNEW(mafResources::mafVME);
    //! </snippet>

    m_EventBus = mafEventBusManager::instance();
    m_VMEManager = mafVMEManager::instance();
    m_VMEManager->shutdown();
    
}

void mafVMETest::cleanupTestCase() {
    mafDEL(m_VME);
    mafUnregisterObject(testVMEPipeDataCustom);
    m_EventBus->notifyEvent("maf.local.resources.hierarchy.request");
    m_EventBus->shutdown();
    mafMessageHandler::instance()->shutdown();
}

void mafVMETest::mafVMEAllocationTest() {
    QVERIFY(m_VME != NULL);
    mafDataSetCollection *collection = m_VME->dataSetCollection();
    QVERIFY(collection != NULL);
}

void mafVMETest::mafVMEDataPipeTest() {
    mafPipeData *dp = m_VME->dataPipe();
    QVERIFY(dp == NULL);

    m_VME->setDataPipe("testVMEPipeDataCustom");
    dp = m_VME->dataPipe();
    QVERIFY(dp != NULL);
}

void mafVMETest::mafVMEMementoTest() {
    // Get a snapshot of the VME (its name is the empty string
    mafMemento *m = m_VME->createMemento();

    // Assign a name to the VME.
    m_VME->setObjectName("Test VME");

    // Assign the previous memento and check the name again.
    // The name should be the empty string again.
    m_VME->setMemento(m);
    QString name;
    name = m_VME->objectName();
    QVERIFY(name.isEmpty());

    // Free the memory associated with the requested memento.
    mafDEL(m);
}

void mafVMETest::mafVMEInteractorTest() {
    mafInteractor *inter = m_VME->activeInteractor();
    QVERIFY(inter != NULL);
    QString interactorType("mafResources::mafInteractorSelection");
    QString checkType(inter->metaObject()->className());
    QCOMPARE(checkType, interactorType);

    //mafInteractor *i = mafNEW(mafInteractor);
    //m_VME->setInteractor(i);

    //inter = m_VME->interactor();
    //QVERIFY(inter != NULL);

    //mafDEL(i);
}

void mafVMETest::mafVMEOutputDataTest() {
    // Ask the output data
    // datapipe exists => the output will be the output of the data pipe.
    // Default output for datapipe is the input VME itself.
    mafDataSet *output = m_VME->outputData();
    QVERIFY(output != NULL);

    // Delete the current data pipe.
    m_VME->setDataPipe(NULL);

    // This time the output data will be the
    // element of the dataset collection at the current timestamp.
    output = m_VME->outputData();
    QVERIFY(output != NULL);
    // This time an empty mafDataSet will be returned.
    QVERIFY(output->dataBoundary() == NULL);
    QVERIFY(output->dataValue() == NULL);
}

void mafVMETest::mafVMEConcurrentAccessTest() {
    QVariantList mainBounds;
    mainBounds << 0.0;
    mainBounds << 100.0;
    mainBounds << 0.0;
    mainBounds << 200.0;
    mainBounds << -20.0;
    mainBounds << 80.0;
    
    QThread thread;
    testVMEConcurrentAccess *accessor = new testVMEConcurrentAccess(m_VME);
    accessor->moveToThread(&thread);
    connect(&thread, SIGNAL(started()), accessor, SLOT(startElaboration()));
    connect(accessor, SIGNAL(finished()), &thread, SLOT(quit()), Qt::DirectConnection);
    
    thread.start();
    qDebug() << mafTr("Thread started...");
    
    mafMemento *memento = m_VME->createMemento();
    
    while (thread.isRunning()) {
        ;
    }
    
    mafDEL(memento);
    delete accessor;
    
    qDebug() << mafTr("**** End of VME concurrent access ****");
}

#include "mafVMETest.moc"
