/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkMAFVolumeSlicer.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkMAFVolumeSlicer.h"

#include <vtkExecutive.h>
#include <vtkDemandDrivenPipeline.h>
#include <vtkFloatArray.h>
//#include <vtkImplicitFunction.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkImageData.h>
#include <vtkRectilinearGrid.h>
#include <vtkCellArray.h>
#include <vtkDoubleArray.h>
#include <vtkLinearTransform.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <vtkMath.h>
#include <math.h>

vtkStandardNewMacro(vtkMAFVolumeSlicer);

typedef unsigned short u_short;
typedef unsigned char u_char;
typedef unsigned int u_int;

#define min(x0, x1) (((x0) < (x1)) ? (x0) : (x1))
#define max(x0, x1) (((x0) > (x1)) ? (x0) : (x1))

static const int SamplingTableSize = 64000;

//----------------------------------------------------------------------------
// Construct with user-specified implicit function; InsideOut turned off; value
// set to 0.0; and generate clip scalars turned off.
vtkMAFVolumeSlicer::vtkMAFVolumeSlicer()
{
    PlaneOrigin[0] = PlaneOrigin[1] = PlaneOrigin[2] = 0.f;
    PlaneAxisX[0] = 1.f;
    PlaneAxisX[1] = PlaneAxisX[2] = 0.f;
    PlaneAxisY[0] = PlaneAxisY[2] = 0.f;
    PlaneAxisY[1] = 1.f;
    
    GlobalPlaneOrigin[0] = GlobalPlaneOrigin[1] = GlobalPlaneOrigin[2] = 0.f;
    GlobalPlaneAxisX[0] = 1.f;
    GlobalPlaneAxisX[1] = GlobalPlaneAxisX[2] = 0.f;
    GlobalPlaneAxisY[0] = GlobalPlaneAxisY[2] = 0.f;
    GlobalPlaneAxisY[1]  = 1.f;
    
    TransformSlice = NULL;
    
    this->Window = 1.f;
    this->Level  = 0.5f;
    
    this->AutoSpacing = true;
    
    this->VoxelCoordinates[0] = this->VoxelCoordinates[1] = this->VoxelCoordinates[2] = NULL;    

    this->SetNumberOfInputPorts(1);
    this->SetNumberOfOutputPorts(2);

    vtkPolyData *output1 = vtkPolyData::New();
    this->GetExecutive()->SetOutputData(0, output1);
    output1->Delete();

    
    vtkImageData *output2 = vtkImageData::New();
    output2->SetExtent(0, 511, 0, 511, 0, 0);
    output2->SetDimensions(512, 512, 1);
    this->GetExecutive()->SetOutputData(1, output2);
    output2->Delete();
    
}

//----------------------------------------------------------------------------
vtkMAFVolumeSlicer::~vtkMAFVolumeSlicer() {
    delete [] this->VoxelCoordinates[0];
    delete [] this->VoxelCoordinates[1];
    delete [] this->VoxelCoordinates[2];
}

//----------------------------------------------------------------------------
int vtkMAFVolumeSlicer::FillInputPortInformation( int /*port*/, vtkInformation* info) {
    // All input ports consume polygonal data. 
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");

    return 1;
}

//----------------------------------------------------------------------------
int vtkMAFVolumeSlicer::FillOutputPortInformation( int port, vtkInformation* info) {
    // All output ports produce polygonal data.
    if (port == 0) {
        info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
    } else {
        info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkImageData");
    }

    return 1;
}

//----------------------------------------------------------------------------
void vtkMAFVolumeSlicer::SetPlaneAxisX(float axis[3]) 
//----------------------------------------------------------------------------
{
    if (vtkMath::Norm(axis) < 1.e-5f)
        return;
    memcpy(this->PlaneAxisX, axis, sizeof(this->PlaneAxisX));
    vtkMath::Normalize(this->PlaneAxisX);
    if (TransformSlice)
    {
        TransformSlice->TransformNormal(PlaneAxisX, GlobalPlaneAxisX);
    }
    else
    {
        memcpy(GlobalPlaneAxisX, PlaneAxisX, sizeof(this->PlaneAxisX));
    }
    this->Modified();
}
//----------------------------------------------------------------------------
void vtkMAFVolumeSlicer::SetPlaneAxisY(float axis[3])
//----------------------------------------------------------------------------
{
    if (vtkMath::Norm(axis) < 1.e-5f)
        return;
    memcpy(this->PlaneAxisY, axis, sizeof(this->PlaneAxisY));
    vtkMath::Normalize(this->PlaneAxisY);
    vtkMath::Cross(this->PlaneAxisY, this->PlaneAxisX, this->PlaneAxisZ);
    vtkMath::Normalize(this->PlaneAxisZ);
    vtkMath::Cross(this->PlaneAxisZ, this->PlaneAxisX, this->PlaneAxisY);
    vtkMath::Normalize(this->PlaneAxisY);
    if (TransformSlice)
    {
        TransformSlice->TransformNormal(PlaneAxisY, GlobalPlaneAxisY);
    }
    else
    {
        memcpy(GlobalPlaneAxisY, PlaneAxisY, sizeof(this->PlaneAxisY));
    }
    this->Modified();
}
//----------------------------------------------------------------------------
void vtkMAFVolumeSlicer::SetPlaneOrigin(double origin[3])
//----------------------------------------------------------------------------
{
    memcpy(PlaneOrigin, origin, sizeof(PlaneOrigin));
    if (TransformSlice)
    {
        TransformSlice->TransformPoint(PlaneOrigin, GlobalPlaneOrigin);
    }
    else
    {
        memcpy(GlobalPlaneOrigin, PlaneOrigin, sizeof(PlaneOrigin));
    }
    this->Modified();
}
//----------------------------------------------------------------------------
void vtkMAFVolumeSlicer::SetPlaneOrigin(double x, double y, double z)
//----------------------------------------------------------------------------
{
    double planeOrigin[3];
    planeOrigin[0] = x;
    planeOrigin[1] = y;
    planeOrigin[2] = z;
    SetPlaneOrigin(planeOrigin);
}


