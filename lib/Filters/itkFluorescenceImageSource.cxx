/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkFluorescenceImageSource.cxx,v $
  Language:  C++
  Date:      $Date: 2009/09/14 13:57:30 $
  Version:   $Revision: 1.9 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkFluorescenceImageSource_txx
#define __itkFluorescenceImageSource_txx

#include "itkFluorescenceImageSource.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkObjectFactory.h"
#include "itkProgressReporter.h"
#include "itkVTKImageImport.h"

#include <vtkImageData.h>
#include <vtkImageExport.h>
#include <vtkImageExtractComponents.h>


namespace itk
{

/**
 *
 */
template <typename TOutputImage>
FluorescenceImageSource<TOutputImage>
::FluorescenceImageSource()
{
  m_ImageSource = NULL;
  m_ExtractedComponent = 0;

  m_Extractor = vtkSmartPointer<vtkImageExtractComponents>::New();
  m_Extractor->SetComponents(m_ExtractedComponent);
  // Defer connection to input source

  m_VTKToITKFilter = new VTKImageToITKImage<TOutputImage>();
  m_VTKToITKFilter->SetInput(m_Extractor->GetOutput());
}


template <typename TOutputImage>
FluorescenceImageSource<TOutputImage>
::~FluorescenceImageSource() {
  delete m_VTKToITKFilter;
}


template <typename TOutputImage>
void
FluorescenceImageSource<TOutputImage>
::SetFluorescenceImageSource(::FluorescenceImageSource* source) {
  m_ImageSource = source;

  this->Modified();
}


template <typename TOutputImage>
void
FluorescenceImageSource<TOutputImage>
::SetParameters(const ParametersType& parameters) {
  int numParameters = GetNumberOfParameters();
  double* doubleParams = new double[numParameters];
  for (int i = 0; i < numParameters; i++) {
    doubleParams[i] = static_cast<double>(parameters[i]);
  }

  m_ImageSource->SetParameters(doubleParams);

  delete[] doubleParams;

  this->Modified();
}


template <typename TOutputImage>
typename FluorescenceImageSource<TOutputImage>::ParametersType
FluorescenceImageSource<TOutputImage>
::GetParameters() const {
  int numParameters = GetNumberOfParameters();
  double* doubleParams = new double[numParameters];

  m_ImageSource->GetParameters(doubleParams);

  ParametersType parameters(numParameters);
  for (int i = 0; i < numParameters; i++) {
    parameters[i] = static_cast<double>(doubleParams[i]);
  }

  delete[] doubleParams;

  return parameters;
}


template <typename TOutputImage>
unsigned int
FluorescenceImageSource<TOutputImage>
::GetNumberOfParameters() const {
  return m_ImageSource->GetNumberOfParameters();
}


/**
 *
 */
template <typename TOutputImage>
void 
FluorescenceImageSource<TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
}


template <typename TOutputImage>
void 
FluorescenceImageSource<TOutputImage>
::GenerateOutputInformation()
{
  TOutputImage *output;
  typename TOutputImage::IndexType index = {{0}};
  typename TOutputImage::SizeType size = {{0}};

  int* sourceSize = m_ImageSource->GetDimensions();
  for (unsigned long i = 0; i < TOutputImage::ImageDimension; i++) {
    size.SetElement(i, static_cast<typename TOutputImage::SizeType::SizeValueType>(sourceSize[i]));
  }
  
  output = this->GetOutput(0);

  typename TOutputImage::RegionType largestPossibleRegion;
  largestPossibleRegion.SetSize( size );
  largestPossibleRegion.SetIndex( index );
  output->SetLargestPossibleRegion( largestPossibleRegion );

  output->SetSpacing(m_ImageSource->GetSpacing());

  double origin[TOutputImage::ImageDimension];
  for (int i = 0; i < TOutputImage::ImageDimension; i++) {
    origin[i] = 0.0;
  }
  output->SetOrigin(origin);
}

//----------------------------------------------------------------------------
template <typename TOutputImage>
void 
FluorescenceImageSource<TOutputImage>
::GenerateData()
{
  vtkImageData* image = m_ImageSource->GenerateFluorescenceStackImage();
  m_Extractor->SetInput(image);
  image->Delete();

  m_VTKToITKFilter->GraftOutput(this->GetOutput());
  m_VTKToITKFilter->Update();
  this->GraftOutput(m_VTKToITKFilter->GetOutput());
}


} // end namespace itk

#endif
