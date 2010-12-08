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
#ifndef __itkDesignAndActualConditionsMicroscopePointSpreadFunctionImageSource_txx
#define __itkDesignAndActualConditionsMicroscopePointSpreadFunctionImageSource_txx

#include "itkDesignAndActualConditionsMicroscopePointSpreadFunctionImageSource.h"

namespace itk
{

//----------------------------------------------------------------------------
template< class TOutputImage >
DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource< TOutputImage >
::DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource()
{
  m_Size.Fill(32);
  m_Spacing.Fill(65.0);
  m_Origin.Fill(0.0);

  // Set default point-spread function model parameters.
  m_PointCenter.Fill(0.0);
  m_EmissionWavelength = 550.0; // in nanometers
  m_NumericalAperture  = 1.4;   // unitless
  m_Magnification      = 60.0;  // unitless

  m_DesignCoverSlipRefractiveIndex     = 1.522; // unitless
  m_ActualCoverSlipRefractiveIndex     = 1.522; // unitless
  m_DesignCoverSlipThickness           = 170.0; // in micrometers
  m_ActualCoverSlipThickness           = 170.0; // in micrometers
  m_DesignImmersionOilRefractiveIndex  = 1.515; // unitless
  m_ActualImmersionOilRefractiveIndex  = 1.515; // unitless
  m_DesignImmersionOilThickness        = 100.0; // in micrometers

  m_DesignSpecimenLayerRefractiveIndex =  1.33; // unitless
  m_ActualSpecimenLayerRefractiveIndex =  1.33; // unitless
  m_PointSourceDepthInSpecimenLayer    =   0.0; // in micrometers
}


//----------------------------------------------------------------------------
template< class TOutputImage >
DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource< TOutputImage >
::~DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource()
{
}


//----------------------------------------------------------------------------
template< class TOutputImage >
void
DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource< TOutputImage >
::SetParameters(const ParametersType& parameters)
{
  int index = 0;

  PointType center;
  center[0] = parameters[index++];
  center[1] = parameters[index++];
  center[2] = parameters[index++];
  this->SetPointCenter(center);

  this->SetEmissionWavelength(parameters[index++]);
  this->SetNumericalAperture(parameters[index++]);
  this->SetMagnification(parameters[index++]);

  this->SetDesignCoverSlipRefractiveIndex(parameters[index++]);
  this->SetActualCoverSlipRefractiveIndex(parameters[index++]);
  this->SetDesignCoverSlipThickness(parameters[index++]);
  this->SetActualCoverSlipThickness(parameters[index++]);
  this->SetDesignImmersionOilRefractiveIndex(parameters[index++]);
  this->SetActualImmersionOilRefractiveIndex(parameters[index++]);
  this->SetDesignImmersionOilThickness(parameters[index++]);

  this->SetDesignSpecimenLayerRefractiveIndex(parameters[index++]);
  this->SetActualSpecimenLayerRefractiveIndex(parameters[index++]);
  this->SetPointSourceDepthInSpecimenLayer(parameters[index++]);
}


//----------------------------------------------------------------------------
template< class TOutputImage >
typename DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource< TOutputImage >::ParametersType
DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource< TOutputImage >
::GetParameters() const
{
  int index = 0;
  ParametersType parameters(this->GetNumberOfParameters());
  for (unsigned int i = 0; i < this->GetPointCenter().Size(); i++)
    {
    parameters[index++] = this->GetPointCenter()[i];
    }

  parameters[index++] = this->GetEmissionWavelength();
  parameters[index++] = this->GetNumericalAperture();
  parameters[index++] = this->GetMagnification();

  parameters[index++] = this->GetDesignCoverSlipRefractiveIndex();
  parameters[index++] = this->GetActualCoverSlipRefractiveIndex();
  parameters[index++] = this->GetDesignCoverSlipThickness();
  parameters[index++] = this->GetActualCoverSlipThickness();
  parameters[index++] = this->GetDesignImmersionOilRefractiveIndex();
  parameters[index++] = this->GetActualImmersionOilRefractiveIndex();
  parameters[index++] = this->GetDesignImmersionOilThickness();

  parameters[index++] = this->GetDesignSpecimenLayerRefractiveIndex();
  parameters[index++] = this->GetActualSpecimenLayerRefractiveIndex();
  parameters[index++] = this->GetPointSourceDepthInSpecimenLayer();

  return parameters;
}


//----------------------------------------------------------------------------
template< class TOutputImage >
unsigned int
DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource< TOutputImage >
::GetNumberOfParameters() const
{
  return m_PointCenter.Size() + 13;
}


#if 0
//----------------------------------------------------------------------------
template< class TOutputImage >
typename DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource< TOutputImage >::ComplexType
DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource< TOutputImage >
::IntegrateFunctor(FunctorType functor, double a, double b, int m,
                   double x, double y, double z)
{
  int n = 2*m + 1;
  double h = 1.0 / static_cast<double>(n-1);

  double r = sqrt(x*x + y*y);

  // Initialize accumulator for integration.
  ComplexType sum(0.0, 0.0);

  // Compute initial terms in Simpson quadrature method.
  sum += (*functor)(this, r, z, a);
  sum += (*functor)(this, r, z, b);

  for (int k = 1; k <= m-1; k++)
    {
    sum += 2.0 * (*functor)(this, r, z, (2*k)*h);
    }

  for (int k = 1; k <= m; k++)
    {
    sum += 4.0 * (*functor)(this, r, z, (2*k-1)*h);
    }

  sum *= h / 3.0;

  return sum;
}
#endif


//----------------------------------------------------------------------------
template< class TOutputImage >
void
DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource< TOutputImage >
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


#if 0
//----------------------------------------------------------------------------
template< class TOutputImage >
typename DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource< TOutputImage >::ComplexType
DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource< TOutputImage >
::OPDTerm(double rho, double n, double t) const
{
  double NA    = this->m_NumericalAperture;
  double n_oil = this->m_ActualImmersionOilRefractiveIndex;
  double NA_rho_sq = NA*NA*rho*rho;
  double NA_rho_over_n_sq = NA_rho_sq/(n*n);
  double n_oil_over_n_sq = (n_oil*n_oil) / (n*n);
  double NA_rho_over_n_oil_sq = NA_rho_sq/(n_oil*n_oil);

  ComplexType sq1(1.0 - NA_rho_over_n_sq);
  sq1 = sqrt(sq1);

  ComplexType sq2(1.0 - NA_rho_over_n_oil_sq);
  sq2 = n_oil_over_n_sq * sqrt(sq2);

  ComplexType result = n*t*(sq1 - sq2);
  return result;
}


//----------------------------------------------------------------------------
template< class TOutputImage >
typename DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource< TOutputImage >::ComplexType
DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource< TOutputImage >
::OPD(double rho) const
{
  double n_oil_d = this->m_DesignImmersionOilRefractiveIndex;
  double t_oil_d = this->m_DesignImmersionOilThickness * 1e-6;
  double n_s     = this->m_ActualSpecimenLayerRefractiveIndex;
  double t_s     = this->m_PointSourceDepthInSpecimenLayer * 1e-6;
  double n_g_d   = this->m_DesignCoverSlipRefractiveIndex;
  double n_g     = this->m_ActualCoverSlipRefractiveIndex;
  double t_g_d   = this->m_DesignCoverSlipThickness * 1e-6;
  double t_g     = this->m_ActualCoverSlipThickness * 1e-6;

  ComplexType t1 = this->OPDTerm(rho, n_s,     t_s);
  ComplexType t2 = this->OPDTerm(rho, n_g,     t_g);
  ComplexType t3 = this->OPDTerm(rho, n_g_d,   t_g_d);
  ComplexType t4 = this->OPDTerm(rho, n_oil_d, t_oil_d);

  ComplexType result = t1 + t2 - t3 - t4;

  return result;
}
#endif


//----------------------------------------------------------------------------
template< class TOutputImage >
double
DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource< TOutputImage >
::ComputeIntegratedPixelValue(const PointType& point)
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


//----------------------------------------------------------------------------
template< class TOutputImage >
void
DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource< TOutputImage >
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
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

