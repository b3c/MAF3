/*=========================================================================

  Program:   Multimod Fundation Library
  Module:    $RCSfile: vtkLocalAxisCoordinate.h,v $
  Language:  C++
  Date:      $Date: 2011/07/01 13:39:06 $
  Version:   $Revision: 1.1 $
  Authors:   Paolo Quadrani
  Project:   MAF3 Framework (http://www.openmaf.org)

==========================================================================
  Copyright (c) 2002/2003 
  CINECA - Interuniversity Consortium (www.cineca.it)
  v. Magnanelli 6/3
  40033 Casalecchio di Reno (BO)
  Italy
  ph. +39-051-6171411 (90 lines) - Fax +39-051-6132198

Use, modification and redistribution of the software, in source or
binary forms, are permitted provided that the following terms and
conditions are met:

1) Redistribution of the source code, in verbatim or modified
   form, must retain the above copyright notice, this license,
   the following disclaimer, and any notices that refer to this
   license and/or the following disclaimer.  

2) Redistribution in binary form must include the above copyright
   notice, a copy of this license and the following disclaimer
   in the documentation or with other materials provided with the
   distribution.

3) Modified copies of the source code must be clearly marked as such,
   and must not be misrepresented as verbatim copies of the source code.

THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES PROVIDE THE SOFTWARE "AS IS"
WITHOUT EXPRESSED OR IMPLIED WARRANTY INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE.  IN NO EVENT SHALL ANY COPYRIGHT HOLDER OR OTHER PARTY WHO MAY
MODIFY AND/OR REDISTRIBUTE THE SOFTWARE UNDER THE TERMS OF THIS LICENSE
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, LOSS OF DATA OR DATA BECOMING INACCURATE
OR LOSS OF PROFIT OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF
THE USE OR INABILITY TO USE THE SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGES.
=========================================================================*/
#ifndef __vtkLocalAxisCoordinate_h
#define __vtkLocalAxisCoordinate_h

#include "vtkMAF_global.h"

#include <vtkCoordinate.h>
#include <vtkProp3D.h>
#include <vtkMatrix4x4.h>
#include <vtkDataSet.h>

class vtkViewport;
//----------------------------------------------------------------------------
class VTK_vtkMAF_EXPORT vtkLocalAxisCoordinate : public vtkCoordinate
//----------------------------------------------------------------------------
{
public:
  //vtkTypeRevisionMacro(vtkLocalAxisCoordinate,vtkCoordinate);

           vtkLocalAxisCoordinate();
  virtual ~vtkLocalAxisCoordinate();

  static vtkLocalAxisCoordinate* New();

  vtkSetObjectMacro(DataSet,vtkDataSet);
  vtkGetObjectMacro(DataSet,vtkDataSet);

  vtkSetObjectMacro(Matrix,vtkMatrix4x4);
  vtkGetObjectMacro(Matrix,vtkMatrix4x4);

  virtual double *GetComputedUserDefinedValue(vtkViewport *viewport);

protected:

private:
  vtkDataSet *DataSet;
  vtkMatrix4x4 *Matrix;
	
	vtkLocalAxisCoordinate(const vtkLocalAxisCoordinate&);  // Not implemented.
  void operator=(const vtkLocalAxisCoordinate&);     // Not implemented.
};
#endif
