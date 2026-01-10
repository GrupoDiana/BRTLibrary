/**
* \class CMultichannelBiquadFilterChain
*
* \brief Declaration of CMultichannelBiquadFilterChain class
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

#ifndef _MULTICHANNEL_BIQUAD_FILTER_CHAIN_HPP
#define _MULTICHANNEL_BIQUAD_FILTER_CHAIN_HPP

#define EPSILON 0.001f


#include <Common/Buffer.hpp>
#include <Common/GlobalParameters.hpp>
#include <ProcessingModules/BiquadFilterChain.hpp>


#define NUMBER_OF_COEFFICIENTS_IN_BIQUAD_SECTION 6

namespace BRTProcessing {

	class CMultichannelBiquadFilterChain {
	public:
		CMultichannelBiquadFilterChain()			
			: initialized { false }
			, enableProcessor { true }
			, numberOfChannels { 0 }
			, numberOfBiquadSectionsPerChannel { 0 }
			, numberOfCoefficientsPerChannel { 0 }
			, biquadChainTable {}
		{
					
		}

		/**
		 * @brief Setup the multichannel biquad filter chain according to the number of channels and biquad sections per channel
		 * @param _numberOfChannels Number of channels
		 * @param _numberOfBiquadSectionsPerChannel Number of biquad sections per channel
		 * @return true if the setup was successful, false otherwise
		 */
		bool Setup(int _numberOfChannels, int _numberOfBiquadSectionsPerChannel) { 
			if (initialized) return false;
			
			if (_numberOfChannels < 1) {
				SET_RESULT(RESULT_ERROR_BADSIZE, "The number of channels has to be greater than 0 in BRTProcessing::CMultichanneBiquadFilterChain");
				return false;
			}

			if (_numberOfBiquadSectionsPerChannel < 1) {
				SET_RESULT(RESULT_ERROR_BADSIZE, "The number of filter stages has to be greater than 0 in BRTProcessing::CMultichanneBiquadFilterChain");
				return false;
			}

			numberOfChannels = _numberOfChannels;
			numberOfBiquadSectionsPerChannel = _numberOfBiquadSectionsPerChannel;
						
			biquadChainTable = std::vector<BRTProcessing::CBiquadFilterChain>(numberOfChannels);
			for (int i = 0; i < numberOfChannels; i++) {
				biquadChainTable[i].AddNFilters(numberOfBiquadSectionsPerChannel);
			}

			numberOfCoefficientsPerChannel = numberOfBiquadSectionsPerChannel * NUMBER_OF_COEFFICIENTS_IN_BIQUAD_SECTION;
			initialized = true;
			return true;
		}
		
		/**
		 * @brief Store the coefficients of the filter for a given channel
		 * @param _channel channel number
		 * @param _coefficients vector of coefficients for the channel
		 */
		bool SetCoefficients(const int & _channel, const std::vector<float>& _coefficients)  {
			if (!initialized) return false;			

			if (_channel < 0 || _channel >= numberOfChannels) {
				SET_RESULT(RESULT_ERROR_OUTOFRANGE, "The channel number is out of range in BRTProcessing::CMultichanneBiquadFilterChain");
				return false;
			}

			if (_coefficients.size() != numberOfCoefficientsPerChannel) {
				SET_RESULT(RESULT_ERROR_BADSIZE, "The number of coefficients is not correct in BRTProcessing::CMultichanneBiquadFilterChain");
				return false;
			}
			SetCoefficients(biquadChainTable[_channel], _coefficients); //Set  coefficients		
			return true;
		}
		
		/**
		 * @brief Enable processor
		 */
		void EnableProcessor() { enableProcessor = true; }
		/**
		 * @brief Disable processor
		 */
		void DisableProcessor() { enableProcessor = false; }
		/**
		 * @brief Get the flag to know if the processor is enabled.
		 * @return true if the processor is enabled, false otherwise
		 */
		bool IsProcessorEnabled() { return enableProcessor; }				

		/**
		 * @brief Filter the input signal with the binaural filter
		 * @param _inLeftBuffer left ear input buffer
		 * @param _inRightBuffer right ear input buffer
		 * @param outLeftBuffer out left ear buffer
		 * @param outRightBuffer out right ear buffer
		 */
		void Process(const int& _channel, const CMonoBuffer<float> & _inBuffer, CMonoBuffer<float> & outBuffer) 
		{
			outBuffer = _inBuffer;
			
			if (!initialized) return;						
			if (!enableProcessor) return;			
						
			if (_channel < 0 || _channel >= numberOfChannels) {
				SET_RESULT(RESULT_ERROR_OUTOFRANGE, "The channel number is out of range in BRTProcessing::CMultichanneBiquadFilterChain::Process");
				return;
			}

			ASSERT(_inBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");
										
			biquadChainTable[_channel].Process(outBuffer);			
		}

		/**
		 * @brief Reset the buffers of the process
		 */
		void ResetBuffers() {
			for (auto& biquadChain : biquadChainTable) {
				biquadChain.ResetBuffers();
			}			
		}

	
	private:
		///////////////////////
		// Private Methods
		///////////////////////		
				
		/**
		 * @brief Set the coefficients of the filter
		 * @param _filter filter to set the coefficients
		 * @param cofficients vector of coefficients
		 */	
		void SetCoefficients(BRTProcessing::CBiquadFilterChain & _filter, const std::vector<float> & cofficients) {

			BRTProcessing::TFiltersChainCoefficients filterCoeficientsVector;
			for (int i = 0; i < numberOfCoefficientsPerChannel; i += NUMBER_OF_COEFFICIENTS_IN_BIQUAD_SECTION) {
				std::vector<float> stage(cofficients.begin() + i, cofficients.begin() + i + NUMBER_OF_COEFFICIENTS_IN_BIQUAD_SECTION);
				filterCoeficientsVector.push_back(stage);
			}
			_filter.SetFromCoefficientsVector(filterCoeficientsVector);
		}

		///////////////////////
		// Private Attributes
		///////////////////////		
		Common::CGlobalParameters globalParameters;		
		std::vector<BRTProcessing::CBiquadFilterChain> biquadChainTable;
		
		bool enableProcessor;				// Flag to enable the processor		
		bool initialized;
		int numberOfChannels;
		int numberOfBiquadSectionsPerChannel;
		int numberOfCoefficientsPerChannel;
	};
}
#endif