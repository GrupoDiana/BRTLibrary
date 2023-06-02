#ifndef _COMMAND_ENTRY_POINT_MANAGER_
#define _COMMAND_ENTRY_POINT_MANAGER_

#include <vector>
#include <memory>
#include <EntryPoint.hpp>
#include <CommonDefinitions.h>

namespace BRTBase {
    class CCommandEntryPointManager {
    public:

        virtual void updateFromCommandEntryPoint(std::string entryPointID) {};

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