/**
* \class CISMParameters
*
* \brief This class implements the ISM common parameters.  
* \date	Oct 2025
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga)||
* \b Contact: areyes@uma.es
*
* \b Copyright: University of Malaga
* 
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/


#ifndef _CISM_PARAMETERS_HPP_
#define _CISM_PARAMETERS_HPP_

#include <Common/Vector3.hpp>
#include <Common/Room.hpp>


namespace BRTEnvironmentModel {

	/**
	 * @brief Struct to share data of the image sources
	 */
	struct TImageSourceData {
		Common::CVector3 location; //Location of the image source
		bool visible; //If the source is visible it should be rendered
		float visibility; //1 if visible, 0 if not, something in the middle if in the transition, where the transition is +/-VISIBILITY_MARGIN width
		std::vector<Common::CWall> reflectionWalls; //list of walls where the source has reflected (last reflection first)
		std::vector<float> reflectionBands; //coeficients, for each octave Band, to be applied to simulate walls' absortion		
	};

	class CISMParameters {
	public:

		CISMParameters() 
			: sampleRate{ 48000 }
			, maxDistanceSourcesToListener{ 100 }
			, transitionMeters { 0 }
			, staticDistanceCriterion{ true }
			, listenerPosition{ Common::CVector3(0, 0, 0) }
		{			
		}


		int sampleRate;					///< Default sample rate in samples/seconds
		
		Common::CRoom room;
		float transitionMeters;          // Transition meters associated with the _windowSlopeDistance		
		float maxDistanceSourcesToListener;		// Maximum distance between the listener and each source image to be considered visible		
		bool staticDistanceCriterion;    // When enabled, the number of potential images is smaller.NO SABEMOS SI DEBE ESTAR. ES UNA SITUACION ESTATICA (NO SE VAN A MOVER LAS FUENTES) AHORRA FUENTES		

		Common::CVector3 listenerPosition;
		
	private:

	};
}
#endif