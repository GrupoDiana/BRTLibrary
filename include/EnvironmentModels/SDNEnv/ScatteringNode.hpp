#ifndef _SDN_SCATTERING_NODE_HPP_
#define _SDN_SCATTERING_NODE_HPP_

#include <EnvironmentModels/SDNenv/SDNNode.hpp>
#include <EnvironmentModels/SDNenv/WaveGuide.hpp>

//Scattering node in SDN srchitecture uses IIR filters to simulate material absorption
class ScatteringNode : public SDNNode
{
public:
	ScatteringNode() { setPosition({ 0, 0, 0 }); };
	~ScatteringNode() {};

	void init(double samplerate, Common::CVector3 position, int nOfConnections,
		WaveGuide* sourceNodeGuide, WaveGuide* nodeListenerGuide)
	{
		setPosition(position);
		inWaveguides = std::vector<WaveGuide*>(nOfConnections, 0);
		outWaveguides = std::vector<WaveGuide*>(nOfConnections, 0);
		scatteringCoeff = 2.0f / nOfConnections;
		this->nOfConnections = nOfConnections;
		sourceGuide = sourceNodeGuide;
		listenerGuide = nodeListenerGuide;

		inSamples = std::vector<float>(nOfConnections);
		toListenerSample = 0;
		totLoudness = 0.0f;
		
		//wallFilters = std::vector<IIRBase>(nOfConnections);

		//std::vector<std::vector<double>> coeffs = dspUtils::getWallFilterCoeffs(samplerate, absorption[0],
		//	absorption[1], absorption[2], absorption[3], absorption[4], absorption[5], absorption[6], absorption[7]);

		//a = coeffs[1];
		//b = coeffs[0];

		//;
		//for (IIRBase& filter : wallFilters)
		//{
		//	filter.init(samplerate, &a, &b);
		//}

	}

	//gather all incoming samples and process them
	void process()
	{

		totLoudness = 0;
		float sourceSample = sourceGuide->getCurrentSample() / 2.0f;

		for (int i = 0; i < nOfConnections; i++)
		{
			inSamples[i] = inWaveguides[i]->getCurrentSample() + sourceSample;

			totLoudness += inSamples[i];
		}

		getAllOutSamples();

	}

	//update local absorption values array
	void setFreqAbsorption(float newValue, int index)
	{
		absorption[index] = newValue;
		newAbsorption = true;
	}
	//calculate filter coefficients from local absorption values array
	//void updateFilterCoeffs(double samplerate)
	//{
	//	std::vector<std::vector<double>> coeffs = dspUtils::getWallFilterCoeffs(samplerate, absorption[0],
	//		absorption[1], absorption[2], absorption[3], absorption[4], absorption[5], absorption[6], absorption[7]);

	//	a = coeffs[1];
	//	b = coeffs[0];

	//	for (IIRBase& filter : wallFilters)
	//	{
	//		filter.clearMemory();
	//	}

	//	newAbsorption = false;
	//}

	bool hasNewAbsorption() { return newAbsorption; }

	std::vector<WaveGuide*> inWaveguides, outWaveguides;

private:

	//send correctly scattered and filtered sample in the waveguides
	void getAllOutSamples()
	{

		toListenerSample = 0;
		int inSampleIndex = 0;

		for (int i = 0; i < nOfConnections; i++)
		{

			if (inWaveguides[i]->getStart() == outWaveguides[i]->getEnd()) //should always enter here by vector construction
			{
				inSampleIndex = i;
			}
			else
			{
				for (int j = 0; j < nOfConnections; j++)
				{
					if (inWaveguides[j]->getStart() == outWaveguides[i]->getEnd())
					{
						inSampleIndex = j;
						break;
					}
				}
			}

			float chInSample = inSamples[inSampleIndex];

			float chSample = totLoudness - chInSample;
			chSample = (chSample * scatteringCoeff) + (chInSample * (scatteringCoeff - 1));

			//wallFilters[i].process(chSample);
			chSample *= 0.8; //temp absorb until filters are implemented

			outWaveguides[i]->pushNextSample(chSample);
			toListenerSample += chSample;
		}

		toListenerSample *= scatteringCoeff;
		listenerGuide->pushNextSample(toListenerSample);

	}

	std::vector<float> inSamples;
	float toListenerSample = 0;
	float totLoudness = 0;

	//std::vector<IIRBase> wallFilters;
	WaveGuide* sourceGuide = 0;
	WaveGuide* listenerGuide = 0;

	int nOfConnections = 0;
	float scatteringCoeff = 0.0f;

	float absorption[SDNParameters::NUM_FREQ] = { 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f };
	bool newAbsorption = false;
	std::vector<double> a, b;

};

#endif