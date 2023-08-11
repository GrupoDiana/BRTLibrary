#ifndef _SDN_DELAY_LINE_HPP_
#define _SDN_DELAY_LINE_HPP_


//delay line implementation for waveguides, uses an all-pass filter to extract samples
class SDNDelayLine
{
public:
	SDNDelayLine() {};
	~SDNDelayLine() {};

	void prepare(double samplerate, int maxLength, float delaySamp)
	{
		maxBufferLength = maxLength;
		circularBuffer.resize(maxBufferLength, 0);
		sampleRate = samplerate;
		delaySamples = delaySamp;
	}

	void storeInDelay(float sample)
	{
		circularBuffer[writeIndex] = sample;
	}

	float& readNextSample()
	{

		float fReadIndex = maxBufferLength + writeIndex - delaySamples;
		int readIndex = (int)fReadIndex;
		float fractionalIndex = fReadIndex - readIndex;
		const float allPassCoeff = fractionalIndex / (2.0f - fractionalIndex);

		readIndex %= maxBufferLength;
		int nextIndex = (readIndex + 1) % maxBufferLength;

		auto sample = allPassCoeff * (circularBuffer[nextIndex] - oldSample) + circularBuffer[readIndex];
		oldSample = sample;
		outSample = sample;


		return outSample;
	}


	void advanceWriteIndex()
	{
		writeIndex++;
		writeIndex %= maxBufferLength;
	}

	void setDelay(float newDelay) { delaySamples = newDelay; };

	double getSampleRate() { return sampleRate; };

private:

	CMonoBuffer<float> circularBuffer;
	float oldSample = 0;
	float outSample = 0;
	int maxBufferLength = 0;
	double sampleRate = 1;
	float delaySamples = 0;
	int writeIndex = 0;

};

#endif