/*
 *  mafUtility.h
 *  mafCore
 *
 *  Created by Paolo Quadrani on 27/03/09.
 *  Copyright 2009 B3C. All rights reserved.
 *
 *  See Licence at: http://tiny.cc/QXJ4D
 *
 */

#ifndef MAFUTILITIES_H
#define MAFUTILITIES_H

extern bool mafEquals(double x, double y);
extern bool mafFloatEquals(float x, float y);
extern double mafRoundToPrecision(double val, unsigned prec);
extern bool mafIsLittleEndian(void);

#endif // MAFUTILITIES_H
