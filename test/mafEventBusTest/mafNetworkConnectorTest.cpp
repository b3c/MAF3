/*
 *  mafNetworkConnectorTest.cpp
 *  mafResourcesTest
 *
 *  Created by Daniele Giunchi on 20/04/10.
 *  Copyright 2009 SCS-B3C. All rights reserved.
 *
 *  See Licence at: http://tiny.cc/QXJ4D
 *
 */

#include "mafEventBusTestList.h"

using namespace mafEventBus;

//------------------------------------------------------------------------------------------
/**
 Class name: testNetworkConnectorCustom
 This class implements the network connector to be tested.
 */
class  testNetworkConnectorCustom : public  mafNetworkConnector {
    Q_OBJECT

public:
    /// Object constructor.
    testNetworkConnectorCustom();

    /// Create and initialize client
    /*virtual*/ void createClient(const QString hostName, const unsigned int port, QMap<QString,QVariant> *advancedParamters = NULL);

    /// Return the string variable initializated and updated from the data pipe.
    /*virtual*/ void createServer(const unsigned int port);

    /// Allow to send a network request.
    /*virtual*/ void send(const QString event_id, mafEventArgumentsList *params, bool externalSend = false);

    /// Start the server.
    /*virtual*/ void startListen();

    /// Return connector status.
    QString connectorStatus();

    /// retrieve instance of object
    /*virtual*/ mafNetworkConnector *clone();

    /// register all the signals and slots
    /*virtual*/ void initializeForEventBus();

private:
    QString m_ConnectorStatus; ///< Test Var.
};

mafNetworkConnector *testNetworkConnectorCustom::clone() {
    return new testNetworkConnectorCustom();
}

void testNetworkConnectorCustom::initializeForEventBus() {
}

testNetworkConnectorCustom::testNetworkConnectorCustom() : mafNetworkConnector(), m_ConnectorStatus("") {
     m_Protocol = "FakeProtocol";
}

void testNetworkConnectorCustom::createServer(const unsigned int port) {
    m_ConnectorStatus = "Server Created - Port: ";
    m_ConnectorStatus.append(QString::number(port));
}

void testNetworkConnectorCustom::startListen() {
    m_ConnectorStatus = "Server Listening";
}

void testNetworkConnectorCustom::createClient(const QString hostName, const unsigned int port, QMap<QString,QVariant> *advancedParamters) {
    m_ConnectorStatus = "Client Created - Host: ";
    m_ConnectorStatus.append(hostName);
    m_ConnectorStatus.append(" Port: ");
    m_ConnectorStatus.append(QString::number(port));
}

void testNetworkConnectorCustom::send(const QString event_id, mafEventArgumentsList *params, bool externalSend) {
    Q_UNUSED(params);
    Q_UNUSED(externalSend);
    m_ConnectorStatus = "Event sent with ID: ";
    m_ConnectorStatus.append(event_id);
}

QString testNetworkConnectorCustom::connectorStatus() {
    return m_ConnectorStatus;
}

//------------------------------------------------------------------------------------------

void mafNetworkConnectorTest::initTestCase() {
    m_NetworkConnector = new testNetworkConnectorCustom();
}

void mafNetworkConnectorTest::cleanupTestCase() {
    if(m_NetworkConnector) delete m_NetworkConnector;
}

void mafNetworkConnectorTest::mafNetworkConnectorAllocationTest() {
    QVERIFY(m_NetworkConnector != NULL);
}

void mafNetworkConnectorTest::mafNetworkConnectorCreateClientAndServerTest() {
    QString res;
    res = "Server Created - Port: 8000";
    m_NetworkConnector->createServer(8000);

    testNetworkConnectorCustom *conn = (testNetworkConnectorCustom *)m_NetworkConnector;
    QCOMPARE(conn->connectorStatus(), res);

    res = "Client Created - Host: localhost Port: 8000";
    m_NetworkConnector->createClient("localhost", 8000);
    QCOMPARE(conn->connectorStatus(), res);
}

void mafNetworkConnectorTest::retrieveProtocolTest() {
    QString res = "FakeProtocol";
    QString check = m_NetworkConnector->protocol();
    QCOMPARE(check, res);
}

#include "mafNetworkConnectorTest.moc"

