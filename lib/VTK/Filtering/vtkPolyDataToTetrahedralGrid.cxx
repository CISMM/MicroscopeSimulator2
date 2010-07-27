/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkPolyDataToTetrahedralGrid.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPolyDataToTetrahedralGrid.h"

#include "vtkCellArray.h"
#include "vtkDoubleArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkTetra.h"
#include "vtkTriangle.h"
#include "vtkUnstructuredGrid.h"
#include "tetgen.h"

vtkCxxRevisionMacro(vtkPolyDataToTetrahedralGrid, "$Revision: 1.4 $");
vtkStandardNewMacro(vtkPolyDataToTetrahedralGrid);

//----------------------------------------------------------------------------
vtkPolyDataToTetrahedralGrid::vtkPolyDataToTetrahedralGrid()
{
  // by default assume filters have one input and one output
  // subclasses that deviate should modify this setting
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}

//----------------------------------------------------------------------------
vtkPolyDataToTetrahedralGrid::~vtkPolyDataToTetrahedralGrid()
{
}

//----------------------------------------------------------------------------
void vtkPolyDataToTetrahedralGrid::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


int vtkPolyDataToTetrahedralGrid::FillInputPortInformation(int port, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
  return 1;
}

//----------------------------------------------------------------------------
tetgenio* vtkPolyDataToTetrahedralGridCreateTetgenio(vtkPolyData *polyData) {
  tetgenio *out = new tetgenio();
  
  // All indices start from 0.
  out->firstnumber = 0;

  out->numberofpoints = polyData->GetNumberOfPoints();
  out->pointlist = new REAL[out->numberofpoints * 3];
  double inputPoints[3];
  for (int i = 0; i < out->numberofpoints; i++) {
    polyData->GetPoint(i, inputPoints);
    out->pointlist[i*3 + 0] = inputPoints[0];
    out->pointlist[i*3 + 1] = inputPoints[1];
    out->pointlist[i*3 + 2] = inputPoints[2];
  }

  out->numberoffacets = polyData->GetNumberOfPolys();
  out->facetlist = new tetgenio::facet[out->numberoffacets];
  out->facetmarkerlist = NULL;

  vtkCellArray *polys = polyData->GetPolys();
  int polyId = 0;
  vtkIdType npts, *pts;
  for (polys->InitTraversal(); polys->GetNextCell(npts, pts); ) {
    tetgenio::facet *f = &out->facetlist[polyId++];
    f->numberofpolygons = 1;
    f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
    f->numberofholes = 0;
    f->holelist = NULL;

    tetgenio::polygon *p = &f->polygonlist[0];
    p->numberofvertices = npts;
    p->vertexlist = new int[p->numberofvertices];
    for (int i = 0; i < npts; i++) {
      p->vertexlist[i] = pts[i];
    }
  }

  return out;
}

//----------------------------------------------------------------------------
// This is the superclasses style of Execute method.  Convert it into
// an imaging style Execute method.
int vtkPolyDataToTetrahedralGrid::RequestData(
  vtkInformation* request,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // Create tetrahedral mesh from input closed manifold
  tetgenio *in, *detect, out;
  in = vtkPolyDataToTetrahedralGridCreateTetgenio(input);

  // First, detect if there are any overlapping polygons in the geometry
  detect = new tetgenio();
  detect->numberofpoints = 0;
  try {
    char options[] = "pdQ";
    tetrahedralize(options, in, detect);
  } catch (...) {
    vtkErrorMacro(<< "Error when testing geometry for overlapping polygons");
  }

  if (detect->numberofpoints == 0) { // Geometry passed the test?
    // This shouldn't be necessary, but it seems to prevent some
    // crashes when tetrahedralizing some geometries.
    delete in;
    in = vtkPolyDataToTetrahedralGridCreateTetgenio(input);

    try {
      char options[] = "pQ";
      tetrahedralize(options, in, &out);
    } catch (...) {
      vtkErrorMacro(<< "Error when generating Delaunay tetrahedralization of geometry");

      // Initialize output data structure
      out.numberofpoints = 0;
      out.numberoftetrahedra = 0;
    }
  } else {
    out.numberofpoints = 0;
    out.numberoftetrahedra = 0;
  }

  delete detect;
  delete in;

  vtkPoints *points = vtkPoints::New();
  points->SetNumberOfPoints(out.numberofpoints);
  for (int i = 0; i < out.numberofpoints; i++) {
    double *ptlist = i*3 + out.pointlist;
    points->InsertPoint(i, i*3 + out.pointlist);
  }
  
  output->Allocate(out.numberoftetrahedra, 100);
  for (int i = 0; i < out.numberoftetrahedra; i++) {
    vtkTetra *tetra = vtkTetra::New();
    tetra->GetPointIds()->SetId(0, out.tetrahedronlist[4*i + 0]);
    tetra->GetPointIds()->SetId(1, out.tetrahedronlist[4*i + 1]);
    tetra->GetPointIds()->SetId(2, out.tetrahedronlist[4*i + 2]);
    tetra->GetPointIds()->SetId(3, out.tetrahedronlist[4*i + 3]);
    output->InsertNextCell(tetra->GetCellType(), tetra->GetPointIds());
    tetra->Delete();
  }


  output->SetPoints(points);
  points->Delete();

  return 1;
}
