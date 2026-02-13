/**
* \class CFIRConvolver
*
* \brief Declaration of CFIRConvolver class
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


#ifndef _FIR_CONVOLVER_
#define _FIR_CONVOLVER_

#include <ProcessingModules/UniformPartitionedConvolution.hpp>
#include <Common/Buffer.hpp>
#include <Common/AddDelayExpansionMethod.hpp>
#include <Common/SourceListenerRelativePositionCalculation.hpp>


#include <memory>
#include <vector>
#include <algorithm>

namespace BRTProcessing {
	class CFIRConvolver  {
	public:
		CFIRConvolver() 
			: initialized { false } 
			, enableProcessor{ false }
			//, enableInterpolation{true}
			//, enableSpatialization{true}
			//, enableITDSimulation{true}
			//, enableParallaxCorrection{true}
			, enableFindNearestIR { true }
			, convolutionBuffersInitialized{false} { }


		/**
		 * @brief Initialize the FIR convolver according to the number of channels
		 * @param _numberOfChannels number of channels
		 * @return true if the setup was successful, false otherwise
		 */
		bool Setup(int _numberOfChannels) { 
			std::lock_guard<std::mutex> l(mutex);
			if (initialized) { 
				ResetConvolutionBuffers();
				channelsConvolvers.clear();
			}
						
			channelsConvolvers.resize(_numberOfChannels);
			for (int i = 0; i < _numberOfChannels; i++) {
				channelsConvolvers.push_back(BRTProcessing::CUniformPartitionedConvolution());
			}
			initialized = true;
			return true; 
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
		 * @brief Enable Find Nearest IR method
		 */
		void EnableFindNearestIR() {
			std::lock_guard<std::mutex> l(mutex);
			enableFindNearestIR = true;
		}
		/**
		* @brief Disable Find Nearest IR method
		*/
		void DisableFindNearestIR() {
			std::lock_guard<std::mutex> l(mutex);
			enableFindNearestIR = false;
		}
		/**
		 * @brief Get the flag to know if Find Nearest IR is enabled.
		 * @return true if Find Nearest IR is enabled, false otherwise
		 */
		bool IsFindNearestIREnabled() { return enableFindNearestIR; }


		void Process(const CMonoBuffer<float> & _inBuffer, CMonoBuffer<float> & _outBuffer, const int & channel, std::weak_ptr<BRTServices::CServicesBase> _irTableWeakPtr) {
			_outBuffer = _inBuffer;
			
			std::lock_guard<std::mutex> l(mutex);
			ASSERT(_inBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");
			
			if (!enableProcessor || !initialized) {				
				return;
			}

			// Check IR table
			std::shared_ptr<BRTServices::CServicesBase> _irTablePtr = _irTableWeakPtr.lock();
			if (!_irTablePtr) {
				SET_RESULT(RESULT_ERROR_NULLPOINTER, "FIR Table pointer is null when trying to use in FIRConvolver::Process");
				return;
			}

			// First time - Initialize convolution buffers
			if (!convolutionBuffersInitialized) {
				InitializedSourceConvolutionBuffers(_irTablePtr);
			}

			std::vector<CMonoBuffer<float>> _IR_partitioned;			
			if (_irTablePtr->IsSpatiallyOriented()) {
				_IR_partitioned = _irTablePtr->GetIRTFPartitionedSpatiallyOriented(0.0f, 0.0f, Common::T_ear(channel), enableFindNearestIR);
			} else {
				_IR_partitioned = _irTablePtr->GetIRTFPartitioned(Common::T_ear(channel));
			}
			if (_IR_partitioned.size() == 0) {
				SET_RESULT(RESULT_ERROR_BADSIZE, "FIRConvolver::Process: No IR partitions found in FIR table for the requested ear", "");
				_outBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);				
				return;
			}
			// DO CONVOLUTION - UPC algorithm with memory			
			channelsConvolvers[channel].ProcessUPConvolutionWithMemory(_inBuffer, _IR_partitioned, _outBuffer);						
		}

		void Process(const CMonoBuffer<float> & _inLeftBuffer, CMonoBuffer<float> & _outLeftBuffer, const CMonoBuffer<float> & _inRightBuffer, CMonoBuffer<float> & _outRightBuffer, std::weak_ptr<BRTServices::CServicesBase> _irTableWeakPtr) {
			_outLeftBuffer = _inLeftBuffer;
			_outRightBuffer = _inRightBuffer;

			std::lock_guard<std::mutex> l(mutex);
			ASSERT(_inLeftBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");
			ASSERT(_inRightBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");

			if (!enableProcessor || !initialized) {
				return;
			}

			if (channelsConvolvers.size() < 2) {
				SET_RESULT(RESULT_ERROR_OUTOFRANGE, "FIRConvolver::Process: Number of channels is less than 2, cannot process stereo output", "");
				_outLeftBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				_outRightBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				return;
			}

			// Check IR table
			std::shared_ptr<BRTServices::CServicesBase> _irTablePtr = _irTableWeakPtr.lock();
			if (!_irTablePtr) {
				SET_RESULT(RESULT_ERROR_NULLPOINTER, "FIR Table pointer is null when trying to use in FIRConvolver::Process");
				return;
			}

			// First time - Initialize convolution buffers
			if (!convolutionBuffersInitialized) {
				InitializedSourceConvolutionBuffers(_irTablePtr);
			}

			//std::vector<CMonoBuffer<float>> _leftIRPartitioned;
			//std::vector<CMonoBuffer<float>> _rightIRPartitioned;
			Common::CEarPair<BRTServices::TFRPartitions> earFRPartitions;
			
			if (_irTablePtr->IsSpatiallyOriented()) {
				earFRPartitions = _irTablePtr->GetFR_SpatiallyOriented_2Ears(0.0f, 0.0f,0.0f, Common::CVector3(), enableFindNearestIR);
			} else {
				earFRPartitions = _irTablePtr->GetFR_2Ears();
			}
			if (earFRPartitions.left.size() == 0 || earFRPartitions.right.size() == 0) {
				SET_RESULT(RESULT_ERROR_BADSIZE, "FIRConvolver::Process: No IR partitions found in FIR table for the requested ear", "");
				_outLeftBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				_outRightBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				return;
			}

			// CONVOLUTION - UPC algorithm with memory
			channelsConvolvers[0].ProcessUPConvolutionWithMemory(_inLeftBuffer, earFRPartitions.left, _outLeftBuffer);
			channelsConvolvers[1].ProcessUPConvolutionWithMemory(_inRightBuffer, earFRPartitions.right, _outRightBuffer);			
		}
									
		void Process(const CMonoBuffer<float> & _inBuffer, CMonoBuffer<float> & _outBuffer, const int & _channel, const Common::CTransform & sourceTransform, const Common::CTransform & listenerTransform, std::weak_ptr<BRTServices::CServicesBase> _irTableWeakPtr) {
			
			_outBuffer = _inBuffer;
			
			std::lock_guard<std::mutex> l(mutex);
			ASSERT(_inBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");
			
			// Check processor flag
			if (!enableProcessor || !initialized) {
				return;
			}		
			
			// Check IR table
			std::shared_ptr<BRTServices::CServicesBase> _irTablePtr = _irTableWeakPtr.lock();
			if (!_irTablePtr) {
				SET_RESULT(RESULT_ERROR_NULLPOINTER, "FIR Table pointer is null when trying to use in FIRConvolver::Process");
				return;
			}
						
			// First time - Initialize convolution buffers
			if (!convolutionBuffersInitialized) {
				InitializedSourceConvolutionBuffers(_irTablePtr);
			}

			// Calculate Source coordinates taking into account Source and Listener transforms
			float leftAzimuth;
			float leftElevation;
			float rightAzimuth;
			float rightElevation;
			float centerAzimuth;
			float centerElevation;
			float interauralAzimuth;

			Common::CSourceListenerRelativePositionCalculation::CalculateSourceListenerRelativePositions(sourceTransform, listenerTransform, _irTablePtr, false, leftElevation, leftAzimuth, rightElevation, rightAzimuth, centerElevation, centerAzimuth, interauralAzimuth);

			// GET IR						
			std::vector<CMonoBuffer<float>> _IR_partitioned;			
			enableFindNearestIR = false;
			_IR_partitioned = _irTablePtr->GetIRTFPartitionedSpatiallyOriented(leftAzimuth, leftElevation, Common::T_ear(_channel), enableFindNearestIR);			
						
			if (_IR_partitioned.size() == 0) {
				SET_RESULT(RESULT_ERROR_BADSIZE, "FIRConvolver::Process: No IR partitions found in FIR table for the requested ear", "");
				_outBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);
				return;
			}
			
			// DO CONVOLUTION - UPC algorithm with memory			
			channelsConvolvers[_channel].ProcessUPConvolutionWithMemory(_inBuffer, _IR_partitioned, _outBuffer);	
		}

		/// Reset convolvers and convolution buffers
		void ResetConvolutionBuffers() {
			
			std::lock_guard<std::mutex> l(mutex);
			convolutionBuffersInitialized = false;
			
			// Reset convolver classes				
			for (auto& convolver : channelsConvolvers) {
				convolver.Reset();
			}
		}
	private:
		
		/////////////////////
		/// PRIVATE Methods        
		/////////////////////

				
		/// Initialize convolvers and convolition buffers		
		void InitializedSourceConvolutionBuffers(std::shared_ptr<BRTServices::CServicesBase>& _irTable) {

			int numOfSubfilters = _irTable->GetTFNumberOfSubfilters();
			int subfilterLength = _irTable->GetTFSubfilterLength();

			//Common::CGlobalParameters globalParameters;
			//outputLeftUPConvolution.Setup(globalParameters.GetBufferSize(), subfilterLength, numOfSubfilters, true);
			//outputRightUPConvolution.Setup(globalParameters.GetBufferSize(), subfilterLength, numOfSubfilters, true);			

			for (auto& convolver : channelsConvolvers) {
				convolver.Setup(globalParameters.GetBufferSize(), subfilterLength, numOfSubfilters, true);
			}

			// Declare variable
			convolutionBuffersInitialized = true;
		}

		/////////////////////
		// Atributes
		////////////////////
		mutable std::mutex mutex; // To avoid access collisions
		Common::CGlobalParameters globalParameters; // Global parameters

		//BRTProcessing::CUniformPartitionedConvolution outputLeftUPConvolution; // Object to make the inverse fft of the left channel with the UPC method
		//BRTProcessing::CUniformPartitionedConvolution outputRightUPConvolution; // Object to make the inverse fft of the rigth channel with the UPC method

		std::vector<BRTProcessing::CUniformPartitionedConvolution> channelsConvolvers; // Vector of convolvers, one per channel
		
		//CMonoBuffer<float> leftChannelDelayBuffer; // To store the delay of the left channel of the expansion method
		//CMonoBuffer<float> rightChannelDelayBuffer; // To store the delay of the right channel of the expansion method

		bool initialized; // Flag to know if the processor has been initialized
		bool enableProcessor; // Flag to enable the processor
		//bool enableSpatialization; // Flags for independent control of processes
		//bool enableInterpolation; // Enables/Disables the interpolation on run time
		//bool enableITDSimulation; // Enables/Disables the ITD on run time
		//bool enableParallaxCorrection; // Enables/Disables the parallax correction on run time
		bool enableFindNearestIR; // Enables/Disables the search for the nearest IR when filtering
		bool convolutionBuffersInitialized; // Flag to check if the convolution buffers have been initialized		
	};
}
#endif