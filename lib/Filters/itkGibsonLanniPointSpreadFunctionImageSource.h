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
#ifndef __itkGibsonLanniPointSpreadFunctionImageSource_h
#define __itkGibsonLanniPointSpreadFunctionImageSource_h

#include "itkDesignAndActualConditionsMicroscopePointSpreadFunctionImageSource.h"
#include "itkNumericTraits.h"

namespace itk
{

namespace Functor {

class GibsonLanniPointSpreadFunctionIntegrand :
    public DesignAndActualConditionsMicroscopePointSpreadFunctionIntegrand
{
public:
  typedef DesignAndActualConditionsMicroscopePointSpreadFunctionIntegrand::ComplexType ComplexType;

  ComplexType operator()(double r, double z, double rho) const
  {
    double bessel = j0(m_K * m_A * rho * r / (0.160 + z));

    return bessel * exp(ComplexType(0.0, 1.0) * this->OPD(rho, z) * m_K) * rho;
  }

};


} // end namespace Functor

/** \class GibsonLanniPointSpreadFunctionImageSource
 * \brief Generate a synthetic point-spread function according to the
 * Gibson-Lanni model.
 *
 * The Gibson-Lanni point-spread function model takes into account optical
 * path differences from the design conditions of an objective in a
 * widefield fluorescence microscope. This image source generates images
 * according to this model. IMPORTANT: Please pay attention to the units
 * each method expects. Some take nanometers, some take micrometers, and some
 * take millimeters.
 *
 * \ingroup DataSources Multithreaded
 */
template< class TOutputImage >
class ITK_EXPORT GibsonLanniPointSpreadFunctionImageSource :
    public DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource< TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef GibsonLanniPointSpreadFunctionImageSource                        Self;
  typedef DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource< TOutputImage >
    Superclass;
  typedef SmartPointer<Self>                                               Pointer;
  typedef SmartPointer<const Self>                                         ConstPointer;

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
  typedef Functor::GibsonLanniPointSpreadFunctionIntegrand FunctorType;

  /** Run-time type information (and related methods). */
  itkTypeMacro(GibsonLanniPointSpreadFunctionImageSource, DesignAndActualConditionsMicroscopePointSpreadFunctionImageSource);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  typedef typename Superclass::ParametersValueType ParametersValueType;
  typedef typename Superclass::ParametersType      ParametersType;

protected:
  GibsonLanniPointSpreadFunctionImageSource();
  ~GibsonLanniPointSpreadFunctionImageSource();
  void PrintSelf(std::ostream& os, Indent indent) const;

  void BeforeThreadedGenerateData();
  void ThreadedGenerateData(const RegionType& outputRegionForThread,
                            int threadId );

  /** Computes the light intensity at a specified point. */
  double ComputeSampleValue(const PointType& point);

private:
  GibsonLanniPointSpreadFunctionImageSource(const GibsonLanniPointSpreadFunctionImageSource&); //purposely not implemented
  void operator=(const GibsonLanniPointSpreadFunctionImageSource&); //purposely not implemented

  FunctorType m_IntegrandFunctor;

};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkGibsonLanniPointSpreadFunctionImageSource.txx"
#endif

#endif
