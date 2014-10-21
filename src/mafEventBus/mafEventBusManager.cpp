/*
 *  mafEventBusManager.cpp
 *  mafEventBus
 *
 *  Created by Paolo Quadrani on 27/03/09.
 *  Copyright 2012 SCS-B3C. All rights reserved.
 *
 *  See License at: http://tiny.cc/QXJ4D
 *
 */

#include "mafEventBusManager.h"
#include "mafTopicRegistry.h"
#include "mafNetworkConnector.h"

#include <QMutex>
#include <QMutexLocker>

using namespace mafEventBus;

mafEventBusManager::mafEventBusManager() : m_EnableEventLogging(false), m_LogEventTopic("*"), m_SkipDetach(false) {
    // Create local event dispatcher.
    m_LocalDispatcher = new mafEventDispatcherLocal();
    m_LocalDispatcher->setObjectName("Local Event Dispatcher");

    // Create the remote event dispatcher.
    m_RemoteDispatcher = new mafEventDispatcherRemote();
    m_RemoteDispatcher->setObjectName("Remote Event Dispatcher");

    qRegisterMetaType<mafEventBus::mafEventPointer>("mafEventBus::mafEventPointer");
    qRegisterMetaType<mafEventBus::mafEventArgumentsListPointer>("mafEventBus::mafEventArgumentsListPointer");
    qRegisterMetaType<mafEventBus::mafRegisterMethodsMap>("mafEventBus::mafRegisterMethodsMap");
    qRegisterMetaType<QVariantList>("QVariantList");
}

mafEventBusManager::~mafEventBusManager() {
    mafNetworkConnectorHash::iterator i = m_NetworkConnectorHash.begin();
    while(i != m_NetworkConnectorHash.end()) {
        delete i.value();
        ++i;
    }
    // clean connection info vector.
    m_ConnectionInfoHash.clear();
    // clean hash for network connectors.
    m_NetworkConnectorHash.clear();

    // disconnect detachFromEventBus
    m_SkipDetach = true;

    if(m_LocalDispatcher) {
        m_LocalDispatcher->resetHashes();
        delete m_LocalDispatcher;
    }
    if(m_RemoteDispatcher) {
        m_RemoteDispatcher->resetHashes();
        delete m_RemoteDispatcher;
    }
}

void mafEventBusManager::plugNetworkConnector(const QString &protocol, mafNetworkConnector *connector) {
    m_NetworkConnectorHash.insert(protocol, connector);
}

bool mafEventBusManager::isLocalSignalPresent(const QString topic) const {
    return m_LocalDispatcher->isLocalSignalPresent(topic);
}

mafEventBusManager* mafEventBusManager::instance() {
    static mafEventBusManager instanceEventBus;
    return &instanceEventBus;
}

void mafEventBusManager::shutdown() {
    mafEventBus::mafTopicRegistry::instance()->shutdown();
}

void mafEventBusManager::initializeNetworkConnectors() {

}

