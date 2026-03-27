/**
* \class CSpatiallyOrientedSOSFilter
*
* \brief Declaration of CSpatiallyOrientedSOSFilter class
* \date	Dec 2025
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga)||
* \b Contact: areyes@uma.es
*
* \b Copyright: University of Malaga
* 
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/

#ifndef _SPATIALLY_ORIENTED_SOS_FILTER_HPP
#define _SPATIALLY_ORIENTED_SOS_FILTER_HPP

#define EPSILON 0.001f


#include <Common/Buffer.hpp>
#include <Common/GlobalParameters.hpp>
#include <Common/SourceListenerRelativePositionCalculation.hpp>
#include <ServiceModules/SphericalSOSTable.hpp>
#include <Filters/FilterBase.hpp>
#include <ProcessingModules/MultichannelBiquadFilterChain.hpp>



#define NUMBER_OF_COEFFICIENTS_IN_BIQUAD_SECTION 6

namespace BRTFilters {

	class CSpatiallyOrientedSOSFilter : public CFilterBase, private BRTProcessing::CMultichannelBiquadFilterChain {
	public:
		CSpatiallyOrientedSOSFilter()
			: CFilterBase { TFilterType::IIR_SOS_SOFA }		
		{
					
		}

		/**
		 * @brief Configure the filter, according to the number of second order stages you tell it to set.
		 * @param _numberOfFilterStages number of second order stages to set
		 */
		bool Setup(int _numberOfChannels, int _numberOfBiquadSectionsPerChannel) override{ 
			
			bool result = BRTProcessing::CMultichannelBiquadFilterChain::Setup(_numberOfChannels, _numberOfBiquadSectionsPerChannel);

			if (!result) {
				BRTProcessing::CMultichannelBiquadFilterChain::DisableProcessor();
				return false;
			}

			if (enable) {
				BRTProcessing::CMultichannelBiquadFilterChain::EnableProcessor();
			} else {
				BRTProcessing::CMultichannelBiquadFilterChain::DisableProcessor();
			}
			return result;			
		}
		
		/**
		 * @brief Store the coefficients of the filter for a given channel
		 * @param _channel channel number
		 * @param _coefficients vector of coefficients for the channel
		 */
		bool SetCoefficients(const int & _channel, const std::vector<float>& _coefficients) override {
			return BRTProcessing::CMultichannelBiquadFilterChain::SetCoefficients(_channel, _coefficients);			
		}
		
		/**
		 * @brief Enable processor
		 */
		void Enable() override {
			enable = true;
			BRTProcessing::CMultichannelBiquadFilterChain::EnableProcessor();
		};

		///**
		// * @brief Disable processor
		// */
		void Disable() override {
			enable = false;
			BRTProcessing::CMultichannelBiquadFilterChain::DisableProcessor();
		};
		

		/**
		 * @brief Filter the input signal with the binaural filter taking into account the source and listener position
		 * @param _channel channel number
		 * @param _inBuffer input buffer
		 * @param outBuffer output buffer
		 * @param sourceTransform source position and orientation
		 * @param listenerTransform listener position and orientation
		 * @param _ear ear (left or right)
		 * @param _SOSFilterWeakPtr filter coefficients service pointer
		 */
		// TODO Not tested yet, to be tested when we have SOS SOFA files not ordered by interaural azimuth
		void Process(const int & _channel, const CMonoBuffer<float> & _inBuffer, CMonoBuffer<float> & outBuffer, const Common::CTransform & sourceTransform, const Common::CTransform & listenerTransform, const Common::T_ear _ear, std::weak_ptr<BRTServices::CServicesBase> & _SOSFilterWeakPtr) override {
			// Check process flag
			if (!enable) {
				outBuffer = _inBuffer;
				return;
			}

			float distance = Common::CSourceListenerRelativePositionCalculation::CalculateSourceListenerDistance(sourceTransform, listenerTransform);

			if (distance > DISTANCE_MODEL_THRESHOLD_NEAR) {
				return;
			}
			if (Common::AreSame(distance, 0, MINIMUM_DISTANCE_SOURCE_LISTENER)) {
				SET_RESULT(RESULT_WARNING, "The source is inside the listener's head.");
				outBuffer = _inBuffer;
				return;
			}

			ASSERT(_inBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");

			// Check listener ILD
			std::shared_ptr<BRTServices::CServicesBase> _SOSFilterPtr = _SOSFilterWeakPtr.lock();
			if (!_SOSFilterPtr) {
				SET_RESULT(RESULT_ERROR_NULLPOINTER, "SOS filter pointer is null when trying to use in BRTProcessing::CBiquadChainTable");
				outBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				return;
			}
			
			float azimuth;
			float elevation;
			Common::CSourceListenerRelativePositionCalculation::CalculateSourceListenerRelativePositions(sourceTransform, listenerTransform, azimuth, elevation);

			//Get coefficients from the ILD table
			std::vector<float> coefficients = _SOSFilterPtr->GetSOSCoefficients_SpatiallyOriented(azimuth, elevation, distance, _ear);

			BRTProcessing::CMultichannelBiquadFilterChain::SetCoefficients(_channel, coefficients); //Set coefficients
			BRTProcessing::CMultichannelBiquadFilterChain::Process(_channel, _inBuffer, outBuffer); // Process the signal
		}

		/**
		 * @brief Filter the input signal with the binaural filter taking into account the source and listener position, but only using the interaural azimuth to get the coefficients. This method is intended to be used with near-field SOS SOFA files, which only have interaural azimuth as spatial parameter.
		 * @param _channel 
		 * @param _inBuffer 
		 * @param outBuffer 
		 * @param sourceTransform 
		 * @param listenerTransform 
		 * @param _ear 
		 * @param _SOSFilterWeakPtr 
		 */
		void ProcessByInterauralAzimuth(const int & _channel, const CMonoBuffer<float> & _inBuffer, CMonoBuffer<float> & outBuffer, const Common::CTransform & sourceTransform, const Common::CTransform & listenerTransform, const Common::T_ear _ear, std::weak_ptr<BRTServices::CServicesBase> & _SOSFilterWeakPtr) override
		{			
			// Check process flag
			if (!enable) { 
				outBuffer = _inBuffer;		
				return;	
			}		
			
			float distance = Common::CSourceListenerRelativePositionCalculation::CalculateSourceListenerDistance(sourceTransform, listenerTransform);			
			
			if (distance > DISTANCE_MODEL_THRESHOLD_NEAR) {
				outBuffer = _inBuffer;
				return;
			}
			if (Common::AreSame(distance, 0, MINIMUM_DISTANCE_SOURCE_LISTENER)) {
				SET_RESULT(RESULT_WARNING, "The source is inside the listener's head.");
				outBuffer = _inBuffer;				
				return;
			}

			ASSERT(_inBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");			
			
			// Check listener ILD
			std::shared_ptr<BRTServices::CServicesBase> _SOSFilterPtr = _SOSFilterWeakPtr.lock();
			if (!_SOSFilterPtr) {
				SET_RESULT(RESULT_ERROR_NULLPOINTER, "SOS filter pointer is null when trying to use in BRTProcessing::CBiquadChainTable");				
				outBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);				
				return;
			}

			float interauralAzimuth = Common::CSourceListenerRelativePositionCalculation::CalculateInterauralAzimuth(sourceTransform, listenerTransform);

			//Get coefficients from the ILD table			
			std::vector<float> coefficients = _SOSFilterPtr->GetSOSCoefficients_SpatiallyOriented(interauralAzimuth, 0, distance, _ear);
												
			BRTProcessing::CMultichannelBiquadFilterChain::SetCoefficients(_channel, coefficients); //Set coefficients						
			BRTProcessing::CMultichannelBiquadFilterChain::Process(_channel, _inBuffer, outBuffer); // Process the signal
		}

		

		/**
		 * @brief Reset the buffers of the process
		 */
		void ResetBuffers() override{
			BRTProcessing::CMultichannelBiquadFilterChain::ResetBuffers();	
		}

	
	private:
		///////////////////////
		// Private Methods
		///////////////////////		

		/// Calculates the parameters derived from the source and listener position
		//float CalculateInterauralAzimuth(const Common::CTransform& _sourceTransform, const Common::CTransform& _listenerTransform)
		//{

		//	//Get azimuth and elevation between listener and source
		//	Common::CVector3 _vectorToListener = _listenerTransform.GetVectorTo(_sourceTransform);
		//	float _distanceToListener = _vectorToListener.GetDistance();

		//	//Check listener and source are in the same position
		//	if (_distanceToListener <= MINIMUM_DISTANCE_SOURCE_LISTENER) {
		//		SET_RESULT(RESULT_WARNING, "The sound source is too close to the centre of the listener's head in BRTProcessing::CNearFieldEffect");
		//		return MINIMUM_DISTANCE_SOURCE_LISTENER;
		//	}						
		//	return _vectorToListener.GetInterauralAzimuthDegrees();			
		//}
				
		///////////////////////
		// Private Attributes
		///////////////////////		
		Common::CGlobalParameters globalParameters;		
	};
}
#endif