/**
* \class UPCAnechoic
*
* \brief Declaration of CUPCAnechoic class interface.
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

#ifndef _CUPCANECHOIC_H_
#define _CUPCANECHOIC_H_

#include <iostream>
#include <vector>
#include <Common/Fprocessor.hpp>
#include <Common/Buffer.hpp>
#include <Common/CommonDefinitions.hpp>

/** \brief Type definition for partitioned HRIR table
*/
typedef std::vector<CMonoBuffer<float>> THRIR_partitioned;

namespace Common {

	/** \details This class implements the necessary algorithms to do the convolution, in frequency domain, between signal and a impulse response using the	Uniformly Partitioned Convolution Algorithm (UPC algorithm)
	*/
	class CUPCAnechoic
	{

	public:

		/** \brief Default constructor
		*   \eh Nothing is reported to the error handler.
		*/
		CUPCAnechoic() : setupDone{ false }, inputSize{ 0 }, impulseResponseMemory{ 0 }, impulseResponseNumberOfSubfilters{ 0 }, impulseResponse_Frequency_Block_Size{ 0 }
		{
		}

		/** \brief Initialize the class and allocate memory.
		*   \details When this method is called, the system initializes variables and allocates memory space for the buffer.
		*	\param [in] _inputSize size of the input signal buffer (B size)
		*	\param [in] _HRIR_Frequency_Block_Size size of the FTT Impulse Response blocks, this number is (2*B + k) = 2^n
		*	\param [in] _HRIR_Block_Number number of blocks in which is divided the impluse response
		*	\param [in] _IRMemory if true, the method with IR memory will be used (otherwise, the method without memory will be used instead)
		*   \eh On error, an error code is reported to the error handler.
		*/
		void Setup(int _inputSize, int _IR_Frequency_Block_Size, int _IR_Block_Number, bool _IRMemory)
		{
			if (setupDone) {
				//Second time that this method has been called - clear all buffers
				storageInput_buffer.clear();
				storageInputFFT_buffer.clear();
				storageHRIR_buffer.clear();
			}

			inputSize = _inputSize;
			impulseResponse_Frequency_Block_Size = _IR_Frequency_Block_Size;
			impulseResponseNumberOfSubfilters = _IR_Block_Number;
			impulseResponseMemory = _IRMemory;

			if (CalculateIsPowerOfTwo(inputSize)) {	
				storageInput_bufferSize = inputSize;	
			}	else {
				storageInput_bufferSize = 2 * CalculateNextPowerOfTwo(inputSize)  - inputSize;
			}

			//Prepare the buffer with the space that we are going to need	
			storageInput_buffer.resize(storageInput_bufferSize, 0.0f);

			//Preparing the vector of buffers that is going to store the history of FFTs	
			storageInputFFT_buffer.resize(impulseResponseNumberOfSubfilters);
			for (int i = 0; i < impulseResponseNumberOfSubfilters; i++) {
				storageInputFFT_buffer[i].resize(impulseResponse_Frequency_Block_Size, 0.0f);
			}
			it_storageInputFFT = storageInputFFT_buffer.begin();

			//Preparing the vector of buffers that is going to store the history of the HRIR	
			if (impulseResponseMemory)
			{
				storageHRIR_buffer.resize(impulseResponseNumberOfSubfilters);
				for (int i = 0; i < impulseResponseNumberOfSubfilters; i++)
				{
					storageHRIR_buffer[i].resize(impulseResponseNumberOfSubfilters);
					for (int j = 0; j < impulseResponseNumberOfSubfilters; j++)
					{
						storageHRIR_buffer[i][j].resize(impulseResponse_Frequency_Block_Size, 0.0f);
					}
				}
				it_storageHRIR = storageHRIR_buffer.begin();
			}

			setupDone = true;
			SET_RESULT(RESULT_OK, "UPC convolver successfully set");
		}
		
