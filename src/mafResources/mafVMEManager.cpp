/*
 *  mafVMEManager.cpp
 *  mafResources
 *
 *  Created by Paolo Quadrani on 30/12/09.
 *  Copyright 2011 SCS-B3C. All rights reserved.
 *
 *  See License at: http://tiny.cc/QXJ4D
 *
 */

#include "mafVMEManager.h"
#include "mafVME.h"
#include "mafSceneNode.h"
#include "mafDataSet.h"

using namespace mafCore;
using namespace mafResources;
using namespace mafEventBus;

#include <QMessageBox>
#include <fstream>

mafVMEManager* mafVMEManager::instance() {
    static mafVMEManager instanceVMEManager;
    return &instanceVMEManager;
}

void mafVMEManager::shutdown() {
    mafDEL(m_VMEHierarchy);
    mafDEL(m_Root);
    m_SelectedVME = NULL;
}

mafVMEManager::mafVMEManager(const QString code_location) : mafObjectBase(code_location), m_SelectedVME(NULL), m_Root(NULL), m_VMEHierarchy(NULL) {
    initializeConnections();
}

mafVMEManager::~mafVMEManager() {
    shutdown();
    
    // Unregister callbacks...
    mafUnregisterLocalCallback("maf.local.resources.vme.add", this, "vmeAdd(mafCore::mafObjectBase *)")
    mafUnregisterLocalCallback("maf.local.resources.vme.remove", this, "vmeRemove(mafCore::mafObjectBase *)")
    mafUnregisterLocalCallback("maf.local.resources.vme.reparent", this, "vmeReparent(mafCore::mafObjectBase *, mafCore::mafObjectBase *)")
    mafUnregisterLocalCallback("maf.local.resources.vme.select", this, "vmeSelect(mafCore::mafObjectBase *)")
    mafUnregisterLocalCallback("maf.local.resources.vme.selected", this, "selectedVME()")
    mafUnregisterLocalCallback("maf.local.resources.vme.absolutePoseMatrix", this, "absolutePoseMatrix(mafCore::mafObjectBase *)")
    mafUnregisterLocalCallback("maf.local.resources.hierarchy.request", this, "requestVMEHierarchy()")
    mafUnregisterLocalCallback("maf.local.resources.hierarchy.new", this, "newVMEHierarchy()")
    
    // Unregister signals...
    mafUnregisterLocalSignal("maf.local.resources.vme.add", this, "attachVMEToHierarchy(mafCore::mafObjectBase *)")
    mafUnregisterLocalSignal("maf.local.resources.vme.remove", this, "detachVMEFromHierarchy(mafCore::mafObjectBase *)")
    mafUnregisterLocalSignal("maf.local.resources.vme.reparent", this, "reparentVMESignal(mafCore::mafObjectBase *, mafCore::mafObjectBase *)")
    mafUnregisterLocalSignal("maf.local.resources.vme.select", this, "selectVME(mafCore::mafObjectBase *)")
    mafUnregisterLocalSignal("maf.local.resources.vme.selected", this, "selectedVMESignal()")
    mafUnregisterLocalSignal("maf.local.resources.vme.absolutePoseMatrix", this, "absolutePoseMatrixSignal(mafCore::mafObjectBase *)")
    mafUnregisterLocalSignal("maf.local.resources.hierarchy.request", this, "requestVMEHierarchySignal()")
    mafUnregisterLocalSignal("maf.local.resources.hierarchy.new", this, "newVMEHierarchySignal()")

    // Remove IDs.
    mafIdProvider *provider = mafIdProvider::instance();
    provider->removeId("maf.local.resources.vme.add");
    provider->removeId("maf.local.resources.vme.remove");
    provider->removeId("maf.local.resources.vme.reparent");
    provider->removeId("maf.local.resources.vme.select");
    provider->removeId("maf.local.resources.vme.selected");
    provider->removeId("maf.local.resources.vme.absolutePoseMatrix");
    provider->removeId("maf.local.resources.hierarchy.request");
    provider->removeId("maf.local.resources.hierarchy.new");
}

