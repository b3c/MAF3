/*
 *  mafOperationManagerTest.cpp
 *  mafResourcesTest
 *
 *  Created by Daniele Giunchi on 11/05/10.
 *  Copyright 2011 SCS-B3C. All rights reserved.
 *
 *  See License at: http://tiny.cc/QXJ4D
 *
 */

#include "mafResourcesTestList.h"

#define kMAX_COUNT 50000

using namespace mafCore;
using namespace mafEventBus;
using namespace mafResources;

/**
 Class name: testEndlessOperation
 This is an example of endless operation that needs to be aborted to terminate the test suite.
 */
class testEndlessOperation : public mafResources::mafOperation {
    Q_OBJECT
    mafSuperclassMacro(mafResources::mafOperation);

public:
    /// Object constructor.
    testEndlessOperation(const QString code_location = "");

protected:
    /// Terminate the execution.
    /*virtual*/ void terminated();
    
public Q_SLOTS:
    /// execution method
    /*virtual*/ void execute();

};

testEndlessOperation::testEndlessOperation(const QString code_location) : mafOperation(code_location) {
    m_CanUnDo = false;
	m_MultiThreaded = true;
    setObjectName("testEndlessOperation");
}


void testEndlessOperation::execute() {
    while ( m_Status != mafOperationStatusAborted ) {
        ;
    }

    if ( m_Status == mafOperationStatusAborted ) {
        return;
    }

    Q_EMIT executionEnded();
}

void testEndlessOperation::terminated() {
    
}

//------------------------------------------------------------------------------------------------------------------------------------------------

/**
 Class name: testNotUndoOperation
 This operation is an example of not undoable operation.
 */
class testNotUndoOperation : public mafResources::mafOperation {
    Q_OBJECT
    mafSuperclassMacro(mafResources::mafOperation);

public:
    /// Object constructor.
    testNotUndoOperation(const QString code_location = "");

    /// Return the internal variable value.
    int val();

protected:
    /// Terminate the execution.
    /*virtual*/ void terminated();
    
public Q_SLOTS:
    /// execution method
    /*virtual*/ void execute();
    
private:
    int m_Val;
};

testNotUndoOperation::testNotUndoOperation(const QString code_location) : mafOperation(code_location), m_Val(0) {
    m_CanUnDo = false;
	m_MultiThreaded = true;
    setObjectName("NotUndoOperation");
}

int testNotUndoOperation::val() {
    return m_Val;
}

void testNotUndoOperation::execute() {
    m_Val = 0;
    int i = 0;

    while ( ++i < kMAX_COUNT ) {
        if ( i % 1000 == 0 ) {
            qDebug() << "Current not undo value " << m_Val;
            m_Val += i;
        }
    }

    Q_EMIT executionEnded();
}

void testNotUndoOperation::terminated() {
    
}

//------------------------------------------------------------------------------------------------------------------------------------------------

/**
 Class name: testUndoOperation
 This operation is an example of undoable operation.
 */
class testUndoOperation : public mafResources::mafOperation {
    Q_OBJECT
    mafSuperclassMacro(mafResources::mafOperation);

public Q_SLOTS:
    /// execution method
    /*virtual*/ void execute();

    /// Allows to call the piece of algorithm that is needed to restore the previous state of the operation's execution.
    /*virtual*/ void unDo();

    /// Allows to call the piece of algorithm that is needed to apply the operation again.
    /*virtual*/ void reDo();
    
public:
    /// Object constructor.
    testUndoOperation(const QString code_location = "");
    
    /// Return the internal variable value.
    int val();
    
protected:
    /// Terminate the execution.
    /*virtual*/ void terminated();
    
private:
    int m_Val;
};

testUndoOperation::testUndoOperation(const QString code_location) : mafOperation(code_location), m_Val(0) {
	m_MultiThreaded = true;
    setObjectName("UndoOperation");
}

int testUndoOperation::val() {
    return m_Val;
}

void testUndoOperation::execute() {
    // Initialize variable
    m_Val = 0;
    int i = 0;

    QString n = this->objectName();
    qDebug() << "Executing " << n << "...";

    // do some heavy calculation...
    while ( ++i < kMAX_COUNT ) {
        if ( i % 1000 == 0 ) {
            qDebug() << n << " current value " << m_Val;
            m_Val += i;
        }
    }

    //Notify vme add
    mafVME *vme = mafNEW(mafResources::mafVME);
    vme->setObjectName(mafTr("Test Vme"));
    mafEventArgumentsList argList;
    argList.append(mafEventArgument(mafCore::mafObjectBase *, vme));
    mafEventBusManager::instance()->notifyEvent("maf.local.resources.vme.add.test", mafEventTypeLocal, &argList);

    // set the final result value.
    m_Val = kMAX_COUNT;

    qDebug() << this->objectName() << " emitting executionEnded...";
    mafDEL(vme);
    Q_EMIT executionEnded();
}

