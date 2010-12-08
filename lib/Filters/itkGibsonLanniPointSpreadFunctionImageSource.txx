/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef __itkGibsonLanniPointSpreadFunctionImageSource_txx
#define __itkGibsonLanniPointSpreadFunctionImageSource_txx

#include "itkGibsonLanniPointSpreadFunctionImageSource.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkMath.h"
#include "itkObjectFactory.h"
#include "itkProgressReporter.h"

namespace itk
{

//----------------------------------------------------------------------------
template< class TOutputImage >
GibsonLanniPointSpreadFunctionImageSource< TOutputImage >
::GibsonLanniPointSpreadFunctionImageSource()
{
}


//----------------------------------------------------------------------------
template< class TOutputImage >
GibsonLanniPointSpreadFunctionImageSource< TOutputImage >
::~GibsonLanniPointSpreadFunctionImageSource()
{
}


//----------------------------------------------------------------------------
template< class TOutputImage >
void
GibsonLanniPointSpreadFunctionImageSource< TOutputImage >
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
template< class TOutputImage >
void
GibsonLanniPointSpreadFunctionImageSource< TOutputImage >
::BeforeThreadedGenerateData()
{
  // Set parameters in the functor
  this->m_IntegrandFunctor.CopySettings(this);
}


//----------------------------------------------------------------------------
template< class TOutputImage >
void
GibsonLanniPointSpreadFunctionImageSource< TOutputImage >
::ThreadedGenerateData(const RegionType& outputRegionForThread, int threadId )
{
  // Support progress methods/callbacks
  ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

  typename TOutputImage::Pointer image = this->GetOutput(0);

  ImageRegionIteratorWithIndex<OutputImageType> it(image, outputRegionForThread);

  for (; !it.IsAtEnd(); ++it)
    {
    IndexType index = it.GetIndex();
    PointType point;
    image->TransformIndexToPhysicalPoint(index, point);

    // Shift the center of the point
    for ( unsigned int i = 0; i < point.Size(); i++ )
      {
      point[i] -= this->m_PointCenter[i];
      }

    it.Set( ComputeSampleValue( point ) );
    progress.CompletedPixel();
    }
}


//----------------------------------------------------------------------------
template< class TOutputImage >
double
GibsonLanniPointSpreadFunctionImageSource< TOutputImage >
::ComputeSampleValue(const PointType& point)
{
  PixelType px = point[0] * 1e-9;
  PixelType py = point[1] * 1e-9;
  PixelType pz = point[2] * 1e-9;

  /* Compute terms that are independent of terms within the integral. */
  double mag = this->m_Magnification;

  // We have to convert to coordinates of the detector points
  double x_o = px * mag;
  double y_o = py * mag;
  double z_o = pz; // No conversion needed

  // Return squared magnitude of the integrated value
  return (PixelType) norm(IntegrateFunctor(this->m_IntegrandFunctor, 0.0, 1.0,
                                           20, x_o, y_o, z_o));
}


} // end namespace itk

#endif
