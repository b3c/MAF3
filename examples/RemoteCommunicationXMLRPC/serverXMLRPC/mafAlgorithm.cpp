/*
 *  mafAlgorithm.cpp
 *  serverXMLRPC
 *
 *  Created by Paolo Quadrani on 17/09/09.
 *  Copyright 2009 SCS-B3C. All rights reserved.
 *
 *  See Licence at: http://tiny.cc/QXJ4D
 *
 */

#include "mafAlgorithm.h"
#include <mafIdProvider.h>
#include <mafEventBusManager.h>

using namespace mafCore;
using namespace mafEventBus;
using namespace mafResources;

mafAlgorithm::mafAlgorithm(const QString code_location) : mafOperation(code_location) {
}

void mafAlgorithm::execute() {
    m_Status = mafOperationStatusExecuting;
    qDebug() << mafTr("Executing testProcess!!!");

    QStringList commandAndParameters;
    commandAndParameters << m_FileNameInput.toLatin1()  \
                        << m_FileNameOutput.toLatin1() \
                        << QString::number(m_IterationParameter);

    bool ok = QProcess::execute("./testProcess" ,commandAndParameters) == 0;
    Q_EMIT executionEnded();
}

void mafAlgorithm::terminated() {
    
}

QString mafAlgorithm::inputFileName() const {
    return m_FileNameInput;
}

QString mafAlgorithm::outputFileName() const {
    return m_FileNameOutput;
}

int mafAlgorithm::iterations() const {
    return m_IterationParameter;
}

void mafAlgorithm::setInputFileName(const QString &f) {
    m_FileNameInput = f;
}

void mafAlgorithm::setOutputFileName(const QString &f){
    m_FileNameOutput = f;    
}

void mafAlgorithm::setIterations(const int i) {
    m_IterationParameter = i;
}
