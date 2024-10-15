/**
* \class ScatteringNode
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
*
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*
*/

#ifndef _SDN_SCATTERING_NODE_HPP_
#define _SDN_SCATTERING_NODE_HPP_

#include <EnvironmentModels/SDNenv/SDNNode.hpp>
#include <EnvironmentModels/SDNenv/WaveGuide.hpp>
#include <EnvironmentModels/SDNEnv/SDNUtils.hpp>

// Scattering node in SDN architecture, uses IIR filters to simulate material absorption
class ScatteringNode : public SDNNode
{
public:
	ScatteringNode() { SetPosition({ 0, 0, 0 }); };
	~ScatteringNode() {};

	/**
	* @brief Initialize the scattering node variables
	* @param samplerate Samplerate of the scattering node
	* @param position Postion of the scattering node as a CVector3
	* @param nOfConnections Number of 2 way connections with other nodes in the model
	* @param sourceNodeGuide Pointer to the waveguide from the source to the node
	* @param nodeListenerGuide Pointer to the waveguide from the node to the listener
	*/
	void Init(double samplerate, Common::CVector3 position, int nOfConnections,
		WaveGuide* sourceNodeGuide, WaveGuide* nodeListenerGuide)
	{
		SetPosition(position);
		inWaveguides = std::vector<WaveGuide*>(nOfConnections, 0);
		outWaveguides = std::vector<WaveGuide*>(nOfConnections, 0);
		scatteringCoefficient = 2.0f / nOfConnections;
		this->nOfConnections = nOfConnections;
		sourceGuide = sourceNodeGuide;
		listenerGuide = nodeListenerGuide;

		inSamples = std::vector<float>(nOfConnections);
		toListenerSample = 0;
		totalLoudness = 0.0f;
		
		wallFilters = std::vector<IIRFilter>(nOfConnections);

		std::vector<std::vector<double>> coeffs = SDNUtils::getWallFilterCoeffs(samplerate, absorption[0],
			absorption[1], absorption[2], absorption[3], absorption[4], absorption[5], absorption[6], absorption[7]);

		a = coeffs[1];
		b = coeffs[0];

		for (IIRFilter& filter : wallFilters)
		{
			filter.init(samplerate, &a, &b);
		}

	}

	/**
	* @brief Scatter incoming samples in the current time step to the outgoing waveguides
	*/
	void Process()
	{

		totalLoudness = 0;
		float sourceSample = sourceGuide->GetCurrentSample() / 2.0f;

		for (int i = 0; i < nOfConnections; i++)
		{
			inSamples[i] = inWaveguides[i]->GetCurrentSample() + sourceSample;

			totalLoudness += inSamples[i];
		}

		GetAllOutSamples();

	}

	/**
	* @brief Update the local absorption values array
	* @param newValue New frequency absorption value in the range [0, 1]
	* @param index Octave band index in the [125, 500, 1000, 2000, 4000, 8000, 16000]Hz array
	*/
	void SetFreqAbsorption(float newValue, int index)
	{
		absorption[index] = newValue;
		newAbsorption = true;
	}
	
	/**
	 * @brief Set the frequency absorption values array
	 * @param newValues New frequency absorption values array
	 */
	void SetFreqAbsortion(std::vector<float> _newValues) {		
		if (_newValues.size() != SDNParameters::NUM_FREQ) {
			return;
		}
		std::copy(_newValues.begin(), _newValues.end(), absorption);
		newAbsorption = true;
	}

	/**
	* @brief Calculate filter coefficients from local absorption values array
	* @param samplerate Sample rate of the filter
	*/
	void updateFilterCoeffs(double samplerate)
	{
		std::vector<std::vector<double>> coeffs = SDNUtils::getWallFilterCoeffs(samplerate, absorption[0],
			absorption[1], absorption[2], absorption[3], absorption[4], absorption[5], absorption[6], absorption[7]);

		a = coeffs[1];
		b = coeffs[0];

		for (IIRFilter& filter : wallFilters)
		{
			filter.clearMemory();
		}

		newAbsorption = false;
	}

	/**
	* @brief Check if an absorption value for any octave band has changed and has not been processed yet
	* @return True if an absorption value for any octave band has changed and has not been processed yet
	*/
	bool HasNewAbsorption() { return newAbsorption; }

	std::vector<WaveGuide*> inWaveguides, outWaveguides;

private:

	//send correctly scattered and filtered sample in the waveguides
	void GetAllOutSamples()
	{

		toListenerSample = 0;
		int inSampleIndex = 0;

		for (int i = 0; i < nOfConnections; i++)
		{

			if (inWaveguides[i]->GetStart() == outWaveguides[i]->GetEnd()) //should always enter here by vector construction
			{
				inSampleIndex = i;
			}
			else
			{
				for (int j = 0; j < nOfConnections; j++)
				{
					if (inWaveguides[j]->GetStart() == outWaveguides[i]->GetEnd())
					{
						inSampleIndex = j;
						break;
					}
				}
			}

			float chInSample = inSamples[inSampleIndex];

			// apply scattering operation to the current sample
			float chSample = totalLoudness - chInSample;
			chSample = (chSample * scatteringCoefficient) + (chInSample * (scatteringCoefficient - 1));

			wallFilters[i].process(chSample);

			outWaveguides[i]->PushNextSample(chSample);
			toListenerSample += chSample;
		}

		toListenerSample *= scatteringCoefficient;
		listenerGuide->PushNextSample(toListenerSample);

	}

	std::vector<float> inSamples;
	float toListenerSample = 0;
	float totalLoudness = 0;

	std::vector<IIRFilter> wallFilters;
	WaveGuide* sourceGuide = 0;
	WaveGuide* listenerGuide = 0;

	int nOfConnections = 0;
	float scatteringCoefficient = 0.0f;

	float absorption[SDNParameters::NUM_FREQ] = { 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f };
	bool newAbsorption = false;
	std::vector<double> a, b;

};

#endif