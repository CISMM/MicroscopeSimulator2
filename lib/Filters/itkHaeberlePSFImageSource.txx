/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkHaeberlePSFImageSource.cxx,v $
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
#ifndef __itkHaeberlePSFImageSource_txx
#define __itkHaeberlePSFImageSource_txx

#include "itkHaeberlePSFImageSource.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkMath.h"
#include "itkObjectFactory.h"
#include "itkProgressReporter.h"

#define INTEGRATE_M 20
#define INTEGRATE_N (2*INTEGRATE_M+1)

namespace itk
{

//----------------------------------------------------------------------------
template <class TOutputImage>
HaeberlePSFImageSource<TOutputImage>
::HaeberlePSFImageSource()
{
  m_Size.Fill(32);
  m_Spacing.Fill(65.0);
  m_Origin.Fill(0.0);
  m_PointCenter.Fill(0.0);

  m_ShearX = 0.0;
  m_ShearY = 0.0;

  // Set default PSF model parameters.
  m_EmissionWavelength   = 550.0; // in nanometers
  m_NumericalAperture    = 1.4;   // unitless
  m_Magnification        = 60.0;  // unitless

  m_CoverSlipRefractiveIndex    = 1.522; // unitless
  m_CoverSlipThickness          = 170.0; // in micrometers
  m_ImmersionOilRefractiveIndex = 1.515; // unitless
  m_ImmersionOilThickness       = 100.0; // in micrometers

  m_SpecimenLayerRefractiveIndex =  1.33; // unitless
}


//----------------------------------------------------------------------------
template <class TOutputImage>
HaeberlePSFImageSource<TOutputImage>
::~HaeberlePSFImageSource()
{
}


//----------------------------------------------------------------------------
template <class TOutputImage>
void
HaeberlePSFImageSource<TOutputImage>
::SetParameters(const ParametersType& parameters)
{
  int index = 0;
  SpacingType spacing;
  spacing[0] = parameters[index++];
  spacing[1] = parameters[index++];
  spacing[2] = parameters[index++];
  SetSpacing(spacing);

  PointType center;
  center[0] = parameters[index++];
  center[1] = parameters[index++];
  center[2] = parameters[index++];
  SetPointCenter(center);

  SetShearX(parameters[index++]);
  SetShearY(parameters[index++]);

  SetEmissionWavelength(parameters[index++]);
  SetNumericalAperture(parameters[index++]);
  SetMagnification(parameters[index++]);

  SetCoverSlipRefractiveIndex(parameters[index++]);
  SetCoverSlipThickness(parameters[index++]);
  SetImmersionOilRefractiveIndex(parameters[index++]);
  SetImmersionOilThickness(parameters[index++]);

  SetSpecimenLayerRefractiveIndex(parameters[index++]);
}


template <class TOutputImage>
typename HaeberlePSFImageSource<TOutputImage>::ParametersType
HaeberlePSFImageSource<TOutputImage>
::GetParameters() const
{
  ParametersType parameters(GetNumberOfParameters());

  int index = 0;
  for ( unsigned int i = 0; i < m_Spacing.Size(); i++ )
    {
    parameters[index++] = GetSpacing()[i];
    }

  for ( unsigned int i = 0; i < m_PointCenter.Size(); i++ )
    {
    parameters[index++] = GetPointCenter()[i];
    }

  parameters[index++] = GetShearX();
  parameters[index++] = GetShearY();

  parameters[index++] = GetEmissionWavelength();
  parameters[index++] = GetNumericalAperture();
  parameters[index++] = GetMagnification();

  parameters[index++] = GetCoverSlipRefractiveIndex();
  parameters[index++] = GetCoverSlipThickness();
  parameters[index++] = GetImmersionOilRefractiveIndex();
  parameters[index++] = GetImmersionOilThickness();

  parameters[index++] = GetSpecimenLayerRefractiveIndex();

  return parameters;
}


//----------------------------------------------------------------------------
template <class TOutputImage>
unsigned int
HaeberlePSFImageSource<TOutputImage>
::GetNumberOfParameters() const
{
  return m_Spacing.Size() + m_PointCenter.Size() + 10;
}


//----------------------------------------------------------------------------
template <class TOutputImage>
void
HaeberlePSFImageSource<TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  unsigned int i;
  os << indent << "Origin: [";
  for ( i=0; i < m_Origin.Size() - 1; i++ )
    {
    os << m_Origin[i] << ", ";
    }
  os << m_Origin[i] << "]" << std::endl;

  os << indent << "Spacing: [";
  for ( i=0; i < m_Spacing.Size() - 1; i++ )
    {
    os << m_Spacing[i] << ", ";
    }
  os << m_Spacing[i] << "] (nanometers)" << std::endl;

  os << indent << "Size: [";
  for ( i=0; i < m_Size.GetSizeDimension() - 1; i++ )
    {
    os << m_Size[i] << ", ";
    }
  os << m_Size[i] << "]" << std::endl;

  os << indent << "PointCenter: [";
  for ( i=0; i < m_PointCenter.Size() - 1; i++ )
    {
    os << m_PointCenter[i] << ", ";
    }
  os << m_PointCenter[i] << "]" << std::endl;

  os << "ShearX: " << m_ShearX << std::endl;
  os << "ShearY: " << m_ShearY << std::endl;

  os << indent << "EmissionWavelength (nanometers): "
     << m_EmissionWavelength << std::endl;

  os << indent << "NumericalAperture: "
     << m_NumericalAperture << std::endl;

