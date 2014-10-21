/*
 *  mafUILoaderTest.cpp
 *  mafGUITest
 *
 *  Created by Paolo Quadrani on 26/10/10.
 *  Copyright 2010 SCS-B3C. All rights reserved.
 *
 *  See Licence at: http://tiny.cc/QXJ4D
 *
 */

#include "mafGUITestList.h"

using namespace mafCore;
using namespace mafEventBus;
using namespace mafGUI;

/**
  Class Name: testmafUILoaderCustom
  Custom UILoader for testing mafUILoader interface.
*/
class testmafUILoaderCustom : public mafUILoader {
    Q_OBJECT

public:
    ///object constructor
    testmafUILoaderCustom();
    /// method for load the file
    /*virtual*/ void uiLoad(const QString& fileName, int ui_type);
    /// check if gui is loaded
    bool isUILoaded() {
        return m_IsUILoaded;
    }

public Q_SLOTS:
   ///return a value when the gui is loaded
   void uiLoaded(mafCore::mafProxyInterface *widget, int ui_type);

private:
     bool m_IsUILoaded;///< variable which represents if the ui is loaded
};

testmafUILoaderCustom::testmafUILoaderCustom() : mafUILoader(), m_IsUILoaded(false) {
}

void testmafUILoaderCustom::uiLoad(const QString& fileName, int ui_type) {
    REQUIRE(!fileName.isEmpty());
    mafProxyInterface *gui = NULL;
    mafEventArgumentsList list;
    list.append(mafEventArgument(mafCore::mafProxyInterface *, gui));
    list.append(mafEventArgument(int, ui_type));
    mafEventBusManager::instance()->notifyEvent("maf.local.gui.uiloaded", mafEventTypeLocal, &list);
}

void testmafUILoaderCustom::uiLoaded(mafCore::mafProxyInterface  *widget, int ui_type) {
    Q_UNUSED(widget);
    qDebug() << QString("ui loaded of type %1").arg(ui_type);
    m_IsUILoaded = true;
}

void mafUILoaderTest::initTestCase() {
    m_UILoader = new testmafUILoaderCustom();
    mafRegisterLocalCallback("maf.local.gui.uiloaded", m_UILoader, "uiLoaded(mafCore::mafProxyInterface *, int)");
}

void mafUILoaderTest::cleanupTestCase() {
    delete m_UILoader;
    m_UILoader = NULL;
    mafEventBusManager::instance()->shutdown();
}

void mafUILoaderTest::mafUILoaderAllocationTest() {
    QVERIFY(m_UILoader != NULL);
}

void mafUILoaderTest::mafUILoaderUILoadTest() {
    QString uiFile(MAF_DATA_DIR);
    uiFile.append("/GUI/testUIFile.ui");
    m_UILoader->uiLoad(uiFile, 0);
    QVERIFY(m_UILoader->isUILoaded() == true);
}

#include "mafUILoaderTest.moc"
