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

#ifndef _ENTRY_POINT_MANAGER_
#define _ENTRY_POINT_MANAGER_

#include <vector>
#include <memory>
#include <Connectivity/EntryPoint.hpp>

namespace BRTConnectivity {
    class CEntryPointManager {

    public:

        virtual void UpdateEntryPointData(std::string entryPointID) = 0;
        virtual void EntryPointCreated(std::string _entryPointID, bool _notify) {};
        virtual void UpdateEntryPointConnections(std::string _entryPointID, int _numberOfConnections) {};
        
       void CreateSamplesEntryPoint(std::string entryPointID, bool _notify = true) {
            //std::shared_ptr<BRTBase::CEntryPointSamplesVector> _newEntryPoint = std::make_shared<BRTBase::CEntryPointSamplesVector >(std::bind(&CEntryPointManager::updateFromEntryPoint, this, std::placeholders::_1), entryPointID, _multiplicity);            
            std::shared_ptr<BRTConnectivity::CEntryPointSamplesVector> _newEntryPoint = CreateGenericEntryPoint<BRTConnectivity::CEntryPointSamplesVector>(entryPointID, _notify);
            
            samplesEntryPoints.push_back(_newEntryPoint);
            EntryPointCreated(entryPointID, _notify);
        }

        void CreateMultipleChannelsEntryPoint(std::string entryPointID, bool _notify) {
            //std::shared_ptr<BRTBase::CEntryPointMultipleSamplesVector> _newEntryPoint = std::make_shared<BRTBase::CEntryPointMultipleSamplesVector >(std::bind(&CEntryPointManager::updateFromEntryPoint, this, std::placeholders::_1), entryPointID, _multiplicity);
			std::shared_ptr<BRTConnectivity::CEntryPointMultipleSamplesVector> _newEntryPoint = CreateGenericEntryPoint<BRTConnectivity::CEntryPointMultipleSamplesVector>(entryPointID, _notify);
            
            multipleSamplesVectorEntryPoints.push_back(_newEntryPoint);                        
            EntryPointCreated(entryPointID, _notify);
        }

        void CreatePositionEntryPoint(std::string entryPointID, bool _notify = false) {
            //std::shared_ptr<BRTBase::CEntryPointTransform> _newEntryPoint = std::make_shared<BRTBase::CEntryPointTransform >(std::bind(&CEntryPointManager::updateFromEntryPoint, this, std::placeholders::_1), entryPointID, _multiplicity);
			std::shared_ptr<BRTConnectivity::CEntryPointTransform> _newEntryPoint = CreateGenericEntryPoint<BRTConnectivity::CEntryPointTransform>(entryPointID, _notify);
            positionEntryPoints.push_back(_newEntryPoint);
            EntryPointCreated(entryPointID, _notify);
        }

        void CreateIDEntryPoint(std::string entryPointID, bool _notify = false) {
            //std::shared_ptr<BRTBase::CEntryPointID> _newEntryPoint = std::make_shared<BRTBase::CEntryPointID >(std::bind(&CEntryPointManager::updateFromEntryPoint, this, std::placeholders::_1), entryPointID, _multiplicity);
			std::shared_ptr<BRTConnectivity::CEntryPointID> _newEntryPoint = CreateGenericEntryPoint<BRTConnectivity::CEntryPointID>(entryPointID, _notify);
            
            idEntryPoints.push_back(_newEntryPoint);
            EntryPointCreated(entryPointID, _notify);
        }

        void CreateHRTFPtrEntryPoint(std::string entryPointID, bool _notify = false) {
            //std::shared_ptr<BRTBase::CEntryPointHRTFPtr> _newEntryPoint = std::make_shared<BRTBase::CEntryPointHRTFPtr>(std::bind(&CEntryPointManager::updateFromEntryPoint, this, std::placeholders::_1), entryPointID, _multiplicity);
			std::shared_ptr<BRTConnectivity::CEntryPointHRTFPtr> _newEntryPoint = CreateGenericEntryPoint<BRTConnectivity::CEntryPointHRTFPtr>(entryPointID, _notify);
            hrtfPtrEntryPoints.push_back(_newEntryPoint);
            EntryPointCreated(entryPointID, _notify);
        }

