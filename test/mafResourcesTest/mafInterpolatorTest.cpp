/*
 *  mafInterpolatorTest.cpp
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
using namespace mafResources;

//------------------------------------------------------------------------------------------
/**
 Class name: testInterpolatorCustom
 This class implements the interpolator to be tested.
 */
class  testInterpolatorCustom : public  mafInterpolator {
    Q_OBJECT
    /// typedef macro.
    mafSuperclassMacro(mafResources::mafInterpolator);

public:
    /// Object constructor.
    testInterpolatorCustom(const QString code_location = "");

    /// Search the item at the given timestamp 't' with the defined interpolation strategy.
    /*virtual*/ mafDataSet *itemAt(mafDataSetMap *collection, double t);

    /// Access the result of the interpolation mechanism.
    QString interpolatedItem() {return m_InterpolatedItem;}

private:
    QString m_InterpolatedItem; ///< Test Var.
};

testInterpolatorCustom::testInterpolatorCustom(const QString code_location) : mafInterpolator(code_location), m_InterpolatedItem("") {
}

mafDataSet *testInterpolatorCustom::itemAt(mafDataSetMap *collection, double t) {
    Q_UNUSED(collection);
    Q_UNUSED(t);
    m_InterpolatedItem = "Interpolated";
    return NULL;
}
//------------------------------------------------------------------------------------------


void mafInterpolatorTest::initTestCase() {
    mafMessageHandler::instance()->installMessageHandler();
    m_Interpolator = mafNEW(testInterpolatorCustom);
}

void mafInterpolatorTest::cleanupTestCase() {
    mafDEL(m_Interpolator);
    mafMessageHandler::instance()->shutdown();
}

void mafInterpolatorTest::mafInterpolatorAllocationTest() {
    QVERIFY(m_Interpolator != NULL);
}

void mafInterpolatorTest::mafInterpolationStrategyTest() {
    QString res("Interpolated");
    mafDataSet *data = m_Interpolator->itemAt(&m_ItemMap, 0.0);
    QVERIFY(data == NULL);
    QCOMPARE(m_Interpolator->interpolatedItem(), res);
}

#include "mafInterpolatorTest.moc"
