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

#ifndef _HRTF_CONVOLVER_
#define _HRTF_CONVOLVER_

#include <ProcessingModules/UniformPartitionedConvolution.hpp>
#include <Common/Buffer.hpp>
#include <Common/AddDelayExpansionMethod.hpp>
#include <Common/SourceListenerRelativePositionCalculation.hpp>


#include <memory>
#include <vector>
#include <algorithm>

//#define EPSILON 0.0001f
//#define ELEVATION_SINGULAR_POINT_UP 90.0
//#define ELEVATION_SINGULAR_POINT_DOWN 270.0

namespace BRTProcessing {
	class CHRTFConvolver  {
	public:
		CHRTFConvolver() : enableProcessor{true}, enableInterpolation{true}, enableSpatialization{true}, enableITDSimulation{true}, enableParallaxCorrection{true}, convolutionBuffersInitialized{false} { }

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
		 * @brief Enable spatialization process for this source
		 */
		void EnableSpatialization() { enableSpatialization = true; }
		/**
		 * @brief Disable spatialization process for this source
		 */
		void DisableSpatialization() { enableSpatialization = false; }
		/**
		 * @brief Get the flag to know if spatialization is enabled.
		 * @return true if spatialization is enabled, false otherwise
		 */
		bool IsSpatializationEnabled() { return enableSpatialization; }
		
		/**
		 * @brief Enable HRTF interpolation method
		 */
		void EnableInterpolation() { enableInterpolation = true; }		
		/**
		 * @brief Disable HRTF interpolation method
		 */
		void DisableInterpolation() { enableInterpolation = false; }
		/**
		 * @brief Get the flag to know if HRTF interpolation is enabled.
		 * @return true if HRTF interpolation is enabled, false otherwise
		 */
		bool IsInterpolationEnabled() { return enableInterpolation; }
		
		/**
		 * @brief Enable ITD method
		 */
		void EnableITDSimulation() { enableITDSimulation = true; }
		/**
		 * @brief Disable ITD method
		 */
		void DisableITDSimulation() { enableITDSimulation = false; }		
		/**
		 * @brief Get the flag to know if ITD simulation is enabled.
		 * @return true if ITD is enabled, false otherwise
		 */
		bool IsITDEnabledSimulation() { return enableITDSimulation; }

		/**
		 * @brief Enable Parallax Correction method
		 */
		void EnableParallaxCorrection() { enableParallaxCorrection = true; }
		
		/**
		 * @brief Disable Parallax Correction method
		 */
		void DisableParallaxCorrection() { enableParallaxCorrection = false; }
		
		/**
		 * @brief Get the flag to know if Parallax Correction is enabled.
		 * @return true if Parallax Correction is enabled, false otherwise
		 */
		bool IsParallaxCorrectionEnabled() { return enableParallaxCorrection; }
		