//----------------------------------------------------------------------------
// Overload standard modified time function. If Clip functions is modified,
// then this object is modified as well.
unsigned long vtkMAFVolumeSlicer::GetMTime()
{
    unsigned long mTime=this->Superclass::GetMTime();
    unsigned long time;

    /*  if ( this->ClipFunction != NULL )
    {
        time = this->ClipFunction->GetMTime();
        mTime = ( time > mTime ? time : mTime );
    }
    */
    
    if (this->TransformSlice && this->TransformSlice->GetMTime() > time) {
        time = this->TransformSlice->GetMTime();
    }
    
    return mTime;
}

//----------------------------------------------------------------------------
vtkImageData *vtkMAFVolumeSlicer::GetTexturedOutput()
{
    vtkDataObject *data = this->GetOutputDataObject(1);
    return vtkImageData::SafeDownCast(data);
}

vtkDataObject *vtkMAFVolumeSlicer::GetInput() {
    vtkInformationVector **iv = this->GetExecutive()->GetInputInformation();
    vtkInformation *inInfo = iv[0]->GetInformationObject(0);
    vtkDataSet *input = vtkDataSet::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
    return input;
}

//----------------------------------------------------------------------------
void vtkMAFVolumeSlicer::PrepareVolume() 
{
    assert(fabs(vtkMath::Norm(this->GlobalPlaneAxisX) - 1.f) < 1.e-5);
    assert(fabs(vtkMath::Norm(this->GlobalPlaneAxisY) - 1.f) < 1.e-5);
    vtkMath::Cross(this->GlobalPlaneAxisX, this->GlobalPlaneAxisY, this->GlobalPlaneAxisZ);
    vtkMath::Normalize(this->GlobalPlaneAxisZ);
    
    if (PreprocessingTime > this->GetInput()->GetMTime() && PreprocessingTime > this->GetMTime())
        return;
    
    vtkImageData       *imageData = vtkImageData::SafeDownCast(this->GetInput());
    vtkRectilinearGrid *gridData  = vtkRectilinearGrid::SafeDownCast(this->GetInput());
    
    if (imageData) 
    {
        double dataSpacing[3];
        imageData->GetDimensions(this->DataDimensions);
        imageData->GetOrigin(this->DataOrigin);
        imageData->GetSpacing(dataSpacing);
        for (int axis = 0; axis < 3; axis++) 
        {
            delete [] this->VoxelCoordinates[axis];
            this->VoxelCoordinates[axis] = new float [this->DataDimensions[axis] + 1];
            float f = this->DataOrigin[axis];
            for (int i = 0; i <= this->DataDimensions[axis]; i++, f += dataSpacing[axis])
                this->VoxelCoordinates[axis][i] = f;
        }
    }
    else 
    {
        gridData->GetDimensions(this->DataDimensions);
        this->DataOrigin[0] = gridData->GetXCoordinates()->GetTuple(0)[0];
        this->DataOrigin[1] = gridData->GetYCoordinates()->GetTuple(0)[0];
        this->DataOrigin[2] = gridData->GetZCoordinates()->GetTuple(0)[0];
        
        for (int axis = 0; axis < 3; axis++) 
        {
            delete [] this->VoxelCoordinates[axis];
            this->VoxelCoordinates[axis] = new float [this->DataDimensions[axis] + 1];
            
            vtkDataArray *coordinates = (axis == 2) ? gridData->GetZCoordinates() : (axis == 1 ? gridData->GetYCoordinates() : gridData->GetXCoordinates());
            const float spacing = *(coordinates->GetTuple(1)) - *(coordinates->GetTuple(0));
            const float blockSpacingThreshold = 0.01f * spacing + 0.001f;
            int i;
            for (i = 0; i < this->DataDimensions[axis]; i++) 
            {
                this->VoxelCoordinates[axis][i] = *(coordinates->GetTuple(i));
                if (i > 0 && fabs(this->VoxelCoordinates[axis][i] - this->VoxelCoordinates[axis][i - 1] - spacing) > blockSpacingThreshold) 
                {
                    // try to correct the coordinates
                    if (i < (this->DataDimensions[axis] - 1) && fabs(*(coordinates->GetTuple(i + 1)) - this->VoxelCoordinates[axis][i - 1] - 2.f * spacing) < blockSpacingThreshold) 
                    {
                        this->VoxelCoordinates[axis][i]     = this->VoxelCoordinates[axis][i - 1] + spacing;
                        this->VoxelCoordinates[axis][i + 1] = this->VoxelCoordinates[axis][i] + spacing;
                        i++;
                    }
                }
            }
            this->VoxelCoordinates[axis][i] = this->VoxelCoordinates[axis][i - 1] + (i > 2 ? (this->VoxelCoordinates[axis][i - 1] - this->VoxelCoordinates[axis][i - 2]) : 0.f);
        }
    }
    
    this->DataBounds[0][0] = min(this->VoxelCoordinates[0][0], this->VoxelCoordinates[0][this->DataDimensions[0] - 1]);
    this->DataBounds[0][1] = max(this->VoxelCoordinates[0][0], this->VoxelCoordinates[0][this->DataDimensions[0] - 1]);
    this->DataBounds[1][0] = min(this->VoxelCoordinates[1][0], this->VoxelCoordinates[1][this->DataDimensions[1] - 1]);
    this->DataBounds[1][1] = max(this->VoxelCoordinates[1][0], this->VoxelCoordinates[1][this->DataDimensions[1] - 1]);
    this->DataBounds[2][0] = min(this->VoxelCoordinates[2][0], this->VoxelCoordinates[2][this->DataDimensions[2] - 1]);
    this->DataBounds[2][1] = max(this->VoxelCoordinates[2][0], this->VoxelCoordinates[2][this->DataDimensions[2] - 1]);
    
    this->SamplingTableMultiplier[0] = SamplingTableSize / (this->DataBounds[0][1] - this->DataBounds[0][0]);
    this->SamplingTableMultiplier[1] = SamplingTableSize / (this->DataBounds[1][1] - this->DataBounds[1][0]);
    this->SamplingTableMultiplier[2] = SamplingTableSize / (this->DataBounds[2][1] - this->DataBounds[2][0]);
    
    this->PreprocessingTime.Modified();
}
//----------------------------------------------------------------------------
void vtkMAFVolumeSlicer::ComputeInputUpdateExtents(vtkDataObject *output) 
//----------------------------------------------------------------------------
{
    vtkDataObject *input = this->GetInput();
    input->SetUpdateExtentToWholeExtent();
}

