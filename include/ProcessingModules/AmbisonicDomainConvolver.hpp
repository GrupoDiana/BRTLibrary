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

#ifndef _AMBISONIC_DOMAIN_CONVOLVER_
#define _AMBISONIC_DOMAIN_CONVOLVER_

#include <Common/UPCAnechoic.hpp>
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
		CAmbisonicDomainConvolver() : earToProcess { Common::T_ear::NONE}, convolutionBuffersInitialized{ false }, numberOfAmbisonicChannels{ 4 } { }


		/*///Enable spatialization process for this source	
		void EnableSpatialization() { enableSpatialization = true; }
		///Disable spatialization process for this source	
		void DisableSpatialization() { enableSpatialization = false; }
		///Get the flag for spatialization process enabling
		bool IsSpatializationEnabled() { return enableSpatialization; }*/
		
		void SetEar(Common::T_ear _ear) { earToProcess = _ear; }


		/*///Enable HRTF interpolation method	
		void EnableInterpolation() { enableInterpolation = true; }
		///Disable HRTF interpolation method
		void DisableInterpolation() { enableInterpolation = false; }
		///Get the flag for HRTF interpolation method
		bool IsInterpolationEnabled() { return enableInterpolation; }*/
						
		void SetAmbisonicOrder(int _ambisonicOrder) {
			numberOfAmbisonicChannels = Common::CAmbisonicEncoder::CalculateNumberOfChannels(_ambisonicOrder);
		};
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
		void Process(std::vector<CMonoBuffer<float>>& _inChannelsBuffers, CMonoBuffer<float>& outBuffer, std::weak_ptr<BRTServices::CAmbisonicBIR>& _listenerAmbisoninBIRWeak) {

			ASSERT(_inChannelsBuffers.size() == numberOfAmbisonicChannels, RESULT_ERROR_BADSIZE, "InChannlesBuffers size has to be equal to the number of channels set", "");
			//ASSERT(_inBuffer.size() == globalParameters.GetBufferSize(), RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the BRT::GlobalParameters method", "");
						
			// Check process flag
			/*if (!enableSpatialization)
			{
				outLeftBuffer = _inBuffer;
				outRightBuffer = _inBuffer;
				return;
			}*/
			// Check listener HRTF
			/*std::shared_ptr<BRTServices::CHRTF> _listenerHRTF = _listenerHRTFWeak.lock();
			if (!_listenerHRTF) {
				SET_RESULT(RESULT_ERROR_NULLPOINTER, "HRTF listener pointer is null when trying to use in HRTFConvolver");
				outBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);				
				return;
			}*/

			// Check listener Ambisonic BIR
			std::shared_ptr<BRTServices::CAmbisonicBIR> _listenerAmbisonicBIR = _listenerAmbisoninBIRWeak.lock();
			if (!_listenerAmbisonicBIR) {
				SET_RESULT(RESULT_ERROR_NULLPOINTER, "AmbisonicBIR pointer is null when trying to use in AmbisonicDomainConvolver");
				outBuffer.Fill(globalParameters.GetBufferSize(), 0.0f);				
				return;
			}
			
			// First time - Initialize convolution buffers
			if (!convolutionBuffersInitialized) { InitializedSourceConvolutionBuffers(_listenerAmbisonicBIR); }
			
			
			std::vector<CMonoBuffer<float>> allChannelsBuffersConvolved (numberOfAmbisonicChannels);
			for (int nChannel = 0; nChannel < _inChannelsBuffers.size(); nChannel++) {				
				std::vector<CMonoBuffer<float>>  oneChannel_ABIR_partitioned;
				oneChannel_ABIR_partitioned = _listenerAmbisonicBIR->GetChannelPartitionedIR_OneEar(nChannel, earToProcess); // GET ABIR
				channelsUPConvolutionVector[nChannel]->ProcessUPConvolutionWithMemory(_inChannelsBuffers[nChannel], oneChannel_ABIR_partitioned, allChannelsBuffersConvolved[nChannel], false);
			}
			
			CMonoBuffer<float> mixedChannels;
			mixedChannels.SetFromMix({ allChannelsBuffersConvolved });			
			mixedChannels.ApplyGain(1.0f / numberOfAmbisonicChannels);

			Common::CUPCAnechoic::CalculateIFFT(mixedChannels, outBuffer);			
		}

		/// Reset convolvers and convolution buffers
		void ResetSourceConvolutionBuffers() {
			convolutionBuffersInitialized = false;			
			// Reset convolver classes		
			channelsUPConvolutionVector.clear();
			/*for (int nChannel = 0; nChannel < numberOfAmbisonicChannels; nChannel++) {
				channelsUPConvolutionVector[nChannel].Reset();
			}						*/
		}

	private:

		// Atributes
		Common::CGlobalParameters globalParameters;
		
		std::vector<std::shared_ptr<Common::CUPCAnechoic>> channelsUPConvolutionVector;		// Object to make the inverse fft of the left channel with the UPC method
				
		Common::T_ear earToProcess;
		int numberOfAmbisonicChannels;
		//bool enableSpatialization;						// Flags for independent control of processes
		//bool enableInterpolation;							// Enables/Disables the interpolation on run time
		bool convolutionBuffersInitialized;
		
		/////////////////////
		/// PRIVATE Methods        
		/////////////////////

		//// Apply doppler effect simulation
		//void ProcessAddDelay_ExpansionMethod(CMonoBuffer<float>& input, CMonoBuffer<float>& output, CMonoBuffer<float>& delayBuffer, int newDelay)
		//{
		//	//Prepare the outbuffer		
		//	if (output.size() != input.size()) { output.resize(input.size()); }

		//	//Prepare algorithm variables
		//	float position = 0;
		//	float numerator = input.size() - 1;
		//	float denominator = input.size() - 1 + newDelay - delayBuffer.size();
		//	float compressionFactor = numerator / denominator;

		//	//Add samples to the output from buffer
		//	for (int i = 0; i < delayBuffer.size(); i++)
		//	{
		//		output[i] = delayBuffer[i];
		//	}

		//	//Fill the others buffers
		//	//if the delay is the same one as the previous frame use a simplification of the algorithm
		//	if (newDelay == delayBuffer.size())
		//	{
		//		//Copy input to output
		//		int j = 0;
		//		for (int i = delayBuffer.size(); i < input.size(); i++)
		//		{
		//			output[i] = input[j++];
		//		}
		//		//Fill delay buffer
		//		for (int i = 0; i < newDelay; i++)
		//		{
		//			delayBuffer[i] = input[j++];
		//		}
		//	}
		//	//else, apply the expansion/compression algorihtm
		//	else
		//	{
		//		int j;
		//		float rest;
		//		int forLoop_end;
		//		//The last loop iteration must be addressed in a special way if newDelay = 0 (part 1)
		//		if (newDelay == 0) { forLoop_end = input.size() - 1; }
		//		else { forLoop_end = input.size(); }

		//		//Fill the output buffer with the new values 
		//		for (int i = delayBuffer.size(); i < forLoop_end; i++)
		//		{
		//			j = static_cast<int>(position);
		//			rest = position - j;
		//			output[i] = input[j] * (1 - rest) + input[j + 1] * rest;
		//			position += compressionFactor;
		//		}

		//		//The last loop iteration must be addressed in a special way if newDelay = 0 (part 2)
		//		if (newDelay == 0)
		//		{
		//			output[input.size() - 1] = input[input.size() - 1];
		//			delayBuffer.clear();
		//		}

		//		//if newDelay!=0 fill out the delay buffer
		//		else
		//		{
		//			//Fill delay buffer 			
		//			CMonoBuffer<float> temp;
		//			temp.reserve(newDelay);
		//			for (int i = 0; i < newDelay - 1; i++)
		//			{
		//				int j = int(position);
		//				float rest = position - j;
		//				temp.push_back(input[j] * (1 - rest) + input[j + 1] * rest);
		//				position += compressionFactor;
		//			}
		//			//Last element of the delay buffer that must be addressed in a special way
		//			temp.push_back(input[input.size() - 1]);
		//			//delayBuffer.swap(temp);				//To use in C++03
		//			delayBuffer = std::move(temp);			//To use in C++11
		//		}
		//	}
		//}//End ProcessAddDelay_ExpansionMethod


		/// Initialize convolvers and convolition buffers		
		void InitializedSourceConvolutionBuffers(std::shared_ptr<BRTServices::CAmbisonicBIR>& _listenerAmbisonicBIR) {

			int numOfSubfilters = _listenerAmbisonicBIR->GetIRNumberOfSubfilters();
			int subfilterLength = _listenerAmbisonicBIR->GetIRSubfilterLength();
			
			for (int nChannel = 0; nChannel < numberOfAmbisonicChannels; nChannel++) {								
				std::shared_ptr<Common::CUPCAnechoic> channelUPConvolver = std::make_shared<Common::CUPCAnechoic>();				
				channelUPConvolver->Setup(globalParameters.GetBufferSize(), subfilterLength, numOfSubfilters, true);
				channelsUPConvolutionVector.push_back(channelUPConvolver);				
			}			
			// Declare variable
			convolutionBuffersInitialized = true;
		}
		

	};
}
#endif