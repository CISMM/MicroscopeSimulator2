/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkUniformPointSampler.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkUniformPointSampler.h"

#include "vtkCellArray.h"
#include "vtkDoubleArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkTriangle.h"
#include <cstdlib>

//----------------------------------------------------------------------------
vtkUniformPointSampler::vtkUniformPointSampler()
{
  // by default assume filters have one input and one output
  // subclasses that deviate should modify this setting
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
  this->Density = 0.0;
  this->UseFixedNumberOfSamples = 0;
  this->NumberOfSamples = 0;
  this->SurfaceArea = 0.0;
}

//----------------------------------------------------------------------------
vtkUniformPointSampler::~vtkUniformPointSampler()
{
}

//----------------------------------------------------------------------------
void vtkUniformPointSampler::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

