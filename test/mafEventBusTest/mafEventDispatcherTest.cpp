/*
 *  mafEventDispatcherTest.cpp
 *  mafResourcesTest
 *
 *  Created by Daniele Giunchi on 22/09/09.
 *  Copyright 2009 SCS-B3C. All rights reserved.
 *
 *  See Licence at: http://tiny.cc/QXJ4D
 *
 */

#include "mafEventBusTestList.h"

using namespace mafEventBus;

//-------------------------------------------------------------------------
/**
 Class name: mafObjectCustom
 Custom object needed for testing.
 */
class testObjectCustomForDispatcher : public QObject {
    Q_OBJECT

public:
    /// constructor.
    testObjectCustomForDispatcher();

    /// Return the var's value.
    int var() {return m_Var;}

    /// register a custom callback
    void registerCustomCallback();

public Q_SLOTS:
    /// Test slot that will increment the value of m_Var when an UPDATE_OBJECT event is raised.
    void updateObject();
    void updateObject2();
    void setObjectValue(int v);

Q_SIGNALS:
    void valueModify(int v);
    void objectModify();

private:
    int m_Var; ///< Test var.
};

testObjectCustomForDispatcher::testObjectCustomForDispatcher() : QObject(), m_Var(0) {
}

void testObjectCustomForDispatcher::updateObject() {
    ++m_Var;
}

void testObjectCustomForDispatcher::updateObject2() {

}

void testObjectCustomForDispatcher::setObjectValue(int v) {
    m_Var = v;
}

void testObjectCustomForDispatcher::registerCustomCallback() {
    mafRegisterLocalCallback("maf.local.custom.topic", this, "updateObject()");
}


//-------------------------------------------------------------------------


void mafEventDispatcherTest::initTestCase() {
    m_ObjTestObserver = new testObjectCustomForDispatcher;
    m_ObjTestObserver->setObjectName("TestObserver");
    m_EventDispatcher = new mafEventBus::mafEventDispatcher;
    m_EventDispatcher->setObjectName("EventDispatcher");
}

void mafEventDispatcherTest::cleanupTestCase() {
    delete m_ObjTestObserver;
    delete m_EventDispatcher;
}

void mafEventDispatcherTest::mafEventDispatcherAllocationTest() {
    QVERIFY(m_EventDispatcher != NULL);
}

void mafEventDispatcherTest::mafEventDispatcherAddAndRemoveObserverAndNotifyEventTest() {
    // Create new Event ID used for callback and event notification.
    QString updateID = "maf.local.dispatcherTest.update";

    mafEvent *properties = new mafEventBus::mafEvent;
    (*properties)[TOPIC] =  updateID;
    (*properties)[TYPE] = mafEventTypeLocal;
    (*properties)[SIGTYPE] = mafSignatureTypeSignal;
    QVariant var;
    var.setValue((QObject*)m_ObjTestObserver);
    (*properties)[OBJECT] = var;
    (*properties)[SIGNATURE] = "objectModify()";
    QVERIFY(m_EventDispatcher->registerSignal(*properties));

    mafEvent *propCallback = new mafEventBus::mafEvent;
    (*propCallback)[TOPIC] =  updateID;
    (*propCallback)[TYPE] = mafEventTypeLocal;
    (*propCallback)[SIGTYPE] = mafSignatureTypeCallback;
    QVariant varobserver;
    varobserver.setValue((QObject*)m_ObjTestObserver);
    (*propCallback)[OBJECT] = varobserver;
    (*propCallback)[SIGNATURE] = "updateObject()";
    QVERIFY(m_EventDispatcher->addObserver(*propCallback));
}

void mafEventDispatcherTest::mafEventDispatcherRegisterAndRemoveSignalAndNotifyEventTest() {
    QString updateID = "maf.local.dispatcherTest.update";

    mafEvent *properties = new mafEventBus::mafEvent;
    (*properties)[TOPIC] =  updateID;
    (*properties)[TYPE] = mafEventTypeLocal;
    (*properties)[SIGTYPE] = mafSignatureTypeSignal;
    QVariant var;
    var.setValue((QObject*)m_ObjTestObserver);
    (*properties)[OBJECT] = var;
    (*properties)[SIGNATURE] = "objectModify()";
    QVERIFY(m_EventDispatcher->removeSignal(*properties));

    QVERIFY(m_EventDispatcher->registerSignal(*properties));

    // Register the callback to update the object custom:
    mafEvent *propCallback = new mafEventBus::mafEvent;
    (*propCallback)[TOPIC] =  updateID;
    (*propCallback)[TYPE] = mafEventTypeLocal;
    (*propCallback)[SIGTYPE] = mafSignatureTypeCallback;
    QVariant varobserver;
    varobserver.setValue((QObject*)m_ObjTestObserver);
    (*propCallback)[OBJECT] = varobserver;
    (*propCallback)[SIGNATURE] = "updateObject()";
    QVERIFY(m_EventDispatcher->addObserver(*propCallback));
}

