/*
 *  mafMementoVMETest.cpp
 *  mafResourcesTest
 *
 *  Created by Roberto Mucci - Daniele Giunchi on 24/05/10.
 *  Copyright 2011 SCS-B3C. All rights reserved.
 *
 *  See License at: http://tiny.cc/QXJ4D
 *
 */

#include "mafResourcesTestList.h"

#if defined(_WIN32) || defined(WIN32)
#define SERIALIZATION_LIBRARY_NAME "mafSerialization.dll"
#else
#ifdef __APPLE__
#define SERIALIZATION_LIBRARY_NAME "mafSerialization.dylib"
#else
#define SERIALIZATION_LIBRARY_NAME "mafSerialization.so"
#endif
#endif


using namespace mafCore;
using namespace mafResources;
using namespace mafEventBus;



//------------------------------------------------------------------------------------------

/**
 Class name: testExtDataType
 This class implements the external data type coming from an external library.
 */
class testExtDataType {
public:
    /// Object constructor.
    testExtDataType(QString v) : m_Value(v) {}

    /// set the new value for the class.
    void setValue(QString v) {m_Value = v;}
    /// Return the inner value.
    QString value() {return m_Value;}

private:
    QString m_Value; ///< Test variable for external data
};



/**
 Class name: testExtDataCodecCustom
 This class implements an external data codec.
 */
class  testExtDataCodecCustom : public  mafCore::mafExternalDataCodec {
    Q_OBJECT
    /// typedef macro.
    mafSuperclassMacro(mafCore::mafExternalDataCodec);

public:
    /// Object constructor.
    testExtDataCodecCustom(const QString code_location = "");

    /// Encode the memento into the output type.
    /*virtual*/ char *encode(bool binary = true);

    /// Decode the output type into the memento.
    /*virtual*/ void decode(const char *input_string, bool binary = true);

private:
    mafProxy<QString> *m_Cont; ///< Test Var.
};

testExtDataCodecCustom::testExtDataCodecCustom(const QString code_location) : mafExternalDataCodec(code_location) {
}

void testExtDataCodecCustom::decode(const char *input_string, bool binary) {
    Q_UNUSED(binary);
    REQUIRE(input_string != NULL);
    m_Cont = new mafProxy<QString>();
    *m_Cont = new QString;
    m_Cont->externalData()->append(input_string);
    this->m_ExternalData = m_Cont;
}

char *testExtDataCodecCustom::encode(bool binary) {
    Q_UNUSED(binary);
    mafProxy<QString> *dataSet = mafProxyPointerTypeCast(QString, this->externalData());
    QString dataString = dataSet->externalData()->toLatin1();
    char *output_string = new char[dataString.size()+1];
    QByteArray ba = dataString.toLatin1();
    memcpy(output_string, ba.data(),dataString.size()+1);
    return output_string;
}

/**
 Class name: testDataPipe
 This class implements the data pipe to be tested.
 */
class testDataPipe : public  mafPipeData {
    Q_OBJECT
    /// typedef macro.
    mafSuperclassMacro(mafResources::mafPipeData);

public:
    /// Object constructor.
    testDataPipe(const QString code_location = "");

    /// Return the string variable initialized and updated from the data pipe.
    QString pipeline() {return m_PipeLine;}

public Q_SLOTS:
    /// Allow to execute and update the pipeline when something change
    /*virtual*/ void updatePipe(double t = -1);

private:
    QString m_PipeLine; ///< Test Var.
};

testDataPipe::testDataPipe(const QString code_location) : mafPipeData(code_location), m_PipeLine("Created") {
}

void testDataPipe::updatePipe(double t) {
    m_PipeLine = "Updated";
    m_PipeLine.append(QString::number(t));
    Superclass::updatePipe(t);
}

//------------------------------------------------------------------------------------------

void mafMementoVMETest::initTestCase() {
    // Create before the instance of the Serialization manager, which will register signals.
    bool res(false);
    res = mafInitializeModule(SERIALIZATION_LIBRARY_NAME);
    QVERIFY(res);

    mafMessageHandler::instance()->installMessageHandler();
    mafResourcesRegistration::registerResourcesObjects();
    mafRegisterObject(testExtDataCodecCustom);
    m_VME = mafNEW(mafResources::mafVME);
}


void mafMementoVMETest::cleanupTestCase() {
    mafDEL(m_VME);
    mafMessageHandler::instance()->shutdown();
}

void mafMementoVMETest::mafMementoVMEDefaultAllocationTest() {
    QVERIFY(m_VME != NULL);
}

void mafMementoVMETest::mafMementoVMECustomAllocationTest() {
    QString testString("testString");
    QString testString2("testString2");

    //Create first dataSet
    mafProxy<testExtDataType> container;
    container = new testExtDataType(testString);
    container.setExternalDataType("testExtDataType");
    container.setExternalCodecType("CUSTOM");
    mafDataSet *dataSet = mafNEW(mafResources::mafDataSet);
    dataSet->setDataValue(&container);

    mafMatrix4x4 *matrix = new mafMatrix4x4();
    matrix->setToIdentity();
    (*matrix)(0,0) = 3.;

    dataSet->setPoseMatrix(matrix);

    //Create second dataSet
    mafProxy<testExtDataType> container2;
    container2 = new testExtDataType(testString2);
    container2.setExternalDataType("testExtDataType");
    container2.setExternalCodecType("CUSTOM");
    mafDataSet *dataSet2 = mafNEW(mafResources::mafDataSet);
    dataSet2->setDataValue(&container2);

    mafMatrix4x4 *matrix2 = new mafMatrix4x4();
    matrix2->setToIdentity();
    (*matrix2)(1,0) = 5;

    dataSet2->setPoseMatrix(matrix2);
    
    mafDataSetCollection *collection = m_VME->dataSetCollection();
    collection->insertItem(dataSet, 0);
    collection->insertItem(dataSet2, 1);

    testDataPipe *dataPipe = mafNEW(testDataPipe);
    m_VME->setDataPipe(dataPipe);
    dataPipe->release();

    //Plug the codec
    QString obj_type = "testExtDataType";
    QString encodeType = "CUSTOM";
    QString codec = "testExtDataCodecCustom";

    mafEventArgumentsList argList;
    argList.append(mafEventArgument(QString, obj_type));
    argList.append(mafEventArgument(QString, encodeType));
    argList.append(mafEventArgument(QString, codec));
    mafEventBusManager::instance()->notifyEvent("maf.local.serialization.plugCodec", mafEventTypeLocal, &argList);

    //! <snippet>
    ////Create the VME Memento that stores dataSetCollection and dataPipe
    ////of the VME.
    mafMemento *memento = m_VME->createMemento();
    //! </snippet>
    QVERIFY(memento != NULL);

    //! <snippet>
    ////Restore the VME throught Memento
    mafVME *returnVME = mafNEW(mafResources::mafVME);
    returnVME->setMemento(memento);

    //! </snippet>
    mafPipeData *check = returnVME->dataPipe();
    QVERIFY(dataPipe->isEqual(check));

    //Check if m_VME memento is equal to returnVME memento
    mafMemento *returnMemento = returnVME->createMemento();
    memento->isEqual(returnMemento);

    mafDEL(dataSet);
    mafDEL(dataSet2);
    mafDEL(returnVME);
    mafDEL(returnMemento);
    mafDEL(memento);
}


#include "mafMementoVMETest.moc"