//----------------------------------------------------------------------------
template<typename InputDataType, typename OutputDataType> void vtkMAFVolumeSlicer::CreateImage(const InputDataType *input, OutputDataType *output, vtkImageData *outputObject) 
//----------------------------------------------------------------------------
{
    // prepare data for sampling
    int dims[3];
    outputObject->GetDimensions(dims);
    const int xs = dims[0], ys = dims[1];
    const int numComp = outputObject->GetNumberOfScalarComponents();
    assert(numComp == this->NumComponents);
    const float dx = outputObject->GetSpacing()[0], dy = outputObject->GetSpacing()[1];
    const float xaxis[3] = { this->GlobalPlaneAxisX[0] * dx * this->SamplingTableMultiplier[0], this->GlobalPlaneAxisX[1] * dx * this->SamplingTableMultiplier[1], this->GlobalPlaneAxisX[2] * dx * this->SamplingTableMultiplier[2]};
    const float yaxis[3] = { this->GlobalPlaneAxisY[0] * dy * this->SamplingTableMultiplier[0], this->GlobalPlaneAxisY[1] * dy * this->SamplingTableMultiplier[1], this->GlobalPlaneAxisY[2] * dy * this->SamplingTableMultiplier[2]};
    const float offset[3] = {(this->GlobalPlaneOrigin[0] - this->DataOrigin[0]) * this->SamplingTableMultiplier[0],
        (this->GlobalPlaneOrigin[1] - this->DataOrigin[1]) * this->SamplingTableMultiplier[1],
        (this->GlobalPlaneOrigin[2] - this->DataOrigin[2]) * this->SamplingTableMultiplier[2]};
    
    // prepare sampling table
    int   *stIndices[3];
    float *stOffsets[3];
    
    for (int c = 0; c < 3; c++) 
    {
        const int indexMultiplier = ((c == 0) ? 1 : (c == 1 ? this->DataDimensions[0] : (this->DataDimensions[0] * this->DataDimensions[1]))) * numComp;
        stIndices[c] = new int   [SamplingTableSize + 1];
        stOffsets[c] = new float [SamplingTableSize + 1];
        
        const float *coords = this->VoxelCoordinates[c];
        const int lastIndex = this->DataDimensions[c] - 1;
        const float coordToIndex = float(SamplingTableSize - 1) / (coords[lastIndex] - coords[0]);
        for (int i = 0, ti0 = 0; i <= lastIndex; i++) 
        {
            const int ti1 = (i != lastIndex) ? (coords[i] - coords[0]) * coordToIndex : (SamplingTableSize);
            if (ti1 <= ti0)
                continue;
            for (int ti = ti0; ti <= ti1; ti++) 
            {
                stIndices[c][ti] = (i - 1) * indexMultiplier;
                stOffsets[c][ti] = float(ti1 - ti) / float(ti1 - ti0);
            }
            ti0 = ti1;
        }
    }
    
    const float shift = this->Window / 2.0 - this->Level;
    const float scale = 1.0 / this->Window;
    
    memset(output, 0, sizeof(OutputDataType) * xs * ys * numComp);
    OutputDataType *pixel = output;
    const InputDataType *samplingPtr[8] = { input, input + numComp, input + this->DataDimensions[0] * numComp, input + (this->DataDimensions[0] + 1) * numComp,
        input + this->DataDimensions[0] * this->DataDimensions[1] * numComp, input + (this->DataDimensions[0] * this->DataDimensions[1] + 1) * numComp, input + (this->DataDimensions[0] * this->DataDimensions[1] + this->DataDimensions[0]) * numComp, input + (this->DataDimensions[0] * this->DataDimensions[1] + this->DataDimensions[0] + 1) * numComp };
    
    for (int yi = 0; yi < ys; yi++) 
    {
        float p[3] = {yi * yaxis[0] + offset[0], yi * yaxis[1] + offset[1], yi * yaxis[2] + offset[2]};
        for (int xi = 0; xi < xs; xi++, p[0] += xaxis[0], p[1] += xaxis[1], p[2] += xaxis[2], pixel += numComp) 
        {
            // find index
            const unsigned int pi[3] = { u_int(p[0]), u_int(p[1]), u_int(p[2])};
            if (pi[0] > SamplingTableSize || pi[1] > SamplingTableSize || pi[2] > SamplingTableSize)
                continue;
            
            // tri-linear interpolation
            int   index = stIndices[0][pi[0]] + stIndices[1][pi[1]] + stIndices[2][pi[2]];
            for (int comp = 0; comp < numComp; comp++) 
            {
                float sample = 0.f;
                for (int z = 0, si = 0; z < 2; z++) 
                {
                    const float zweight = z ? 1.f - stOffsets[2][pi[2]] : stOffsets[2][pi[2]];
                    for (int y = 0; y < 2; y++) 
                    {
                        const float yzweight = (y ? 1.f - stOffsets[1][pi[1]] : stOffsets[1][pi[1]]) * zweight;
                        for (int x = 0; x < 2; x++, si++)
                            sample += samplingPtr[si][index + comp] * (x ? 1.f - stOffsets[0][pi[0]] : stOffsets[0][pi[0]]) * yzweight;
                    }
                }
                
                // mapping
                //clip(((sample + shift) * scale), pixel[comp]);
                pixel[comp] = sample;
            }
        }	
    }
    delete [] stIndices[0];
    delete [] stIndices[1];
    delete [] stIndices[2];
    delete [] stOffsets[0];
    delete [] stOffsets[1];
    delete [] stOffsets[2];
}
//----------------------------------------------------------------------------
void vtkMAFVolumeSlicer::CalculateTextureCoordinates(const float point[3], const int size[2], const double spacing[2], float ts[2]) 
//----------------------------------------------------------------------------
{
    const float c[3]  = { point[0] - this->GlobalPlaneOrigin[0], point[1] - this->GlobalPlaneOrigin[1], point[2] - this->GlobalPlaneOrigin[2] };
    
	float tx = (c[0] * GlobalPlaneAxisY[1] - c[1] * GlobalPlaneAxisY[0]) / (GlobalPlaneAxisX[0] * GlobalPlaneAxisY[1] - GlobalPlaneAxisX[1] * GlobalPlaneAxisY[0]);
	float ty = (c[0] * GlobalPlaneAxisX[1] - c[1] * GlobalPlaneAxisX[0]) / (GlobalPlaneAxisX[0] * GlobalPlaneAxisY[1] - GlobalPlaneAxisX[1] * GlobalPlaneAxisY[0]);
	if (fabs(GlobalPlaneAxisX[0] * GlobalPlaneAxisY[1] - GlobalPlaneAxisX[1] * GlobalPlaneAxisY[0]) < 1.e-10f) {
		tx = (c[0] * GlobalPlaneAxisY[2] - c[2] * GlobalPlaneAxisY[0]) / (GlobalPlaneAxisX[0] * GlobalPlaneAxisY[2] - GlobalPlaneAxisX[2] * GlobalPlaneAxisY[0]);
		ty = (c[0] * GlobalPlaneAxisX[2] - c[2] * GlobalPlaneAxisX[0]) / (GlobalPlaneAxisX[0] * GlobalPlaneAxisY[2] - GlobalPlaneAxisX[2] * GlobalPlaneAxisY[0]);
    }
    ts[0] =  tx / (size[0] * spacing[0]);
    ts[1] = -ty / (size[1] * spacing[1]);
}
//----------------------------------------------------------------------------
void vtkMAFVolumeSlicer::SetSliceTransform(vtkLinearTransform *trans)
//----------------------------------------------------------------------------
{
    TransformSlice = trans;
    Modified();
}