		/** \brief Process the Uniformed Partitioned Convolution of the input signal with one impulse response
		*   \details This method performs the convolution between the input signal and the partitioned HRIR using the UPC* method, returning the FFT of the output.
		*   \details *Wefers, F. (2015). Partitioned convolution algorithms for real-time auralization (Vol. 20). Logos Verlag Berlin GmbH.
		*	\param [in] inBuffer_Time input signal buffer of B size
		*	\param [in] IR buffer structure that contains the HRIR divided in subfilters. Each subfilter with a size of HRIR_Frequency_Block_Size size  = 2*B
		*	\param [out] outBuffer FFT of the output signal of 2*B size (complex numbers). After the IIFT is done, only the last B samples are significant
		*   \eh Nothing is reported to the error handler.
		*/
		//void ProcessUPConvolution(const CMonoBuffer<float>& inBuffer_Time, const TOneEarHRIRPartitionedStruct & IR, CMonoBuffer<float>& outBuffer);
		void ProcessUPConvolution(const CMonoBuffer<float>& inBuffer_Time, const std::vector<CMonoBuffer<float>>& IR, CMonoBuffer<float>& outBuffer)
		{
			CMonoBuffer<float> sum;
			sum.resize(impulseResponse_Frequency_Block_Size, 0.0f);
			CMonoBuffer<float> temp;

			if (!setupDone) { 
				SET_RESULT(RESULT_ERROR_NOTSET, "Storage buffer to perform UP convolution has not been initialized");
				return; 
			}

			if (inBuffer_Time.size() == inputSize) {

				//Step 1- extend the input time signal buffer in order to have double length
				std::vector<float> inBuffer_Time_dobleSize;
				inBuffer_Time_dobleSize.reserve(inputSize * 2);
				inBuffer_Time_dobleSize.insert(inBuffer_Time_dobleSize.begin(), storageInput_buffer.begin(), storageInput_buffer.end());
				inBuffer_Time_dobleSize.insert(inBuffer_Time_dobleSize.end(), inBuffer_Time.begin(), inBuffer_Time.end());
				storageInput_buffer = inBuffer_Time;			//Store current input signal

				//Step 2,3 - FFT of the input signal
				CMonoBuffer<float> inBuffer_Frequency;
				Common::CFprocessor::CalculateFFT(inBuffer_Time_dobleSize, inBuffer_Frequency);
				*it_storageInputFFT = inBuffer_Frequency;		//Store the new input FFT into the first FTT history buffers

				//Step 4, 5 - Multiplications and sums
				auto it_product = it_storageInputFFT;

				for (int i = 0; i < impulseResponseNumberOfSubfilters; i++) {
					Common::CFprocessor::ProcessComplexMultiplication(*it_product, IR[i], temp);
					sum += temp;
					if (it_product == storageInputFFT_buffer.begin()) {
						it_product = storageInputFFT_buffer.end() - 1;
					}
					else {
						it_product--;
					}
				}
				//Move iterator waiting for the next input block
				if (it_storageInputFFT == storageInputFFT_buffer.end() - 1) {
					it_storageInputFFT = storageInputFFT_buffer.begin();
				}
				else {
					it_storageInputFFT++;
				}
				// Make the IIF
				CMonoBuffer<float> ouputBuffer_temp;
				Common::CFprocessor::CalculateIFFT(sum, ouputBuffer_temp);
				//We are left only with the final half of the result
				int halfsize = (int)(ouputBuffer_temp.size() * 0.5f);
				CMonoBuffer<float> temp_OutputBlock(ouputBuffer_temp.begin() + halfsize, ouputBuffer_temp.end());
				outBuffer = std::move(temp_OutputBlock);			//To use in C++11

			}
			else {
				//TODO: handle size errors
			}
		}

