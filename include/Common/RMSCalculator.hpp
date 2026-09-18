/**
* \class CRMSCalculator
*
* \brief Declaration of CRMSCalculator class
* \date	January 2025
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


#ifndef _CRMS_CALCULATOR_HPP_
#define _CRMS_CALCULATOR_HPP_

#include <cmath>
#include <cstddef>
#include <deque>

#include <Common/Buffer.hpp>
#include <Common/ErrorHandler.hpp>

namespace Common {

	/**
	* @brief Calculates the RMS of the samples contained in the last N blocks.
	*
	* The temporal window is rounded up to a whole number of audio blocks.
	* Before the window is full, only the samples received so far are used.
	*/	
class CRMSCalculator
	{

	public:		
		/**
		* @param num_frames Maximum number of audio blocks in the window.
		* A value of zero is treated as one block.
		*/		
		CRMSCalculator(std::size_t num_frames = 10)
			: max_frames { num_frames > 0 ? num_frames : 1 } {
		}
		

		/**
		* @brief Configures the window duration and clears previous measurements.
		* Invalid parameters leave the existing configuration unchanged.
		*/
		void SetNumberOfFrames(int _windowSizeMS, int _sampleRate, int _bufferSize) {

			if (_windowSizeMS <= 0 || _sampleRate <= 0 || _bufferSize <= 0) {
				SET_RESULT(RESULT_ERROR_INVALID_PARAM,	"RMS window, sample rate and buffer size must be positive.");
				return;
			}

			const double requiredBlocks = (static_cast<double>(_windowSizeMS)  * static_cast<double>(_sampleRate))
				/ (1000.0 * static_cast<double>(_bufferSize));

			max_frames = static_cast<std::size_t>(std::ceil(requiredBlocks));

			if (max_frames == 0) {
				max_frames = 1;
			}
			Reset();
		}
				
		/**
		* @brief Returns the RMS of all samples in the current window.		
		* Empty buffers return zero and do not modify the history.
		*/
		float Process(const CMonoBuffer<float> & buffer) {
			if (buffer.empty()) {
				return 0.0f;
			}

			TBlockStatistics block;
			block.sampleCount = buffer.size();

			for (const float sample : buffer) {
				const double value = static_cast<double>(sample);
				block.sumSquares += value * value;
			}

			if (history.size() >= max_frames) {
				history.pop_front();
			}

			history.push_back(block);

			// Sum the short block history again to avoid accumulated
			// subtraction errors when old blocks leave the window.
			double totalSumSquares = 0.0;
			std::size_t totalSampleCount = 0;

			for (const auto & item : history) {
				totalSumSquares += item.sumSquares;
				totalSampleCount += item.sampleCount;
			}

			return static_cast<float>(std::sqrt(totalSumSquares	/ static_cast<double>(totalSampleCount)));
		}

		/**
		* @brief Returns the RMS of one buffer, without a temporal history.
		*/
		static float InstantProcess(const CMonoBuffer<float> & buffer) {
			if (buffer.empty()) {
				return 0.0f;
			}

			double sumSquares = 0.0;
			for (const float sample : buffer) {
				const double value = static_cast<double>(sample);
				sumSquares += value * value;
			}
			return static_cast<float>(std::sqrt(sumSquares / static_cast<double>(buffer.size())));
		}

		/**
		* @brief Clears measurements while preserving the configured window.
		*/
		void Reset() {
			history.clear();
		}


	private:						
		struct TBlockStatistics {
			double sumSquares = 0.0;
			std::size_t sampleCount = 0;
		};

		std::size_t max_frames;
		std::deque<TBlockStatistics> history;
	};
}

#endif