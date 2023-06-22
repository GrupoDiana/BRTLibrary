/**
* \class CBiquadFilter
*
* \brief Declaration of CBiquadFilter class
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

#ifndef _CBIQUADILTER_H_
#define _CBIQUADILTER_H_

#include <Common/Buffer.hpp>
#include "CommonDefinitions.hpp"

#include <cmath>
#include <iomanip>

#define _USE_MATH_DEFINES // TODO: Test in windows! Might also be problematic for other platforms??
#define DEFAULT_SAMPLING_RATE 44100


namespace Common {

	/** \brief Type definition for specifying the type of filter
	*/
	enum T_filterType {
		LOWPASS = 0,	///< Low pass filter
		HIGHPASS = 1,	///< High pass filter
		BANDPASS = 2	///< Band pass filter
	};

	/** \brief Type definition for a vector of filter coefficients for one biquad
	*	\details Order: b0, b1, b2, a1, a2  
	*/
	typedef std::vector<float> TBiquadCoefficients; 


	/** \details This class implements a biquad filter (two poles and two zeros).
	\n Useful diagrams can be found in:	https://en.wikipedia.org/wiki/Digital_biquad_filter */
	class CBiquadFilter
	{
	public:
		///////////////////
		// PUBLIC METHODS
		//////////////////

		/** \brief Default constructor.
		*	\details By default, sets sampling frequency to 44100Hz.
		*   \eh Nothing is reported to the error handler.
		*/
		CBiquadFilter()		
		{
			// error handler: Trust in SetSamplingFreq for result

			z1_l = 0;
			z2_l = 0;
			z1_r = 0;
			z2_r = 0;

			b0 = 1;
			b1 = 0;
			b2 = 0;
			a1 = 0;
			a2 = 0;

			new_b0 = 1;
			new_b1 = 0;
			new_b2 = 0;
			new_a1 = 0;
			new_a2 = 0;
			crossfadingNeeded = false;
			new_z1_l = 0;
			new_z2_l = 0;
			new_z1_r = 0;
			new_z2_r = 0;

			SetCoefficients(1, 0, 0, 0, 0);

			generalGain = 1.0f;

			SetSamplingFreq(DEFAULT_SAMPLING_RATE);
		}


		/** \brief Set up the filter
		*	\param [in] samplingRate sampling frequency, in Hertzs
		*	\param [in] b0 coefficient b0
		*	\param [in] b1 coefficient b1
		*	\param [in] b2 coefficient b2
		*	\param [in] a1 coefficient a1
		*	\param [in] a2 coefficient a2
		*   \eh On error, an error code is reported to the error handler.
		*/		
		void Setup(float samplingRate, float b0, float b1, float b2, float a1, float a2)
		{
			samplingFreq = samplingRate;
			SetCoefficients(b0, b1, b2, a1, a2);
		}


		/** \brief Set up the filter
		*	\param [in] samplingRate sampling frequency, in Hertzs
		*	\param [in] frequency relevant frequency (cutoff or band center)
		*	\param [in] Q Q factor
		*	\param [in] filterType type of filter
		*   \eh On error, an error code is reported to the error handler.
		*/
		void Setup(float samplingRate, float frequency, float Q, T_filterType filterType)		
		{
			samplingFreq = samplingRate;
			SetCoefficients(frequency, Q, filterType);
		}

		/** \brief Set up coefficients of the filter
		*	\param [in] b0 coefficient b0
		*	\param [in] b1 coefficient b1
		*	\param [in] b2 coefficient b2
		*	\param [in] a1 coefficient a1
		*	\param [in] a2 coefficient a2
		*   \eh Nothing is reported to the error handler.
		*/
		void SetCoefficients(float b0, float b1, float b2, float a1, float a2)		
		{
			crossfadingNeeded = true;

			new_b0 = b0;
			new_b1 = b1;
			new_b2 = b2;
			new_a1 = a1;
			new_a2 = a2;

			new_z1_l = 0;
			new_z2_l = 0;
			new_z1_r = 0;
			new_z2_r = 0;
		}

		/** \brief Set up coefficients of the filter
		*	\param [in] coefficients coefficients array. Order: b0, b1, a1, a2  
		*   \eh Nothing is reported to the error handler. */
		void SetCoefficients(float *coefficients)		
		{
			//SET_RESULT(RESULT_OK, "");
			SetCoefficients(coefficients[0], coefficients[1], coefficients[2], coefficients[3], coefficients[4]);
		}

		/** \brief Set up coefficients of the filter
		*	\param [in] coefficients coefficients vector. Order: b0, b1, a1, a2  
		*   \eh Nothing is reported to the error handler. */		
		void SetCoefficients(TBiquadCoefficients& coefficients)
		{			
			if (coefficients.size() == 6) {				
				float a0 = coefficients[3];
				SetCoefficients(coefficients[0] / a0, coefficients[1] / a0, coefficients[2] / a0, coefficients[4] / a0, coefficients[5] / a0);
			}
			else if (coefficients.size() == 5) {
				SetCoefficients(coefficients[0], coefficients[1], coefficients[2], coefficients[3], coefficients[4]);
			}
			else {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "A vector with 5 or 6 coefficients was expected in BiquadFilter definition.");
			}
		}

		/** \brief Set up coefficients of the filter
		*	\details Lowpass, Highpass and Bandpass filters are Butterworth design
		*	\param [in] frequency relevant frequency (cutoff or band center)
		*	\param [in] Q Q factor
		*	\param [in] filterType type of filter
		*   \eh On error, an error code is reported to the error handler.
		*/
		void SetCoefficients(float frequency, float Q, T_filterType filterType)		
		{
			if (filterType == LOWPASS)
				SetCoefsFor_LPF(frequency, Q);

			else if (filterType == HIGHPASS)
				SetCoefsFor_HPF(frequency, Q);

			else if (filterType == BANDPASS)
				SetCoefsFor_BandPassFilter(frequency, Q);
		}

		/** \brief Set the sampling frequency at which audio samples were acquired
		*	\param [in] _samplingFreq sampling frequency, in Hertzs
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		void SetSamplingFreq(float _samplingFreq)		
		{
			if (_samplingFreq < 0.1)
			{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Sampling frequency for biquad filter is invalid");
				return;
			}

			SET_RESULT(RESULT_OK, "Sampling frequency for biquad filter succesfully set");
			samplingFreq = _samplingFreq;
		}

		/** \brief Filter the input data according to the filter setup.
		*	\param [in] inBuffer input buffer
		*	\param [out] outBuffer output buffer
		*	\param [in] addResult when true, samples resulting from the	filtering process are added to the current value of the output buffer.
		*	\pre Input and output buffers must have the same size, which should be greater than 0.
		*   \eh On error, an error code is reported to the error handler.
		*/
		void Process(CMonoBuffer<float> &inBuffer, CMonoBuffer<float> & outBuffer, bool addResult = false)		
		{
			int size = inBuffer.size();

			if (size <= 0)
			{
				//SET_RESULT(RESULT_ERROR_INVALID_PARAM, "The input buffer is empty");
				SET_RESULT(RESULT_ERROR_BADSIZE, "Attempt to process a biquad filter with an empty input buffer");
				return;
			}
			else if (size != outBuffer.size())
			{
				//SET_RESULT( RESULT_ERROR_INVALID_PARAM, "Input and output buffers size must agree" );
				SET_RESULT(RESULT_ERROR_BADSIZE, "Attempt to process a biquad filter with different sizes for input and output buffers");
				return;
			}

			//SET_RESULT(RESULT_OK, "");

			// This is expression of the biquad filter but the implementation follows a more efficient
			// approach in which only 2 delays cells are used.
			//   See schemes in: https://en.wikipedia.org/wiki/Digital_biquad_filter
			//   y(n) = b0.x(n) + b1.x(n-1) + b2.x(n-2) + a1.y(n-1) + a2.y(n-2) 	

			if (crossfadingNeeded && size > 0)  // size > 1 to avoid division by zero if size were 1 while calculating alpha
			{
				for (int c = 0; c < size; c++)
				{
					// To ensure alpha is in [0,1] we use -2 because the buffer is stereo
					double alpha = ((double)c) / ((double)(size - 1));

					double     sample = ProcessSample(inBuffer[c], a1, a2, b0, b1, b2, z1_l, z2_l);
					double new_sample = ProcessSample(inBuffer[c], new_a1, new_a2, new_b0, new_b1, new_b2, new_z1_l, new_z2_l);

					double res = sample * (1.0 - alpha) + new_sample * alpha;

					outBuffer[c] = addResult ? outBuffer[c] + res : res;
				}

				UpdateAttributesAfterCrossfading();
			}
			else
			{
				for (int c = 0; c < size; c++)
				{
					double res = ProcessSample(inBuffer[c], a1, a2, b0, b1, b2, z1_l, z2_l);
					outBuffer[c] = addResult ? outBuffer[c] + res : res;
				}
			}
			AvoidNanValues();
		}


		/**
		\overload
		*/
		void Process(CMonoBuffer<float> &buffer)		
		{
			int size = buffer.size();

			if (size <= 0)
			{
				SET_RESULT(RESULT_ERROR_BADSIZE, "Attempt to process a biquad filter with an empty input buffer");
				return;
			}

			//SET_RESULT(RESULT_OK, "Biquad filter process succesfull");

			if (crossfadingNeeded)
			{
				for (int c = 0; c < size; c++)
				{
					double alpha = ((double)c) / ((double)(size - 1));

					double     sample = ProcessSample(buffer[c], a1, a2, b0, b1, b2, z1_l, z2_l);
					double new_sample = ProcessSample(buffer[c], new_a1, new_a2, new_b0, new_b1, new_b2, new_z1_l, new_z2_l);

					buffer[c] = sample * (1.0 - alpha) + new_sample * alpha;
				}

				UpdateAttributesAfterCrossfading();
			}
			else
			{
				for (int c = 0; c < size; c++)
					buffer[c] = ProcessSample(buffer[c], a1, a2, b0, b1, b2, z1_l, z2_l);
			}

			AvoidNanValues();
		}

		/** \brief Set the gain of the filter 
		*	\param [in] _gain filter gain 
		*   \eh Nothing is reported to the error handler.
		*/
		void SetGeneralGain(float _gain)		
		{
			generalGain = _gain;
		}

		/** \brief Get the gain of the filter
		*	\retval gain filter gain
		*   \eh Nothing is reported to the error handler.
		*/
		float GetGeneralGain()
		{
			return generalGain;
		}


	private:
		////////////////////
		// PRIVATE METHODS
		///////////////////
		
		/// Prevent the filter from ending up in unstable states
		void AvoidNanValues()		
		{
			// FIXME: IIRs filters can eventually end up in a non stable state that can lead the filter output
			//      to +/-Inf. To prevent this situation we reset the delay cells of the filter when this happens.
			//    A known scenario in whinch this happens is this: In the binaural test app when two sound sources
			//   are played at the same time using anechoic an reverb and one of the sourcers is moved beyond the 
			//   far distances threshold, the LPF of the distances can end up with this unstable state.

			if (std::isnan(z1_l)) z1_l = 0;
			if (std::isnan(z2_l)) z2_l = 0;
			if (std::isnan(z1_r)) z1_r = 0;
			if (std::isnan(z2_r)) z2_r = 0;

			if (std::isnan(new_z1_l)) new_z1_l = 0;
			if (std::isnan(new_z2_l)) new_z2_l = 0;
			if (std::isnan(new_z1_r)) new_z1_r = 0;
			if (std::isnan(new_z2_r)) new_z2_r = 0;
		}

		/// Calculates the coefficients of a biquad band-pass filter.
		bool SetCoefsFor_BandPassFilter(double centerFreqHz, double Q)		
		{
			if (samplingFreq < 0.1 || Q < 0.0000001 || centerFreqHz > samplingFreq / 2.0) // To prevent aliasing problems
			{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Cutoff frequency of biquad (bandpass) filter is higher than Nyquist frequency");
				return false;
			}

			try // -> To handle division by 0
			{
				double K = std::tan(M_PI * centerFreqHz / samplingFreq);

				double norm = 1 / (1 + K / Q + K * K);
				double _b0 = K / Q * norm;
				double _b1 = 0;
				double _b2 = -_b0;
				double _a1 = 2 * (K * K - 1) * norm;
				double _a2 = (1 - K / Q + K * K) * norm;

				SetCoefficients(_b0, _b1, _b2, _a1, _a2);

				SET_RESULT(RESULT_OK, "Bandpass filter coefficients of biquad filter succesfully set");

				return true;
			}
			catch (std::exception e)
			{
				//SET_RESULT(RESULT_ERROR_INVALID_PARAM, "");
				SET_RESULT(RESULT_ERROR_DIVBYZERO, "Division by zero setting coefficients for bandpass biquad filter");
				return false;
			}
		}

		/// Calculate the coefficients of a biquad low-pass filter.
		bool SetCoefsFor_LPF(double cutoffFreq, double Q)		
		{
			if (samplingFreq < 0.1 || cutoffFreq > samplingFreq / 2.0) // To prevent aliasing problems
			{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Cutoff frequency of biquad (LPF) filter is higher than Nyquist frequency");
				return false;
			}

			try // -> To handle division by 0
			{
				double K = std::tan(M_PI * cutoffFreq / samplingFreq);

				double norm = 1 / (1 + K / Q + K * K);
				double _b0 = K * K * norm;
				double _b1 = 2 * _b0;
				double _b2 = _b0;
				double _a1 = 2 * (K * K - 1) * norm;
				double _a2 = (1 - K / Q + K * K) * norm;

				SetCoefficients(_b0, _b1, _b2, _a1, _a2);

				//SET_RESULT(RESULT_OK, "LPF filter coefficients of biquad filter succesfully set");

				return true;
			}
			catch (std::exception e)
			{
				//SET_RESULT(RESULT_ERROR_INVALID_PARAM, "");
				SET_RESULT(RESULT_ERROR_DIVBYZERO, "Division by zero setting coefficients for LPF biquad filter");
				return false;
			}
		}
		
		/// Calculates the coefficients of a biquad high-pass filter.     		
		bool SetCoefsFor_HPF(double cutoffFreq, double Q)
		{
			if (samplingFreq < 0.1 || cutoffFreq > samplingFreq / 2.0) // To prevent aliasing problems
			{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Cutoff frequency of biquad (HPF) filter is higher than Nyquist frequency");
				return false;
			}

			try // -> To handle division by 0
			{
				double K = std::tan(M_PI * cutoffFreq / samplingFreq);

				double norm = 1 / (1 + K / Q + K * K);
				double _b0 = 1 * norm;
				double _b1 = -2 * _b0;
				double _b2 = _b0;
				double _a1 = 2 * (K * K - 1) * norm;
				double _a2 = (1 - K / Q + K * K) * norm;

				SetCoefficients(_b0, _b1, _b2, _a1, _a2);

				SET_RESULT(RESULT_OK, "HPF filter coefficients of biquad filter succesfully set");

				return true;
			}
			catch (std::exception e)
			{
				//SET_RESULT(RESULT_ERROR_INVALID_PARAM, "");
				SET_RESULT(RESULT_ERROR_DIVBYZERO, "Division by zero setting coefficients for HPF biquad filter");
				return false;
			}
		}

		// Does the basic processing of the biquad filter. Receives the current sample, the coefficients and the delayed samples
		// Returns the result of the biquad filter 
		double ProcessSample(const double sample, const double a1, const double a2, const double b0, const double b1, const double b2, double &z1, double &z2)		
		{
			// This is expression of the biquad filter but the implementation follows a more efficient
			// approach in which only 2 delays cells are used.
			//   See schemes in: https://en.wikipedia.org/wiki/Digital_biquad_filter
			//   y(n) = b0.x(n) + b1.x(n-1) + b2.x(n-2) + a1.y(n-1) + a2.y(n-2) 	

			double m_l = sample - a1 * z1 - a2 * z2;
			double res = generalGain * (float)(b0 * m_l + b1 * z1 + b2 * z2);
			z2 = z1;
			z1 = m_l;
			return res;
		}

		/// Set current coefficients to new cofficients and updates the delay cells and the crossfadingNeeded attribute.
		void UpdateAttributesAfterCrossfading()
		{
			crossfadingNeeded = false;

			z1_l = new_z1_l;
			z2_l = new_z2_l;
			z1_r = new_z1_r;
			z2_r = new_z2_r;

			b0 = new_b0;
			b1 = new_b1;
			b2 = new_b2;
			a1 = new_a1;
			a2 = new_a2;

		}

		////////////////
		// ATTRIBUTES
		////////////////
		float generalGain;                                              // Gain applied to every sample obtained with Process

		double samplingFreq;                                            // Keep the sampling rate at which audio samples were taken
		double z1_l, z2_l, z1_r, z2_r;                                  // Keep last values to implement the delays of the filter (left and right channels)
		double b0, b1, b2, a1, a2;                                      // Coeficients of the Butterworth filter

		double new_b0, new_b1, new_b2, new_a1, new_a2;                  // New coefficients to implement cross fading
		double new_z1_l, new_z2_l, new_z1_r, new_z2_r;                  // Keep last values to implement the delays of the filter (left and right channels)
		bool   crossfadingNeeded;                                       // True when cross fading must be applied in the next frame
	};
}
#endif
