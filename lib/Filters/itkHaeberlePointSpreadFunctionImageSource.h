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
#ifndef __itkHaeberlePointSpreadFunctionImageSource_h
#define __itkHaeberlePointSpreadFunctionImageSource_h

#include "itkDesignAndActualConditionsMicroscopePointSpreadFunctionImageSource.h"
#include "itkNumericTraits.h"

namespace itk
{

namespace Functor {

class HaeberlePointSpreadFunctionCommon :
    public DesignAndActualConditionsMicroscopePointSpreadFunctionIntegrand
{
public:

  typedef DesignAndActualConditionsMicroscopePointSpreadFunctionIntegrand::ComplexType ComplexType;

  template< class TSource >
  void CopySettings( const TSource* source )
  {
    DesignAndActualConditionsMicroscopePointSpreadFunctionIntegrand::CopySettings(source);

    // k_0 is not defined in the paper. I'm assuming it is the
    // wavenumber of the emission wavelength in vacuum.
    this->k_0 = 2.0 * itk::Math::pi / (this->m_EmissionWavelength * 1e-9);

    this->k_1 = this->k_0 * this->m_ActualCoverSlipRefractiveIndex;
  }

protected:
  double k_0;
  double k_1;

protected:
  inline double Theta2(double theta_1) const
  {
    double n1 = this->m_ActualImmersionOilRefractiveIndex;
    double n2 = this->m_ActualCoverSlipRefractiveIndex;
    return asin(n1*sin(theta_1) / n2);
  }

  inline double Theta3(double theta_2) const
  {
    double n2 = this->m_ActualCoverSlipRefractiveIndex;
    double n3 = this->m_ActualSpecimenLayerRefractiveIndex;
    return asin(n2*sin(theta_2) / n3);
  }

  inline double tii1s(int i, double n[3], double theta[3]) const
  {
    return (2*n[i]*cos(theta[i])) /
      (n[i]*cos(theta[i]) + n[i+1]*cos(theta[i+1]));
  }

  inline double tii1p(int i, double n[3], double theta[3]) const
  {
    return (2*n[i]*cos(theta[i])) /
      (n[i+1]*cos(theta[i]) + n[i] * cos(theta[i+1]));
  }

  inline ComplexType CommonTerms(double theta_1, double z) const
  {
    double rho = this->m_ActualImmersionOilRefractiveIndex * sin(theta_1)
      / this->m_NumericalAperture;
    ComplexType I(0.0, 1.0);
    ComplexType c1 = sqrt(cos(theta_1)) * sin(theta_1);
    ComplexType c2 = exp(I*k_0*OPD(rho, z));

    return c1 * c2;
  }

  inline void AssembleIndicesOfRefraction(double n[3]) const
  {
    n[0] = this->m_ActualImmersionOilRefractiveIndex;
    n[1] = this->m_ActualCoverSlipRefractiveIndex;
    n[2] = this->m_ActualSpecimenLayerRefractiveIndex;
  }

  inline void AssembleAnglesOfIncidence(double theta_1, double theta[3]) const
  {
    theta[0] = theta_1;
    theta[1] = Theta2(theta_1);
    theta[2] = Theta3(theta[1]);
  }

};


class HaeberlePointSpreadFunctionI0illIntegrand :
    public HaeberlePointSpreadFunctionCommon
{
public:

  typedef HaeberlePointSpreadFunctionCommon::ComplexType ComplexType;

  ComplexType operator()(double r, double z, double theta_1) const
  {
    double n[3], theta[3];
    this->AssembleIndicesOfRefraction(n);
    this->AssembleAnglesOfIncidence(theta_1, theta);

    ComplexType uniqueTerm =
      (tii1s(0, n, theta) * tii1s(1, n, theta) +
       tii1p(0, n, theta) * tii1p(1, n, theta)*cos(theta[2])) *
      j0(this->k_1*r*sin(theta_1));

    return this->CommonTerms(theta_1, z) * uniqueTerm;
  }

};

class HaeberlePointSpreadFunctionI1illIntegrand :
    public HaeberlePointSpreadFunctionCommon
{
public:

  typedef HaeberlePointSpreadFunctionCommon::ComplexType ComplexType;

  ComplexType operator()(double r, double z, double theta_1) const
  {
    double n[3], theta[3];
    this->AssembleIndicesOfRefraction(n);
    this->AssembleAnglesOfIncidence(theta_1, theta);

    ComplexType uniqueTerm = tii1p(0, n, theta) * tii1p(1, n, theta) *
      cos(theta[2]) * j1(this->k_1*r*sin(theta_1));

    return this->CommonTerms(theta_1, z) * uniqueTerm;
  }

};

class HaeberlePointSpreadFunctionI2illIntegrand :
    public HaeberlePointSpreadFunctionCommon
{
public:

  typedef HaeberlePointSpreadFunctionCommon::ComplexType ComplexType;

  ComplexType operator()(double r, double z, double theta_1) const
  {
    double n[3], theta[3];
    this->AssembleIndicesOfRefraction(n);
    this->AssembleAnglesOfIncidence(theta_1, theta);

    ComplexType uniqueTerm =
      (tii1s(0, n, theta) * tii1s(1, n, theta) -
       tii1p(0, n, theta) * tii1p(1, n, theta) * cos(theta[2])) *
      jn(2, this->k_1*r*sin(theta_1));

    return this->CommonTerms(theta_1, z) * uniqueTerm;
  }

};

} // end namespace Functor

/** \class HaeberlePointSpreadFunctionImageSource
 * \brief Generate a synthetic point-spread function according to the
 * Haeberle model.
 *
 * The Haeberle point-spread function model is based on the vectorial
 * model of light propagation in widefield fluorescence
 * microscopes. This image source generates images according to this
 * IMPORTANT: Please pay attention to the units each method
 * expects. Some take nanometers, some take micrometers, and some
 * take millimeters.
 *
 * \ingroup DataSources Multithreaded
 */
template <class TOutputImage>
class ITK_EXPORT HaeberlePointSpreadFunctionImageSource :
    public DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource< TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef HaeberlePointSpreadFunctionImageSource Self;
  typedef DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource< TOutputImage >
    Superclass;
  typedef SmartPointer<Self>                     Pointer;
  typedef SmartPointer<const Self>               ConstPointer;