//----------------------------------------------------------------------------
void vtkMAFVolumeSlicer::GeneratePolygonalOutput() {
    this->PrepareVolume();
    
    //polygonal generation
    vtkPolyData *output = vtkPolyData::SafeDownCast(this->GetOutputDataObject(0));
    output->Reset();
    
    // define the plane
    if (this->GetTexturedOutput()) 
    {
        this->GetTexturedOutput()->Update();
        memcpy(this->GlobalPlaneOrigin, this->GetTexturedOutput()->GetOrigin(), sizeof(this->GlobalPlaneOrigin));
    }
    
    const float d = -(this->GlobalPlaneAxisZ[0] * this->GlobalPlaneOrigin[0] + this->GlobalPlaneAxisZ[1] * this->GlobalPlaneOrigin[1] + this->GlobalPlaneAxisZ[2] * this->GlobalPlaneOrigin[2]);
    
    // intersect plane with the bounding box
    float points[12][3];
    int   numberOfPoints = 0;
    bool  processedPoints[12];
    memset(processedPoints, 0, sizeof(processedPoints));
    
    int i=0;
    for (i = 0; i < 3; i++) 
    {
        const int j = (i + 1) % 3, k = (i + 2) % 3;
        
        for (int jj = 0; jj < 2; jj++) 
        {
            for (int kk = 0; kk < 2; kk++) 
            {
                float (&p)[3] = points[numberOfPoints];
                p[j] = this->DataBounds[j][jj];
                p[k] = this->DataBounds[k][kk];
                p[i] = -(d + this->GlobalPlaneAxisZ[j] * p[j] + this->GlobalPlaneAxisZ[k] * p[k]);
                
                if (fabs(this->GlobalPlaneAxisZ[i]) < 1.e-10)
                    continue; // a special case (0 / 0) should be handled automatically in another iteration
                p[i] /= this->GlobalPlaneAxisZ[i];
                
                // check that p[i] is in inside the box
                if (p[i] < this->DataBounds[i][0] || p[i] > this->DataBounds[i][1])
                    continue;
                
                // ignore the same points (can it really happen?)
                int ii;
                for (ii = 0; ii < numberOfPoints; ii++) 
                {
                    if (vtkMath::Distance2BetweenPoints(p, points[ii]) < 1.e-10)
                        break;
                }
                if (ii != numberOfPoints)
                    continue;
                
                // add point
                numberOfPoints++;
            }
        }
    }
    
    if (numberOfPoints <= 2)
        return;
    
    // find image parameters for texture mapping
    double spacing[3];
    int size[2];
    if (this->GetTexturedOutput()) 
    {
        vtkImageData *texture = this->GetTexturedOutput();
        int extent[6];
        //assert(texture->GetSource() != this); //doesn't mean anything compare a texture with this object!
        texture->UpdateInformation();
        texture->GetWholeExtent(extent);
        if (extent[0] >= extent[1])
            texture->GetExtent(extent);
        size[0] = extent[1] - extent[0] + 1;
        size[1] = extent[3] - extent[2] + 1;
        texture->GetSpacing(spacing);
    }
    else 
    {
        vtkErrorMacro(<<"No texture specified");
        return;
    }
    
    // organize points
    vtkPoints *pointsObj = output->GetPoints();
    if (output->GetPoints() == NULL) 
    {
        pointsObj = vtkPoints::New();
        output->SetPoints(pointsObj);
        pointsObj->Delete();
    }
    pointsObj->Allocate(numberOfPoints, 1);
    vtkDoubleArray *tsObj = NULL;//vtkDoubleArray::SafeDownCast(output->GetPointData()->GetTCoords());
    if (tsObj == NULL) 
    {
        tsObj = vtkDoubleArray::New();
        tsObj->SetNumberOfComponents(2);
    }
    tsObj->Allocate(2 * numberOfPoints, 1);
    
    // create a clockwise polygon
    vtkCellArray *polys = vtkCellArray::New();
    polys->Allocate(polys->EstimateSize(1, numberOfPoints));
    vtkIdType pointIds[12];
    pointIds[0] = 0;
    processedPoints[0] = true;
	
    double maxVectorNorm = 0.;
    int longestVector = 1;
    for (i = 0; i < numberOfPoints; i++) 
    {
        if (size[0] > 0 && size[1] > 0) 
        {
            float ts[2];
            this->CalculateTextureCoordinates(points[i], size, spacing, ts);
            tsObj->InsertNextTuple(ts);
        }
        pointsObj->InsertNextPoint(points[i]);
        if (i > 0) 
        {
            points[i][0] -= points[0][0];
            points[i][1] -= points[0][1];
            points[i][2] -= points[0][2];
            double norm = vtkMath::Norm(points[i]);
            if (norm > maxVectorNorm) 
            {
                maxVectorNorm = norm;
                longestVector = i;
            }
            vtkMath::Normalize(points[i]);
        }
        processedPoints[i] = false;
    }
	
    for (i = 1; i < numberOfPoints; i++) 
    {
		double minSAngle = 99999.;
		int    nextPoint = 0;
		for (int j = 1; j < numberOfPoints; j++) 
        {
			float angleVector[3];
			vtkMath::Cross(points[longestVector], points[j], angleVector);
			double norm = vtkMath::Norm(angleVector) * (vtkMath::Dot(angleVector, this->GlobalPlaneAxisZ) > 0. ? -1. : 1.);
			if (!processedPoints[j] && norm <= minSAngle) 
            {
				minSAngle = norm;
				nextPoint = j;
			}
        }
		pointIds[i] = nextPoint; 
		processedPoints[nextPoint] = true;
    }    
	
    polys->InsertNextCell(numberOfPoints, pointIds);
    output->SetPolys(polys);
    polys->Delete();
    
    output->GetPointData()->SetTCoords(tsObj);
    tsObj->Delete();

    //end polygonal generation
    output->Modified();
}

