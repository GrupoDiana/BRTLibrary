/**
* \class CAudioMixer
*
* \brief Declaration of CAudioMixer
* \date	Feb 2025
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga)||
* \b Contact: areyes@uma.es
*
* \b Copyright: University of Malaga
* 
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM (https://www.sonicom.eu/) ||
*
* \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreements no. 101017743
* 
* This class is part of the Binaural Rendering Toolbox (BRT), coordinated by A. Reyes-Lecuona (areyes@uma.es) and L. Picinali (l.picinali@imperial.ac.uk)
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/

#ifndef _CAUDIO_MIXER_HPP_
#define _CAUDIO_MIXER_HPP_

#include <Common/Buffer.hpp>
#include <algorithm>
#include <vector>

namespace Common {

class CAudioMixer {
public:
	CAudioMixer()
		: bufferSize(0)
		, buffersReceived(0)
		, mixBuffer(CMonoBuffer<float>())
		, sampleCount(std::vector<size_t>()) {
	}

	CAudioMixer(size_t bufferSize)
		: bufferSize(bufferSize)
		, buffersReceived(0)
		, mixBuffer(CMonoBuffer<float>(bufferSize, 0))
		, sampleCount(std::vector<size_t>(bufferSize, 0)) {
	}

	/**
		 * @brief Add buffer
		 * @param channel 
		 * @param buffer 
		 */
	bool AddBuffer(const CMonoBuffer<float> & _newBuffer) {
		if (_newBuffer.size() != bufferSize) {
			return false;
		}
		//mixBuffer += _newBuffer;

		for (size_t i = 0; i < bufferSize; i++) {
			if (_newBuffer[i] != 0.0f) { // only add non-zero samples
				mixBuffer[i] += _newBuffer[i];
				sampleCount[i]++;
			}
		}

		buffersReceived++;
		return true;
	}

	/**
		 * @brief Get mixed buffer and reset accumulation
		 * @return mixed buffer
		 */
	CMonoBuffer<float> GetMixedBuffer(bool _normalization = true) {

		if (buffersReceived == 0) return CMonoBuffer<float>(bufferSize, 0.0f);

		CMonoBuffer<float> returnBuffer(bufferSize);

		if (_normalization) {
			// Normalisation: dividing each sample by the number of contributions
			for (size_t i = 0; i < bufferSize; i++) {
				if (sampleCount[i] > 0) {
					returnBuffer[i] = mixBuffer[i] / static_cast<float>(sampleCount[i]);
				}
			}
		} else {
			returnBuffer = mixBuffer;
		}

		// Reset buffers and counter for next frame
		std::fill(mixBuffer.begin(), mixBuffer.end(), 0.0f);
		std::fill(sampleCount.begin(), sampleCount.end(), 0);
		buffersReceived = 0;

		return returnBuffer;
	}

private:
	size_t bufferSize;
	size_t buffersReceived; // number of buffers received in the current frame

	CMonoBuffer<float> mixBuffer; // mixed buffer
	std::vector<size_t> sampleCount; // count of contributions per sample
};
}
#endif