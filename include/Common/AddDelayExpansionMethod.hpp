/**
* \class CAddDelayExpansionMethod
*
* \brief Declaration of CAddDelayExpansionMethod class interface.
* \date	November 2023
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

#ifndef _CADD_DELAY_EXPANSION_METHOD_HPP_
#define _CADD_DELAY_EXPANSION_METHOD_HPP_

#include <Common/Buffer.hpp>

namespace Common {

	class CAddDelayExpansionMethod {
	public:

		
		/**
		 * @brief Adds or removes a delay to a buffer, by doing an expansion or compression. 
		 * @param input Buffer to be processed, to which the delay is to be applied
		 * @param output Result of processing
		 * @param delayBuffer buffer containing the information of the last frame. This buffer will also be updated.
		 * @param newDelay Delay to be applied
		*/
		static void ProcessAddDelay_ExpansionMethod(CMonoBuffer<float>& input, CMonoBuffer<float>& output, CMonoBuffer<float>& delayBuffer, int newDelay)
		{
			//Prepare the outbuffer		
			if (output.size() != input.size()) { output.resize(input.size()); }

			//Prepare algorithm variables
			float position = 0;
			float numerator = input.size() - 1;
			float denominator = input.size() - 1 + newDelay - delayBuffer.size();
			float compressionFactor = numerator / denominator;

			//Add samples to the output from buffer
			for (int i = 0; i < delayBuffer.size(); i++)
			{
				output[i] = delayBuffer[i];
			}

			//Fill the others buffers
			//if the delay is the same one as the previous frame use a simplification of the algorithm
			if (newDelay == delayBuffer.size())
			{
				//Copy input to output
				int j = 0;
				for (int i = delayBuffer.size(); i < input.size(); i++)
				{
					output[i] = input[j++];
				}
				//Fill delay buffer
				for (int i = 0; i < newDelay; i++)
				{
					delayBuffer[i] = input[j++];
				}
			}
			//else, apply the expansion/compression algorihtm
			else
			{
				int j;
				float rest;
				int forLoop_end;
				//The last loop iteration must be addressed in a special way if newDelay = 0 (part 1)
				if (newDelay == 0) { forLoop_end = input.size() - 1; }
				else { forLoop_end = input.size(); }

				//Fill the output buffer with the new values 
				for (int i = delayBuffer.size(); i < forLoop_end; i++)
				{
					j = static_cast<int>(position);
					rest = position - j;
					output[i] = input[j] * (1 - rest) + input[j + 1] * rest;
					position += compressionFactor;
				}

				//The last loop iteration must be addressed in a special way if newDelay = 0 (part 2)
				if (newDelay == 0)
				{
					output[input.size() - 1] = input[input.size() - 1];
					delayBuffer.clear();
				}

				//if newDelay!=0 fill out the delay buffer
				else
				{
					//Fill delay buffer 			
					CMonoBuffer<float> temp;
					temp.reserve(newDelay);
					for (int i = 0; i < newDelay - 1; i++)
					{
						int j = int(position);
						float rest = position - j;
						temp.push_back(input[j] * (1 - rest) + input[j + 1] * rest);
						position += compressionFactor;
					}
					//Last element of the delay buffer that must be addressed in a special way
					temp.push_back(input[input.size() - 1]);
					//delayBuffer.swap(temp);				//To use in C++03
					delayBuffer = std::move(temp);			//To use in C++11
				}
			}
		}
	};
}
#endif