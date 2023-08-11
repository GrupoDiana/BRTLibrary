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

	SDNNode* getStart() { return startNode; };
	SDNNode* getEnd() { return endNode; };
	float getDistance() { return distance; };
	float getAttenuation() { return attenuation; };
	void setDistance(float newDist) 
	{
		oldDistance = distance;
		targetDistance = newDist;
		interpolationIndex = 0;
	}
	void setAttenuation(float newValue) { attenuation = newValue; };

	void prepare(double samplerate, SDNNode* start, SDNNode* end, float dist)
	{
		startNode = start;
		endNode = end;
		distance = dist;
		oldDistance = dist;
		targetDistance = dist;
		toSamplesConst = samplerate / SDNParameters::SOUND_SPEED;
		interpDurationInSamples = (int)(samplerate * SDNParameters::SMOOTHING_TIME_SECONDS);

		delay.prepare(samplerate, ceilf(sqrtf(SDNParameters::ROOM_MAX_DIMENSION * SDNParameters::ROOM_MAX_DIMENSION * 3.0f) * toSamplesConst),
			distance * toSamplesConst);
	}

	float& getCurrentSample() { return delay.readNextSample(); };

	//push sample into the delay-line
	void pushNextSample(float sample)
	{
		if (attenuation != 1.0f) sample *= attenuation;
		delay.storeInDelay(sample);
	}

	//advance delay-line by one sample
	void stepForward() { delay.advanceWriteIndex(); };

	void interpolateDistance()
	{
		distance = oldDistance + ((targetDistance - oldDistance) * (interpolationIndex / (float)interpDurationInSamples));
		delay.setDelay(distance * toSamplesConst);
		interpolationIndex++;
	}

	bool isInterpolating() { return interpolationIndex <= interpDurationInSamples; }


private:
	SDNNode* startNode;
	SDNNode* endNode;
	SDNDelayLine delay;

	float distance, oldDistance, targetDistance;
	int interpolationIndex = 0;
	int interpDurationInSamples = 0;
	float toSamplesConst = 0;
	float attenuation;

};

#endif