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
* \b Contributions: (additional authors/contributors can be added here) 
*
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Copyright: University of Malaga 2023. Code based in the 3DTI Toolkit library (https://github.com/3DTune-In/3dti_AudioToolkit) with Copyright University of Malaga and Imperial College London - 2018
*
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/

#ifndef _CBILATERAL_AMBISONIC_ENCODER_HPP
#define _CBILATERAL_AMBISONIC_ENCODER_HPP

#include <Common/UPCAnechoic.hpp>
#include <Common/Buffer.hpp>
#include <Common/AmbisonicEncoder.hpp>
#include <Common/AddDelayExpansionMethod.hpp>
#include <Common/SourceListenerRelativePositionCalculation.hpp>
#include <ServiceModules/HRTF.hpp>
#include <ProcessingModules/NearFieldEffect.hpp>

#include <memory>
#include <vector>
#include <algorithm>

//#define EPSILON 0.0001f
//#define ELEVATION_SINGULAR_POINT_UP 90.0
//#define ELEVATION_SINGULAR_POINT_DOWN 270.0

namespace BRTProcessing {
	class CBilateralAmbisonicEncoder {
	public:
		CBilateralAmbisonicEncoder() : ambisonicOrder{ 1 }, ambisonicNormalization { Common::TAmbisonicNormalization::N3D}, enableInterpolation{true} {
		
			//leftAmbisonicEncoder.Setup(ambisonicOrder, ambisonicNormalization);
			//rightAmbisonicEncoder.Setup(ambisonicOrder, ambisonicNormalization);
			ambisonicEncoder.Setup(ambisonicOrder, ambisonicNormalization);
		}


		/*///Enable spatialization process for this source	
		void EnableSpatialization() { enableSpatialization = true; }
		///Disable spatialization process for this source	
		void DisableSpatialization() { enableSpatialization = false; }
		///Get the flag for spatialization process enabling
		bool IsSpatializationEnabled() { return enableSpatialization; }*/
		
		///Enable HRTF interpolation method	
		/*void EnableInterpolation() { enableInterpolation = true; }
		///Disable HRTF interpolation method
		void DisableInterpolation() { enableInterpolation = false; }
		///Get the flag for HRTF interpolation method
		bool IsInterpolationEnabled() { return enableInterpolation; }*/

		void SetAmbisonicOrder(int _ambisonicOrder) { 
			std::lock_guard<std::mutex> l(mutex);
			ambisonicOrder = _ambisonicOrder; 
			//leftAmbisonicEncoder.Setup(ambisonicOrder, ambisonicNormalization);
			//rightAmbisonicEncoder.Setup(ambisonicOrder, ambisonicNormalization);
			ambisonicEncoder.Setup(ambisonicOrder, ambisonicNormalization);
		}

		int GetAmbisonicOrder() {	return ambisonicOrder; }
		
		void SetAmbisonicNormalization(Common::TAmbisonicNormalization _ambisonicNormalization) {
			std::lock_guard<std::mutex> l(mutex);
			ambisonicNormalization = _ambisonicNormalization;
			//leftAmbisonicEncoder.Setup(ambisonicOrder, ambisonicNormalization);
			//rightAmbisonicEncoder.Setup(ambisonicOrder, ambisonicNormalization);
			ambisonicEncoder.Setup(ambisonicOrder, ambisonicNormalization);
		}


		///Enable near field effect for this source
		void EnableNearFieldEffect() { nearFieldEffectProcess.EnableNearFieldEffect(); };
		///Disable near field effect for this source
		void DisableNearFieldEffect() { nearFieldEffectProcess.DisableNearFieldEffect(); };
		///Get the flag for near field effect enabling
		bool IsNearFieldEffectEnabled() { return nearFieldEffectProcess.IsNearFieldEffectEnabled(); };

				
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
		void Process(CMonoBuffer<float>& _inBuffer, std::vector<CMonoBuffer<float>>& leftChannelsBuffers, std::vector<CMonoBuffer<float>>& rightChannelsBuffers, Common::CTransform& sourceTransform, Common::CTransform& listenerTransform, std::weak_ptr<BRTServices::CHRTF>& _listenerHRTFWeak, std::weak_ptr<BRTServices::CILD>& _listenerILDWeak) {

			std::lock_guard<std::mutex> l(mutex);

			//if (!enableSpatialization) { return; }
			ASSERT(_inBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");
		
			//leftAmbisonicEncoder.InitAmbisonicChannels(leftChannelsBuffers, globalParameters.GetBufferSize());
			//rightAmbisonicEncoder.InitAmbisonicChannels(rightChannelsBuffers, globalParameters.GetBufferSize());
			ambisonicEncoder.InitAmbisonicChannels(leftChannelsBuffers, globalParameters.GetBufferSize());
			ambisonicEncoder.InitAmbisonicChannels(rightChannelsBuffers, globalParameters.GetBufferSize());

			//// Check process flag
			//if (!enableSpatialization)
			//{
			//	outLeftBuffer = _inBuffer;
			//	outRightBuffer = _inBuffer;
			//	return;
			//}

			// Check listener HRTF
			std::shared_ptr<BRTServices::CHRTF> _listenerHRTF = _listenerHRTFWeak.lock();
			if (!_listenerHRTF) {
				SET_RESULT(RESULT_ERROR_NULLPOINTER, "HRTF listener pointer is null when trying to use in Bilateral Ambisonic Encoder");
				///leftChannelsBuffers = std::vector<CMonoBuffer<float>>(leftAmbisonicEncoder.GetTotalChannels(), CMonoBuffer<float>(globalParameters.GetBufferSize(), 0.0f));
				//rightChannelsBuffers = std::vector<CMonoBuffer<float>>(rightAmbisonicEncoder.GetTotalChannels(), CMonoBuffer<float>(globalParameters.GetBufferSize(), 0.0f));				
				return;
			}
			
			// TODO 
			//Check if the source is in the same position as the listener head. If yes, do not apply spatialization
			float distanceToListener = Common::CSourceListenerRelativePositionCalculation::CalculateSourceListenerDistance(sourceTransform, listenerTransform);
			if (distanceToListener <= _listenerHRTF->GetHeadRadius())
			{
				SET_RESULT(RESULT_WARNING, "The source is inside the listener's head.");
				// Ambisonic enoder just in front
				//leftAmbisonicEncoder.EncodedIR(CMonoBuffer<float>(globalParameters.GetBufferSize(), 0.0f), leftChannelsBuffers, 0, 0);
				//rightAmbisonicEncoder.EncodedIR(CMonoBuffer<float>(globalParameters.GetBufferSize(), 0.0f), rightChannelsBuffers, 0, 0);

				ambisonicEncoder.EncodedIR(CMonoBuffer<float>(globalParameters.GetBufferSize(), 0.0f), leftChannelsBuffers, 0, 0);
				ambisonicEncoder.EncodedIR(CMonoBuffer<float>(globalParameters.GetBufferSize(), 0.0f), rightChannelsBuffers, 0, 0);

				//leftAmbisonicEncoder.EncodedIR(_inBuffer, leftChannelsBuffers, 90, 0);
				//rightAmbisonicEncoder.EncodedIR(_inBuffer, rightChannelsBuffers, -90, 0);

				return;
			}

			// First time - Initialize convolution buffers
			//if (!convolutionBuffersInitialized) { InitializedSourceConvolutionBuffers(_listenerHRTF); }

			// Calculate Source coordinates taking into account Source and Listener transforms
			float leftAzimuth;
			float leftElevation;
			float rightAzimuth;
			float rightElevation;
			float centerAzimuth;
			float centerElevation;
			float interauralAzimuth;

			Common::CSourceListenerRelativePositionCalculation::CalculateSourceListenerRelativePositions(sourceTransform, listenerTransform, _listenerHRTF, leftElevation, leftAzimuth, rightElevation, rightAzimuth, centerElevation, centerAzimuth, interauralAzimuth);
			
			// GET DELAY
			uint64_t leftDelay; 				///< Delay, in number of samples
			uint64_t rightDelay;				///< Delay, in number of samples

			BRTServices::THRIRPartitionedStruct delays = _listenerHRTF->GetHRIRDelay(Common::T_ear::BOTH, centerAzimuth, centerElevation, enableInterpolation);
			leftDelay	= delays.leftDelay;
			rightDelay	= delays.rightDelay;
			
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
			//leftAmbisonicEncoder.EncodedIR(nearFilteredLeftEarBuffer, leftChannelsBuffers, leftAzimuth, leftElevation);
			//rightAmbisonicEncoder.EncodedIR(nearFilteredRightEarBuffer, rightChannelsBuffers, rightAzimuth, rightElevation);
			ambisonicEncoder.EncodedIR(nearFilteredLeftEarBuffer, leftChannelsBuffers, leftAzimuth, leftElevation);
			ambisonicEncoder.EncodedIR(nearFilteredRightEarBuffer, rightChannelsBuffers, rightAzimuth, rightElevation);
		}

		/// Reset convolvers and convolution buffers
		void ResetBuffers() {
			std::lock_guard<std::mutex> l(mutex);
			//Init buffer to store delay to be used in the ProcessAddDelay_ExpansionMethod method
			leftChannelDelayBuffer.clear();
			rightChannelDelayBuffer.clear();
		}
	private:
		/// Atributes
		mutable std::mutex mutex;								// Thread management
		Common::CGlobalParameters globalParameters;				// Get access to global render parameters
		CNearFieldEffect nearFieldEffectProcess;				// NearField effect processor instance
		//Common::CAmbisonicEncoder leftAmbisonicEncoder;			// Left ear encoder
		//Common::CAmbisonicEncoder rightAmbisonicEncoder;		// Right ear enconder
		Common::CAmbisonicEncoder ambisonicEncoder;				// Ambisonic encoder

		CMonoBuffer<float> leftChannelDelayBuffer;				// To store the delay of the left channel of the expansion method
		CMonoBuffer<float> rightChannelDelayBuffer;				// To store the delay of the right channel of the expansion method
		int ambisonicOrder;
		Common::TAmbisonicNormalization ambisonicNormalization;
		
		bool enableInterpolation;								// Enables/Disables the interpolation
		
				
		
		/// PRIVATE Methods        				
		/// Initialize convolvers and convolition buffers		
		void InitializedSourceConvolutionBuffers(std::shared_ptr<BRTServices::CHRTF>& _listenerHRTF) {		
			leftChannelDelayBuffer.clear();
			rightChannelDelayBuffer.clear();
		}		
	};
}
#endif