//----------------------------------------------------------------------------
void vtkMAFVolumeSlicer::GenerateTextureOutput()
{
    this->PrepareVolume();
    
    //texture generation
    vtkImageData *outputObject = this->GetTexturedOutput();
    
    int extent[6];
    outputObject->GetWholeExtent(extent);
    outputObject->SetExtent(extent);
    outputObject->SetNumberOfScalarComponents(this->NumComponents);
    outputObject->AllocateScalars();
    
    vtkDataSet *data = vtkDataSet::SafeDownCast(this->GetInput());
    const void *inputPointer  = data->GetPointData()->GetScalars()->GetVoidPointer(0);
    const void *outputPointer = outputObject->GetPointData()->GetScalars()->GetVoidPointer(0);
    
    switch (data->GetPointData()->GetScalars()->GetDataType()) 
    {
        case VTK_CHAR: //---------------------------------------------
            switch (outputObject->GetPointData()->GetScalars()->GetDataType()) 
        {
            case VTK_CHAR:
                this->CreateImage((const char*)inputPointer, (char*)outputPointer, outputObject);
                break;
            case VTK_UNSIGNED_CHAR:
                this->CreateImage((const char*)inputPointer, (unsigned char*)outputPointer, outputObject);
                break;
            case VTK_SHORT:
                this->CreateImage((const char*)inputPointer, (short*)outputPointer, outputObject);
                break;
            case VTK_UNSIGNED_SHORT:
                this->CreateImage((const char*)inputPointer, (unsigned short*)outputPointer, outputObject);
                break;
            case VTK_FLOAT:
                this->CreateImage((const char*)inputPointer, (float*)outputPointer, outputObject);
                break;
            default:
                vtkErrorMacro(<< "vtkMAFVolumeSlicer: Scalar type is not supported");
                return;
        }
            break;
        case VTK_UNSIGNED_CHAR: //------------------------------------
            switch (outputObject->GetPointData()->GetScalars()->GetDataType()) 
        {
            case VTK_CHAR:
                this->CreateImage((const unsigned char*)inputPointer, (char*)outputPointer, outputObject);
                break;
            case VTK_UNSIGNED_CHAR:
                this->CreateImage((const unsigned char*)inputPointer, (unsigned char*)outputPointer, outputObject);
                break;
            case VTK_SHORT:
                this->CreateImage((const unsigned char*)inputPointer, (short*)outputPointer, outputObject);
                break;
            case VTK_UNSIGNED_SHORT:
                this->CreateImage((const unsigned char*)inputPointer, (unsigned short*)outputPointer, outputObject);
                break;
            case VTK_FLOAT:
                this->CreateImage((const unsigned char*)inputPointer, (float*)outputPointer, outputObject);
                break;
            default:
                vtkErrorMacro(<< "vtkMAFVolumeSlicer: Scalar type is not supported");
                return;
        }
            break;
        case VTK_SHORT: //--------------------------------------------
            switch (outputObject->GetPointData()->GetScalars()->GetDataType()) 
        {
            case VTK_CHAR:
                this->CreateImage((const short*)inputPointer, (char*)outputPointer, outputObject);
                break;
            case VTK_UNSIGNED_CHAR:
                this->CreateImage((const short*)inputPointer, (unsigned char*)outputPointer, outputObject);
                break;
            case VTK_SHORT:
                this->CreateImage((const short*)inputPointer, (short*)outputPointer, outputObject);
                break;
            case VTK_UNSIGNED_SHORT:
                this->CreateImage((const short*)inputPointer, (unsigned short*)outputPointer, outputObject);
                break;
            case VTK_FLOAT:
                this->CreateImage((const short*)inputPointer, (float*)outputPointer, outputObject);
                break;
            default:
                vtkErrorMacro(<< "vtkMAFVolumeSlicer: Scalar type is not supported");
                return;
        }
            break;
        case VTK_UNSIGNED_SHORT: //-----------------------------------
            switch (outputObject->GetPointData()->GetScalars()->GetDataType()) 
        {
            case VTK_CHAR:
                this->CreateImage((const unsigned short*)inputPointer, (char*)outputPointer, outputObject);
                break;
            case VTK_UNSIGNED_CHAR:
                this->CreateImage((const unsigned short*)inputPointer, (unsigned char*)outputPointer, outputObject);
                break;
            case VTK_SHORT:
                this->CreateImage((const unsigned short*)inputPointer, (short*)outputPointer, outputObject);
                break;
            case VTK_UNSIGNED_SHORT:
                this->CreateImage((const unsigned short*)inputPointer, (unsigned short*)outputPointer, outputObject);
                break;
            case VTK_FLOAT:
                this->CreateImage((const unsigned short*)inputPointer, (float*)outputPointer, outputObject);
                break;
            default:
                vtkErrorMacro(<< "vtkMAFVolumeSlicer: Scalar type is not supported");
                return;
        }
            break;
        case VTK_FLOAT: //--------------------------------------------
            switch (outputObject->GetPointData()->GetScalars()->GetDataType()) 
        {
            case VTK_CHAR:
                this->CreateImage((const float*)inputPointer, (char*)outputPointer, outputObject);
                break;
            case VTK_UNSIGNED_CHAR:
                this->CreateImage((const float*)inputPointer, (unsigned char*)outputPointer, outputObject);
                break;
            case VTK_SHORT:
                this->CreateImage((const float*)inputPointer, (short*)outputPointer, outputObject);
                break;
            case VTK_UNSIGNED_SHORT:
                this->CreateImage((const float*)inputPointer, (unsigned short*)outputPointer, outputObject);
                break;
            case VTK_FLOAT:
                this->CreateImage((const float*)inputPointer, (float*)outputPointer, outputObject);
                break;
            default:
                vtkErrorMacro(<< "vtkMAFVolumeSlicer: Scalar type is not supported");
                return;
        }
            break;
        case VTK_DOUBLE: //--------------------------------------------
            switch (outputObject->GetPointData()->GetScalars()->GetDataType()) 
        {
            case VTK_CHAR:
                this->CreateImage((const double*)inputPointer, (char*)outputPointer, outputObject);
                break;
            case VTK_UNSIGNED_CHAR:
                this->CreateImage((const double*)inputPointer, (unsigned char*)outputPointer, outputObject);
                break;
            case VTK_SHORT:
                this->CreateImage((const double*)inputPointer, (short*)outputPointer, outputObject);
                break;
            case VTK_UNSIGNED_SHORT:
                this->CreateImage((const double*)inputPointer, (unsigned short*)outputPointer, outputObject);
                break;
            case VTK_FLOAT:
                this->CreateImage((const double*)inputPointer, (float*)outputPointer, outputObject);
                break;
            case VTK_DOUBLE:
                this->CreateImage((const double*)inputPointer, (double*)outputPointer, outputObject);
                break;
            default:
                vtkErrorMacro(<< "vtkMAFVolumeSlicer: Scalar type is not supported");
                return;
        }
            break;
        default:
            vtkErrorMacro(<< "vtkMAFVolumeSlicer: Scalar type is not supported");
            return;
    }

    //end texture generation
    outputObject->Modified();
}