		/** \brief Process data from input buffer to generate spatialization by convolution
		*	\param [in] inBuffer input buffer with anechoic audio
		* *	\param [in] sourceTransform transform of the source
		* *	\param [in] listenerPosition transform of the listener
		* *	\param [in] listenerHRTFWeak weak smart pointer to the listener HRTF
		*	\param [out] outLeftBuffer output mono buffer with spatialized audio for the left channel
		*	\param [out] outRightBuffer output mono buffer with spatialized audio for the right channel
		*   \eh The error handler is informed if the size of the input buffer differs from that stored in the global
		*       parameters and if the HRTF of the listener is null.		   
		*/		
		void Process(CMonoBuffer<float>& _inBuffer, CMonoBuffer<float>& outLeftBuffer, CMonoBuffer<float>& outRightBuffer, Common::CTransform& sourceTransform, Common::CTransform& listenerTransform, std::weak_ptr<BRTServices::CServicesBase>& _listenerHRTFWeak) {
			ASSERT(_inBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");
			
			// Check processor flag
			if (!enableProcessor) { 
				outLeftBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				outRightBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				return; 
			} 

			// Check process flag
			if (!enableSpatialization)
			{
				outLeftBuffer = _inBuffer;
				outRightBuffer = _inBuffer;
				return;
			}

			// Check listener HRTF
			//std::shared_ptr<BRTServices::CHRTF> _listenerHRTF = _listenerHRTFWeak.lock();
			std::shared_ptr<BRTServices::CServicesBase> _listenerHRTF = _listenerHRTFWeak.lock();
			if (!_listenerHRTF) {
				SET_RESULT(RESULT_ERROR_NULLPOINTER, "HRTF listener pointer is null when trying to use in HRTFConvolver");
				outLeftBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				outRightBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				return;
			}
			
			//Check if the source is in the same position as the listener head. If yes, do not apply spatialization
			float distanceToListener = Common::CSourceListenerRelativePositionCalculation::CalculateSourceListenerDistance(sourceTransform, listenerTransform);
			if (distanceToListener <= _listenerHRTF->GetHeadRadius())
			{
				SET_RESULT(RESULT_WARNING, "The source is inside the listener's head.");
				outLeftBuffer = _inBuffer;
				outRightBuffer = _inBuffer;
				return;
			}

			// First time - Initialize convolution buffers
			if (!convolutionBuffersInitialized) { InitializedSourceConvolutionBuffers(_listenerHRTF); }

			// Calculate Source coordinates taking into account Source and Listener transforms
			float leftAzimuth;
			float leftElevation;
			float rightAzimuth;
			float rightElevation;
			float centerAzimuth;
			float centerElevation;
			float interauralAzimuth;

			Common::CSourceListenerRelativePositionCalculation::CalculateSourceListenerRelativePositions(sourceTransform, listenerTransform, _listenerHRTF, enableParallaxCorrection,leftElevation, leftAzimuth, rightElevation, rightAzimuth, centerElevation, centerAzimuth, interauralAzimuth);

			// GET HRTF
			std::vector<CMonoBuffer<float>>  leftHRIR_partitioned;
			std::vector<CMonoBuffer<float>>  rightHRIR_partitioned;

			leftHRIR_partitioned = _listenerHRTF->GetHRIRPartitioned(Common::T_ear::LEFT, leftAzimuth, leftElevation, enableInterpolation, listenerTransform);
			rightHRIR_partitioned = _listenerHRTF->GetHRIRPartitioned(Common::T_ear::RIGHT, rightAzimuth, rightElevation, enableInterpolation, listenerTransform);
						
			// DO CONVOLUTION			
			CMonoBuffer<float> leftChannel_withoutDelay;
			CMonoBuffer<float> rightChannel_withoutDelay;
			//UPC algorithm with memory
			outputLeftUPConvolution.ProcessUPConvolutionWithMemory(_inBuffer, leftHRIR_partitioned, leftChannel_withoutDelay);
			outputRightUPConvolution.ProcessUPConvolutionWithMemory(_inBuffer, rightHRIR_partitioned, rightChannel_withoutDelay);

			// GET DELAY
			uint64_t leftDelay; 				///< Delay, in number of samples
			uint64_t rightDelay;				///< Delay, in number of samples

			if (enableITDSimulation){
				BRTServices::THRIRPartitionedStruct delays = _listenerHRTF->GetHRIRDelay(Common::T_ear::BOTH, centerAzimuth, centerElevation, enableInterpolation, listenerTransform);
				leftDelay = delays.leftDelay;
				rightDelay = delays.rightDelay;
			}
			else {
				leftDelay = 0;
				rightDelay = 0;
			}
			// ADD Delay
			Common::CAddDelayExpansionMethod::ProcessAddDelay_ExpansionMethod(leftChannel_withoutDelay, outLeftBuffer, leftChannelDelayBuffer, leftDelay);
			Common::CAddDelayExpansionMethod::ProcessAddDelay_ExpansionMethod(rightChannel_withoutDelay, outRightBuffer, rightChannelDelayBuffer, rightDelay);			
		}

		/// Reset convolvers and convolution buffers
		void ResetSourceConvolutionBuffers() {
			convolutionBuffersInitialized = false;
			// Reset convolver classes
			outputLeftUPConvolution.Reset();
			outputRightUPConvolution.Reset();
			//Init buffer to store delay to be used in the ProcessAddDelay_ExpansionMethod method
			leftChannelDelayBuffer.clear();
			rightChannelDelayBuffer.clear();
		}
	private:

		// Atributes
		Common::CGlobalParameters globalParameters;

		BRTProcessing::CUniformPartitionedConvolution outputLeftUPConvolution; // Object to make the inverse fft of the left channel with the UPC method
		BRTProcessing::CUniformPartitionedConvolution outputRightUPConvolution; // Object to make the inverse fft of the rigth channel with the UPC method

		CMonoBuffer<float> leftChannelDelayBuffer;			// To store the delay of the left channel of the expansion method
		CMonoBuffer<float> rightChannelDelayBuffer;			// To store the delay of the right channel of the expansion method

		bool enableProcessor;								// Flag to enable the processor
		bool enableSpatialization;							// Flags for independent control of processes
		bool enableInterpolation;							// Enables/Disables the interpolation on run time
		bool enableITDSimulation;							// Enables/Disables the ITD on run time
		bool enableParallaxCorrection;						// Enables/Disables the parallax correction on run time
		bool convolutionBuffersInitialized;					// Flag to check if the convolution buffers have been initialized		

		/////////////////////
		/// PRIVATE Methods        
		/////////////////////

				
		/// Initialize convolvers and convolition buffers		
		void InitializedSourceConvolutionBuffers(std::shared_ptr<BRTServices::CServicesBase>& _listenerHRTF) {

			int numOfSubfilters = _listenerHRTF->GetHRIRNumberOfSubfilters();
			int subfilterLength = _listenerHRTF->GetHRIRSubfilterLength();

			//Common::CGlobalParameters globalParameters;
			outputLeftUPConvolution.Setup(globalParameters.GetBufferSize(), subfilterLength, numOfSubfilters, true);
			outputRightUPConvolution.Setup(globalParameters.GetBufferSize(), subfilterLength, numOfSubfilters, true);
			//Init buffer to store delay to be used in the ProcessAddDelay_ExpansionMethod method
			leftChannelDelayBuffer.clear();
			rightChannelDelayBuffer.clear();

			// Declare variable
			convolutionBuffersInitialized = true;
		}		
	};
}
#endif