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

namespace itk
{

/**
 *
 */
template <class TOutputImage>
FluorescenceImageSource<TOutputImage>
::FluorescenceImageSource()
{
  m_Size    = new unsigned long [TOutputImage::GetImageDimension()];
  m_Spacing = new double [TOutputImage::GetImageDimension()];
  m_Origin  = new double [TOutputImage::GetImageDimension()];

  //Initial image is 0 pixels in each direction.
  for (unsigned int i=0; i<TOutputImage::GetImageDimension(); i++)
    {
    m_Size[i] = 0;
    m_Spacing[i] = 1.0;
    m_Origin[i] = 0.0;
    }

}


template <class TOutputImage>
FluorescenceImageSource<TOutputImage>
::~FluorescenceImageSource()
{
  delete [] m_Size;
  delete [] m_Spacing;
  delete [] m_Origin;
}


template <class TOutputImage>
void
FluorescenceImageSource<TOutputImage>
::SetParameters(const ParametersType& parameters) {
  Array<double> doubleParams(GetNumberOfParameters());
  for (unsigned int i = 0; i < GetNumberOfParameters(); i++) {
    doubleParams[i] = static_cast<float>(parameters[i]);
  }

  int index = 0;

  // TODO - hook up parameters to the fluorescence image generator
}


template <class TOutputImage>
typename FluorescenceImageSource<TOutputImage>::ParametersType
FluorescenceImageSource<TOutputImage>
::GetParameters() const {
  Array<double> doubleParams(GetNumberOfParameters());

  int index = 0;

  // TODO - look up parameters from fluorescence image generator

  ParametersType parameters(GetNumberOfParameters());
  for (unsigned int i = 0; i < GetNumberOfParameters(); i++) {
    parameters[i] = static_cast<double>(doubleParams[i]);
  }

  return parameters;
}


template <class TOutputImage>
unsigned int
FluorescenceImageSource<TOutputImage>
::GetNumberOfParameters() const {
  // TODO - count up active parameters
  
  return 23;
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

  os << indent << "Spacing: [";
  for (i=0; i < TOutputImage::ImageDimension - 1; i++)
    {
    os << m_Spacing[i] << ", ";
    }
  os << m_Spacing[i] << "] (nanometers)" << std::endl;

  os << indent << "Size: [";
  for (i=0; i < TOutputImage::ImageDimension - 1; i++)
    {
    os << m_Size[i] << ", ";
    }
  os << m_Size[i] << "]" << std::endl;

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
  size.SetSize( m_Size );
  
  output = this->GetOutput(0);

  typename TOutputImage::RegionType largestPossibleRegion;
  largestPossibleRegion.SetSize( size );
  largestPossibleRegion.SetIndex( index );
  output->SetLargestPossibleRegion( largestPossibleRegion );

  output->SetSpacing(m_Spacing);
  output->SetOrigin(m_Origin);
}

//----------------------------------------------------------------------------
template <typename TOutputImage>
void 
FluorescenceImageSource<TOutputImage>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
                       int threadId )
{
  itkDebugMacro(<<"Generating a random image of scalars");

  // Support progress methods/callbacks
  ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());
       
  typedef typename TOutputImage::PixelType ScalarType;
  typename TOutputImage::Pointer image = this->GetOutput(0);

  ImageRegionIteratorWithIndex<TOutputImage> it(image, outputRegionForThread);

#if 0
  int zSlice = -1;
  complex_t opdCache[INTEGRATE_N];

  for (; !it.IsAtEnd(); ++it)
    {
    typename TOutputImage::IndexType index = it.GetIndex();
    typename TOutputImage::PointType point;
    image->TransformIndexToPhysicalPoint(index, point);

    for (int i = 0; i < 3; i++)
      point[i] -= m_PointCenter[i];

    // See if we have switched slices. If so, we need to precompute some
    // integral terms for the new slice.
    if (zSlice != index[2]) {
      PrecomputeOPDTerms(opdCache, point[2] * 1e-9);
      zSlice = index[2];
    }

    it.Set( ComputeIntegratedPixelValue(opdCache, point) );
    progress.CompletedPixel();
    }
#endif
}


} // end namespace itk

#endif