bool mafEventBusManager::addEventProperty(const mafEvent &props) const {
    bool result(false);
    QString topic = props[TOPIC].toString();
    QObject *obj = props[OBJECT].value<QObject*>();

    /// add to connection infos array this connection
    /// chek also if already exists and one of observer or signal is empty
    if(!m_ConnectionInfoHash.contains(topic)) {
        mafEventBusConnectionInfo info;
        info.type = (mafEventType) props[TYPE].toInt();
        if(props[SIGTYPE].toInt() == mafSignatureTypeCallback) {
            info.slotName.push_back(props[SIGNATURE].toString());
            info.observers.push_back(obj);
        } else { //is a signal
            info.emitter = obj;
            info.signalName.append(props[SIGNATURE].toString());
        }
        //info.callers;
        //info.counter;
        m_ConnectionInfoHash.insert(topic,info);
    } else {
        //check the type, and complete the information
        mafEventBusConnectionInfo info = m_ConnectionInfoHash.value(topic);
        if(props[SIGTYPE].toInt() == mafSignatureTypeCallback) {
            info.slotName.push_back(props[SIGNATURE].toString());
            info.observers.push_back(obj);
            m_ConnectionInfoHash.insert(topic,info);
        } else { //no sense if is a signal
        }
    }



    if(props[TYPE].toInt() == mafEventTypeLocal) {
        // Local event dispatching.
        if(props[SIGTYPE].toInt() == mafSignatureTypeCallback) {
            result = m_LocalDispatcher->addObserver(props);
        } else {
            //Add topic to the mafTopicRegistry
            result = mafEventBus::mafTopicRegistry::instance()->registerTopic(topic, obj);
            if(!result) {
                return result;
            }
            result = m_LocalDispatcher->registerSignal(props);
        }
    } else {
        // Remote event dispatching.
        if(props[SIGTYPE].toInt() == mafSignatureTypeCallback) {
            result = m_RemoteDispatcher->addObserver(props);
        } else {
            //Add topic to the mafTopicRegistry
            result = mafEventBus::mafTopicRegistry::instance()->registerTopic(topic, obj);
            if(!result) {
                return result;
            }
            result = m_RemoteDispatcher->registerSignal(props);
        }
    }

    result = connect(obj, SIGNAL(destroyed()), this, SLOT(detachObjectFromBus()));
    return result;
}

void mafEventBusManager::detachObjectFromBus() {
    if(m_SkipDetach) {
        return;
    }

    QObject *obj = QObject::sender();
    removeObserver(obj, "", false);
    removeSignal(obj, "", false);
}

void mafEventBusManager::removeObserver(const QObject *obj, const QString topic, bool qt_disconnect) {
    /// remove to connection infos array this connection
    mafEventBusConnectionInfo info = m_ConnectionInfoHash.value(topic);
    int counter = 0;
    foreach (QObject *o, info.observers) {
        if(o == obj) {
            info.observers.remove(counter);
            info.slotName.remove(counter);
            break;
        }
        counter++;
    }


    if(obj == NULL) {
        return;
    }
    m_LocalDispatcher->removeObserver(obj, topic, qt_disconnect);
    m_RemoteDispatcher->removeObserver(obj, topic, qt_disconnect);
}

void mafEventBusManager::removeSignal(const QObject *obj, QString topic, bool qt_disconnect) {
    /// remove to connection infos array this connection
    mafEventBusConnectionInfo info = m_ConnectionInfoHash.value(topic);
    info.emitter = NULL;
    info.signalName = "";


    if(obj == NULL) {
        return;
    }
    //remove topic from the mafTopicRegistry
    bool result = mafEventBus::mafTopicRegistry::instance()->unregisterTopic(topic);
    if(result) {
        return;
    }

    m_LocalDispatcher->removeSignal(obj, topic, qt_disconnect);
    m_RemoteDispatcher->removeSignal(obj, topic, qt_disconnect);
}

bool mafEventBusManager::removeEventProperty(const mafEvent &props) const {
    bool result(false);
    QString topic = props[TOPIC].toString();
    QObject *obj = props[OBJECT].value<QObject*>();

    /// remove from connection info
    if(props[SIGTYPE].toInt() == mafSignatureTypeCallback) {
        mafEventBusConnectionInfo info = m_ConnectionInfoHash.value(topic);
        int counter = 0;
        foreach (QObject *o, info.observers) {
            if(o == obj) {
                info.observers.remove(counter);
                info.slotName.remove(counter);
                break;
            }
            counter++;
        }
    } else {
        mafEventBusConnectionInfo info = m_ConnectionInfoHash.value(topic);
        info.emitter = NULL;
        info.signalName = "";
    }


    if(props.eventType() == mafEventTypeLocal) {
        // Local event dispatching.
        if(props[SIGTYPE].toInt() == mafSignatureTypeCallback) {
            result = m_LocalDispatcher->removeObserver(props);
        } else {
            result = m_LocalDispatcher->removeSignal(props);
            if (result){
                result = mafEventBus::mafTopicRegistry::instance()->unregisterTopic(topic);
            }
        }
    } else {
        // Remote event dispatching.
        if(props[SIGTYPE].toInt() == mafSignatureTypeCallback) {
            result = m_RemoteDispatcher->removeObserver(props);
        } else {
            result = m_RemoteDispatcher->removeSignal(props);
            if (result){
                result = mafEventBus::mafTopicRegistry::instance()->unregisterTopic(topic);
            }
        }
    }
    return result;
}

