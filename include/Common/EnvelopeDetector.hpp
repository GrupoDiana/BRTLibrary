/**
* \class CEnvelopeDetector
*
* \brief Declaration of CEnvelopeDetector class
* \date	October 2023
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
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/


#ifndef _CENVELOPMENT_DETECTOR_H_
#define _CENVELOPMENT_DETECTOR_H_

#include <Common/Buffer.hpp>

//#include <Common/DynamicCompressorMono.h>
#include <Common/ErrorHandler.hpp>
#include <cmath>
//#include "Defaults.h"
#include <math.h>

#define EPSILON_ 0.00001


namespace Common {

	/** \details Class used to detect the envelope of an audio signal
	*/
	class CEnvelopeDetector
	{
	public:                                                             // PUBLIC METHODS

		/** \brief Default constructor
		*	\details By default, sets sampling rate to 44100Hz, attack time to 20ms and release time to 100ms
		*/
		CEnvelopeDetector() 
		{
			samplingRate = 44100;
			envelope = 0;

			SetAttackTime(20);  // m_fAttackTime -> Will be initialized with this call
			SetReleaseTime(100);  // m_fReleaseTime -> Will be initialized with this call
		}

		/** \brief Set the sample rate
		*	\param [in] samplingRate sample rate, in Hertzs
		*   \eh Nothing is reported to the error handler.
		*/
		void Setup(int _samplingRate) 
		{
			samplingRate = _samplingRate;
		}

		/** \brief Set the attack time
		*	\param [in] attackTime_ms attack time, in milliseconds
		*   \eh Nothing is reported to the error handler.
		*/
		void SetAttackTime(float attackTime_ms)
		{
			float den = attackTime_ms * samplingRate;

			if (den > EPSILON_)
				m_fAttackTime = std::exp(1000.0 * std::log(0.01) / den);
			else
				m_fAttackTime = 0;

			m_fAttackTime_ms = attackTime_ms;
		}

		/** \brief Returns the attack time in ms
		*	\retval attack attack time, in ms
		*   \eh Nothing is reported to the error handler.
		*/
		float GetAttackTime() { return m_fAttackTime_ms; }

		/** \brief Set the release time
		*	\param [in] releaseTime_ms release time, in milliseconds
		*   \eh Nothing is reported to the error handler.
		*/
		void SetReleaseTime(float releaseTime_ms)
		{
			float den = releaseTime_ms * samplingRate;

			if (den > EPSILON_)
				m_fReleaseTime = std::exp(1000.0 * std::log(0.01) / den);
			else
				m_fReleaseTime = 0;

			m_fReleaseTime_ms = releaseTime_ms;
		}

		/** \brief Returns the release time in ms
		*	\retval release release time, in ms
		*   \eh Nothing is reported to the error handler.
		*/
		float GetReleaseTime() { return m_fReleaseTime_ms; }

		/** \brief Returns the envelope sample for the input sample
		*	\param [in] input_sample input sample value
		*	\retval output_sample output sample value
		*   \eh Nothing is reported to the error handler.
		*/
		float ProcessSample(float input_sample)
		{
			input_sample = std::fabs(input_sample);

			if (input_sample > envelope) envelope = m_fAttackTime * (envelope - input_sample) + input_sample;
			else                         envelope = m_fReleaseTime * (envelope - input_sample) + input_sample;

			return envelope;
		}


	private:
		// PRIVATE ATTRIBUTES
		float envelope;					// Current envelop sample (the last sample returned by ProcessSample).	
		float samplingRate;             // Samplig Rate of the processed audio (samples per second).
		float m_fAttackTime;            // Value used in the difference equation to obtain the envelop
		float m_fReleaseTime;           // Value used in the difference equation to obtain the envelop 
		float m_fAttackTime_ms;         // Attack time in ms
		float m_fReleaseTime_ms;        // Release time in ms
	};
}//end namespace Common

#endif