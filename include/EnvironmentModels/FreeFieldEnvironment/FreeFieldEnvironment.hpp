/**
* \class CFreeFieldEnvironment
*
* \brief This class implements the free field processing. Applies the effects of free space propagation to a single source
* \date	Oct 2024
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo ||
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

#include <memory>
#include <Common/ErrorHandler.hpp>
#include <Common/GlobalParameters.hpp>
#include <ProcessingModules/DistanceAttenuator.hpp>
#include <Common/Waveguide.hpp>
#include <Base/BRTManager.hpp>



#ifndef _C_FREE_FIELD_ENVIRONMENT_HPP_
#define _C_FREE_FIELD_ENVIRONMENT_HPP_
namespace BRTEnvironmentModel { 
	class CFreeFieldEnvironment {
	public:
		CFreeFieldEnvironment()
			: enableProcessor { true } { 
		
			distanceAttenuation.Setup(globalParameters.distanceAttenuationFactorDB, globalParameters.referenceAttenuationDistance);
		}

		/**
		 * @brief Enable processor
		 */
		void EnableProcessor() { 
			std::lock_guard<std::mutex> l(mutex);	
			enableProcessor = true; 
		}
		/**
		 * @brief Disable processor
		 */
		void DisableProcessor() {
			std::lock_guard<std::mutex> l(mutex);	
			enableProcessor = false; 
		}
		
		/**
		 * @brief Get the flag to know if the processor is enabled.
		 * @return true if the processor is enabled, false otherwise
		 */
		bool IsProcessorEnabled() { return enableProcessor; }

		/**
		 * @brief Enable distance attenuation for this waveguide
		 */
		void EnableDistanceAttenuation() { 
			std::lock_guard<std::mutex> l(mutex);	
			distanceAttenuation.EnableProcessor(); 
		}
		
		/**
		 * @brief Disable distance attenuation for this waveguide
		 */
		void DisableDistanceAttenuation() { 
			std::lock_guard<std::mutex> l(mutex);	
			distanceAttenuation.DisableProcessor(); 
		}
				 
		/**
		 * @brief Get the flag for distance attenuation enabling
		 * @return true if the distance attenuation is enabled, false otherwise
		 */
		bool IsDistanceAttenuationEnabled() { return distanceAttenuation.IsProcessorEnabled(); }

		/**
		 * @brief Enable propagation delay for this waveguide
		 */
		void EnablePropagationDelay() { 
			std::lock_guard<std::mutex> l(mutex);	
			channelSourceListener.EnablePropagationDelay(); 
		}

		/**
		 * @brief Disable propagation delay for this waveguide
		 */
		void DisablePropagationDelay() { 
			std::lock_guard<std::mutex> l(mutex);	
			channelSourceListener.DisablePropagationDelay(); 
		}
		
		/**
		 * @brief Get the flag for propagation delay enabling
		 * @return true if the propagation delay is enabled, false otherwise
		 */
		bool IsPropagationDelayEnabled() { return channelSourceListener.IsPropagationDelayEnabled(); }
		
		/**
		 * @brief Set the distance attenuation factor in dB
		 * @param _distanceAttenuationFactorDB Attenuation factor in dB
		 */
		void SetDistanceAttenuationFactor(float _distanceAttenuationFactorDB) {
			std::lock_guard<std::mutex> l(mutex);
			distanceAttenuation.SetDistanceAttenuationFactor(_distanceAttenuationFactorDB);			
		}

		/**
		 * @brief Get the distance attenuation factor in dB
		 * @return Distance attenuation factor in decibels
		 */
		float GetDistanceAttenuationFactor() {			
			return distanceAttenuation.GetDistanceAttenuationFactor();
		}

		 /**
         * @brief Set the distance attenuation reference distance. 
         * @param _attenuationReference Distance at which the attenuation is 0 dB, in meters.
         */
		void SetReferenceAttenuationDistance(float _attenuationReferenceDistance) {
			std::lock_guard<std::mutex> l(mutex);
			distanceAttenuation.SetReferenceAttenuationDistance(_attenuationReferenceDistance);
		}

		/**
         * @brief Get the distance attenuation reference distance. This is the distance at which the attenuation is 0 dB
         * @return attenuation reference distance in meters.
         */
		float GetReferenceAttenuationDistance() {
			return distanceAttenuation.GetReferenceAttenuationDistance();
		}

		/**
		 * @brief Process the input buffer
		 * @param _inBuffer Input buffer
		 * @param _outBuffer Output buffer
		 * @param _sourceTransform Source transform
		 * @param _listenerTransform Listener transform
		 */
		void Process(const CMonoBuffer<float> & _inBuffer, CMonoBuffer<float> & _outBuffer, const Common::CTransform& _sourceTransform, const Common::CTransform& _listenerTransform, Common::CTransform& _effectiveSourceTransform ) { 
			
			std::lock_guard<std::mutex> l(mutex);
			ASSERT(_inBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");
			
			if (!enableProcessor) {
				_outBuffer = _inBuffer;
				return;
			}

			// Process the waveguide						
			Common::CVector3 waveGuideOutSourcePosition;
			channelSourceListener.PushBack(_inBuffer, _sourceTransform.GetPosition(), _listenerTransform.GetPosition());			
			CMonoBuffer<float> _waveGuideOutBuffer;
			channelSourceListener.PopFront(_waveGuideOutBuffer, _listenerTransform.GetPosition(), waveGuideOutSourcePosition);

			if (channelSourceListener.IsPropagationDelayEnabled()) {
				_effectiveSourceTransform = _sourceTransform;			
				_effectiveSourceTransform.SetPosition(waveGuideOutSourcePosition);		
				// Process the distance attenuation	
				distanceAttenuation.Process(_waveGuideOutBuffer, _outBuffer, _effectiveSourceTransform, _listenerTransform);
			} else {
				// Process the distance attenuation	
				_effectiveSourceTransform = _sourceTransform;				
				distanceAttenuation.Process(_inBuffer, _outBuffer, _sourceTransform, _listenerTransform);
			}									
		}
	
		/**
		 * @brief Reset the buffers
		 */
		void ResetBuffers() {
			std::lock_guard<std::mutex> l(mutex);
			channelSourceListener.Reset();
			//distanceAttenuation.Reset();
		}
	
	private:
		/// Atributes
		mutable std::mutex mutex;								// Thread management
		
		bool enableProcessor;									// Flag to enable the processor
		Common::CGlobalParameters globalParameters;				// Get access to global render parameters		
		
		BRTProcessing::CDistanceAttenuator distanceAttenuation; // Distance attenuation processor		
		Common::CWaveguide channelSourceListener;				// Waveguide processor
		//CLongDistanceFilter longDistanceFilter;				// Long distance filter
		
		

	};
}
#endif