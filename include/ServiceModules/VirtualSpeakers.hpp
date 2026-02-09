/**
* \class CVirtualSpeakers
*
* \brief Declaration of CVirtualSpeakers class
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
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/

#ifndef _CVIRTUAL_SPEAKERS_HPP_
#define _CVIRTUAL_SPEAKERS_HPP_


#include <unordered_map>
#include <ServiceModules/ServicesBase.hpp>

namespace BRTServices {

	class CVirtualSpeakers {


		/** \brief Type definition for the AmbisonicIR table*/
		typedef std::unordered_map<int, TOrientation> TVirtualSpeakerPositionTable;


	public:
		CVirtualSpeakers() : ambisonicOrder{ 0 }, initialized{ false } {}

		/**
		 * @brief Sets the position of the virtual loudspeakers according to an ambisonic order.
		 * @param _ambisonicOrder Ambisonic order
		 * @note Right now it only works for ambisonic orders from 1 to 3.
		*/
		void Setup(int _ambisonicOrder) {
			if (initialized) { Reset(); }
			if (_ambisonicOrder == 1) { InitializeOrder1(); }
			else if (_ambisonicOrder == 2) { InitializeOrder2(); }
			else if (_ambisonicOrder == 3) { InitializeOrder3(); }
			else {
				//TODO Initialise virtual loudspeaker for generic ambisonic order 
				return;
			}
			initialized = true;
		}

		/**
		 * @brief Clear all
		*/
		void Reset() {
			initialized = false;
			ambisonicOrder = 0;
			virtualSpeakerPositionTable.clear();
		}

		/**
		 * @brief Returns list of virtual speakers orientation
		 * @return Virtual speaker position list
		*/
		std::vector< TOrientation> GetVirtualSpeakersPositions() {

			std::vector< TOrientation> virtualSpeakerOrientationList;

			if (initialized) {
				for (auto& it : virtualSpeakerPositionTable) {
					virtualSpeakerOrientationList.push_back(it.second);
				}
			}

			return virtualSpeakerOrientationList;
		}

		/**
		 * @brief Returns the number of virtual speakers
		 * @return Number of virtual speakers
		*/
		int GetTotalVirtualSpeakers()
		{
			return virtualSpeakerPositionTable.size();
		}

		/**
		 * @brief Return one virtual speaker orientation
		 * @param _virtualSpeakerID ID of the virtual speaker, a int between 1 and NumberOfVirtualSpeakers
		 * @return Virtual speaker orientation. Returns (0, 0) if the ID doen't exist.
		*/
		TOrientation GetVirtualSpeakerOrientation(int _virtualSpeakerID) {
			auto it = virtualSpeakerPositionTable.find(_virtualSpeakerID);						
			if (it != virtualSpeakerPositionTable.end())
			{
				return it->second;
			}
			return TOrientation(0, 0);
		}

	private:

		int ambisonicOrder;
		bool initialized;
		TVirtualSpeakerPositionTable virtualSpeakerPositionTable;
		

		// Sets the position of virtual speakers for order 1
		void InitializeOrder1() {			
			virtualSpeakerPositionTable = {
				{1, TOrientation(90, 0)},
				{2, TOrientation(270, 0)},
				{3, TOrientation(0, 90)},
				{4, TOrientation(0, 270)},
				{5, TOrientation(0, 0)},
				{6, TOrientation(180, 0)}
			};
		}
		
		// Sets the position of virtual speakers for order 2
		void InitializeOrder2() {			
			virtualSpeakerPositionTable = {
				{1, TOrientation(328.28, 0)},
				{2, TOrientation(31.72, 0)},
				{3, TOrientation(148.28, 0)},
				{4, TOrientation(211.72, 0)},
				{5, TOrientation(270, 328.28)},
				{6, TOrientation(90, 328.28)},
				{7, TOrientation(270, 31.72)},
				{8, TOrientation(90, 31.72)},
				{9, TOrientation(180, 301.72)},
				{10, TOrientation(0, 301.72)},
				{11, TOrientation(180, 58.28)},
				{12, TOrientation(0, 58.28)}
			};
		}

		// Sets the position of virtual speakers for order 3
		void InitializeOrder3() {
			
			virtualSpeakerPositionTable = {
				{1 , TOrientation(290.91, 0)},
				{2 , TOrientation(69.1, 0)},
				{3 , TOrientation(249.1, 0)},
				{4 , TOrientation(110.91, 0)},
				{5 , TOrientation(315, 35.26)},
				{6 , TOrientation(45, 35.26)},
				{7 , TOrientation(225, 35.26)},
				{8 , TOrientation(135, 35.26)},
				{9 , TOrientation(315, 324.74)},
				{10, TOrientation(45, 324.74)},
				{11, TOrientation(225, 324.74)},
				{12, TOrientation(135, 324.74)},
				{13, TOrientation(0, 339.1)},
				{14, TOrientation(180, 339.1)},
				{15, TOrientation(0, 20.91)},
				{16, TOrientation(180, 20.91)},
				{17, TOrientation(270, 69.1)},
				{18, TOrientation(90, 69.1)},
				{19, TOrientation(270, 290.91)},
				{20, TOrientation(90, 290.91)},
			};
		}
	};
}


#endif