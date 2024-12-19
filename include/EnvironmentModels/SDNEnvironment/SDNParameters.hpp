/**
* \class SDNParameters
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
#ifndef _SDN_PARAMETERS_HPP_
#define _SDN_PARAMETERS_HPP_

namespace SDNParameters
{
	constexpr auto SOUND_SPEED = 343;
	constexpr char axishelper[6] = { 'x', 'x', 'y', 'y', 'z', 'z' };
	constexpr int NUM_WALLS = 6;
	constexpr int NUM_FREQ = 8;
	constexpr int NUM_WAVEGUIDES_TO_OUTPUT = 7;
	constexpr float ROOM_MAX_DIMENSION = 100.0f;
	constexpr float SMOOTHING_TIME_SECONDS = 0.015f;
	constexpr double MINUS_INFINITY_DB = -100.0;
}

#endif