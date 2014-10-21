/*
 *  mafIdProvider.cpp
 *  mafCore
 *
 *  Created by Paolo Quadrani on 27/03/09.
 *  Copyright 2011 SCS-B3C. All rights reserved.
 *  
 *  See License at: http://tiny.cc/QXJ4D
 *  
 */

#include "mafIdProvider.h"
#include "mafObjectRegistry.h"

using namespace mafCore;


mafIdProvider::mafIdProvider() {
    m_Id = 0;
}

mafIdProvider::~mafIdProvider() {
    m_Id = -1; // Invalid ID.
    m_IdHash.clear();
}

mafIdProvider* mafIdProvider::instance() {
    static mafIdProvider instanceProvider;
    return &instanceProvider;
}

mafId mafIdProvider::createNewId(const QString id_name) {
    if(id_name.isEmpty()) {
        QString name;
        name.append("OBJECT_ID_");
        name.append(QString::number(m_Id));
        m_IdHash.insert(m_Id, name);
    } else {
        // ID with custom name: check if not already inserted!! No multiple names are accepted.
        mafId id_value = idValue(id_name);
        if(id_value == -1) {
            m_IdHash.insert(m_Id, id_name);
        } else {
            // if the Event id is already defined, return the previous defined ID value.
            QByteArray ba = mafTr("ID with name '%1'' has been already defined!").arg(id_name).toLatin1();
            qWarning("%s", ba.data());
            return id_value;
        }
    }
    mafId returnValue = m_Id;
    ++m_Id;
    return returnValue;
}

bool mafIdProvider::removeId(const QString id_name) {
    int removed_items = 0;
    mafId id_value = idValue(id_name);
    if(m_IdHash.contains(id_value)) {
        removed_items = m_IdHash.remove(id_value);
    }
    return removed_items != 0;
}

bool mafIdProvider::setIdName(const mafId id, const QString id_name) {
    if(m_IdHash.contains(id) && idValue(id_name) == -1) {
        // id exists and name not yet used, so can be assigned to the id.
        m_IdHash.insert(id, id_name);
        return true;
    }
    return false;
}

const QString mafIdProvider::idName(const mafId id_value) const {
    return m_IdHash.value(id_value);
}

mafId mafIdProvider::idValue(const QString id_name) const {
    return m_IdHash.key(id_name, -1);
}

void mafIdProvider::shutdown() {
    m_IdHash.clear();
}
