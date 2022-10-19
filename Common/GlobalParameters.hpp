#ifndef _CGLOBAL_PARAMETERS_H_
#define _CGLOBAL_PARAMETERS_H_

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
#define DISTANCE_MODEL_THRESHOLD_NEAR 1.95				///< Reference distance and near-distance threshold, in meters
#endif 
#ifndef DISTANCE_MODEL_THRESHOLD_FAR
#define DISTANCE_MODEL_THRESHOLD_FAR 15					///< Far-distance threshold, in meters
#endif
#ifndef EPSILON_ATTACK_SAMPLES
#define EPSILON_ATTACK_SAMPLES  0.001f					///< Attack sample lower limit attenuation in simple attenuation distance (used in ApplyGainExponentially method)
#endif
#ifndef ATTACK_TIME_DISTANCE_ATTENUATION
#define ATTACK_TIME_DISTANCE_ATTENUATION 100			///< Attack time for gradual attenuation in simple attenuation distance (used in ApplyGainExponentially method)
#endif
#ifndef DEFAULT_LISTENER_HEAD_RADIOUS
#define DEFAULT_LISTENER_HEAD_RADIOUS  0.0875f
#endif 

#include <Common/Transform.h>
#include <Common/Vector3.h>

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
		}
		// Get buffer size
		int GetBufferSize()	{ return bufferSize; }				

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