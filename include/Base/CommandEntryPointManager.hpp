/**
* \class CBuffer
*
* \brief Declaration of CBuffer class
* \date	June 2023
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco, F. Morales-Benitez ||
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

#ifndef _COMMAND_ENTRY_POINT_MANAGER_
#define _COMMAND_ENTRY_POINT_MANAGER_

#include <vector>
#include <memory>
#include <EntryPoint.hpp>
#include <CommonDefinitions.hpp>

namespace BRTBase {
    class CCommandEntryPointManager {
    public:

        virtual void updateFromCommandEntryPoint(std::string entryPointID) = 0;

        void CreateCommandEntryPoint(/*std::string entryPointID = "command", int _multiplicity = 1*/) {
            std::string entryPointID = static_cast<std::string>(Common::COMMAND_ENTRY_POINT_ID);
            int _multiplicity = 1;
            commandsEntryPoint = std::make_shared<BRTBase::CEntryPointCommand>(std::bind(&CCommandEntryPointManager::updateFromCommandEntryPoint, this, std::placeholders::_1), entryPointID, _multiplicity);
        }

        void connectCommandEntryTo(std::shared_ptr<BRTBase::CExitPointCommand> _exitPoint) {
            std::string entryPointID = static_cast<std::string>(Common::COMMAND_ENTRY_POINT_ID);
            //if (_entryPoint) {
            _exitPoint->attach(*commandsEntryPoint.get());
            SET_RESULT(RESULT_OK, "Connection done correctly with this entry point " + entryPointID);
            //}
            //else {
                //ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            //}
        }

        void disconnectCommandEntryTo(std::shared_ptr<BRTBase::CExitPointCommand> _exitPoint) {
            std::string entryPointID = static_cast<std::string>(Common::COMMAND_ENTRY_POINT_ID);
            //if (_entryPoint) {
            _exitPoint->detach(commandsEntryPoint.get());
            SET_RESULT(RESULT_OK, "Disconnection done correctly with this entry point " + entryPointID);
            //}
            //else {
                //ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            //}
        }

        std::shared_ptr<BRTBase::CEntryPointCommand >  GetCommandEntryPoint() {
            return commandsEntryPoint;
        }
    
    private:
        std::shared_ptr<BRTBase::CEntryPointCommand> commandsEntryPoint;

    };
}
#endif