/*
 *  mafDataSetTest.cpp
 *  mafResourcesTest
 *
 *  Created by Paolo Quadrani - Daniele Giunchi on 14/07/14.
 *  Copyright 2009 SCS-B3C. All rights reserved.
 *
 *  See Licence at: http://tiny.cc/QXJ4D
 *
 */

#include "mafResourcesTestList.h"

using namespace mafCore;
using namespace mafEventBus;
using namespace mafResources;

//------------------------------------------------------------------------------------------

/**
 Class name: testExternalDataType
 This class implements the external data type coming from an external library.
 */
class testExternalDataType {
public:
    /// Object constructor.
    testExternalDataType(int v) : m_Value(v) {}

    /// set the new value for the class.
    void setValue(int v) {m_Value = v;}
    /// Return the inner value.
    int value() {return m_Value;}

private:
    int m_Value; ///< Test variable for external data
};

/**
 Class name: testExternalBoundary
 This class implements the external data boundary.
 */
class testExternalBoundary {
public:
    int m_Range[2]; ///< Boundary var.
};

class testDataBoundaryCustom : public mafDataBoundaryAlgorithm {
    Q_OBJECT
public:
    /// Object constructor.
    testDataBoundaryCustom(const QString code_location = "");

    /// Object destructor.
    ~testDataBoundaryCustom();

    /// Boundary algorithm.
    /*virtual*/ mafProxyInterface *calculateBoundary(mafProxyInterface *data, mafMatrix4x4 *m);

    /// Boundary algorithm.
    /*virtual*/ mafCore::mafProxyInterface *calculateBoundary(double bounds[6], mafMatrix4x4 *matrix = NULL);

    /// Return bounds of the bounding box.
    /*virtual*/ void bounds(double bounds[6]);

private:
    mafProxy<testExternalBoundary> m_Boundary; ///< boundary data.
};

testDataBoundaryCustom::testDataBoundaryCustom(const QString code_location) : mafDataBoundaryAlgorithm(code_location) {
    m_Boundary = new testExternalBoundary();
    m_Boundary->m_Range[0] = m_Boundary->m_Range[1] = 0;
}

testDataBoundaryCustom::~testDataBoundaryCustom() {
}

mafProxyInterface *testDataBoundaryCustom::calculateBoundary(mafProxyInterface *data, mafMatrix4x4 *m) {
    Q_UNUSED(m);
    mafProxy<testExternalDataType> *container = mafProxyPointerTypeCast(testExternalDataType, data);
    testExternalDataType *value = container->externalData();
    if(value != NULL) {
        int v = value->value();
        if(v < m_Boundary->m_Range[0]) m_Boundary->m_Range[0] = v;
        if(v > m_Boundary->m_Range[1]) m_Boundary->m_Range[1] = v;
    }
    return &m_Boundary;
}

void testDataBoundaryCustom::bounds(double bounds[6]) {
    Q_UNUSED(bounds);
}

mafCore::mafProxyInterface *testDataBoundaryCustom::calculateBoundary(double bounds[6], mafMatrix4x4 *matrix) {
    Q_UNUSED(bounds);
    Q_UNUSED(matrix);
    return NULL;
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
/**
 Class name: testDataSetObserver
 This class implements the observer object for mafDataSet.
 It track back all the changes made into the mafDataSet.
 */
class testDataSetObserver : public mafObjectBase {
    Q_OBJECT
    /// typedef macro.
    mafSuperclassMacro(mafCore::mafObject);

public:
    /// Object constructor.
    testDataSetObserver(const QString code_location = "");

public Q_SLOTS:
    void dataDidConnected();
    void dataDidDisconnected();
};

testDataSetObserver::testDataSetObserver(QString code_location) : mafObjectBase(code_location) {
}

void testDataSetObserver::dataDidConnected() {
    qDebug() << mafTr("Dataset I'm observing has been connected!!");
}

void testDataSetObserver::dataDidDisconnected() {
    qDebug() << mafTr("Dataset I'm observing has been disconnected!!");
}

void mafDataSetTest::initTestCase() {
    mafMessageHandler::instance()->installMessageHandler();
    mafEventBusManager::instance();

    //! <snippet>
    m_DataSet = mafNEW(mafResources::mafDataSet);
    //! </snippet>

    // instantiate the observer and connect it to the event bus.
    m_Observer = mafNEW(testDataSetObserver);
}

void mafDataSetTest::cleanupTestCase() {
    // Free allocated memory
    mafDEL(m_DataSet);
    mafDEL(m_Observer);
    // Shutdown eventbus singleton and core singletons.
    mafEventBusManager::instance()->shutdown();
    mafMessageHandler::instance()->shutdown();
}

void mafDataSetTest::mafDataSetAllocationTest() {
    QVERIFY(m_DataSet != NULL);
}

void mafDataSetTest::mafDataSetValueTest() {
    // Create the container with the external data type.
    mafProxy<testExternalDataType> container;
    container = new testExternalDataType(3);

    // ... and assign it to the mafDataSet; should start the connected event.
    m_DataSet->setDataValue(&container);

    // ... and now disconnect external data from mafDataSet
    // disconnect event should be raised.
    m_DataSet->setDataValue(NULL);

    testExternalDataType *external_data = container;
    delete external_data;
}

void mafDataSetTest::mafDataSetBoundaryTest() {
    // Create the boundary class and assign it to the mafDataSet.
    // The mafDataSet will manage the deletation of the algorithm when a new algorithm
    // will be assigned or when the mafDataSet itself will die.
    testDataBoundaryCustom *boundaryAlgorithm = mafNEW(testDataBoundaryCustom);
    m_DataSet->setBoundaryAlgorithm(boundaryAlgorithm);

    // Create the custom data and asign it to the mafDataSet.
    mafProxy<testExternalDataType> container;
    container = new testExternalDataType(3);
    m_DataSet->setDataValue(&container);

    // Try to setDataValue again, nothing should happen..
    m_DataSet->setDataValue(&container);

    // Retrieve the boundary (should be 0 - 3).
    mafProxyInterface *boundary_container = m_DataSet->dataBoundary();
    mafProxy<testExternalBoundary> *boundary = mafProxyPointerTypeCast(testExternalBoundary, boundary_container);
    int r0 = (*boundary)->m_Range[0];
    int r1 = (*boundary)->m_Range[1];
    QVERIFY(r0 == 0);
    QVERIFY(r1 == 3);

    testExternalDataType *external_data = container;
    delete external_data;
}

#include "mafDataSetTest.moc"
