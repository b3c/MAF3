/*
 *  mafResourceTest.cpp
 *  mafResourcesTest
 *
 *  Created by Paolo Quadrani - Daniele Giunchi on 22/09/09.
 *  Copyright 2009 SCS-B3C. All rights reserved.
 *
 *  See Licence at: http://tiny.cc/QXJ4D
 *
 */

#include "mafResourcesTestList.h"

using namespace mafCore;
using namespace mafEventBus;
using namespace mafResources;

void mafResourceTest::initTestCase() {
    mafMessageHandler::instance()->installMessageHandler();
    mafEventBusManager::instance();
    m_Resource = mafNEW(mafResources::mafResource);
}

/// Cleanup test variables memory allocation.
void mafResourceTest::cleanupTestCase() {
    mafDEL(m_Resource);
    mafEventBusManager::instance()->shutdown();
    mafMessageHandler::instance()->shutdown();
}

void mafResourceTest::mafResourceAllocationTest() {
    QVERIFY(m_Resource != NULL);
}

void mafResourceTest::mafResourceAddInputTest() {
    mafResource *obj1 = mafNEW(mafResources::mafResource);
    obj1->setObjectName("Obj1");

    //! <snippet>
    mafResource *obj2 = mafNEW(mafResources::mafResource);
    obj2->setObjectName("Obj2");

    int idx = m_Resource->addInput(obj1);
    //! </snippet>
    QVERIFY(idx == 0);

    idx = m_Resource->setInput(obj2, 0);
    QVERIFY(idx == 0);
    mafResourceList *il =  m_Resource->inputList();
    int num_elem = il->count();
    QVERIFY(num_elem == 1);
    QVERIFY(obj2->isEqual(il->at(0)));

    idx = m_Resource->addInput(obj1);
    QVERIFY(idx == 1);

    idx = m_Resource->setInput(obj1, 0);
    QVERIFY(idx == 1);
    QVERIFY(obj2->isEqual(il->at(0)));

    mafDEL(obj1);
    mafDEL(obj2);
}

void mafResourceTest::mafResourceRemoveInputTest() {
    mafResource *obj1 = mafNEW(mafResources::mafResource);
    obj1->setObjectName("Obj1");

    mafResource *obj2 = mafNEW(mafResources::mafResource);
    obj2->setObjectName("Obj2");

    m_Resource->addInput(obj1);
    mafResourceList *il =  m_Resource->inputList();
    //! <snippet>
    m_Resource->removeInput(0);
    //! </snippet>
    int num_elem = il->count();

    QVERIFY(num_elem == 0);

    m_Resource->addInput(obj2);

    // try to remove out of range index.
    m_Resource->removeInput(5);
    num_elem = il->count();
    QVERIFY(num_elem == 1);

    // try to remove object not present.
    m_Resource->removeInput(obj1);
    num_elem = il->count();
    QVERIFY(num_elem == 1);

    mafDEL(obj1);
    mafDEL(obj2);
}

void mafResourceTest::mafResourceCreateMementoTest() {
    mafResource *obj1 = mafNEW(mafResources::mafResource);
    obj1->setObjectName("Obj1");

    m_Resource->addInput(obj1);
    m_Resource->setObjectName("Test Name");

    mafMemento *m = m_Resource->createMemento();

    QVERIFY(m != NULL);

    mafResource *myRes = mafNEW(mafResources::mafResource);
    // Assign it to the new one object so they become equals.
    // (the 'deep_memento flag at true to be copied also the hash value)
    myRes->setMemento(m, true);

    QVERIFY(myRes->isEqual(m_Resource));

    mafDEL(myRes);
    mafDEL(m);
    mafDEL(obj1);
}

#include "mafResourceTest.moc"