        void CreateILDPtrEntryPoint(std::string entryPointID, bool _notify = false) {
            //std::shared_ptr<BRTBase::CEntryPointILDPtr> _newEntryPoint = std::make_shared<BRTBase::CEntryPointILDPtr>(std::bind(&CEntryPointManager::updateFromEntryPoint, this, std::placeholders::_1), entryPointID, _multiplicity);
			std::shared_ptr<BRTConnectivity::CEntryPointILDPtr> _newEntryPoint = CreateGenericEntryPoint<BRTConnectivity::CEntryPointILDPtr>(entryPointID, _notify);
            ildPtrEntryPoints.push_back(_newEntryPoint);
            EntryPointCreated(entryPointID, _notify);
        }

        void CreateABIRPtrEntryPoint(std::string entryPointID, bool _notify = false) {            
            std::shared_ptr<BRTConnectivity::CEntryPointABIRPtr> _newEntryPoint = CreateGenericEntryPoint<BRTConnectivity::CEntryPointABIRPtr>(entryPointID, _notify);
            abirPtrEntryPoints.push_back(_newEntryPoint);
            EntryPointCreated(entryPointID, _notify);
        }

        void CreateHRBRIRPtrEntryPoint(std::string entryPointID, bool _notify = false) {            
            std::shared_ptr<BRTConnectivity::CEntryPointHRBRIRPtr> _newEntryPoint = CreateGenericEntryPoint<BRTConnectivity::CEntryPointHRBRIRPtr>(entryPointID, _notify);
            hrbrirPtrEntryPoints.push_back(_newEntryPoint);
            EntryPointCreated(entryPointID, _notify);
        }


        template <class T>
        std::shared_ptr<T> CreateGenericEntryPoint(std::string entryPointID, bool _notify) {
            std::shared_ptr<T> _newEntryPoint = std::make_shared<T>(std::bind(&CEntryPointManager::UpdateEntryPointData, this, std::placeholders::_1), entryPointID, _notify);
            return _newEntryPoint;
        }
                       

