#ifndef _SDN_WAVEGUIDE_HPP_
#define _SDN_WAVEGUIDE_HPP_

#include <EnvironmentModels/SDNenv/SDNParameters.hpp>
#include <EnvironmentModels/SDNenv/SDNNode.hpp>
#include <EnvironmentModels/SDNenv/SDNDelayLine.hpp>

class WaveGuide
{
public:
	WaveGuide() {};
	~WaveGuide() {};

	SDNNode* GetStart() { return startNode; };
	SDNNode* GetEnd() { return endNode; };
	float GetDistance() { return distance; };
	float GetAttenuation() { return attenuation; };

	/**
	* @brief Changes the length of the waveguide, to reach the new value InterpolateDistance() needs to be called
	*		 until IsInterpolating() returns false
	* @param newDist New waveguide length in meters
	*/
	void SetDistance(float newDist) 
	{
		oldDistance = distance;
		targetDistance = newDist;
		interpolationIndex = 0;
	}

	/**
	* @brief Changes the attenuation applied by the waveguide
	* @param newValue New attenuation as a multiplier of the input samples. Any value >1 will result in an amplification
	*/
	void SetAttenuation(float newValue) { attenuation = newValue; };


	/**
	* @brief Initialize the WaveGuide variables
	* @param samplerate Samplerate of the delay line
	* @param start Pointer to the start node of the waveguide
	* @param end Pointer to the end node of the waveguide
	* @param distance Length of the waveguide in meters
	*/
	void Prepare(double samplerate, SDNNode* start, SDNNode* end, float distance)
	{
		startNode = start;
		endNode = end;
		this->distance = distance;
		oldDistance = distance;
		targetDistance = distance;
		toSamplesConstant = samplerate / SDNParameters::SOUND_SPEED;
		interpolationDurationInSamples = (int)(samplerate * SDNParameters::SMOOTHING_TIME_SECONDS);

		//max length of the delay is calculated from the geometry of the room 
		delay.Prepare(samplerate, ceilf(sqrtf(SDNParameters::ROOM_MAX_DIMENSION * SDNParameters::ROOM_MAX_DIMENSION * 3.0f) * toSamplesConstant),
			distance * toSamplesConstant);
	}

	/**
	* @brief Reads the sample at the output of the delay line for the current time step
	* @return Returns the reference to the sample at the output of the delay line for the current time step
	*/
	float& GetCurrentSample() { return delay.ReadNextSample(); };

	/**
	* @brief Push sample into the delay-line
	* @param sample Sample to save into the delay-line
	*/
	void PushNextSample(float sample)
	{
		if (attenuation != 1.0f) sample *= attenuation;
		delay.StoreInDelay(sample);
	}

	/**
	* @brief Advance the delay-line by one sample
	*/
	void StepForward() { delay.AdvanceWriteIndex(); };

	/**
	* @brief Interpolates between the old distance and the new target distance over a duration in samples
	*/
	void InterpolateDistance()
	{
		distance = oldDistance + ((targetDistance - oldDistance) * (interpolationIndex / (float)interpolationDurationInSamples));
		delay.SetDelay(distance * toSamplesConstant);
		interpolationIndex++;
	}

	/**
	* @brief Checks if the interpolation between waveguide lengths has finished
	* @return True if the interpolation is still occurring
	*/
	bool IsInterpolating() { return interpolationIndex <= interpolationDurationInSamples; }


private:
	SDNNode* startNode;
	SDNNode* endNode;
	SDNDelayLine delay;

	float distance, oldDistance, targetDistance;
	int interpolationIndex = 0;
	int interpolationDurationInSamples = 0;
	float toSamplesConstant = 0;
	float attenuation;

};

#endif