  /** Typedef for the output image PixelType. */
  typedef TOutputImage                             OutputImageType;
  typedef typename OutputImageType::PixelType      PixelType;
  typedef typename OutputImageType::IndexType      IndexType;
  typedef typename OutputImageType::RegionType     RegionType;
  typedef typename OutputImageType::PointType      PointType;
  typedef typename OutputImageType::PointValueType PointValueType;
  typedef typename OutputImageType::SpacingType    SpacingType;
  typedef typename OutputImageType::SizeType       SizeType;
  typedef typename OutputImageType::SizeValueType  SizeValueType;

  itkStaticConstMacro(ImageDimension, unsigned int,
		      TOutputImage::ImageDimension);

  /** Typedef for complex type. */
  typedef typename Superclass::ComplexType ComplexType;

  /** Typedef for functor. */
  typedef Functor::HaeberlePointSpreadFunctionI0illIntegrand FunctorTypeI0ill;
  typedef Functor::HaeberlePointSpreadFunctionI1illIntegrand FunctorTypeI1ill;
  typedef Functor::HaeberlePointSpreadFunctionI2illIntegrand FunctorTypeI2ill;

  /** Run-time type information (and related methods). */
  itkTypeMacro(HaeberlePointSpreadFunctionImageSource, ParametricImageSource);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  typedef typename Superclass::ParametersValueType ParametersValueType;
  typedef typename Superclass::ParametersType      ParametersType;

protected:
  HaeberlePointSpreadFunctionImageSource();
  ~HaeberlePointSpreadFunctionImageSource();
  void PrintSelf(std::ostream& os, Indent indent) const;

  void BeforeThreadedGenerateData();
  void ThreadedGenerateData(const RegionType& outputRegionForThread,
                                    int threadId );

  /** Computes the light intensity at a specified point. */
  double ComputeSampleValue(const PointType& point);

private:
  HaeberlePointSpreadFunctionImageSource(const HaeberlePointSpreadFunctionImageSource&); //purposely not implemented
  void operator=(const HaeberlePointSpreadFunctionImageSource&); //purposely not implemented

  FunctorTypeI0ill m_I0illFunctor;
  FunctorTypeI1ill m_I1illFunctor;
  FunctorTypeI2ill m_I2illFunctor;

};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkHaeberlePointSpreadFunctionImageSource.txx"
#endif

#endif