		/** \brief Make the Uniformed Partitioned Convolution of the input signal using also last input signal buffers (method with memory)
		*   \details This method performs the convolution between the input signal and the partitioned HRIR (the HRIR has been stored for each input signal block) using the UPC* method, returning the FFT of the output.
		*   \details *Wefers, F. (2015). Partitioned convolution algorithms for real-time auralization (Vol. 20). Logos Verlag Berlin GmbH.
		*	\param [in] inBuffer_Time input signal buffer of B size
		*	\param [in] IR buffer structure that contains the HRIR divided in subfilters. Each subfilter with a size of HRIR_Frequency_Block_Size size  = 2*B
		*	\param [out] outBuffer FFT of the output signal of 2*B size (complex numbers). After the IIFT is done, only the last B samples are significant
		*   \eh Nothing is reported to the error handler.
		*/
		//void ProcessUPConvolutionWithMemory(const CMonoBuffer<float>& inBuffer_Time, const TOneEarHRIRPartitionedStruct & IR, CMonoBuffer<float>& outBuffer);
		void ProcessUPConvolutionWithMemory(const CMonoBuffer<float>& inBuffer_Time, const std::vector<CMonoBuffer<float>>& IR, CMonoBuffer<float>& outBuffer)
		{			
			CMonoBuffer<float> sum;
			sum.resize(impulseResponse_Frequency_Block_Size, 0.0f);
			CMonoBuffer<float> temp;

			ASSERT(inBuffer_Time.size() == inputSize, RESULT_ERROR_BADSIZE, "Bad input size, don't match with the size setting up in the setup method", "");
			ASSERT(impulseResponseNumberOfSubfilters == IR.size(), RESULT_ERROR_BADSIZE, "Bad input size, the number of impulse response partitions does not correspond to what is expected.", "Has this class been initialised correctly?");
			
			if (impulseResponseMemory && setupDone)
			{
				if (inBuffer_Time.size() == inputSize && IR.size() != 0)
				{
					//Step 1- extend the input time signal buffer in order to have double length
					std::vector<float> inBuffer_Time_dobleSize;
					inBuffer_Time_dobleSize.reserve(inputSize + storageInput_bufferSize );
					inBuffer_Time_dobleSize.insert(inBuffer_Time_dobleSize.begin(), storageInput_buffer.begin(), storageInput_buffer.end());
					inBuffer_Time_dobleSize.insert(inBuffer_Time_dobleSize.end(), inBuffer_Time.begin(), inBuffer_Time.end());
					
					//Store current input signal
					//storageInput_buffer = inBuffer_Time;					
					std::vector<float> tempStorageInput;
					tempStorageInput.reserve(storageInput_bufferSize);
					tempStorageInput.insert(tempStorageInput.begin(), storageInput_buffer.end() -(storageInput_bufferSize - inputSize), storageInput_buffer.end());
					tempStorageInput.insert(tempStorageInput.end(), inBuffer_Time.begin(), inBuffer_Time.end());
					storageInput_buffer = tempStorageInput;

					//Step 2,3 - FFT of the input signal
					CMonoBuffer<float> inBuffer_Frequency;
					Common::CFprocessor::CalculateFFT(inBuffer_Time_dobleSize, inBuffer_Frequency);
					//Store the new input FFT into the first FTT history buffers
					*it_storageInputFFT = inBuffer_Frequency;

					//Store the HRIR input signal in the storage HRIR matrix
					*it_storageHRIR = IR;

					//Step 4, 5 - Multiplications and sums
					auto it_product = it_storageInputFFT;
					auto it_HRIR_multiplicationFactor = it_storageHRIR;

					for (int i = 0; i < impulseResponseNumberOfSubfilters; i++) {
						Common::CFprocessor::ProcessComplexMultiplication(*it_product, (*it_HRIR_multiplicationFactor)[i], temp);
						sum += temp;
						if (it_product == storageInputFFT_buffer.begin()) {
							it_product = storageInputFFT_buffer.end() - 1;
						}
						else {
							it_product--;
						}

						if (it_HRIR_multiplicationFactor == storageHRIR_buffer.end() - 1) {
							it_HRIR_multiplicationFactor = storageHRIR_buffer.begin();
						}
						else {
							it_HRIR_multiplicationFactor++;
						}
					}
					//Move iterator waiting for the next input block
					if (it_storageInputFFT == storageInputFFT_buffer.end() - 1) {
						it_storageInputFFT = storageInputFFT_buffer.begin();
					}
					else {
						it_storageInputFFT++;
					}

					//Move iterator waiting for the next input block
					if (it_storageHRIR == storageHRIR_buffer.begin()) {
						it_storageHRIR = storageHRIR_buffer.end() - 1;
					}
					else {
						it_storageHRIR--;
					}

					// Make the IIF
					CMonoBuffer<float> ouputBuffer_temp;
					Common::CFprocessor::CalculateIFFT(sum, ouputBuffer_temp);
					//We are left only with the final half of the result
					//int halfsize = (int)(ouputBuffer_temp.size() * 0.5f);
					CMonoBuffer<float> temp_OutputBlock(ouputBuffer_temp.end() - inputSize, ouputBuffer_temp.end());
					outBuffer = std::move(temp_OutputBlock);			//To use in C++11

				}
				else
				{
					SET_RESULT(RESULT_ERROR_BADSIZE, "The input buffer size is not correct or there is not a valid HRTF loded");
					outBuffer.resize(inBuffer_Time.size(), 0.0f);
				}
			}
			else
			{
				SET_RESULT(RESULT_ERROR_NOTSET, "HRTF storage buffer to perform UP convolution with memory has not been initialized");
			}

		}
		
		/** \brief Reset class state and clean convolution buffers 
		*   \details After calling this method it is necessary to do a setup again.
		*   \details 
		*/
		void Reset() {
			if (setupDone) {				
				setupDone = false;
				storageInput_buffer.clear();
				storageInputFFT_buffer.clear();
				storageHRIR_buffer.clear();				
				inputSize = 0;
				impulseResponseMemory = 0;
				impulseResponseNumberOfSubfilters = 0;
				impulseResponse_Frequency_Block_Size = 0;
			}
		}

	private:
		// ATTRIBUTES	
		int inputSize;								//Size of the inputs buffer				
		int impulseResponse_Frequency_Block_Size;	//Size of the HRIR buffer
		int impulseResponseNumberOfSubfilters;		//Number of blocks in which is divided the HRIR
		int storageInput_bufferSize;					//Number of samples to be saved in each audio loop
		bool impulseResponseMemory;					//Indicate if HRTF storage buffer has to be prepared to do UPC with memory
		bool setupDone;								//It's true when setup has been called at least once
				
		std::vector<float> storageInput_buffer;						//To store the last input signal
		std::vector<std::vector<float>> storageInputFFT_buffer;			//To store the history of input signals FFTs 
		std::vector<std::vector<float>>::iterator it_storageInputFFT;	//Declare a general iterator to keep the head of the FTTs buffer
		std::vector<THRIR_partitioned> storageHRIR_buffer;			//To store the HRIR of the orientation of the previous frames
		std::vector<THRIR_partitioned>::iterator it_storageHRIR;		//Declare a general iterator to keep the head of the storageHRIR_buffer		
	};
}
#endif