void mafEventDispatcherTest::isSignalPresentTest() {
    QString updateID = "maf.local.dispatcherTest.update";
    // Register the callback to update the object custom:
    QVERIFY(m_EventDispatcher->isLocalSignalPresent(updateID));
}

void mafEventDispatcherTest::removeObserverTest() {
    QString updateID = "maf.local.dispatcherTest.update";

    // remove the observer from the updateID topics...
    QVERIFY(m_EventDispatcher->removeObserver(m_ObjTestObserver, updateID));

    // Add again the test object as observer...
    mafEvent *propCallback = new mafEventBus::mafEvent;
    (*propCallback)[TOPIC] =  updateID;
    (*propCallback)[TYPE] = mafEventTypeLocal;
    (*propCallback)[SIGTYPE] = mafSignatureTypeCallback;
    QVariant varobserver;
    varobserver.setValue((QObject*)m_ObjTestObserver);
    (*propCallback)[OBJECT] = varobserver;
    (*propCallback)[SIGNATURE] = "updateObject()";
    QVERIFY(m_EventDispatcher->addObserver(*propCallback));

    // remove the observer from all the topics...
    QVERIFY(m_EventDispatcher->removeObserver(m_ObjTestObserver, ""));
}

void mafEventDispatcherTest::removeItemTest() {
    QString updateID = "maf.local.dispatcherTest.update";


    // Add again the test object as observer...
    mafEvent *propCallback = new mafEventBus::mafEvent;
    (*propCallback)[TOPIC] =  updateID;
    (*propCallback)[TYPE] = mafEventTypeLocal;
    (*propCallback)[SIGTYPE] = mafSignatureTypeCallback;
    QVariant varobserver;
    varobserver.setValue((QObject*)m_ObjTestObserver);
    (*propCallback)[OBJECT] = varobserver;
    (*propCallback)[SIGNATURE] = "updateObject()";
    QVERIFY(m_EventDispatcher->addObserver(*propCallback));

    // remove the observer from all the topics...

    mafEvent *propCallback2 = new mafEventBus::mafEvent;
    (*propCallback2)[TOPIC] =  updateID;
    (*propCallback2)[TYPE] = mafEventTypeLocal;
    (*propCallback2)[SIGTYPE] = mafSignatureTypeCallback;
    (*propCallback2)[OBJECT] = varobserver;
    (*propCallback2)[SIGNATURE] = "updateObject2()";
    QVERIFY(m_EventDispatcher->addObserver(*propCallback2));

    //this will be removed
    QVERIFY(m_EventDispatcher->removeObserver(*propCallback));

    // this will be removed and will cover the code of iterator which simple go to the next element
    QVERIFY(m_EventDispatcher->removeObserver(*propCallback2));

}

void mafEventDispatcherTest::removeSignalTest() {
    QString updateID = "maf.local.dispatcherTest.update";

    // remove the signal from the updateID topics...
    // ...but don't need to make a qt disconnect because all observer has been disconnected already on previous test case.
    QVERIFY(m_EventDispatcher->removeSignal(m_ObjTestObserver, updateID, false));

    mafEvent *properties = new mafEventBus::mafEvent;
    (*properties)[TOPIC] =  updateID;
    (*properties)[TYPE] = mafEventTypeLocal;
    (*properties)[SIGTYPE] = mafSignatureTypeSignal;
    QVariant var;
    var.setValue((QObject*)m_ObjTestObserver);
    (*properties)[OBJECT] = var;
    (*properties)[SIGNATURE] = "objectModify()";
    QVERIFY(m_EventDispatcher->registerSignal(*properties));

    QVERIFY(m_EventDispatcher->removeSignal(m_ObjTestObserver, "", false));
}


void mafEventDispatcherTest::reverseOrderRegistrationTest() {
    QString updateID = "maf.local.dispatcherTest.custom";

    // Register the callback to update the object custom:
    mafEvent *propCallback = new mafEventBus::mafEvent;
    (*propCallback)[TOPIC] =  updateID;
    (*propCallback)[TYPE] = mafEventTypeLocal;
    (*propCallback)[SIGTYPE] = mafSignatureTypeCallback;
    QVariant varobserver;
    varobserver.setValue((QObject*)m_ObjTestObserver);
    (*propCallback)[OBJECT] = varobserver;
    (*propCallback)[SIGNATURE] = "updateObject()";
    QVERIFY(m_EventDispatcher->addObserver(*propCallback));

    mafEvent *properties = new mafEventBus::mafEvent;
    (*properties)[TOPIC] =  updateID;
    (*properties)[TYPE] = mafEventTypeLocal;
    (*properties)[SIGTYPE] = mafSignatureTypeSignal;
    QVariant var;
    var.setValue((QObject*)m_ObjTestObserver);
    (*properties)[OBJECT] = var;
    (*properties)[SIGNATURE] = "objectModify()";
    QVERIFY(m_EventDispatcher->registerSignal(*properties));
}

void mafEventDispatcherTest::isLocalSignalPresentTest() {
    QVERIFY(m_EventDispatcher->isLocalSignalPresent("maf.wrong.topic") == false);
}

#include "mafEventDispatcherTest.moc"
