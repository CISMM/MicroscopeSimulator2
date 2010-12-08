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

class HaeberlePointSpreadFunctionI0illIntegrand :
    public DesignAndActualConditionsMicroscopePointSpreadFunctionIntegrand
{
public:

  typedef DesignAndActualConditionsMicroscopePointSpreadFunctionIntegrand::ComplexType ComplexType;

  ComplexType operator()(double r, double z, double rho) const
  {
    return ComplexType(0.0);
  }

};

class HaeberlePointSpreadFunctionI1illIntegrand :
    public DesignAndActualConditionsMicroscopePointSpreadFunctionIntegrand
{
public:

  typedef DesignAndActualConditionsMicroscopePointSpreadFunctionIntegrand::ComplexType ComplexType;

  ComplexType operator()(double r, double z, double rho) const
  {
    return ComplexType(0.0);
  }

};

class HaeberlePointSpreadFunctionI2illIntegrand :
    public DesignAndActualConditionsMicroscopePointSpreadFunctionIntegrand
{
public:

  typedef DesignAndActualConditionsMicroscopePointSpreadFunctionIntegrand::ComplexType ComplexType;

  ComplexType operator()(double r, double z, double rho) const
  {
    return ComplexType(0.0);
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
