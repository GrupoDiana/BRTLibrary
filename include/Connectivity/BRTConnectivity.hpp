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

#ifndef _C_BRT_CONNECTIVITY_HPP_
#define _C_BRT_CONNECTIVITY_HPP_

#include <Connectivity/AdvancedEntryPointManager.hpp>
#include <Connectivity/ExitPointManager.hpp>
#include <Connectivity/CommandEntryPointManager.hpp>

namespace BRTConnectivity {
	class CBRTConnectivity : public CAdvancedEntryPointManager, public CExitPointManager, public CCommandEntryPointManager { 
	
	public:

		CBRTConnectivity() {
			CreateCommandEntryPoint();
		}

		/**
         * @brief This method shall be called whenever a command is received at the command entry point.
        */
		virtual void UpdateCommand() = 0;

	private:
		/**
         * @brief In this method, notification is received that a new command is received at command entry point
         * @param entryPointID 
        */
		void UpdateFromCommandEntryPoint(std::string entryPointID) override {
			BRTConnectivity::CCommand _command = GetCommandEntryPoint()->GetData();
			if (!_command.isNull()) {
				UpdateCommand();
			}
		}

	};
}
#endif