  os << indent << "Magnification: "
     << m_Magnification << std::endl;

  os << indent << "CoverSlipRefractiveIndex: "
     << m_CoverSlipRefractiveIndex << std::endl;

  os << indent << "CoverSlipThickness (micrometers): "
     << m_CoverSlipThickness << std::endl;

  os << indent << "ImmersionOilRefractiveIndex: "
     << m_ImmersionOilRefractiveIndex << std::endl;

  os << indent << "ImmersionOilThickness (micrometers): "
     << m_ImmersionOilThickness << std::endl;

  os << indent << "SpecimenLayerRefractiveIndex: "
     << m_SpecimenLayerRefractiveIndex << std::endl;

}

//----------------------------------------------------------------------------
template <typename TOutputImage>
void
HaeberlePSFImageSource<TOutputImage>
::GenerateOutputInformation()
{
  OutputImageType *output;
  IndexType index = {{0}};
  SizeType size( m_Size );

  output = this->GetOutput(0);

  RegionType largestPossibleRegion;
  largestPossibleRegion.SetSize( size );
  largestPossibleRegion.SetIndex( index );
  output->SetLargestPossibleRegion( largestPossibleRegion );

  output->SetSpacing(m_Spacing);
  output->SetOrigin(m_Origin);
}

//----------------------------------------------------------------------------
template <typename TOutputImage>
void
HaeberlePSFImageSource<TOutputImage>
::ThreadedGenerateData(const RegionType& outputRegionForThread, int threadId )
{
  // Support progress methods/callbacks
  ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

  typename TOutputImage::Pointer image = this->GetOutput(0);

  ImageRegionIteratorWithIndex<OutputImageType> it(image, outputRegionForThread);

  int zSlice = -1;

  for (; !it.IsAtEnd(); ++it)
    {
    IndexType index = it.GetIndex();
    PointType point;
    image->TransformIndexToPhysicalPoint(index, point);

    // Apply x and y shear here
    point[0] = point[0] - m_ShearX*(point[2] - m_PointCenter[2]);
    point[1] = point[1] - m_ShearY*(point[2] - m_PointCenter[2]);

    // Shift the center of the point
    for (int i = 0; i < 3; i++) point[i] -= m_PointCenter[i];

    // See if we have switched slices. If so, we need to precompute some
    // integral terms for the new slice.
    if (zSlice != index[2])
      {
      zSlice = index[2];
      }

    //it.Set( ComputeSampleValue(point) );
    it.Set( static_cast<PixelType>(threadId) );
    progress.CompletedPixel();
    }
}


//----------------------------------------------------------------------------
template <typename TOutputImage>
double
HaeberlePSFImageSource<TOutputImage>
::ComputeSampleValue(typename TOutputImage::PointType& point)
{
  PixelType px = point[0] * 1e-9;
  PixelType py = point[1] * 1e-9;
  PixelType pz = point[2] * 1e-9;

  /* Compute terms that are independent of terms within the integral. */
  double K = 2.0f*itk::Math::pi / (m_EmissionWavelength * 1e-9);
  double NA = m_NumericalAperture;
  double mag = m_Magnification;

  // We have to convert to coordinates of the detector points
  double x_o = px * mag;
  double y_o = py * mag;
  double z_o = pz; // No conversion needed

  // Compute common terms in all steps of the integration
  double r_o = sqrt((x_o*x_o) + (y_o*y_o));

  // Compute integration of the formula
  double h = 1.0 / static_cast<double>(INTEGRATE_N-1);

  // Accumulator for integration.
  ComplexType sum(0.0, 0.0);

#if 0
  // Compute initial terms in Simpson quadrature method.
  sum += IntegralTerm(opdCache, K, a, z_d, 0, h, r_o, z_o);

  sum += IntegralTerm(opdCache, K, a, z_d, INTEGRATE_N-1, h, r_o, z_o);

  for (int k = 1; k <= INTEGRATE_M-1; k++)
    {
    sum += 2.0*IntegralTerm(opdCache, K, a, z_d, 2*k, h, r_o, z_o);
    }

  for (int k = 1; k <= INTEGRATE_M; k++)
    {
    sum += 4.0*IntegralTerm(opdCache, K, a, z_d, 2*k-1, h, r_o, z_o);
    }

  sum *= (h/3.0f);
#endif

  // Return squared magnitude of the integrated value
  return static_cast<PixelType>( norm(sum) );
}


//----------------------------------------------------------------------------
template <typename TOutputImage>
double
HaeberlePSFImageSource<TOutputImage>
::ComputeIntegratedPixelValue(typename TOutputImage::PointType& point)
{
  double integrated = 0.0f;

  // Evaluate over a grid
  int divs = 1;
  double dx = m_Spacing[0] / static_cast<double>(divs);
  double dy = m_Spacing[1] / static_cast<double>(divs);
  for (int iy = 0; iy < divs; iy++)
    {
    for (int ix = 0; ix < divs; ix++)
      {
      PointType samplePoint;
      double fx = (static_cast<double>(ix) + 0.5f) * dx;
      double fy = (static_cast<double>(iy) + 0.5f) * dy;
      samplePoint[0] = point[0] - 0.5*m_Spacing[0] + fx;
      samplePoint[1] = point[1] - 0.5*m_Spacing[1] + fy;
      samplePoint[2] = point[2];

      integrated += ComputeSampleValue(samplePoint);
      }
    }

  return integrated;
}


} // end namespace itk

#endif
