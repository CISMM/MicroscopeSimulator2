/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkGibsonLanniPointSpreadFunctionImageSource.cxx,v $
  Language:  C++
  Date:      $Date: 2010/05/17 15:41:35 $
  Version:   $Revision: 1.12 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkGibsonLanniPointSpreadFunctionImageSource_txx
#define __itkGibsonLanniPointSpreadFunctionImageSource_txx

#include "itkGibsonLanniPointSpreadFunctionImageSource.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkMath.h"
#include "itkObjectFactory.h"
#include "itkProgressReporter.h"

#define INTEGRATE_M 20
#define INTEGRATE_N (2*INTEGRATE_M+1)

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

  // TODO - Set up precomputed optical path difference terms that are
  // independent of the sample point.

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
    for ( int i = 0; i < 3; i++ ) point[i] -= this->m_PointCenter[i];

    it.Set( ComputeSampleValue( point ) );
    progress.CompletedPixel();
    }
}


#if 0
//----------------------------------------------------------------------------
template< class TOutputImage >
void
GibsonLanniPointSpreadFunctionImageSource< TOutputImage >
::PrecomputeOPDTerms(double z_o)
{
  double K = 2.0f*itk::Math::pi / (m_EmissionWavelength * 1e-9);
  double h = 1.0f / static_cast<double>(INTEGRATE_N-1);
  double NA = m_NumericalAperture;
  double mag = m_Magnification;

  for (int i = 0; i < INTEGRATE_N; i++) {
    double rho = static_cast<double>(i)*h;
    ComplexType W = OPD(rho, z_o) * K;
    ComplexType I(0.0f, 1.0f);
    opdCache[i] = exp(I*W);
  }

}
#endif


#if 0
//----------------------------------------------------------------------------
template< class TOutputImage >
typename GibsonLanniPointSpreadFunctionImageSource< TOutputImage >::ComplexType
GibsonLanniPointSpreadFunctionImageSource< TOutputImage >
::IntegralTerm(const Self* instance, double r, double z, double rho)
{
  double K = instance->GetK();
  double A = instance->GetA();
  double bessel = j0(K * A * rho * r / 0.080);

  return bessel * exp(ComplexType(0.0, 1.0) * instance->OPD(rho) * K) * rho;
}
#endif


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