void testUndoOperation::unDo() {
    qDebug() << this->objectName() << " performs unDo...";
    m_Val = 0;
}

void testUndoOperation::reDo() {
    qDebug() << this->objectName() << " performs reDo...";
    m_Val = kMAX_COUNT;
}

void testUndoOperation::terminated() {
    
}

/**
 Class name: testAutoCancelSingleThreadOperation
 This is an example of endless operation that needs to be aborted to terminate the test suite (SINGLE THREAD)
 */
class testAutoCancelSingleThreadOperation : public mafResources::mafOperation {
    Q_OBJECT
    mafSuperclassMacro(mafResources::mafOperation);
    
public:
    /// Object constructor.
    testAutoCancelSingleThreadOperation(const QString code_location = "");
    ~testAutoCancelSingleThreadOperation();
protected:
    /// Terminate the execution.
    /*virtual*/ void terminated();
    
    public Q_SLOTS:
    /// execution method
    /*virtual*/ void execute();

private:
    mafObject *m_TestObjectDestroyInDestructor;
    mafObject *m_TestObjectDestroyInTerminate;
    
};

testAutoCancelSingleThreadOperation::testAutoCancelSingleThreadOperation(const QString code_location) : mafOperation(code_location), m_TestObjectDestroyInDestructor(NULL), m_TestObjectDestroyInTerminate(NULL) {
    m_CanUnDo = false;
    m_MultiThreaded = false;
    setObjectName("testAutoCancelSingleThreadOperation");
    m_TestObjectDestroyInDestructor = mafNEW(mafCore::mafObject);
    m_TestObjectDestroyInDestructor->setObjectName("TestObject1");
}

testAutoCancelSingleThreadOperation::~testAutoCancelSingleThreadOperation() {
    mafDEL(m_TestObjectDestroyInDestructor);
}

void testAutoCancelSingleThreadOperation::execute() {
    m_TestObjectDestroyInTerminate = mafNEW(mafCore::mafObject);
    m_TestObjectDestroyInTerminate->setObjectName("TestObject2");
    QTest::qWait(1000);
        
    Q_EMIT executionCanceled();
    return;
}

void testAutoCancelSingleThreadOperation::terminated() {
    mafDEL(m_TestObjectDestroyInTerminate);
}


/**
 Class name: testAutoCancelMultiThreadOperation
 This is an example of endless operation that needs to be aborted to terminate the test suite (MULTI THREAD)
 */
class testAutoCancelMultiThreadOperation : public mafResources::mafOperation {
    Q_OBJECT
    mafSuperclassMacro(mafResources::mafOperation);
    
public:
    /// Object constructor.
    testAutoCancelMultiThreadOperation(const QString code_location = "");
    ~testAutoCancelMultiThreadOperation();
protected:
    /// Terminate the execution.
    /*virtual*/ void terminated();
    
    public Q_SLOTS:
    /// execution method
    /*virtual*/ void execute();
    
private:
    mafObject *m_TestObjectDestroyInDestructor;
    mafObject *m_TestObjectDestroyInTerminate;
    
};

testAutoCancelMultiThreadOperation::testAutoCancelMultiThreadOperation(const QString code_location) : mafOperation(code_location), m_TestObjectDestroyInDestructor(NULL), m_TestObjectDestroyInTerminate(NULL) {
    m_CanUnDo = false;
    m_MultiThreaded = false;
    setObjectName("testAutoCancelMultiThreadOperation");
    m_TestObjectDestroyInDestructor = mafNEW(mafCore::mafObject);
    m_TestObjectDestroyInDestructor->setObjectName("TestObject1");
}

testAutoCancelMultiThreadOperation::~testAutoCancelMultiThreadOperation() {
    mafDEL(m_TestObjectDestroyInDestructor);
}

void testAutoCancelMultiThreadOperation::execute() {
    m_TestObjectDestroyInTerminate = mafNEW(mafCore::mafObject);
    m_TestObjectDestroyInTerminate->setObjectName("TestObject2");
    QTest::qWait(1000);
    
    Q_EMIT executionCanceled();
    return;
}

void testAutoCancelMultiThreadOperation::terminated() {
    mafDEL(m_TestObjectDestroyInTerminate);
}



//==========================================================================================
// Test Suite
//==========================================================================================

