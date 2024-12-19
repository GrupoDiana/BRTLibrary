/**
* \class SDNDelayLine
*
* \brief   		   
* \date Sep 2023
* 
* \authors  Developer's team (University of Milan), in alphabetical order: F. Avanzini, D. Fantini , M. Fontana, G. Presti,
* Coordinated by F. Avanzini (University of Milan) ||
*
* \b Contact: federico.avanzini@unimi.it
*
* \b Copyright: University of Milan - 2023
*
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM (https://www.sonicom.eu/) ||
* \b Website: https://www.sonicom.eu/
*
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*
*/
#ifndef _SDN_DELAY_LINE_HPP_
#define _SDN_DELAY_LINE_HPP_


// Delay line implementation for waveguides, uses an all-pass filter to extract samples
class SDNDelayLine
{
public:
	SDNDelayLine() {};
	~SDNDelayLine() {};

	/**
	* @brief Initialize the delay line variables
	* @param samplerate Samplerate of the delay line
	* @param maxLength Size of the delay line buffer
	* @param delaySamp Initial delay value in samples, can be a fractional value
	*/
	void Prepare(double samplerate, int maxLength, float delaySamp)
	{
		maxBufferLength = maxLength;
		circularBuffer.resize(maxBufferLength, 0);
		sampleRate = samplerate;
		delaySamples = delaySamp;
	}

	/**
	* @brief Save a sample at the current write index
	* @param sample Sample to save into the buffer
	*/
	void StoreInDelay(float sample)
	{
		circularBuffer[writeIndex] = sample;
	}

	/**
	* @brief Reads the sample at the output of the delay line for the current time step
	* @return Returns the reference to the sample at the output of the delay line for the current time step
	*/
	float& ReadNextSample()
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

	/**
	* @brief Advance the write index by one sample
	*/
	void AdvanceWriteIndex()
	{
		writeIndex++;
		writeIndex %= maxBufferLength;
	}

	/**
	* @brief Set a new delay
	* @param newDelay New delay in number of samples, can be a fractional value
	*/
	void SetDelay(float newDelay) { delaySamples = newDelay; };

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