        //// Conections
		void connectSamplesEntryTo(std::shared_ptr<BRTConnectivity::CExitPointSamplesVector> _exitPoint, std::string entryPointID) {
			std::shared_ptr<BRTConnectivity::CEntryPointSamplesVector> _entryPoint2 = GetSamplesEntryPoint(entryPointID);
            if (_entryPoint2) {
                _exitPoint->attach(*_entryPoint2.get());                
                UpdateEntryPointConnections(entryPointID, _entryPoint2->AddConnection());
                SET_RESULT(RESULT_OK, "Connection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }

        void disconnectSamplesEntryTo(std::shared_ptr<BRTConnectivity::CExitPointSamplesVector> _exitPoint, std::string entryPointID) {
			std::shared_ptr<BRTConnectivity::CEntryPointSamplesVector> _entryPoint2 = GetSamplesEntryPoint(entryPointID);
            if (_entryPoint2) {
                _exitPoint->detach(_entryPoint2.get());                
                UpdateEntryPointConnections(entryPointID, _entryPoint2->RemoveConnection());
                SET_RESULT(RESULT_OK, "Disconnection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }

        void connectMultipleSamplesVectorsEntryTo(std::shared_ptr<BRTConnectivity::CExitPointMultipleSamplesVector> _exitPoint, std::string entryPointID) {
			std::shared_ptr<BRTConnectivity::CEntryPointMultipleSamplesVector> _entryPoint2 = GetMultipleSamplesVectorEntryPoint(entryPointID);
            if (_entryPoint2) {
                _exitPoint->attach(*_entryPoint2.get());                
                UpdateEntryPointConnections(entryPointID, _entryPoint2->AddConnection());
                SET_RESULT(RESULT_OK, "Connection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }

        void disconnectMultipleSamplesVectorsEntryTo(std::shared_ptr<BRTConnectivity::CExitPointMultipleSamplesVector> _exitPoint, std::string entryPointID) {
			std::shared_ptr<BRTConnectivity::CEntryPointMultipleSamplesVector> _entryPoint2 = GetMultipleSamplesVectorEntryPoint(entryPointID);
            if (_entryPoint2) {
                _exitPoint->detach(_entryPoint2.get());                
                UpdateEntryPointConnections(entryPointID, _entryPoint2->RemoveConnection());
                SET_RESULT(RESULT_OK, "Disconnection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }


        void connectPositionEntryTo(std::shared_ptr<BRTConnectivity::CExitPointTransform> _exitPoint, std::string entryPointID) {
			std::shared_ptr<BRTConnectivity::CEntryPointTransform> _entryPoint = GetPositionEntryPoint(entryPointID);
            if (_entryPoint) {
                _exitPoint->attach(*_entryPoint.get());
                SET_RESULT(RESULT_OK, "Connection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }

        void disconnectPositionEntryTo(std::shared_ptr<BRTConnectivity::CExitPointTransform> _exitPoint, std::string entryPointID) {
			std::shared_ptr<BRTConnectivity::CEntryPointTransform> _entryPoint = GetPositionEntryPoint(entryPointID);
            if (_entryPoint) {
                _exitPoint->detach(_entryPoint.get());
                SET_RESULT(RESULT_OK, "Disconnection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }

        void connectHRTFEntryTo(std::shared_ptr<BRTConnectivity::CExitPointHRTFPtr> _exitPoint, std::string entryPointID) {
			std::shared_ptr<BRTConnectivity::CEntryPointHRTFPtr> _entryPoint = GetHRTFPtrEntryPoint(entryPointID);
            if (_entryPoint) {
                _exitPoint->attach(*_entryPoint.get());
                SET_RESULT(RESULT_OK, "Connection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }
        
        void disconnectHRTFEntryTo(std::shared_ptr<BRTConnectivity::CExitPointHRTFPtr> _exitPoint, std::string entryPointID) {
			std::shared_ptr<BRTConnectivity::CEntryPointHRTFPtr> _entryPoint = GetHRTFPtrEntryPoint(entryPointID);
            if (_entryPoint) {
                _exitPoint->detach(_entryPoint.get());
                SET_RESULT(RESULT_OK, "Disconnection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }
        //
		void connectHRBRIREntryTo(std::shared_ptr<BRTConnectivity::CExitPointHRBRIRPtr> _exitPoint, std::string entryPointID) {
			std::shared_ptr<BRTConnectivity::CEntryPointHRBRIRPtr> _entryPoint = GetHRBRIRPtrEntryPoint(entryPointID);
            if (_entryPoint) {
                _exitPoint->attach(*_entryPoint.get());
                SET_RESULT(RESULT_OK, "Connection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }

        void disconnectHRBRIREntryTo(std::shared_ptr<BRTConnectivity::CExitPointHRBRIRPtr> _exitPoint, std::string entryPointID) {
			std::shared_ptr<BRTConnectivity::CEntryPointHRBRIRPtr> _entryPoint = GetHRBRIRPtrEntryPoint(entryPointID);
            if (_entryPoint) {
                _exitPoint->detach(_entryPoint.get());
                SET_RESULT(RESULT_OK, "Disconnection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }
        //

        void connectILDEntryTo(std::shared_ptr<BRTConnectivity::CExitPointILDPtr> _exitPoint, std::string entryPointID) {
			std::shared_ptr<BRTConnectivity::CEntryPointILDPtr> _entryPoint = GetILDPtrEntryPoint(entryPointID);
            if (_entryPoint) {
                _exitPoint->attach(*_entryPoint.get());
                SET_RESULT(RESULT_OK, "Connection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }

        void disconnectILDEntryTo(std::shared_ptr<BRTConnectivity::CExitPointILDPtr> _exitPoint, std::string entryPointID) {
			std::shared_ptr<BRTConnectivity::CEntryPointILDPtr> _entryPoint = GetILDPtrEntryPoint(entryPointID);
            if (_entryPoint) {
                _exitPoint->detach(_entryPoint.get());
                SET_RESULT(RESULT_OK, "Disconnection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }

        void connectABIREntryTo(std::shared_ptr<BRTConnectivity::CExitPointABIRPtr> _exitPoint, std::string entryPointID) {
			std::shared_ptr<BRTConnectivity::CEntryPointABIRPtr> _entryPoint = GetABIRPtrEntryPoint(entryPointID);
            if (_entryPoint) {
                _exitPoint->attach(*_entryPoint.get());
                SET_RESULT(RESULT_OK, "Connection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }

        void disconnectABIREntryTo(std::shared_ptr<BRTConnectivity::CExitPointABIRPtr> _exitPoint, std::string entryPointID) {
			std::shared_ptr<BRTConnectivity::CEntryPointABIRPtr> _entryPoint = GetABIRPtrEntryPoint(entryPointID);
            if (_entryPoint) {
                _exitPoint->detach(_entryPoint.get());
                SET_RESULT(RESULT_OK, "Disconnection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }
        
        void connectIDEntryTo(std::shared_ptr<BRTConnectivity::CExitPointID> _exitPoint, std::string entryPointID) {
			std::shared_ptr<BRTConnectivity::CEntryPointID> _entryPoint2 = GetIDEntryPoint(entryPointID);
            if (_entryPoint2) {
                _exitPoint->attach(*_entryPoint2.get());
                SET_RESULT(RESULT_OK, "Connection done correctly with this entry point " + entryPointID);
            }
            else {
                ASSERT(false, RESULT_ERROR_INVALID_PARAM, "There is no entry point with this id " + entryPointID, "");
            }
        }

        void disconnectIDEntryTo(std::shared_ptr<BRTConnectivity::CExitPointID> _exitPoint, std::string entryPointID) {
			std::shared_ptr<BRTConnectivity::CEntryPointID> _entryPoint2 = GetIDEntryPoint(entryPointID);
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
        std::shared_ptr<BRTConnectivity::CEntryPointHRTFPtr> GetHRTFPtrEntryPoint(std::string _id) {
            for (auto& it : hrtfPtrEntryPoints) {
                if (it->GetID() == _id) { return it; }
            }
            return nullptr;
        }

        std::shared_ptr<BRTConnectivity::CEntryPointHRBRIRPtr> GetHRBRIRPtrEntryPoint(std::string _id) {
            for (auto& it : hrbrirPtrEntryPoints) {
                if (it->GetID() == _id) { return it; }
            }
            return nullptr;
        }

        std::shared_ptr<BRTConnectivity::CEntryPointILDPtr> GetILDPtrEntryPoint(std::string _id) {
            for (auto& it : ildPtrEntryPoints) {
                if (it->GetID() == _id) { return it; }
            }
            return nullptr;
        }

        std::shared_ptr<BRTConnectivity::CEntryPointABIRPtr> GetABIRPtrEntryPoint(std::string _id) {
            for (auto& it : abirPtrEntryPoints) {
                if (it->GetID() == _id) { return it; }                
            }
            return nullptr;
        }

        std::shared_ptr<BRTConnectivity::CEntryPointTransform> GetPositionEntryPoint(std::string _id) {
            for (auto& it : positionEntryPoints) {
                if (it->GetID() == _id) { return it; }
            }
            return nullptr;
        }

        std::shared_ptr<BRTConnectivity::CEntryPointSamplesVector> GetSamplesEntryPoint(std::string _id) {
            for (auto& it : samplesEntryPoints) {
                if (it->GetID() == _id) { return it; }
            }
            return nullptr;
        }

        std::shared_ptr<BRTConnectivity::CEntryPointMultipleSamplesVector> GetMultipleSamplesVectorEntryPoint(std::string _id) {
            for (auto& it : multipleSamplesVectorEntryPoints) {
                if (it->GetID() == _id) { return it; }
            }
            return nullptr;
        }
        

        std::shared_ptr<BRTConnectivity::CEntryPointID> GetIDEntryPoint(std::string _id) {
            for (auto& it : idEntryPoints) {
                if (it->GetID() == _id) { return it; }
            }
            return nullptr;
        }
        

    private:

        std::vector<std::shared_ptr <BRTConnectivity::CEntryPointSamplesVector>> samplesEntryPoints;
		std::vector<std::shared_ptr <BRTConnectivity::CEntryPointMultipleSamplesVector>> multipleSamplesVectorEntryPoints;
        std::vector<std::shared_ptr <BRTConnectivity::CEntryPointTransform > > positionEntryPoints;        
        std::vector<std::shared_ptr <BRTConnectivity::CEntryPointHRTFPtr>> hrtfPtrEntryPoints;
        std::vector<std::shared_ptr <BRTConnectivity::CEntryPointILDPtr>> ildPtrEntryPoints;
        std::vector<std::shared_ptr <BRTConnectivity::CEntryPointABIRPtr>> abirPtrEntryPoints;
        std::vector<std::shared_ptr <BRTConnectivity::CEntryPointID> > idEntryPoints;
        std::vector<std::shared_ptr <BRTConnectivity::CEntryPointHRBRIRPtr>> hrbrirPtrEntryPoints;
        
    };
};
#endif