//----------------------------------------------------------------------------
int vtkMAFVolumeSlicer::RequestInformation(
    vtkInformation* vtkNotUsed(request),
    vtkInformationVector** vtkNotUsed(inputVector),
    vtkInformationVector* vtkNotUsed(outputVector))
{
    if (GetInput()==NULL)
        return 0;

    vtkImageData *output = this->GetTexturedOutput();

    int dims[3];
    output->GetDimensions(dims);
    if (dims[2] != 1)
    {
        dims[2] = 1;
        output->SetDimensions(dims);
    }
    output->SetWholeExtent(output->GetExtent());
    output->SetUpdateExtentToWholeExtent();

    if (this->AutoSpacing) 
    { // select spacing
        if (TransformSlice)
        {
            TransformSlice->TransformPoint(PlaneOrigin, GlobalPlaneOrigin);
            TransformSlice->TransformNormal(PlaneAxisX, GlobalPlaneAxisX);
            TransformSlice->TransformNormal(PlaneAxisY, GlobalPlaneAxisY);
        }
        else
        {
            memcpy(GlobalPlaneOrigin, PlaneOrigin, sizeof(PlaneOrigin));
            memcpy(GlobalPlaneAxisX, PlaneAxisX, sizeof(PlaneAxisX));
            memcpy(GlobalPlaneAxisY, PlaneAxisY, sizeof(PlaneAxisY));
        }
        this->PrepareVolume();
        const float d = -(this->GlobalPlaneAxisZ[0] * this->GlobalPlaneOrigin[0] + this->GlobalPlaneAxisZ[1] * this->GlobalPlaneOrigin[1] + this->GlobalPlaneAxisZ[2] * this->GlobalPlaneOrigin[2]);

        // intersect plane with the bounding box
        double spacing[3] = {1.f, 1.f, 1.f};
        float t[24][2], minT = VTK_FLOAT_MAX, maxT = VTK_FLOAT_MIN, minS = VTK_FLOAT_MAX, maxS = VTK_FLOAT_MIN;
        int    numberOfPoints = 0;
        for (int i = 0; i < 3; i++) 
        {
            const int j = (i + 1) % 3, k = (i + 2) % 3;

            for (int jj = 0; jj < 2; jj++) 
            {
                for (int kk = 0; kk < 2; kk++) 
                {
                    float p[3];
                    p[j] = this->DataBounds[j][jj];
                    p[k] = this->DataBounds[k][kk];
                    p[i] = -(d + this->GlobalPlaneAxisZ[j] * p[j] + this->GlobalPlaneAxisZ[k] * p[k]);

                    if (fabs(this->GlobalPlaneAxisZ[i]) < 1.e-10)
                        continue;
                    p[i] /= this->GlobalPlaneAxisZ[i];
                    if (p[i] >= this->DataBounds[i][0] && p[i] <= this->DataBounds[i][1]) 
                    {
                        this->CalculateTextureCoordinates(p, (int*)dims, spacing, t[numberOfPoints]);
                        if (t[numberOfPoints][0] > maxT)
                            maxT = t[numberOfPoints][0];
                        if (t[numberOfPoints][0] < minT)
                            minT = t[numberOfPoints][0];
                        if (t[numberOfPoints][1] > maxS)
                            maxS = t[numberOfPoints][1];
                        if (t[numberOfPoints][1] < minS)
                            minS = t[numberOfPoints][1];
                        numberOfPoints++; // add point
                    }
                }
            }
        }

        // find spacing now
        float maxSpacing = max(maxS - minS, maxT - minT);
        spacing[0] = spacing[1] = max(maxSpacing, 1.e-8f);
        output->SetSpacing(spacing);
        if (fabs(minT) > 1.e-3 || fabs(minS) > 1.e-3) 
        {
            this->GlobalPlaneOrigin[0] += minT * this->GlobalPlaneAxisX[0] * dims[0] + minS * this->GlobalPlaneAxisY[0] * dims[1];
            this->GlobalPlaneOrigin[1] += minT * this->GlobalPlaneAxisX[1] * dims[0] + minS * this->GlobalPlaneAxisY[1] * dims[1];
            this->GlobalPlaneOrigin[2] += minT * this->GlobalPlaneAxisX[2] * dims[0] + minS * this->GlobalPlaneAxisY[2] * dims[1];
            this->Modified();
        }
    }
    output->SetOrigin(this->GlobalPlaneOrigin);

    return 1;
}