void mafEventBusManager::notifyEvent(const QString topic, mafEventType ev_type, mafEventArgumentsList *argList, QGenericReturnArgument *returnArg, bool synch /*= true*/) const {
	QMutex mutex;
    QMutexLocker locker(&mutex);
    if(m_EnableEventLogging) {
        if(m_LogEventTopic == "*" || m_LogEventTopic == topic) {
            qDebug() << mafTr("Event notification for TOPIC: %1").arg(topic);
        }
    }

    /// increment in correct connection info item inside the array, the counter and insert the caller if possible
    mafEventBusConnectionInfo info = m_ConnectionInfoHash.value(topic);
    info.counter++;


    //event dispatched in local channel
    mafEvent *event_dic = new mafEvent(topic, ev_type, synch);
    notifyEvent(*event_dic, argList, returnArg);
    delete event_dic;
}

void mafEventBusManager::notifyEvent(const mafEvent &event_dictionary, mafEventArgumentsList *argList, QGenericReturnArgument *returnArg) const {
	QMutex mutex;
    QMutexLocker locker(&mutex);
    //event dispatched in remote channel
    if(event_dictionary[TYPE].toInt() == mafEventTypeLocal) {
        m_LocalDispatcher->notifyEvent(event_dictionary, argList, returnArg);
    } else {
        m_RemoteDispatcher->notifyEvent(event_dictionary, argList);
    }
}

void mafEventBusManager::enableEventLogging(bool enable) {
    m_EnableEventLogging = enable;
}

void mafEventBusManager::logEventTopic(const QString topic) {
    m_LogEventTopic = topic;
}

void mafEventBusManager::logAllEvents() {
    m_LogEventTopic = "*";
}

bool mafEventBusManager::createServer(const QString &communication_protocol, unsigned int listen_port) {
    if(m_NetworkConnectorHash.count() == 0) {
        initializeNetworkConnectors();
    }

    bool res(m_NetworkConnectorHash.contains(communication_protocol));
    if(res) {
        mafNetworkConnector *connector = m_NetworkConnectorHash.value(communication_protocol);
        m_RemoteDispatcher->setNetworkConnectorServer(connector);
        //mafNetworkConnector *connector = m_RemoteDispatcher->networkConnectorServer();
        res = connector != NULL;
        if(res) {
            m_RemoteDispatcher->networkConnectorServer()->createServer(listen_port);
        }
    }
    return res;
}

void mafEventBusManager::startListen() {
    mafNetworkConnector *connector = m_RemoteDispatcher->networkConnectorServer();
    if(connector) {
        connector->startListen();
    } else {
        QByteArray ba = mafTr("Server can not start. Create it first, then call startListen again!!").toLatin1();
        qWarning("%s", ba.data());
    }
}

bool mafEventBusManager::createClient(const QString &communication_protocol, const QString &server_host, unsigned int port) {
    if(m_NetworkConnectorHash.count() == 0) {
        initializeNetworkConnectors();
    }

    bool res(m_NetworkConnectorHash.contains(communication_protocol));
    if(res) {
        m_RemoteDispatcher->setNetworkConnectorClient(m_NetworkConnectorHash.value(communication_protocol));
        mafNetworkConnector *connector = m_RemoteDispatcher->networkConnectorClient();
        res = connector != NULL;
        if(res) {
            m_RemoteDispatcher->networkConnectorClient()->createClient(server_host, port);
        }
    }
    return res;
}

const mafEventBusConnectionInfoHash &mafEventBusManager::connectionInfosDump() {
    return m_ConnectionInfoHash;
}