void mafVMEManager::initializeConnections() {
    // Create the IDs required to add a resource to the management system.
    mafIdProvider *provider = mafIdProvider::instance();
    provider->createNewId("maf.local.resources.vme.add");
    provider->createNewId("maf.local.resources.vme.remove");
    provider->createNewId("maf.local.resources.vme.reparent");
    provider->createNewId("maf.local.resources.vme.select");
    provider->createNewId("maf.local.resources.vme.selected");
    provider->createNewId("maf.local.resources.vme.absolutePoseMatrix");
    provider->createNewId("maf.local.resources.hierarchy.request");
    provider->createNewId("maf.local.resources.hierarchy.new");
    provider->createNewId("maf.local.resources.hierarchy.root");


    // Register API signals.
    mafRegisterLocalSignal("maf.local.resources.vme.add", this, "attachVMEToHierarchy(mafCore::mafObjectBase *)")
    mafRegisterLocalSignal("maf.local.resources.vme.remove", this, "detachVMEFromHierarchy(mafCore::mafObjectBase *)")
    mafRegisterLocalSignal("maf.local.resources.vme.reparent", this, "reparentVMESignal(mafCore::mafObjectBase *, mafCore::mafObjectBase *)")
    mafRegisterLocalSignal("maf.local.resources.vme.select", this, "selectVME(mafCore::mafObjectBase *)")
    mafRegisterLocalSignal("maf.local.resources.vme.selected", this, "selectedVMESignal()")
    mafRegisterLocalSignal("maf.local.resources.vme.absolutePoseMatrix", this, "absolutePoseMatrixSignal(mafCore::mafObjectBase *)")
    mafRegisterLocalSignal("maf.local.resources.hierarchy.request", this, "requestVMEHierarchySignal()")
    mafRegisterLocalSignal("maf.local.resources.hierarchy.new", this, "newVMEHierarchySignal()")
    mafRegisterLocalSignal("maf.local.resources.hierarchy.root", this, "rootSignal()")

    
    // Register private callbacks to the instance of the manager..
    mafRegisterLocalCallback("maf.local.resources.vme.add", this, "vmeAdd(mafCore::mafObjectBase *)")
    mafRegisterLocalCallback("maf.local.resources.vme.remove", this, "vmeRemove(mafCore::mafObjectBase *)")
    mafRegisterLocalCallback("maf.local.resources.vme.reparent", this, "vmeReparent(mafCore::mafObjectBase *, mafCore::mafObjectBase *)")
    mafRegisterLocalCallback("maf.local.resources.vme.select", this, "vmeSelect(mafCore::mafObjectBase *)")
    mafRegisterLocalCallback("maf.local.resources.vme.selected", this, "selectedVME()")
    mafRegisterLocalCallback("maf.local.resources.vme.absolutePoseMatrix", this, "absolutePoseMatrix(mafCore::mafObjectBase *)")
    mafRegisterLocalCallback("maf.local.resources.hierarchy.request", this, "requestVMEHierarchy()")
    mafRegisterLocalCallback("maf.local.resources.hierarchy.new", this, "newVMEHierarchy()")
    mafRegisterLocalCallback("maf.local.resources.hierarchy.root", this, "root()")

}

void mafVMEManager::vmeSelect(mafCore::mafObjectBase *vme) {
    mafVME *vme_to_select = qobject_cast<mafResources::mafVME *>(vme);
    if(NULL == vme_to_select) {
        QByteArray ba = mafTr("Trying to select an object that not represent a mafVME.").toLatin1();
        qWarning("%s", ba.data());
        return;
    }
    // VME has been selected.
    m_SelectedVME = vme_to_select;
}

void mafVMEManager::vmeAdd(mafCore::mafObjectBase *vme) {
    if(NULL == vme) {
        QByteArray ba = mafTr("Trying to add an object that not represent a mafVME.").toLatin1();
        qWarning("%s", ba.data());
        return;
    }

    // VME has been added.
    connect(vme, SIGNAL(destroyed()), this, SLOT(vmeDestroyed()), Qt::DirectConnection);
    if(m_VMEHierarchy) {
        m_VMEHierarchy->addHierarchyNode(vme, m_SelectedVME);
    }
}

