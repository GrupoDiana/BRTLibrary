/**
* \class CSOSFilter
*
* \brief Declaration of CSOSFilter class. It implements binaural filtering from second-order stages.
* \date	Nov 2025
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco, F. Morales-Benitez ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga)||
* \b Contact: areyes@uma.es
*
* \b Copyright: University of Malaga
* 
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: 3D Tune-In (https://www.3dtunein.eu) and SONICOM (https://www.sonicom.eu/) ||
*
* \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreements no. 644051 and no. 101017743
* 
* This class is part of the Binaural Rendering Toolbox (BRT), coordinated by A. Reyes-Lecuona (areyes@uma.es) and L. Picinali (l.picinali@imperial.ac.uk)
* Code based in the 3DTI Toolkit library (https://github.com/3DTune-In/3dti_AudioToolkit).
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/
#ifndef _SOS_FILTER_HPP
#define _SOS_FILTER_HPP

#define EPSILON 0.001f


#include <Common/Buffer.hpp>
#include <Common/GlobalParameters.hpp>
#include <Filters/FilterBase.hpp>
#include <ProcessingModules/MultichannelBiquadFilterChain.hpp>



#define NUMBER_OF_COEFFICIENTS_IN_BIQUAD_SECTION 6

namespace BRTFilters {

	class CSOSFilter : public CFilterBase, private BRTProcessing::CMultichannelBiquadFilterChain {
	public:
		CSOSFilter()
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
		 * @brief Filter the input signal with the binaural filter
		 * @param _inBuffer input buffer
		 * @param outBuffer output buffer
		 * @param _channel 
		 */
		void Process(const CMonoBuffer<float> & _inBuffer, CMonoBuffer<float> & outBuffer, const int & _channel) override
		{
			BRTProcessing::CMultichannelBiquadFilterChain::Process(_channel, _inBuffer, outBuffer);						
		}

		/**
		 * @brief Reset the buffers of the process
		 */
		void ResetBuffers() {
			BRTProcessing::CMultichannelBiquadFilterChain::ResetBuffers();			
		}
	
	private:

	};
}
#endif