void mafOperationManagerTest::initTestCase() {
    
    mafRegisterLocalSignal("maf.local.resources.vme.add.test", this, "vmeAddSignalTest(mafCore::mafObjectBase *)")
    mafRegisterLocalCallback("maf.local.resources.vme.add.test", this, "vmeAddTest(mafCore::mafObjectBase *)")
    mafMessageHandler::instance()->installMessageHandler();

    m_EventBus = mafEventBusManager::instance();
    m_VMEManager = mafVMEManager::instance();

    //Request hierarchy
    mafHierarchyPointer hierarchy = NULL;
    
    QGenericReturnArgument ret_val = mafEventReturnArgument(mafCore::mafHierarchyPointer, hierarchy);
    mafEventBus::mafEventBusManager::instance()->notifyEvent("maf.local.resources.hierarchy.request", mafEventTypeLocal, NULL, &ret_val);
    //Select root
    mafObject *root;
    ret_val = mafEventReturnArgument(mafCore::mafObject *, root);
    mafEventBus::mafEventBusManager::instance()->notifyEvent("maf.local.resources.hierarchy.root", mafEventTypeLocal, NULL, &ret_val);

    m_OperationManager = mafOperationManager::instance();

    // Register all the creatable objects for the mafResources module.
    mafResourcesRegistration::registerResourcesObjects();

    // Register custom operations.
    mafRegisterObject(testEndlessOperation);
    mafRegisterObject(testNotUndoOperation);
    mafRegisterObject(testUndoOperation);
    mafRegisterObject(testAutoCancelSingleThreadOperation);
    mafRegisterObject(testAutoCancelMultiThreadOperation);
}


void mafOperationManagerTest::cleanupTestCase() {
    qDebug() << "cleanup test suite...";
    m_OperationManager->shutdown();
    m_VMEManager->shutdown();

    // Unregister custom operations.
    mafUnregisterObject(testEndlessOperation);
    mafUnregisterObject(testNotUndoOperation);
    mafUnregisterObject(testUndoOperation);
    mafUnregisterObject(testAutoCancelSingleThreadOperation);
    mafUnregisterObject(testAutoCancelMultiThreadOperation);


    //restore vme manager status
    m_EventBus->notifyEvent("maf.local.resources.hierarchy.request");

    m_EventBus->shutdown();
    mafMessageHandler::instance()->shutdown();
}

mafCore::mafObjectBase *mafOperationManagerTest::startOperation(QString opType) {
    mafEventArgumentsList argList;
    argList.append(mafEventArgument(QString, opType));
    m_EventBus->notifyEvent("maf.local.resources.operation.start", mafEventTypeLocal, &argList);

    mafCore::mafObjectBase *op = NULL;
    QGenericReturnArgument ret_val = mafEventReturnArgument(mafCore::mafObjectBase *, op);
    m_EventBus->notifyEvent("maf.local.resources.operation.currentRunning", mafEventTypeLocal, NULL, &ret_val);
    return op;
}

const mafExecutionPool *mafOperationManagerTest::retrievePool() {
    const mafExecutionPool *executionPool = NULL;
    QGenericReturnArgument pool_val = mafEventReturnArgument(const mafExecutionPool *, executionPool);
    m_EventBus->notifyEvent("maf.local.resources.operation.executionPool", mafEventTypeLocal, NULL, &pool_val);
    return executionPool;
}

 void mafOperationManagerTest::vmeAddTest(mafCore::mafObjectBase *vme) {
     QVERIFY(vme);
 }

void mafOperationManagerTest::mafOperationManagerAllocationTest() {
	
    QVERIFY(m_OperationManager != NULL);

    m_ExecutionPool = this->retrievePool();
    QVERIFY(m_ExecutionPool != NULL);

    int poolSize = m_ExecutionPool->size();
    QVERIFY(poolSize == 0);
	
}


void mafOperationManagerTest::cancelStartTest() {
    mafCore::mafObjectBase *op = this->startOperation("testEndlessOperation");

    // Cancel the operation's execution
    mafEventBusManager::instance()->notifyEvent("maf.local.resources.operation.stop", mafEventTypeLocal);

    int poolSize = m_ExecutionPool->size();
    QVERIFY(poolSize == 0);
}

