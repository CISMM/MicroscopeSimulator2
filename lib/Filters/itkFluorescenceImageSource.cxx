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

#include <vtkImageExport.h>
#include <vtkImageExtractComponents.h>

namespace itk
{

/**
 *
 */
template <class TOutputImage>
FluorescenceImageSource<TOutputImage>
::FluorescenceImageSource()
{
  m_Origin  = new double [TOutputImage::GetImageDimension()];

  for (unsigned int i=0; i<TOutputImage::GetImageDimension(); i++)
    {
    m_Origin[i] = 0.0;
    }

  // TODO - set up extract components

  // TODO - set up VTK image exporter

  // TODO - set up ITK image importer

}


template <class TOutputImage>
FluorescenceImageSource<TOutputImage>
::~FluorescenceImageSource()
{
  delete [] m_Origin;
}


template <class TOutputImage>
void
FluorescenceImageSource<TOutputImage>
::SetParameters(const ParametersType& parameters) {
  int numParameters = GetNumberOfParameters();
  double* doubleParams = new double[numParameters];
  for (unsigned int i = 0; i < numParameters; i++) {
    doubleParams[i] = static_cast<double>(parameters[i]);
  }

  m_ImageSource->SetParameters(doubleParams);

  delete[] doubleParams;
}


template <class TOutputImage>
typename FluorescenceImageSource<TOutputImage>::ParametersType
FluorescenceImageSource<TOutputImage>
::GetParameters() const {
  int numParameters = GetNumberOfParameters();
  double* doubleParams = new double[numParameters];

  m_ImageSource->GetParameters(doubleParams);

  ParametersType parameters(numParameters);
  for (unsigned int i = 0; i < numParameters; i++) {
    parameters[i] = static_cast<double>(doubleParams[i]);
  }

  delete[] doubleParams;
}


template <class TOutputImage>
unsigned int
FluorescenceImageSource<TOutputImage>
::GetNumberOfParameters() const {
  return m_ImageSource->GetNumberOfParameters();
}


/**
 *
 */
template <class TOutputImage>
void 
FluorescenceImageSource<TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  unsigned int i;
  os << indent << "Origin: [";
  for (i=0; i < TOutputImage::ImageDimension - 1; i++)
    {
    os << m_Origin[i] << ", ";
    }
  os << m_Origin[i] << "]" << std::endl;
}

//----------------------------------------------------------------------------
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
  size.SetSize( sourceSize );
  
  output = this->GetOutput(0);

  typename TOutputImage::RegionType largestPossibleRegion;
  largestPossibleRegion.SetSize( size );
  largestPossibleRegion.SetIndex( index );
  output->SetLargestPossibleRegion( largestPossibleRegion );

  output->SetSpacing(m_ImageSource->GetSpacing());
  output->SetOrigin(m_Origin);
}

//----------------------------------------------------------------------------
template <typename TOutputImage>
void 
FluorescenceImageSource<TOutputImage>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
                       int threadId )
{

  // Support progress methods/callbacks
  ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());
       
  typedef typename TOutputImage::PixelType ScalarType;
  typename TOutputImage::Pointer image = this->GetOutput(0);

  ImageRegionIteratorWithIndex<TOutputImage> it(image, outputRegionForThread);


}


} // end namespace itk

#endif