  os << indent << "EmissionWavelength (nanometers): "
     << m_EmissionWavelength << std::endl;

  os << indent << "NumericalAperture: "
     << m_NumericalAperture << std::endl;

  os << indent << "Magnification: "
     << m_Magnification << std::endl;

  os << indent << "DesignCoverSlipRefractiveIndex: "
     << m_DesignCoverSlipRefractiveIndex << std::endl;

  os << indent << "ActualCoverSlipRefractiveIndex: "
     << m_ActualCoverSlipRefractiveIndex << std::endl;

  os << indent << "DesignCoverSlipThickness (micrometers): "
     << m_DesignCoverSlipThickness << std::endl;

  os << indent << "ActualCoverSlipThickness (micrometers): "
     << m_ActualCoverSlipThickness << std::endl;

  os << indent << "DesignImmersionOilRefractiveIndex: "
     << m_DesignImmersionOilRefractiveIndex << std::endl;

  os << indent << "ActualImmersionOilRefractiveIndex: "
     << m_ActualImmersionOilRefractiveIndex << std::endl;

  os << indent << "DesignImmersionOilThickness (micrometers): "
     << m_DesignImmersionOilThickness << std::endl;

  os << indent << "DesignSpecimenLayerRefractiveIndex: "
     << m_DesignSpecimenLayerRefractiveIndex << std::endl;

  os << indent << "ActualSpecimenLayerRefractiveIndex: "
     << m_ActualSpecimenLayerRefractiveIndex << std::endl;

}

} // namespace itk


#endif // __itkDesignAndActualConditionsMicroscopePointSpreadFunctionImageSource_txx