int vtkMAFVolumeSlicer::ProcessRequest(vtkInformation* request,
                                         vtkInformationVector** inputVector,
                                         vtkInformationVector* outputVector)
{
    // generate the data
    if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA()))
    {
        return this->RequestData(request, inputVector, outputVector);
    }
    
    if(request->Has(vtkStreamingDemandDrivenPipeline::REQUEST_UPDATE_EXTENT()))
    {
        return this->RequestUpdateExtent(request, inputVector, outputVector);
    }
    
    // execute information
    if(request->Has(vtkDemandDrivenPipeline::REQUEST_INFORMATION()))
    {
        return this->RequestInformation(request, inputVector, outputVector);
    }
    
    return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}

int vtkMAFVolumeSlicer::RequestUpdateExtent(
                                              vtkInformation* vtkNotUsed(request),
                                              vtkInformationVector** inputVector,
                                              vtkInformationVector* vtkNotUsed(outputVector))
{
    int numInputPorts = this->GetNumberOfInputPorts();
    for (int i=0; i<numInputPorts; i++)
    {
        int numInputConnections = this->GetNumberOfInputConnections(i);
        for (int j=0; j<numInputConnections; j++)
        {
            vtkInformation* inputInfo = inputVector[i]->GetInformationObject(j);
            inputInfo->Set(vtkStreamingDemandDrivenPipeline::EXACT_EXTENT(), 1);
        }
    }
    return 1;
}


//----------------------------------------------------------------------------
int vtkMAFVolumeSlicer::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
    // get the info objects
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    // get the input and output
    vtkDataSet *input = vtkDataSet::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkPolyData *output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

    this->NumComponents = input->GetPointData()->GetNumberOfComponents();

    this->GetTexturedOutput()->SetScalarType(input->GetPointData()->GetScalars()->GetDataType());
    double spc[3] = {.33, .33, 1.};
    if(input->IsA("vtkImageData") || input->IsA("vtkStructuredPoints")) {
        ((vtkImageData *)input)->GetSpacing(spc);
    }
    this->GetTexturedOutput()->SetSpacing(spc);

    this->GeneratePolygonalOutput();
    this->GenerateTextureOutput();

//    output->Squeeze();

    return 1;
}

//----------------------------------------------------------------------------
void vtkMAFVolumeSlicer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

/*  if ( this->ClipFunction )
    {
    os << indent << "Clip Function: " << this->ClipFunction << "\n";
    }
  else
    {
    os << indent << "Clip Function: (none)\n";
    }
 */
}
