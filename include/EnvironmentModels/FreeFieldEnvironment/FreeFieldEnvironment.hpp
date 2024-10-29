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
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Copyright: University of Malaga
*
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/

#include <memory>
#include <Common/ErrorHandler.hpp>
#include <Common/GlobalParameters.hpp>
#include <Common/DistanceAttenuator.hpp>
#include <Base/BRTManager.hpp>


#ifndef _C_FREE_FIELD_ENVIRONMENT_HPP_
#define _C_FREE_FIELD_ENVIRONMENT_HPP_
namespace BRTEnvironmentModel { 
	class CFreeFieldEnvironment {
	public:
		CFreeFieldEnvironment()
			: enableProcessor { true } { }

		/**
		 * @brief Enable processor
		 */
		void EnableProcessor() { 
			distanceAttenuation.EnableProcessor();
			enableProcessor = true; 
		}
		/**
		 * @brief Disable processor
		 */
		void DisableProcessor() { 
			distanceAttenuation.DisableProcessor();
			enableProcessor = false; 
		}
		/**
		 * @brief Get the flag to know if the processor is enabled.
		 * @return true if the processor is enabled, false otherwise
		 */
		bool IsProcessorEnabled() { return enableProcessor; }
	
		
		
		void Process(const CMonoBuffer<float> & _inBuffer, CMonoBuffer<float> & _outBuffer, Common::CTransform _sourceTransform, Common::CTransform _listenerTransform) { 
			
			std::lock_guard<std::mutex> l(mutex);
			ASSERT(_inBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");
			
			if (_inBuffer.size() != 0) {
				distanceAttenuation.Process(_inBuffer, _outBuffer, _sourceTransform, _listenerTransform);				
			}  
		}
	
	
	
	private:
		/// Atributes
		mutable std::mutex mutex;								// Thread management
		Common::CGlobalParameters globalParameters;				// Get access to global render parameters		
		BRTProcessing::CDistanceAttenuator distanceAttenuation; // Distance attenuation processor
		//CWaveGuide waveGuide;		
		//CLongDistanceFilter longDistanceFilter;
		
		bool enableProcessor; // Flag to enable the processor
	};
}
#endif