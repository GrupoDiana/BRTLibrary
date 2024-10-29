/**
* \class CBRTConnectivity
*
* \brief This class includes the elements that provide BRT connectivity. The idea is that classes that need it include this one.
* \date	Oct 2024
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga)||
* \b Contact: areyes@uma.es
*
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Copyright: University of Malaga
*
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/

#ifndef _C_BRT_CONNECTIVITY_HPP_
#define _C_BRT_CONNECTIVITY_HPP_

#include <Base/AdvancedEntryPointManager.hpp>
#include <Base/ExitPointManager.hpp>

namespace BRTBase {
	class CBRTConnectivity: public CAdvancedEntryPointManager, public CExitPointManager { 
	
	};
}
#endif