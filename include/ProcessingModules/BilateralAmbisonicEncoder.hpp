/**
* \class CHRTFConvolver
*
* \brief Declaration of CHRTFConvolver class interface.
* \date	June 2023
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

#ifndef _CBILATERAL_AMBISONIC_ENCODER_HPP
#define _CBILATERAL_AMBISONIC_ENCODER_HPP

#include <ProcessingModules/UniformPartitionedConvolution.hpp>
#include <Common/Buffer.hpp>
#include <ProcessingModules/AmbisonicEncoder.hpp>
#include <Common/AddDelayExpansionMethod.hpp>
#include <Common/SourceListenerRelativePositionCalculation.hpp>
#include <ProcessingModules/BinauralFilter.hpp>
#include <ServiceModules/HRTF.hpp>

#include <memory>
#include <vector>
#include <algorithm>

namespace BRTProcessing {
	class CBilateralAmbisonicEncoder {
	public:
		CBilateralAmbisonicEncoder() 
			: enableProcessor{ true }
			, ambisonicOrder { 1 }
			, ambisonicNormalization { BRTProcessing::TAmbisonicNormalization::N3D }
			, enableInterpolation{ true }
			, enableITDSimulation{ false }
			, enableParallaxCorrection{ true } {
					
			ambisonicEncoder.Setup(ambisonicOrder, ambisonicNormalization);
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
		 * @brief 
		 * @param _ambisonicOrder 
		 */
		void SetAmbisonicOrder(int _ambisonicOrder) { 
			std::lock_guard<std::mutex> l(mutex);

			if (ambisonicOrder == _ambisonicOrder) { return; }
			ambisonicOrder = _ambisonicOrder; 
			//leftAmbisonicEncoder.Setup(ambisonicOrder, ambisonicNormalization);
			//rightAmbisonicEncoder.Setup(ambisonicOrder, ambisonicNormalization);
			ambisonicEncoder.Setup(ambisonicOrder, ambisonicNormalization);
		}

		int GetAmbisonicOrder() {	return ambisonicOrder; }
		
		void SetAmbisonicNormalization(BRTProcessing::TAmbisonicNormalization _ambisonicNormalization) {
			std::lock_guard<std::mutex> l(mutex);
			
			if (ambisonicNormalization == _ambisonicNormalization) { return; }
			ambisonicNormalization = _ambisonicNormalization;
			//leftAmbisonicEncoder.Setup(ambisonicOrder, ambisonicNormalization);
			//rightAmbisonicEncoder.Setup(ambisonicOrder, ambisonicNormalization);
			ambisonicEncoder.Setup(ambisonicOrder, ambisonicNormalization);
		}


		///Enable ITD simulation for this source
		void EnableITDSimulation() { enableITDSimulation = true; };
		///Disable ITD simulation for this source
		void DisableITDSimulation() { enableITDSimulation = false; };
		///Get the flag for ITD simulation enabling
		bool IsITDSimulationEnabled() { return enableITDSimulation; };

		///Enable near field effect for this source
		void EnableNearFieldEffect() { nearFieldEffectProcess.EnableProcessor(); };
		///Disable near field effect for this source
		void DisableNearFieldEffect() { nearFieldEffectProcess.DisableProcessor(); };
		///Get the flag for near field effect enabling
		bool IsNearFieldEffectEnabled() { return nearFieldEffectProcess.IsProcessorEnabled(); };

		void EnableParallaxCorrection() { enableParallaxCorrection = true; };
		void DisableParallaxCorrection() { enableParallaxCorrection = false; };
		bool IsParallaxCorrectionEnabled() { return enableParallaxCorrection; };
				
		/**
		 * @brief Process the input buffer data to generate the bilaterar ambisonic channels for spatialisation using the virtual ambisonic method.
		 * @param _inBuffer Input buffer with audio samples
		 * @param leftChannelsBuffers Left ear ambisonic channels
		 * @param rightChannelsBuffers Right ear ambisonic channels
		 * @param sourceTransform Transform of the source
		 * @param listenerTransform Transform of the listener
		 * @param _listenerHRTFWeak Weak smart pointer to the listener HRTF
		 * @param _listenerILDWeak Weak smart pointer to the listener ILD
		*/
		void Process(CMonoBuffer<float>& _inBuffer, std::vector<CMonoBuffer<float>>& leftChannelsBuffers, std::vector<CMonoBuffer<float>>& rightChannelsBuffers, Common::CTransform& sourceTransform, Common::CTransform& listenerTransform, std::weak_ptr<BRTServices::CServicesBase>& _listenerHRTFWeak, std::weak_ptr<BRTServices::CSOSFilters>& _listenerILDWeak) {

			std::lock_guard<std::mutex> l(mutex);
			
			ASSERT(_inBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");		
			
			ambisonicEncoder.InitAmbisonicChannels(leftChannelsBuffers, globalParameters.GetBufferSize());
			ambisonicEncoder.InitAmbisonicChannels(rightChannelsBuffers, globalParameters.GetBufferSize());

			// Check if the processor is enabled
			if (!enableProcessor) {
				ambisonicEncoder.EncodedIR(CMonoBuffer<float>(globalParameters.GetBufferSize(), 0.0f), leftChannelsBuffers, 0, 0);
				ambisonicEncoder.EncodedIR(CMonoBuffer<float>(globalParameters.GetBufferSize(), 0.0f), rightChannelsBuffers, 0, 0);
				return;
			}

			// Check listener HRTF
			std::shared_ptr<BRTServices::CServicesBase> _listenerHRTF = _listenerHRTFWeak.lock();
			if (!_listenerHRTF) {
				SET_RESULT(RESULT_ERROR_NULLPOINTER, "HRTF listener pointer is null when trying to use in Bilateral Ambisonic Encoder");				
				return;
			}
			
			// TODO 
			//Check if the source is in the same position as the listener head. If yes, do not apply spatialization
			float distanceToListener = Common::CSourceListenerRelativePositionCalculation::CalculateSourceListenerDistance(sourceTransform, listenerTransform);
			if (distanceToListener <= _listenerHRTF->GetHeadRadius())
			{
				SET_RESULT(RESULT_WARNING, "The source is inside the listener's head.");
				
				ambisonicEncoder.EncodedIR(CMonoBuffer<float>(globalParameters.GetBufferSize(), 0.0f), leftChannelsBuffers, 0, 0);
				ambisonicEncoder.EncodedIR(CMonoBuffer<float>(globalParameters.GetBufferSize(), 0.0f), rightChannelsBuffers, 0, 0);				
				return;
			}
						
			// Calculate Source coordinates taking into account Source and Listener transforms
			float leftAzimuth;
			float leftElevation;
			float rightAzimuth;
			float rightElevation;
			float centerAzimuth;
			float centerElevation;
			float interauralAzimuth;

			Common::CSourceListenerRelativePositionCalculation::CalculateSourceListenerRelativePositions(sourceTransform, listenerTransform, _listenerHRTF, enableParallaxCorrection, leftElevation, leftAzimuth, rightElevation, rightAzimuth, centerElevation, centerAzimuth, interauralAzimuth);
			
			// GET DELAY
			uint64_t leftDelay; 				///< Delay, in number of samples
			uint64_t rightDelay;				///< Delay, in number of samples
			if (enableITDSimulation) {
				BRTServices::THRIRPartitionedStruct delays = _listenerHRTF->GetHRIRDelay(Common::T_ear::BOTH, centerAzimuth, centerElevation, enableInterpolation, listenerTransform);
				leftDelay = delays.leftDelay;
				rightDelay = delays.rightDelay;
			}
			else {
				leftDelay = 0;
				rightDelay = 0;
			}
			// ADD Delay
			CMonoBuffer<float> delayedLeftEarBuffer;
			CMonoBuffer<float> delayedRightEarBuffer;			
			Common::CAddDelayExpansionMethod::ProcessAddDelay_ExpansionMethod(_inBuffer, delayedLeftEarBuffer, leftChannelDelayBuffer, leftDelay);
			Common::CAddDelayExpansionMethod::ProcessAddDelay_ExpansionMethod(_inBuffer, delayedRightEarBuffer, rightChannelDelayBuffer, rightDelay);
			
			// Near Field Proccess
			CMonoBuffer<float> nearFilteredLeftEarBuffer;
			CMonoBuffer<float> nearFilteredRightEarBuffer;			
			nearFieldEffectProcess.Process(delayedLeftEarBuffer, delayedRightEarBuffer, nearFilteredLeftEarBuffer, nearFilteredRightEarBuffer, sourceTransform, listenerTransform, _listenerILDWeak);

			// Ambisonic Encoder						
			ambisonicEncoder.EncodedIR(nearFilteredLeftEarBuffer, leftChannelsBuffers, leftAzimuth, leftElevation);
			ambisonicEncoder.EncodedIR(nearFilteredRightEarBuffer, rightChannelsBuffers, rightAzimuth, rightElevation);
		}

		/// Reset convolvers and convolution buffers
		void ResetBuffers() {
			std::lock_guard<std::mutex> l(mutex);
			//Init buffer to store delay to be used in the ProcessAddDelay_ExpansionMethod method
			leftChannelDelayBuffer.clear();
			rightChannelDelayBuffer.clear();
			nearFieldEffectProcess.ResetProcessBuffers();
		}

	private:
		/// Atributes
		mutable std::mutex mutex;								// Thread management
		Common::CGlobalParameters globalParameters;				// Get access to global render parameters
		BRTProcessing::CBinauralFilter nearFieldEffectProcess; // NearField effect processor instance
		//Common::CAmbisonicEncoder leftAmbisonicEncoder;			// Left ear encoder
		//Common::CAmbisonicEncoder rightAmbisonicEncoder;		// Right ear enconder
		BRTProcessing::CAmbisonicEncoder ambisonicEncoder; // Ambisonic encoder

		CMonoBuffer<float> leftChannelDelayBuffer;				// To store the delay of the left channel of the expansion method
		CMonoBuffer<float> rightChannelDelayBuffer;				// To store the delay of the right channel of the expansion method
		int ambisonicOrder;
		BRTProcessing::TAmbisonicNormalization ambisonicNormalization;
		
		bool enableProcessor;								// Flag to enable the processor
		bool enableInterpolation;							// Enables/Disables the interpolation
		bool enableITDSimulation;							// Enables/Disables the ITD simulation
		bool enableParallaxCorrection;						// Enables/Disables the parallax correction
		
		/// PRIVATE Methods        				
		/// Initialize convolvers and convolition buffers		
		void InitializedSourceConvolutionBuffers(std::shared_ptr<BRTServices::CHRTF>& _listenerHRTF) {		
			leftChannelDelayBuffer.clear();
			rightChannelDelayBuffer.clear();
		}		
	};
}
#endif