void mafOperationManagerTest::abortExecutionTest() {
    const mafCore::mafObjectBase *op = this->startOperation("testEndlessOperation");

    // Start the operation's execution
    mafEventBusManager::instance()->notifyEvent("maf.local.resources.operation.execute", mafEventTypeLocal);
    // Print debug message (possible and done immediately because the operation execute in background).
    qDebug() << mafTr("start background execution for ") << op->objectName();

    // Get the operation's worker.
    QThread *obj = m_ExecutionPool->at(0);
    mafOperationWorker *worker = qobject_cast<mafResources::mafOperationWorker *>(obj);
    
    // Create a timer to abort the endless loop after a fixed amount of time.
    QTime dieTime = QTime::currentTime().addSecs(1);
    while(QTime::currentTime() < dieTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
    }

    worker->abortExecution();

    //loop.exec();
    QTime dieTimeAFter = QTime::currentTime().addSecs(3);
    while(QTime::currentTime() < dieTimeAFter) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 3);
    }


    qDebug() << "****************** Execution ABORTED ******************";

    // The endless operation has been aborted => removed from the execution pool.
    int poolSize = m_ExecutionPool->size();
    QVERIFY(poolSize == 0);
}


void mafOperationManagerTest::undoRedoExecutionTest() {

    QTime dieTime = QTime::currentTime().addSecs(1);
    while(QTime::currentTime() < dieTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
    }

    const mafCore::mafObjectBase *op = this->startOperation("testUndoOperation");
    ((QObject *)op)->setObjectName("Operation_1");
    m_EventBus->notifyEvent("maf.local.resources.operation.execute", mafEventTypeLocal);
    op = this->startOperation("testUndoOperation");
    ((QObject *)op)->setObjectName("Operation_2");
    m_EventBus->notifyEvent("maf.local.resources.operation.execute", mafEventTypeLocal);
    op = this->startOperation("testUndoOperation");
    ((QObject *)op)->setObjectName("Operation_3");
    m_EventBus->notifyEvent("maf.local.resources.operation.execute", mafEventTypeLocal);
    op = this->startOperation("testUndoOperation");
    ((QObject *)op)->setObjectName("Operation_4");
    m_EventBus->notifyEvent("maf.local.resources.operation.execute", mafEventTypeLocal);
    
    
    QTime dieTimeAFter = QTime::currentTime().addSecs(3);
    while(QTime::currentTime() < dieTimeAFter) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 3);
    }

    qDebug() << "****************** Execution ended ******************";
    int undoStackSize;
    QGenericReturnArgument ret_val = mafEventReturnArgument(int, undoStackSize);
    m_EventBus->notifyEvent("maf.local.resources.operation.sizeUndoStack", mafEventTypeLocal, NULL, &ret_val);
    
    QVERIFY(undoStackSize == 4);
    
    m_EventBus->notifyEvent("maf.local.resources.operation.undo", mafEventTypeLocal);
    m_EventBus->notifyEvent("maf.local.resources.operation.undo", mafEventTypeLocal);
    
    m_EventBus->notifyEvent("maf.local.resources.operation.redo", mafEventTypeLocal);
    m_EventBus->notifyEvent("maf.local.resources.operation.undo", mafEventTypeLocal);
    
    op = this->startOperation("testUndoOperation");
    ((QObject *)op)->setObjectName("Operation_5");
    m_EventBus->notifyEvent("maf.local.resources.operation.execute", mafEventTypeLocal);
    
    dieTimeAFter = QTime::currentTime().addSecs(3);
    while(QTime::currentTime() < dieTimeAFter) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 3);
    }
    
    ret_val = mafEventReturnArgument(int, undoStackSize);
    m_EventBus->notifyEvent("maf.local.resources.operation.sizeUndoStack", mafEventTypeLocal, NULL, &ret_val);
    
    QVERIFY(undoStackSize == 3);
    
    op = this->startOperation("testNotUndoOperation");
    ((QObject *)op)->setObjectName("Operation_6");
    m_EventBus->notifyEvent("maf.local.resources.operation.execute", mafEventTypeLocal);
    
    
    dieTimeAFter = QTime::currentTime().addSecs(3);
    while(QTime::currentTime() < dieTimeAFter) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 3);
    }
    
    qDebug() << "****************** Execution ended ******************";
    ret_val = mafEventReturnArgument(int, undoStackSize);
    m_EventBus->notifyEvent("maf.local.resources.operation.sizeUndoStack", mafEventTypeLocal, NULL, &ret_val);
    
    QVERIFY(undoStackSize == 0);
}

void mafOperationManagerTest::cancelExecutionTest() {
    const mafCore::mafObjectBase *opst = this->startOperation("testAutoCancelSingleThreadOperation");
    
    // Cancel the operation's execution
    mafEventBusManager::instance()->notifyEvent("maf.local.resources.operation.execute", mafEventTypeLocal);

    
    const mafCore::mafObjectBase *opmt = this->startOperation("testAutoCancelMultiThreadOperation");
    
    // Cancel the operation's execution
    mafEventBusManager::instance()->notifyEvent("maf.local.resources.operation.execute", mafEventTypeLocal);

}

#include "mafOperationManagerTest.moc"
