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
#ifndef __itkHaeberlePointSpreadFunctionImageSource_txx
#define __itkHaeberlePointSpreadFunctionImageSource_txx

#include "itkHaeberlePointSpreadFunctionImageSource.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkMath.h"
#include "itkObjectFactory.h"
#include "itkProgressReporter.h"

#include <algorithm>

namespace itk
{

//----------------------------------------------------------------------------
template <class TOutputImage>
HaeberlePointSpreadFunctionImageSource<TOutputImage>
::HaeberlePointSpreadFunctionImageSource()
{
}


//----------------------------------------------------------------------------
template <class TOutputImage>
HaeberlePointSpreadFunctionImageSource<TOutputImage>
::~HaeberlePointSpreadFunctionImageSource()
{
}


//----------------------------------------------------------------------------
template <class TOutputImage>
void
HaeberlePointSpreadFunctionImageSource<TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
template <typename TOutputImage>
void
HaeberlePointSpreadFunctionImageSource< TOutputImage >
::BeforeThreadedGenerateData()
{
  // Set parameters in the functor
  this->m_I0illFunctor.CopySettings(this);
  this->m_I1illFunctor.CopySettings(this);
  this->m_I2illFunctor.CopySettings(this);
}


//----------------------------------------------------------------------------
template <typename TOutputImage>
void
HaeberlePointSpreadFunctionImageSource<TOutputImage>
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
template <typename TOutputImage>
double
HaeberlePointSpreadFunctionImageSource<TOutputImage>
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


  // Using the alpha suggested by the paper (asin(NA / actual
  // immersion oil RI) yields an uncomputable angle of incidence in
  // the specimen layer. Taking the minimum angle from this angle and
  // the angle of incidence in the immersion oil layer that yields a
  // 90 degrees angle of incidence in the specimen layer makes the
  // integral computable.
  double alpha = std::min(asin(this->m_NumericalAperture / this->m_ActualImmersionOilRefractiveIndex),
                          asin(this->m_ActualSpecimenLayerRefractiveIndex / this->m_ActualImmersionOilRefractiveIndex));
  ComplexType I0ill = IntegrateFunctor(this->m_I0illFunctor, 0.0, alpha,
                                       20, x_o, y_o, z_o);
  ComplexType I1ill = IntegrateFunctor(this->m_I1illFunctor, 0.0, alpha,
                                       20, x_o, y_o, z_o);
  ComplexType I2ill = IntegrateFunctor(this->m_I2illFunctor, 0.0, alpha,
                                       20, x_o, y_o, z_o);

  // Return a weighted sum of the squared magnitudes of the three
  // integral values.
  return static_cast<PixelType>( norm(I0ill) + 2.0*norm(I1ill) + norm(I2ill) );
}


} // end namespace itk

#endif
