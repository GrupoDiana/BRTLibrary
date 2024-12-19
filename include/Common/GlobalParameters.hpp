/**
* \class CGlobalParamenter
*
* \brief Declaration Global parameters
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

#ifndef _CGLOBAL_PARAMETERS_HPP_
#define _CGLOBAL_PARAMETERS_HPP_

//#include <iostream>

#ifndef DEFAULT_SAMPLE_RATE
#define DEFAULT_SAMPLE_RATE 44100						///< Default sample rate in samples/seconds
#endif
#ifndef DEFAULT_BUFFER_SIZE
#define DEFAULT_BUFFER_SIZE 512							///< Default buffer size in samples
#endif
#ifndef DEFAULT_REVERB_ATTENUATION_DB
#define DEFAULT_REVERB_ATTENUATION_DB -3.01f			///< Default reverb attenuation with distance, in decibels
#endif
#ifndef DEFAULT_ANECHOIC_ATTENUATION_DB
#define DEFAULT_ANECHOIC_ATTENUATION_DB	-6.0206f		///< log10f(0.5f) * 20.0f Default anechoic attenuation with distance, in decibels 
#endif
#ifndef DEFAULT_SOUND_SPEED
#define DEFAULT_SOUND_SPEED	343.0f						///< Default sound speed, in meters per second (m/s)
#endif 
#ifndef DISTANCE_MODEL_THRESHOLD_NEAR
#define DISTANCE_MODEL_THRESHOLD_NEAR 2 				///< Reference distance for the near-distance threshold, in meters
#endif 
#ifndef DISTANCE_MODEL_THRESHOLD_FAR
#define DISTANCE_MODEL_THRESHOLD_FAR 15					///< Far-distance threshold, in meters
#endif
#ifndef EPSILON_ATTACK_SAMPLES
#define EPSILON_ATTACK_SAMPLES  0.001f					///< Attack sample lower limit attenuation in simple attenuation distance (used in ApplyGainExponentially method)
#endif
#ifndef REFERENCE_DISTANCE_ATTENUATION
#define REFERENCE_DISTANCE_ATTENUATION 1				///< Reference distance for attenuation by distance in meters	
#endif
#ifndef ATTACK_TIME_DISTANCE_ATTENUATION
#define ATTACK_TIME_DISTANCE_ATTENUATION 100			///< Attack time for gradual attenuation in simple attenuation distance (used in ApplyGainExponentially method)
#endif
#ifndef MINIMUM_DISTANCE_SOURCE_LISTENER
#define MINIMUM_DISTANCE_SOURCE_LISTENER 0.0001f		///< Minimun distance allowed betwwen source and listener in metres. It only serves to solve the numerical problem
#endif 


#include <Common/Transform.hpp>
#include <Common/Vector3.hpp>
#include <Common/CommonDefinitions.hpp>

namespace Common {
		
	// Comes from Audiostate and Magnitudes of the 3D-Tune-In toolkit
	class CGlobalParameters
	{
		//Monostate Patttern
	private:
		static inline int	bufferSize				= DEFAULT_BUFFER_SIZE;
		static inline int	sampleRate				= DEFAULT_SAMPLE_RATE;
		static inline float anechoicAttenuationDB	= DEFAULT_ANECHOIC_ATTENUATION_DB;				// Constant for modeling the attenuation due to distance in anechoic process, in decibel units
		static inline float reverbAttenuationDB		= DEFAULT_REVERB_ATTENUATION_DB;				// Constant for modeling the attenuation due to distance in reverb process, in decibel units
		static inline float soundSpeed				= DEFAULT_SOUND_SPEED;							// Constant for modeling sound speed

	public:
		CGlobalParameters() = default;


		////////////////////////
		// GET - SET METHODS	
		////////////////////////
		// Set the buffer size
		void SetBufferSize(int _bufferSize) { 
			bufferSize = _bufferSize;
			if (!CalculateIsPowerOfTwo(_bufferSize)) {
				SET_RESULT(RESULT_WARNING, "This buffer size is not a power of two, so processing will not be as efficient as it could be. Convolution and FFT operations will be done on the next largest number that is a power of two.");
			} else {
				SET_RESULT(RESULT_OK, "This buffer size has been set correctly.");
			}

		}
		// Get buffer size
		int GetBufferSize()	const { return bufferSize; }				

		void SetSampleRate(int _sampleRate) {
			sampleRate = _sampleRate;
		}
		int GetSampleRate() const { return sampleRate; }
		
						
		
		// Set distance attenuation constant for anechoic process  
		void SetAnechoicDistanceAttenuation(float _anechoicAttenuationDB)
		{
			//SetAttenuation(anechoicAttenuation, anechoicAttenuationDB, anechoicAttenuationGAIN, units);
			if (_anechoicAttenuationDB > 0.0f)
			{
				//SET_RESULT(RESULT_ERROR_PHYSICS, "Attenuation constant in decibels must be a negative value");
				return;
			}
			anechoicAttenuationDB = _anechoicAttenuationDB;

			//SET_RESULT(RESULT_OK, "Anechoic distance attenuation succesfully set");
		}
		// Get distance attenuation constant for anechoic process	
		float GetAnechoicDistanceAttenuation() const
		{
			return anechoicAttenuationDB;
		}

		// Set distance attenuation constant for reverb process 
		void SetReverbDistanceAttenuation(float _reverbAttenuationDB)
		{
			//SetAttenuation(reverbAttenuation, reverbAttenuationDB, reverbAttenuationGAIN, units);
			if (_reverbAttenuationDB > 0.0f)
			{
				//SET_RESULT(RESULT_ERROR_PHYSICS, "Attenuation constant in decibels must be a negative value");
				return;
			}
			reverbAttenuationDB = _reverbAttenuationDB;

			//SET_RESULT(RESULT_OK, "Reverb distance attenuation succesfully set");
		}
	
		// Get distance attenuation constant for anechoic process 		
		float GetReverbDistanceAttenuation() const
		{
			return reverbAttenuationDB;
		}

		// Set sound speed in m/s
		void SetSoundSpeed(float _soundSpeed)
		{
			if (_soundSpeed < 0.0f)
			{
				//SET_RESULT(RESULT_ERROR_PHYSICS, "Sound speed must be a positive value");
				return;
			}

			soundSpeed = _soundSpeed;

			//SET_RESULT(RESULT_OK, "Sound speed succesfully set");

		}

		// Get sound speed in m/s
		float GetSoundSpeed() const
		{
			return soundSpeed;
		}
	};    
}
#endif
