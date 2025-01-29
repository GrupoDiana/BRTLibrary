/**
* \class CRMSCalculator
*
* \brief Declaration of CRMSCalculator class
* \date	January 2025
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


#ifndef _CRMS_CALCULATOR_HPP_
#define _CRMS_CALCULATOR_HPP_

#include <Common/Buffer.hpp>

#include <Common/ErrorHandler.hpp>
#include <cmath>
#include <math.h>

#define EPSILON_ 0.00001


namespace Common {

	/** \details Class used to detect the envelope of an audio signal
	*/
class CRMSCalculator
	{
	public:
		// PUBLIC METHODS

		/** \brief Default constructor
		*	\details By default, sets sampling rate to 44100Hz, attack time to 20ms and release time to 100ms
		*/
		CRMSCalculator(size_t num_frames = 10)
			: max_frames(num_frames)
			, sum_rms { 0 }	
		{}
				
		/**
		 * @brief Calculate the RMS value of a buffer
		 * @param buffer buffer with samples
		 * @return rms value of the buffer
		 */
		float Process(const CMonoBuffer<float> & buffer)
		{
			double sum_squares = 0;
			for (int i = 0; i < buffer.size(); i++) {
				sum_squares += buffer[i] * buffer[i];
			}
			double rms = std::sqrt(sum_squares / buffer.size());
			
			
			// Update moving average
			if (rms_history.size() == max_frames) {
				sum_rms -= rms_history.front();
				rms_history.pop_front();
			}

			rms_history.push_back(rms);
			sum_rms += rms;
						
			return sum_rms / rms_history.size();
		}


	private:
		
		double sum_rms; /// last calculated RMS value
		std::deque<double> rms_history;
		size_t max_frames;
		
	};
}

#endif