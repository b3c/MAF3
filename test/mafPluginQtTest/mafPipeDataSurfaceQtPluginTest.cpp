/*
 *  mafPipeDataSurfaceQtPluginTest.cpp
 *  mafPluginTest
 *
 *  Created by Daniele Giunchi on 14/12/09.
 *  Copyright 2012 SCS-B3C. All rights reserved.
 *
 *  See License at: http://tiny.cc/QXJ4D
 *
 */

#include "mafPipeDataSurfaceQtPluginTest.h"

using namespace mafPluginQtTest;

mafPipeDataSurfaceQtPluginTest::mafPipeDataSurfaceQtPluginTest(const QString code_location) : mafPipeData(code_location) {
    QByteArray ba = mafTr("qt plugin pipeline created!!").toLatin1();
    qDebug("%s", ba.data());
}

mafPipeDataSurfaceQtPluginTest::~mafPipeDataSurfaceQtPluginTest() {
}

void mafPipeDataSurfaceQtPluginTest::updatePipe(double t) {
    QByteArray ba = mafTr("qt plugin executed at time %1!!").arg(t).toLatin1();
    qDebug("%s", ba.data());
    Superclass::updatePipe(t);
}
