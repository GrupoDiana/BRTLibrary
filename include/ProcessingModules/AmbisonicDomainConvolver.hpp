/**
* \class CAmbisonicDomainConvolver
*
* \brief Declaration of CAmbisonicDomainConvolver class interface.
* \date	November 2023
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

#ifndef _AMBISONIC_DOMAIN_CONVOLVER_
#define _AMBISONIC_DOMAIN_CONVOLVER_

#include <ProcessingModules/UniformPartitionedConvolution.hpp>
#include <Common/Buffer.hpp>
#include <ServiceModules/HRTF.hpp>
#include <ServiceModules/AmbisonicBIR.hpp>

#include <memory>
#include <vector>
#include <algorithm>

#define EPSILON 0.0001f
#define ELEVATION_SINGULAR_POINT_UP 90.0
#define ELEVATION_SINGULAR_POINT_DOWN 270.0

namespace BRTProcessing {
	class CAmbisonicDomainConvolver  {
	public:
		CAmbisonicDomainConvolver(Common::T_ear _earToProcess) : enableProcessor{ true }, earToProcess { _earToProcess }, convolutionBuffersInitialized{ false }, numberOfAmbisonicChannels{ 4 } { }


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
		 * @brief Set the ambisonic order to be user
		 * @param _ambisonicOrder 
		*/
		void SetAmbisonicOrder(int _ambisonicOrder) {
			std::lock_guard<std::mutex> l(mutex);
			numberOfAmbisonicChannels = BRTProcessing::CAmbisonicEncoder::CalculateNumberOfChannels(_ambisonicOrder);
			ResetBuffers();
		};
	
		/**
		 * @brief Performs the frequency convolution between the input channels in the ambisonic domain and the IR of the virtual loudspeakers also in the ambisonic domain. The result is in time domain.
		 * @param _inChannelsBuffers Ambisonic input channels 
		 * @param outBuffer Output channel in time in the real domain.
		 * @param _listenerAmbisoninBIRWeak Smart pointer to impulse responses in the ambisonic domain of virtual loudspeakers.
		*/
		void Process(std::vector<CMonoBuffer<float>>& _inChannelsBuffers, CMonoBuffer<float>& outBuffer, std::weak_ptr<BRTServices::CAmbisonicBIR>& _listenerAmbisoninBIRWeak, Common::CTransform& _listenerTransform) {

			std::lock_guard<std::mutex> l(mutex);
			
			// Check if the processor is enabled
			if (!enableProcessor) {
				outBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				return;
			}
						
			// Check the number of channels
			if (_inChannelsBuffers.size() != numberOfAmbisonicChannels) {
				SET_RESULT(RESULT_ERROR_BADSIZE, "InChannlesBuffers size has to be equal to the number of channels set. This usually occurs because the ambisonic order has been changed during reproduction.");
				outBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				return;
			}

			// Check listener Ambisonic BIR pointer
			std::shared_ptr<BRTServices::CAmbisonicBIR> _listenerAmbisonicBIR = _listenerAmbisoninBIRWeak.lock();
			if (!_listenerAmbisonicBIR) {
				SET_RESULT(RESULT_ERROR_NULLPOINTER, "AmbisonicBIR pointer is null when trying to use in AmbisonicDomainConvolver.");				
				outBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);				
				return;
			}
			
			// Check listener Ambisonic BIR data ready
			if (!_listenerAmbisonicBIR->IsReady()) {
				SET_RESULT(RESULT_WARNING, "AmbisonicBIR is not ready to provide IRs. This usually occurs because the ambisonic order has been changed during reproduction.");
				outBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				return;
			}

			// First time - Initialize convolution buffers
			if (!convolutionBuffersInitialized) { InitializedSourceConvolutionBuffers(_listenerAmbisonicBIR); }
			
			// Process
			std::vector<CMonoBuffer<float>> allChannelsBuffersConvolved (numberOfAmbisonicChannels);
			for (int nChannel = 0; nChannel < _inChannelsBuffers.size(); nChannel++) {				
				
				std::vector<CMonoBuffer<float>>	oneChannel_ABIR_partitioned = _listenerAmbisonicBIR->GetChannelPartitionedIR_OneEar(nChannel, earToProcess, _listenerTransform); // GET ABIR								
				if (oneChannel_ABIR_partitioned.size() == 0) {
					SET_RESULT(RESULT_ERROR_BADSIZE, "Failure to obtain an IR from AmbisonicIR. This usually occurs because the ambisonic order has been changed during reproduction.");
					outBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
					return;
				}
				channelsUPConvolutionVector[nChannel]->ProcessUPConvolutionWithMemory(_inChannelsBuffers[nChannel], oneChannel_ABIR_partitioned, allChannelsBuffersConvolved[nChannel], false);
			}
			// Mixer
			CMonoBuffer<float> mixedChannels;
			mixedChannels.SetFromMix({ allChannelsBuffersConvolved });			
			mixedChannels.ApplyGain(1.0f / numberOfAmbisonicChannels);
			// InverseFFT
			BRTProcessing::CUniformPartitionedConvolution::CalculateIFFT(mixedChannels, outBuffer);			
		}
		
		/**
		 * @brief Reset convolvers and convolution buffers
		*/
		void ResetChannelsConvolutionBuffers() {				
			std::lock_guard<std::mutex> l(mutex);
			ResetBuffers();
		}

	private:

		mutable std::mutex mutex;

		// Atributes
		Common::CGlobalParameters globalParameters;		
		std::vector<std::shared_ptr<BRTProcessing::CUniformPartitionedConvolution>> channelsUPConvolutionVector; // Object to make the inverse fft of the left channel with the UPC method				
		
		Common::T_ear earToProcess;							// Ear to process
		int numberOfAmbisonicChannels;						// Number of ambisonic channels
		bool convolutionBuffersInitialized;					// Flag to check if the convolution buffers are initialized
		bool enableProcessor;								// Flag to enable the processor


		/////////////////////
		/// PRIVATE Methods        
		/////////////////////

		/// Initialize convolvers and convolition buffers		
		void InitializedSourceConvolutionBuffers(std::shared_ptr<BRTServices::CAmbisonicBIR>& _listenerAmbisonicBIR) {

			int numOfSubfilters = _listenerAmbisonicBIR->GetIRNumberOfSubfilters();
			int subfilterLength = _listenerAmbisonicBIR->GetIRSubfilterLength();
			
			for (int nChannel = 0; nChannel < numberOfAmbisonicChannels; nChannel++) {								
				std::shared_ptr<BRTProcessing::CUniformPartitionedConvolution> channelUPConvolver = std::make_shared<BRTProcessing::CUniformPartitionedConvolution>();				
				channelUPConvolver->Setup(globalParameters.GetBufferSize(), subfilterLength, numOfSubfilters, true);
				channelsUPConvolutionVector.push_back(channelUPConvolver);				
			}			
			// Declare variable
			convolutionBuffersInitialized = true;
		}
		
		/// Reset convolution buffers
		void ResetBuffers() {			
			convolutionBuffersInitialized = false;
			channelsUPConvolutionVector.clear();
		}

	};
}
#endif