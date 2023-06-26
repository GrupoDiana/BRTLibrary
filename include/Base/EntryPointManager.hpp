/**
* \class CEntryPointManager
*
* \brief Declaration of CEntryPointManager class
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
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/

#ifndef _ENTRY_POINT_MANAGER_
#define _ENTRY_POINT_MANAGER_

#include <vector>
#include <memory>
#include <Base/EntryPoint.hpp>

namespace BRTBase {
    class CEntryPointManager {

    public:

        virtual void updateFromEntryPoint(std::string entryPointID) = 0;       
        virtual void addToUpdateStack(std::string _id, int _multiplicity) {};


    

        void CreateSamplesEntryPoint(std::string entryPointID, int _multiplicity = 1) {
            std::shared_ptr<BRTBase::CEntryPointSamplesVector> _newEntryPoint = std::make_shared<BRTBase::CEntryPointSamplesVector >(std::bind(&CEntryPointManager::updateFromEntryPoint, this, std::placeholders::_1), entryPointID, _multiplicity);
            samplesEntryPoints.push_back(_newEntryPoint);
            addToUpdateStack(entryPointID, _multiplicity);
        }

        void CreatePositionEntryPoint(std::string entryPointID, int _multiplicity = 0) {
            std::shared_ptr<BRTBase::CEntryPointTransform> _newEntryPoint = std::make_shared<BRTBase::CEntryPointTransform >(std::bind(&CEntryPointManager::updateFromEntryPoint, this, std::placeholders::_1), entryPointID, _multiplicity);
            positionEntryPoints.push_back(_newEntryPoint);
            addToUpdateStack(entryPointID, _multiplicity);
        }

        void CreateIDEntryPoint(std::string entryPointID, int _multiplicity = 0) {
            std::shared_ptr<BRTBase::CEntryPointID> _newEntryPoint = std::make_shared<BRTBase::CEntryPointID >(std::bind(&CEntryPointManager::updateFromEntryPoint, this, std::placeholders::_1), entryPointID, _multiplicity);
            idEntryPoints.push_back(_newEntryPoint);
            addToUpdateStack(entryPointID, _multiplicity);
        }

        void CreateHRTFPtrEntryPoint(std::string entryPointID, int _multiplicity = 0) {
            std::shared_ptr<BRTBase::CEntryPointHRTFPtr> _newEntryPoint = std::make_shared<BRTBase::CEntryPointHRTFPtr>(std::bind(&CEntryPointManager::updateFromEntryPoint, this, std::placeholders::_1), entryPointID, _multiplicity);
            hrtfPtrEntryPoints.push_back(_newEntryPoint);
            addToUpdateStack(entryPointID, _multiplicity);
        }

        void CreateILDPtrEntryPoint(std::string entryPointID, int _multiplicity = 0) {
            std::shared_ptr<BRTBase::CEntryPointILDPtr> _newEntryPoint = std::make_shared<BRTBase::CEntryPointILDPtr>(std::bind(&CEntryPointManager::updateFromEntryPoint, this, std::placeholders::_1), entryPointID, _multiplicity);
            ildPtrEntryPoints.push_back(_newEntryPoint);
            addToUpdateStack(entryPointID, _multiplicity);
        }

        /*void CreateSRTFPtrEntryPoint(std::string entryPointID, int _multiplicity = 0) {
            std::shared_ptr<BRTBase::CEntryPointSRTFPtr> _newEntryPoint = std::make_shared<BRTBase::CEntryPointSRTFPtr>(std::bind(&CEntryPointManager::updateFromEntryPoint, this, std::placeholders::_1), entryPointID, _multiplicity);
            srtfPtrEntryPoints.push_back(_newEntryPoint);
            addToUpdateStack(entryPointID, _multiplicity);
        }*/

        //void CreateCommandEntryPoint(/*std::string entryPointID = "command", int _multiplicity = 1*/) {
        //    std::string entryPointID = static_cast<std::string>(Common::COMMAND_ENTRY_POINT_ID);
        //    int _multiplicity = 1;
        //    commandsEntryPoint = std::make_shared<BRTBase::CEntryPointCommand>(std::bind(&CEntryPointManager::updateFromCommandEntryPoint, this, std::placeholders::_1), entryPointID, _multiplicity);
        //    //addToUpdateStack(entryPointID, _multiplicity);            
        //}


        ////
        void connectSamplesEntryTo(std::shared_ptr<BRTBase::CExitPointSamplesVector> _exitPoint, std::string entryPointID) {
            std::shared_ptr<BRTBase::CEntryPointSamplesVector> _entryPoint2 = GetSamplesEntryPoint(entryPointID);
            if (_entryPoint2) {
                _exitPoint->attach(*_entryPoint2.get());
                SET_RESULT(RESULT_OK, "Connection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }

        void disconnectSamplesEntryTo(std::shared_ptr<BRTBase::CExitPointSamplesVector> _exitPoint, std::string entryPointID) {
            std::shared_ptr<BRTBase::CEntryPointSamplesVector> _entryPoint2 = GetSamplesEntryPoint(entryPointID);
            if (_entryPoint2) {
                _exitPoint->detach(_entryPoint2.get());
                SET_RESULT(RESULT_OK, "Disconnection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }

        void connectPositionEntryTo(std::shared_ptr<BRTBase::CExitPointTransform> _exitPoint, std::string entryPointID) {
            std::shared_ptr<BRTBase::CEntryPointTransform> _entryPoint = GetPositionEntryPoint(entryPointID);
            if (_entryPoint) {
                _exitPoint->attach(*_entryPoint.get());
                SET_RESULT(RESULT_OK, "Connection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }

        void disconnectPositionEntryTo(std::shared_ptr<BRTBase::CExitPointTransform> _exitPoint, std::string entryPointID) {
            std::shared_ptr<BRTBase::CEntryPointTransform> _entryPoint = GetPositionEntryPoint(entryPointID);
            if (_entryPoint) {
                _exitPoint->detach(_entryPoint.get());
                SET_RESULT(RESULT_OK, "Disconnection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }

        void connectHRTFEntryTo(std::shared_ptr<BRTBase::CExitPointHRTFPtr> _exitPoint, std::string entryPointID) {
            std::shared_ptr<BRTBase::CEntryPointHRTFPtr> _entryPoint = GetHRTFPtrEntryPoint(entryPointID);
            if (_entryPoint) {
                _exitPoint->attach(*_entryPoint.get());
                SET_RESULT(RESULT_OK, "Connection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }

        void disconnectHRTFEntryTo(std::shared_ptr<BRTBase::CExitPointHRTFPtr> _exitPoint, std::string entryPointID) {
            std::shared_ptr<BRTBase::CEntryPointHRTFPtr> _entryPoint = GetHRTFPtrEntryPoint(entryPointID);
            if (_entryPoint) {
                _exitPoint->detach(_entryPoint.get());
                SET_RESULT(RESULT_OK, "Disconnection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }

        void connectILDEntryTo(std::shared_ptr<BRTBase::CExitPointILDPtr> _exitPoint, std::string entryPointID) {
            std::shared_ptr<BRTBase::CEntryPointILDPtr> _entryPoint = GetILDPtrEntryPoint(entryPointID);
            if (_entryPoint) {
                _exitPoint->attach(*_entryPoint.get());
                SET_RESULT(RESULT_OK, "Connection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }

        void disconnectILDEntryTo(std::shared_ptr<BRTBase::CExitPointILDPtr> _exitPoint, std::string entryPointID) {
            std::shared_ptr<BRTBase::CEntryPointILDPtr> _entryPoint = GetILDPtrEntryPoint(entryPointID);
            if (_entryPoint) {
                _exitPoint->detach(_entryPoint.get());
                SET_RESULT(RESULT_OK, "Disconnection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }

        //void connectCommandEntryTo(std::shared_ptr<BRTBase::CExitPointCommand> _exitPoint) {
        //    std::string entryPointID = static_cast<std::string>(Common::COMMAND_ENTRY_POINT_ID);
        //    //if (_entryPoint) {
        //    _exitPoint->attach(*commandsEntryPoint.get());
        //    SET_RESULT(RESULT_OK, "Connection done correctly with this entry point " + entryPointID);
        //    //}
        //    //else {
        //        //ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
        //    //}
        //}

        void connectIDEntryTo(std::shared_ptr<BRTBase::CExitPointID> _exitPoint, std::string entryPointID) {
            std::shared_ptr<BRTBase::CEntryPointID> _entryPoint2 = GetIDEntryPoint(entryPointID);
            if (_entryPoint2) {
                _exitPoint->attach(*_entryPoint2.get());
                SET_RESULT(RESULT_OK, "Connection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }

        void disconnectIDEntryTo(std::shared_ptr<BRTBase::CExitPointID> _exitPoint, std::string entryPointID) {
            std::shared_ptr<BRTBase::CEntryPointID> _entryPoint2 = GetIDEntryPoint(entryPointID);
            if (_entryPoint2) {
                _exitPoint->detach(_entryPoint2.get());
                SET_RESULT(RESULT_OK, "Disconnection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }

        //void disconnectCommandEntryTo(std::shared_ptr<BRTBase::CExitPointCommand> _exitPoint) {
        //    std::string entryPointID = static_cast<std::string>(Common::COMMAND_ENTRY_POINT_ID);
        //    //if (_entryPoint) {
        //    _exitPoint->detach(commandsEntryPoint.get());
        //    SET_RESULT(RESULT_OK, "Disconnection done correctly with this entry point " + entryPointID);
        //    //}
        //    //else {
        //        //ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
        //    //}
        //}

        // Find entry/exit point in vectors 
        std::shared_ptr<BRTBase::CEntryPointHRTFPtr >  GetHRTFPtrEntryPoint(std::string _id) {
            for (auto& it : hrtfPtrEntryPoints) {
                if (it->GetID() == _id) { return it; }
            }
            return nullptr;
        }

        std::shared_ptr<BRTBase::CEntryPointILDPtr >  GetILDPtrEntryPoint(std::string _id) {
            for (auto& it : ildPtrEntryPoints) {
                if (it->GetID() == _id) { return it; }
            }
            return nullptr;
        }

      /*  std::shared_ptr<BRTBase::CEntryPointSRTFPtr >  GetSRTFPtrEntryPoint(std::string _id) {
            for (auto& it : srtfPtrEntryPoints) {
                if (it->GetID() == _id) { return it; }
            }
            return nullptr;
        }*/

        std::shared_ptr<BRTBase::CEntryPointTransform >  GetPositionEntryPoint(std::string _id) {
            for (auto& it : positionEntryPoints) {
                if (it->GetID() == _id) { return it; }
            }
            return nullptr;
        }

        std::shared_ptr<BRTBase::CEntryPointSamplesVector> GetSamplesEntryPoint(std::string _id) {
            for (auto& it : samplesEntryPoints) {
                if (it->GetID() == _id) { return it; }
            }
            return nullptr;
        }

        /*std::shared_ptr<BRTBase::CEntryPointCommand >  GetCommandEntryPoint() {
            return commandsEntryPoint;
        }*/

        std::shared_ptr<BRTBase::CEntryPointID> GetIDEntryPoint(std::string _id) {
            for (auto& it : idEntryPoints) {
                if (it->GetID() == _id) { return it; }
            }
            return nullptr;
        }
        

    private:

        std::vector<std::shared_ptr<BRTBase::CEntryPointSamplesVector> > samplesEntryPoints;
        std::vector<std::shared_ptr <BRTBase::CEntryPointTransform > > positionEntryPoints;
        //std::shared_ptr<BRTBase::CEntryPointCommand> commandsEntryPoint;
        std::vector<std::shared_ptr <BRTBase::CEntryPointHRTFPtr>> hrtfPtrEntryPoints;
        std::vector<std::shared_ptr <BRTBase::CEntryPointILDPtr>> ildPtrEntryPoints;
        //std::vector<std::shared_ptr <BRTBase::CEntryPointSRTFPtr>> srtfPtrEntryPoints;
        std::vector<std::shared_ptr<BRTBase::CEntryPointID> > idEntryPoints;


    };
};
#endif