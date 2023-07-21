/**
* \class CFprocessor
*
* \brief Declaration of CFprocessor class interface.
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

#ifndef _CFPROCESSOR_HPP_
#define _CFPROCESSOR_HPP_

//#include "Buffer.h"
//#include <math.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "fftsg.hpp"
#include "Buffer.hpp"

#ifndef THRESHOLD
#define THRESHOLD 0.0000001f
#endif

namespace Common {

	/** \details This class implements the necessary algorithms to do the convolution, in frequency domain, between signal and a impulse response.
	*/
	class CFprocessor
	{

	public:

		/** \brief Default constructor
		*/
		CFprocessor() : inputSize{ 0 }, IRSize{ 0 }, FFTBufferSize{ 0 }, setupDone{ false }, ip_size {0}, normalizeCoef{0}, w_size{0}
		{
		}
		
		/** \brief Calculate the FFT of B points the input signal. Where B = 2^n = (N + k).
		*   \details This method will extend the input buffer with zeros (k) until be power of 2 and then made the FFT.
		*	\param [in] inputAudioBuffer_time vector containing the samples of input signal in time-domain. N is this buffer size.
		*	\param [out] outputAudioBuffer_frequency FFT of the input signal. Have a size of B * 2, because contains the real and imaginary parts of each B point.
		*/
		static void CalculateFFT(const std::vector<float>& inputAudioBuffer_time, std::vector<float>& outputAudioBuffer_frequency)		
		{
			int inputBufferSize = inputAudioBuffer_time.size();

			ASSERT(inputBufferSize != 0, RESULT_ERROR_BADSIZE, "Bad input size when setting up frequency convolver", "");

			if (inputBufferSize > 0) //Just in case error handler is off
			{
				///////////////////////////////
				// Calculate FFT/output size //
				///////////////////////////////
				int FFTBufferSize = inputBufferSize;
				//Check if if power of two, if not round up to the next highest power of 2 
				if (!CalculateIsPowerOfTwo(FFTBufferSize)) {
					FFTBufferSize = CalculateNextPowerOfTwo(FFTBufferSize);
				}
				FFTBufferSize *= 2;							//We multiplicate by 2 because we need to store real and imaginary part

				///////////////////////////////////////////////////////////////////////////////
				// Calculate auxiliary arrays size, necessary to use the Takuya OOURA library
				///////////////////////////////////////////////////////////////////////////////
				int ip_size = std::sqrt(FFTBufferSize / 2) + 2;		//Size of the auxiliary array w. This come from lib documentation/examples.
				int w_size = FFTBufferSize * 5 / 4;					//Size of the auxiliary array w. This come from lib documentation/examples.
				std::vector<int> ip(ip_size);						//Define the auxiliary array ip 
				std::vector<double> w(w_size);						//Define the auxiliary array w			
				ip[0] = 0;											//w[],ip[] are initialized if ip[0] == 0.

				//////////////
				// Make FFT //
				//////////////			
				std::vector<double> inputAudioBuffer_frequency(FFTBufferSize, 0.0f);			//Initialize the vector of doubles to store the FFT					
				ProcessAddImaginaryPart(inputAudioBuffer_time, inputAudioBuffer_frequency);			//Copy the input vector into an vector of doubles and insert the imaginary part.											
				cdft(FFTBufferSize, 1, inputAudioBuffer_frequency.data(), ip.data(), w.data());	//Make the FFT

				////////////////////
				// Prepare Output //
				////////////////////	
				//Copy to the output float vector			
				if (outputAudioBuffer_frequency.size() != FFTBufferSize) { outputAudioBuffer_frequency.resize(FFTBufferSize); }
				for (int i = 0; i < inputAudioBuffer_frequency.size(); i++) {
					outputAudioBuffer_frequency[i] = static_cast<float>(inputAudioBuffer_frequency[i]);
				}
			}
		}

		/** \brief Calculate the FFT of B points of the input signal in order to make a convolution with other vector. Where B = (N + P + k).
		*   \details This method will extend the input buffer with zeros (k) until reaching the size of B = 2^n = (N + P + k) and then make the FFT.
		*	\param [in] inputAudioBuffer_time vector containing the samples of input signal in time-domain. N is this buffer size.
		*	\param [out] outputAudioBuffer_frequency FFT of the input signal. Have a size of B * 2 = (N + P + k) * 2, because contains the real and imaginary parts of each B point.
		*	\param [in] irDataLength is P, the size in the time domain of the other vector which is going to do the convolved with this one in the frequency domain (multiplication).
		*/
		static void CalculateFFT(const std::vector<float>& inputAudioBuffer_time, std::vector<float>& outputAudioBuffer_frequency, int irDataLength)		
		{
			int inputBufferSize = inputAudioBuffer_time.size();

			ASSERT(inputBufferSize > 0, RESULT_ERROR_BADSIZE, "Bad input size when setting up frequency convolver", "");
			ASSERT(irDataLength > 0, RESULT_ERROR_BADSIZE, "Bad ABIR size when setting up frequency convolver", "");

			if ((inputBufferSize > 0) && (irDataLength > 0)) //Just in case error handler is off
			{
				///////////////////////////////
				// Calculate FFT/output size //
				///////////////////////////////
				int FFTBufferSize = inputBufferSize + irDataLength;
				//Check if if power of two, if not round up to the next highest power of 2 
				if (!CalculateIsPowerOfTwo(FFTBufferSize)) {
					FFTBufferSize = CalculateNextPowerOfTwo(FFTBufferSize);
				}
				FFTBufferSize *= 2;			//We multiplicate by 2 because we need to store real and imaginary part

				///////////////////////////////////////////////////////////////////////////////
				// Calculate auxiliary arrays size, necessary to use the Takuya OOURA library
				///////////////////////////////////////////////////////////////////////////////
				int ip_size = std::sqrt(FFTBufferSize / 2) + 2;		//Size of the auxiliary array w. This come from lib documentation/examples.
				int w_size = FFTBufferSize * 5 / 4;					//Size of the auxiliary array w. This come from lib documentation/examples.
				std::vector<int> ip(ip_size);						//Define the auxiliary array ip 
				std::vector<double> w(w_size);						//Define the auxiliary array w			
				ip[0] = 0;											//w[],ip[] are initialized if ip[0] == 0.

				//////////////
				// Make FFT //
				//////////////	
				std::vector<double> inputAudioBuffer_frequency(FFTBufferSize, 0.0f);			//Initialize the vector of doubles to store the FFT					
				ProcessAddImaginaryPart(inputAudioBuffer_time, inputAudioBuffer_frequency);			//Copy the input vector into an vector of doubles and insert the imaginary part.
				cdft(FFTBufferSize, 1, inputAudioBuffer_frequency.data(), ip.data(), w.data());	//Make the FFT

				////////////////////
				// Prepare Output //
				////////////////////			
				outputAudioBuffer_frequency.resize(FFTBufferSize);
				for (int i = 0; i < inputAudioBuffer_frequency.size(); i++) {
					outputAudioBuffer_frequency[i] = static_cast<float>(inputAudioBuffer_frequency[i]);
				}
			}
		}

		/** \brief Get the IFFT of K points of the input signal buffer. 
		*   \details This method makes the IFFT of the input buffer. This method doesn't implement OLA or OLS algothim, it doesn't resolve the inverse convolution.
		*   \param [in] inputAudioBuffer_frequency Vector of samples storing the output signal in frecuency domain. This buffers has to be size of K
		*   \param [out] outputAudioBuffer_time Vector of samples where the IFFT of the output signal will be returned in time domain. This vector will have a size of K/2.
		*	\pre inputAudioBuffer_frequency has to have the same size that the one returned by any of the CalculateFFT_ methods.
		*   \throws May throw exceptions and errors to debugger
		*/
		static void CalculateIFFT(const std::vector<float>& inputAudioBuffer_frequency, std::vector<float>& outputAudioBuffer_time)		       
		{
			int inputBufferSize = inputAudioBuffer_frequency.size();
			ASSERT(inputBufferSize > 0, RESULT_ERROR_BADSIZE, "Bad input size", "");

			if (inputBufferSize > 0) //Just in case error handler is off
			{
				//////////////////////////////
				// Calculate output size	//
				//////////////////////////////
				int FFTBufferSize = inputBufferSize;

				///////////////////////////////////////////////////////////////////////////////
				// Calculate auxiliary arrays size, necessary to use the Takuya OOURA library
				///////////////////////////////////////////////////////////////////////////////
				int ip_size = std::sqrt(FFTBufferSize / 2) + 2;		//Size of the auxiliary array w. This come from lib documentation/examples.
				int w_size = FFTBufferSize * 5 / 4;					//Size of the auxiliary array w. This come from lib documentation/examples.																				
				std::vector<int> ip(ip_size);						//Define the auxiliary array ip 
				std::vector<double> w(w_size);						//Define the auxiliary array w			
				ip[0] = 0;											//w[],ip[] are initialized if ip[0] == 0.

				///////////////
				// Make IFFT //
				///////////////																
				std::vector<double> outBuffer_temp(inputAudioBuffer_frequency.begin(), inputAudioBuffer_frequency.end());	//Convert to double
				cdft(FFTBufferSize, -1, outBuffer_temp.data(), ip.data(), w.data());										//Make the IFFT

				////////////////////
				// Prepare Output //
				////////////////////	
				int outBufferSize = inputAudioBuffer_frequency.size() / 2;	//Locar var to move throught the outbuffer
				if (outputAudioBuffer_time.size() != outBufferSize) { outputAudioBuffer_time.resize(outBufferSize); }
				float normalizeCoef = 2.0f / FFTBufferSize;			//Store the normalize coef for the FFT-1	
				//Fill out the output signal buffer
				for (int i = 0; i < outBufferSize; i++) {
					outputAudioBuffer_time[i] = static_cast<float>(CalculateRoundToZero(outBuffer_temp[2 * i] * normalizeCoef));
				}
			}
		}

		/** \brief Process complex multiplication between the elements of two vectors.
		*   \details This method makes the complex multiplication of vector samples: (a+bi)(c+di) = (ac-bd)+i(ad+bc)
		*   \param [in] x Vector of samples that has real and imaginary parts interlaced. x[i] = Re[Xj], x[i+1] = Img[Xj]
		*   \param [in] h Vector of samples that has real and imaginary parts interlaced. h[i] = Re[Hj], h[i+1] = Img[Hj]
		*	\param [out] y Complex multiplication of x and h vectors
		*	\pre Both vectors (x and h) have to be the same size
		*   \throws May throw exceptions and errors to debugger
		*/
		static void ProcessComplexMultiplication(const std::vector<float>& x, const std::vector<float>& h, std::vector<float>& y)		       
		{
			ASSERT(x.size() == h.size(), RESULT_ERROR_BADSIZE, "Complex multiplication in frequency convolver requires two vectors of the same size", "");

			if (x.size() == h.size())	//Just in case error handler is off
			{
				y.resize(x.size());
				int end = (int)y.size() * 0.5f;
				for (int i = 0; i < end; i++)
				{
					float a = x[2 * i];
					float b = x[2 * i + 1];
					float c = (h[2 * i]);
					float d = (h[2 * i + 1]);

					y[2 * i] = a * c - b * d;
					y[2 * i + 1] = a * d + b * c;
				}
			}
		}

		/** \brief Process a buffer with complex numbers to get two separated vectors one with the modules and other with the phases.
		*   \details This method return two vectors with the module and phase of the vector introduced.
		*   \param [in] inputBuffer Vector of samples that has real and imaginary parts interlaced. inputBuffer[i] = Re[Xj], x[i+1] = Img[Xj]
		*	\param [out] moduleBuffer Vector of real numbers that are the module of the complex numbers. moduleBuffer[i] = sqrt(inputBuffer[2 * i]^2 * inputBuffer[2 * i + 1]^2)
		*	\param [out] phaseBuffer  Vector of real numbers that are the argument of the complex numbers.	phaseBuffer [i] = atan(inputBuffer[2 * i + 1] / inputBuffer[2 * i]^2)		
		*   \throws May throw exceptions and errors to debugger
		*/
		static void ProcessToModulePhase(const std::vector<float>& inputBuffer, std::vector<float>& moduleBuffer, std::vector<float>& phaseBuffer)		       
		{
			ASSERT(inputBuffer.size() > 0, RESULT_ERROR_BADSIZE, "Bad input size", "");

			if (inputBuffer.size() > 0) {

				moduleBuffer.clear();
				phaseBuffer.clear();

				int end = (int)(inputBuffer.size() * 0.5);

				for (int i = 0; i < end; i++)
				{
					float real = inputBuffer[2 * i];
					float img = inputBuffer[2 * i + 1];

					moduleBuffer.push_back(std::sqrt(real * real + img * img));
					phaseBuffer.push_back(std::atan2(img, real));
				}
			}
		}

		/** \brief Process a buffer with complex numbers to get two separated vectors one with the powers and other with the phases.
		*   \details This method return two vectors with the power and phase of the vector introduced.
		*   \param [in] inputBuffer Vector of samples that has real and imaginary parts interlaced. inputBuffer[i] = Re[Xj], x[i+1] = Img[Xj]
		*	\param [out] powerBuffer Vector of real numbers that are the power of the complex numbers. moduleBuffer[i] = inputBuffer[2 * i]^2 * inputBuffer[2 * i + 1]^2
		*	\param [out] phaseBuffer  Vector of real numbers that are the argument of the complex numbers.	phaseBuffer [i] = atan(inputBuffer[2 * i + 1] / inputBuffer[2 * i]^2)
		*   \throws May throw exceptions and errors to debugger
		*/
		static void ProcessToPowerPhase(const std::vector<float>& inputBuffer, std::vector<float>& powerBuffer, std::vector<float>& phaseBuffer)		       
		{
			ASSERT(inputBuffer.size() > 0, RESULT_ERROR_BADSIZE, "Bad input size", "");

			if (inputBuffer.size() > 0) {

				powerBuffer.clear();
				phaseBuffer.clear();

				int end = (int)(inputBuffer.size() * 0.5);

				for (int i = 0; i < end; i++)
				{
					float real = inputBuffer[2 * i];
					float img = inputBuffer[2 * i + 1];

					powerBuffer.push_back(real * real + img * img);
					phaseBuffer.push_back(std::atan2(img, real));
				}
			}
		}

		/** \brief Process two buffers with module and phase of complex numbers, in order to get a vector with complex numbers in binomial way
		*   \details This method return one vectors that has real and imaginary parts interlaced. inputBuffer[i] = Re[Xj], x[i+1] = Img[Xj]
		*   \param [in] moduleBuffer Vector of samples that represents the module of complex numbers.
		*   \param [in] phaseBuffer Vector of samples that represents the argument of complex numbers.
			\param [out] outputBuffer Vector of samples that has real and imaginary parts interlaced. outputBuffer[i] = Re[Xi] = moduleBuffer[i] * cos(phaseBuffer[i]), outputBuffer[i+1] = Img[Xi] = moduleBuffer[i] * sin(phaseBuffer[i])
		*   \throws May throw exceptions and errors to debugger
		*/
		static void ProcessToRealImaginary(const std::vector<float>& moduleBuffer, const std::vector<float>& phaseBuffer, std::vector<float>& outputBuffer)		       
		{
			ASSERT(moduleBuffer.size() > 0, RESULT_ERROR_BADSIZE, "Bad input size moduleBuffer", "");
			ASSERT(phaseBuffer.size() > 0, RESULT_ERROR_BADSIZE, "Bad input size phaseBuffer", "");
			ASSERT(moduleBuffer.size() == phaseBuffer.size(), RESULT_ERROR_BADSIZE, "Bad input size, moduleBuffer and phaseBuffer should have the same size", "");

			if ((moduleBuffer.size() == moduleBuffer.size()) && (moduleBuffer.size() > 0))
			{
				outputBuffer.clear();
				for (int i = 0; i < moduleBuffer.size(); i++)
				{
					float a = moduleBuffer[i] * std::cos(phaseBuffer[i]);
					float b = moduleBuffer[i] * std::sin(phaseBuffer[i]);

					outputBuffer.push_back(a);
					outputBuffer.push_back(b);
				}
			}
		}

		/** \brief Initialize the class and allocate memory in other to use the CalculateIFFT_OLA method.
		*   \details When this method is called, the system initializes variables and allocates memory space for the buffer.
		*	\param [in] _inputSize size of the input signal buffer (L size)
		*	\param [in] _impulseResponseSize size of the Impulse Response, which is the size of the buffer that contains the AIR or HRIR signal (P size)
		*   \throws May throw exceptions and errors to debugger
		*/		
		void SetupIFFT_OLA(int _inputSize, int _AIRSize)
		{
			ASSERT(_inputSize > 0, RESULT_ERROR_BADSIZE, "Bad input size when setting up frequency convolver", "");
			ASSERT(_AIRSize > 0, RESULT_ERROR_BADSIZE, "Bad ABIR size when setting up frequency convolver", "");

			if ((_inputSize > 0) && (_AIRSize > 0))		//Just in case error handler is off
			{
				if (setupDone) {
					storageBuffer.clear();		//If is the second time that this method has been called -> clear all buffers
				}

				///////////////////////////////
				// Calculate FFT/output size //
				///////////////////////////////
				inputSize = _inputSize;
				IRSize = _AIRSize;
				FFTBufferSize = inputSize + IRSize;
				//Check if if power of two, if not round up to the next highest power of 2 
				if (!CalculateIsPowerOfTwo(FFTBufferSize)) {
					FFTBufferSize = CalculateNextPowerOfTwo(FFTBufferSize);
				}
				storageBuffer.resize(FFTBufferSize);		//Prepare the buffer with the space that we are going to needed
				normalizeCoef = 1.0f / FFTBufferSize;		//Store the normalize coef for the FFT-1
				FFTBufferSize *= 2;							//We multiplicate by 2 because we need to store real and imaginary part

				///////////////////////////////////////////////////////////////////////////////
				// Calculate auxiliary arrays size, necessary to use the Takuya OOURA library
				///////////////////////////////////////////////////////////////////////////////
				ip_size = std::sqrt(FFTBufferSize / 2) + 2;		//Size of the auxiliary array w. This come from lib documentation/examples.
				w_size = FFTBufferSize * 5 / 4;				//Size of the auxiliary array w. This come from lib documentation/examples.

				setupDone = true;
				SET_RESULT(RESULT_OK, "Frequency convolver succesfully set");
			}
		}//SetupIFFT_OLA

		/** \brief Calculate the IFFT of the output signal using OLA (Overlap-Add) algorithm.
		*   \details This method makes the IFFT of the signal using OLA (Overlap-Add) algorithm. Do the IFFT, adds the samples obtained with buffer samples in order to get output signal, and updates the buffer.
		*   \param [in] signal_frequency Vector of samples storing the output signal in frecuency domain.
		*   \param [out] signal_time Vector of samples where the IFFT of the output signal will be returned in time domain. This vector will have the size indicated in \link SetupIFFT_OLA \endlink method.
		*	\pre signal_frequency has to have the same size that the one returned by any of the CalculateFFT_ methods.
		*   \throws May throw exceptions and errors to debugger
		*	\sa CalculateFFT_Input, CalculateFFT_IR
		*/		
		void CalculateIFFT_OLA(const std::vector<float>& inputBuffer_frequency, std::vector<float>& outputBuffer_time)
		{
			ASSERT(inputBuffer_frequency.size() == FFTBufferSize, RESULT_ERROR_BADSIZE, "Incorrect size of input buffer when computing inverse FFT in frequency convolver", "");
			ASSERT(setupDone, RESULT_ERROR_NOTINITIALIZED, "SetupIFFT_OLA method should be called before call this method", "");

			if ((setupDone) && (inputBuffer_frequency.size() == FFTBufferSize))	//Just in case error handler is off
			{
				///////////////////////////////////////////////////////////////////////////////
				// Calculate auxiliary arrays size, necessary to use the Takuya OOURA library
				///////////////////////////////////////////////////////////////////////////////
				//Prepare the FFT						
				std::vector<int> ip(ip_size);	//Define the auxiliary array ip 
				std::vector<double> w(w_size);	//Define the auxiliary array w			
				ip[0] = 0;						//w[],ip[] are initialized if ip[0] == 0.

				///////////////
				// Make IFFT //
				///////////////				
				std::vector<double> outBuffer_temp(inputBuffer_frequency.begin(), inputBuffer_frequency.end());		//Convert to double
				cdft(FFTBufferSize, -1, outBuffer_temp.data(), ip.data(), w.data());								//Make the FFT-1

				////////////////////
				// Prepare Output //
				////////////////////
				ProcessOutputBuffer_IFFT_OverlapAddMethod(outBuffer_temp, outputBuffer_time);
			}
		}

		/** \brief This method check if a number is a power of 2
		*	\param [in] integer to check
		*	\param [out] return true if the number is power of two
		*/
		/*static bool CalculateIsPowerOfTwo(int x)		
		{
			return (x != 0) && ((x & (x - 1)) == 0);
		}*/

	private:
		// ATTRIBUTES	
		int inputSize;			//Size of the inputs buffer		
		int IRSize;				//Size of the AmbiIR buffer
		int FFTBufferSize;		//Size of the outputbuffer and zeropadding buffers	
		double normalizeCoef;		//Coef to normalize the Inverse FFT
		int ip_size;			//Size of the auxiliary array ip;
		int w_size;				//Size of the auxiliary array w;	
		bool setupDone;			//It's true when setup has been called at least once
		std::vector<double> storageBuffer;		//To store the results of the convolution


		// METHODS 	

		// brief This method copies the input vector into an array and insert the imaginary part.
		static void ProcessAddImaginaryPart(const std::vector<float>& input, std::vector<double>& output)
		{
			ASSERT(output.size() >= 2 * input.size(), RESULT_ERROR_BADSIZE, "Output buffer size must be at least twice the input buffer size when adding imaginary part in frequency convolver", "");

			for (int i = 0; i < input.size(); i++)
			{
				output[2 * i] = static_cast<double> (input[i]);		
			}
		}


		//This method copy the FFT-1 output array into the storage vector, remove the imaginary part and normalize the output.			
		void ProcessOutputBuffer_IFFT_OverlapAddMethod(std::vector<double>& input_ConvResultBuffer, std::vector<float>& outBuffer)
		{
			//Prepare the outbuffer
			if (outBuffer.size() < inputSize)
			{
				outBuffer.resize(inputSize);
			}
			//Check buffer sizes	
			ASSERT(outBuffer.size() == inputSize, RESULT_ERROR_BADSIZE, "OutBuffer size has to be zero or equal to the input size indicated by the setup method", "");

			int outBufferSize = inputSize;	//Locar var to move throught the outbuffer

			//Fill out the output signal buffer
			for (int i = 0; i < outBufferSize; i++)
			{
				if (i < storageBuffer.size()) {
					outBuffer[i] = static_cast<float>(storageBuffer[i] + CalculateRoundToZero(input_ConvResultBuffer[2 * i] * normalizeCoef));
				}
				else
				{
					outBuffer[i] = static_cast<float>(CalculateRoundToZero(input_ConvResultBuffer[2 * i] * normalizeCoef));
				}
			}
			//Fill out the storage buffer to be used in the next call
			std::vector<double> temp;
			temp.reserve(0.5f * input_ConvResultBuffer.size() - outBufferSize);
			int inputConvResult_size = 0.5f * input_ConvResultBuffer.size();	//Locar var to move to the end of the input_ConvResultBuffer
			for (int i = outBufferSize; i < inputConvResult_size; i++)
			{
				if (i < storageBuffer.size())
				{
					temp.push_back(storageBuffer[i] + CalculateRoundToZero(input_ConvResultBuffer[2 * i] * normalizeCoef));
				}
				else
				{
					temp.push_back(CalculateRoundToZero(input_ConvResultBuffer[2 * i] * normalizeCoef));
				}
			}
			//storageBuffer.swap(temp);				//To use in C++03
			storageBuffer = std::move(temp);			//To use in C++11
		}

		//This method rounds to zero a value that is very close to zero.
		static double CalculateRoundToZero(double number)
		{
			if (std::abs(number) < THRESHOLD) { return 0.0f; }
			else { return number; }
		}

		////This method Round up to the next highest power of 2 
		//static int CalculateNextPowerOfTwo(int v)		
		//{
		//	v--;
		//	v |= v >> 1;
		//	v |= v >> 2;
		//	v |= v >> 4;
		//	v |= v >> 8;
		//	v |= v >> 16;
		//	v++;
		//	return v;
		//}
	};
}//end namespace Common
#endif
