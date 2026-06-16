/**
* \class CCommandList
*
* \brief Declaration of CCommandList class
* \date	June 2026
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

#ifndef COMMAND_LIST_HPP
#define COMMAND_LIST_HPP

#include <string>
namespace BRTConnectivity {
class CCommandList {

public:
	
	static inline const std::string COMMAND_OVERALL_STOP = "/stop";
	static inline const std::string COMMAND_OVERALL_ENABLE_MODEL = "/enableModel";

	static inline const std::string COMMAND_SOURCE_SET_GAIN = "/source/gain";
	static inline const std::string COMMAND_SOURCE_STOP = "/source/stop";
	static inline const std::string COMMAND_SOURCE_ENABLE_DIRECTIVITY = "/source/enableDirectivity";	
	
	static inline const std::string COMMAND_LISTENER_ENABLE_SPATIALIZATION = "/listener/enableSpatialization";
	static inline const std::string COMMAND_LISTENER_ENABLE_INTERPOLATION = "/listener/enableInterpolation";
	static inline const std::string COMMAND_LISTENER_ENABLE_NEAR_FIELD_EFFECT = "/listener/enableNearFieldEffect";
	static inline const std::string COMMAND_LISTENER_ENABLE_ITD = "/listener/enableITD";
	static inline const std::string COMMAND_LISTENER_ENABLE_PARALLAX_CORRECTION = "/listener/enableParallaxCorrection";
	static inline const std::string COMMAND_LISTENER_SET_AMBISONICS_ORDER = "/listener/setAmbisonicsOrder";
	static inline const std::string COMMAND_LISTENER_SET_AMBISONICS_NORMALIZATION = "/listener/setAmbisonicsNormalization";
	static inline const std::string COMMAND_LISTENER_SET_DISTANCE_ATTENUATION_FACTOR = "/listener/setDistanceAttenuationFactor";
	static inline const std::string COMMAND_LISTENER_ENABLE_DISTANCE_ATTENUATION = "/listener/enableDistanceAttenuation";

	static inline const std::string COMMAND_ENVIRONMENT_ENABLE_DIRECT_PATH = "/environment/enableDirectPath";
	static inline const std::string COMMAND_ENVIRONMENT_ENABLE_REVERB_PATH = "/environment/enableReverbPath";
	static inline const std::string COMMAND_ENVIRONMENT_ENABLE_PROPAGATION_DELAY = "/environment/enablePropagationDelay";
	static inline const std::string COMMAND_ENVIRONMENT_ENABLE_DISTANCE_ATTENUATION = "/environment/enableDistanceAttenuation";
	static inline const std::string COMMAND_ENVIRONMENT_SET_DISTANCE_ATTENUATION_FACTOR = "/environment/setDistanceAttenuationFactor";
	static inline const std::string COMMAND_ENVIRONMENT_SET_FADE_ZONE_MARGIN = "/environment/setFadeZoneMargin";
	static inline const std::string COMMAND_ENVIRONMENT_SET_MAX_DISTANCE_SOURCES_TO_LISTENER = "/environment/setMaxDistanceSourcesToListener";
	static inline const std::string COMMAND_ENVIRONMENT_SET_REFLECTION_ORDER = "/environment/setReflectionOrder";
	static inline const std::string COMMAND_ENVIRONMENT_SET_ROOM = "/environment/setRoom";
};
}
#endif