void mafVMEManager::vmeRemove(mafCore::mafObjectBase *vme) {
    mafVME *vme_to_remove = qobject_cast<mafResources::mafVME *>(vme);
    if(NULL == vme_to_remove) {
        QByteArray ba = mafTr("Trying to remove an object that not represent a mafVME.").toLatin1();
        qWarning("%s", ba.data());
        return;
    }
    // VME has been removed.
    vme_to_remove->detatchFromTree();

    // select root.
    mafEventArgumentsList argList;
    argList.append(mafEventArgument( mafCore::mafObjectBase *, m_Root));
    mafEventBusManager::instance()->notifyEvent("maf.local.resources.vme.select", mafEventTypeLocal, &argList);

    m_VMEHierarchy->removeHierarchyNode(vme);
}

void mafVMEManager::vmeReparent(mafCore::mafObjectBase *vme, mafCore::mafObjectBase *vmeParent) {
  if (m_VMEHierarchy != NULL) {
    QString objName = vme->metaObject()->className();
    if (objName.contains("mafSceneNode")) {
      mafSceneNode *obj = qobject_cast<mafSceneNode *>(vme);
      mafSceneNode *objParent = qobject_cast<mafSceneNode *>(vmeParent);
      m_VMEHierarchy->reparentHierarchyNode(obj->vme(), objParent->vme());
    } else {
        m_VMEHierarchy->reparentHierarchyNode(vme, vmeParent);
    }
  }
}


void mafVMEManager::vmeDestroyed() {
    mafVME *vme = (mafVME *)QObject::sender();
    if ( vme == m_SelectedVME ) {
        m_SelectedVME = NULL;
    }
}

//void mafVMEManager::removeVME(mafVME *vme) {
    // Disconnect the manager from the vme
//    disconnect(vme, SIGNAL(destroyed()),this, SLOT(vmeDestroyed()));
    // remove the VME from the managed resources and manage the active resource if the removed VME is the active one.
//    if ( vme == m_SelectedVME ) {
//        m_SelectedVME = NULL;
//    }
    //m_VMEHierarchy->removeHierarchyNode(vme); // DEPRECATED
//}

mafCore::mafHierarchyPointer mafVMEManager::requestVMEHierarchy() {
    if ( m_VMEHierarchy == NULL ) {
         m_VMEHierarchy = mafNEW(mafCore::mafHierarchy);
    }
	
    if (m_Root == NULL) {
        //Create a new root.
        m_Root = mafNEW(mafResources::mafVME);
        m_Root->setObjectName("root");
        m_Root->setProperty("iconType", "mafRoot");

        //Add root to hierarchy
        mafEventArgumentsList argList;
        argList.append(mafEventArgument(mafCore::mafObjectBase *, m_Root));
        mafEventBusManager::instance()->notifyEvent("maf.local.resources.vme.add", mafEventTypeLocal, &argList);
        
        //Select root
        mafEventBusManager::instance()->notifyEvent("maf.local.resources.vme.select", mafEventTypeLocal, &argList);
		//DEBUG_VAR_FILE("C:\\Projects\\MAF3.buildcmakeTESTCMAKE\\build\\bin\\Release\\debugLog.txt", m_Root)
     }

     return m_VMEHierarchy;
}

mafCore::mafHierarchyPointer mafVMEManager::newVMEHierarchy() {
    shutdown();
    return requestVMEHierarchy();
}

mafMatrix4x4Pointer mafVMEManager::absolutePoseMatrix(mafCore::mafObjectBase *vme) {
    //calculate absolute matrix navigating hierarchy from leaf to root and compose the absolute pose matrix 
    //for vme.
    mafMatrix4x4 *matrix = qobject_cast<mafVME *>(vme)->dataSetCollection()->itemAtCurrentTime()->poseMatrix();
    mafMatrix4x4 *result = new mafMatrix4x4(*matrix);
    
    if(m_VMEHierarchy == NULL) {
        return NULL;
    }
    
    QObject *selectedNode = m_VMEHierarchy->currentData();
    m_VMEHierarchy->moveTreeIteratorToNode(vme);
    while(m_VMEHierarchy->hasParent()) {
        m_VMEHierarchy->moveTreeIteratorToParent();
        mafVME *parentVME = qobject_cast<mafVME *>(m_VMEHierarchy->currentData());
        mafMatrix4x4 *matrixParentVME = parentVME->dataSetCollection()->poseMatrix();
        *result = (*matrixParentVME) * (*result);
    }
        
    m_VMEHierarchy->moveTreeIteratorToNode(selectedNode);
    
    return result;
}

