#ifndef _PROCESSOR_BASE_
#define _PROCESSOR_BASE_

#include <Base/EntryPoint.hpp>
#include <Base/ExitPoint.h>
#include <Base/EntryPointPtr.hpp>
#include <Common/CommonDefinitions.h>
#include <vector>

namespace BRTBase {    
    class CWaitingEntrypoint {
    public:
        CWaitingEntrypoint(std::string _id, int _multiplicity) : id{ _id }, multiplicity{ _multiplicity }, received{ false } {}
    
        std::string id;
        int multiplicity;
        bool received;
    };

    class CProcessorBase {
    public:
        CProcessorBase() {}
        virtual ~CProcessorBase() {}
        virtual void Update(std::string entryPointID) = 0;

        // Create Entry and Exit points
        void CreateSamplesEntryPoint(std::string entryPointID, int _multiplicity = 1) {
            std::shared_ptr<CEntryPointSamplesVector> _newEntryPoint = std::make_shared<BRTBase::CEntryPointSamplesVector >(std::bind(&CProcessorBase::updateFromEntryPoint, this, std::placeholders::_1), entryPointID, _multiplicity);
            samplesEntryPoints.push_back(_newEntryPoint);      
            addToUpdateStack(entryPointID, _multiplicity);
        }

        void CreatePositionEntryPoint(std::string entryPointID, int _multiplicity = 0) {
            std::shared_ptr<CEntryPointTransform> _newEntryPoint = std::make_shared<BRTBase::CEntryPointTransform >(std::bind(&CProcessorBase::updateFromEntryPoint, this, std::placeholders::_1), entryPointID, _multiplicity);
            positionEntryPoints.push_back(_newEntryPoint);
            addToUpdateStack(entryPointID, _multiplicity);
        }
        
        
        void CreateHRTFPtrEntryPoint(std::string entryPointID, int _multiplicity = 0) {
            std::shared_ptr<CEntryPointHRTFPtr> _newEntryPoint = std::make_shared<BRTBase::CEntryPointHRTFPtr>(std::bind(&CProcessorBase::updateFromEntryPoint, this, std::placeholders::_1), entryPointID, _multiplicity);
            hrtfPtrEntryPoints.push_back(_newEntryPoint);
            addToUpdateStack(entryPointID, _multiplicity);
        }

        void CreateSamplesExitPoint(std::string exitPointID) {
            std::shared_ptr<BRTBase::CExitPointSamplesVector> _newExitPoint = std::make_shared<BRTBase::CExitPointSamplesVector>(exitPointID);
            samplesExitPoints.push_back(_newExitPoint);
        }

        //// Connections
        void connectSamplesEntryTo(std::shared_ptr<BRTBase::CExitPointSamplesVector> _exitPoint, std::string entryPointID) {
            std::shared_ptr<BRTBase::CEntryPointSamplesVector> _entryPoint2 = GetSamplesEntryPoint(entryPointID);
            if (_entryPoint2) { _exitPoint->attach(*_entryPoint2.get()); }            
        }

        std::shared_ptr<BRTBase::CEntryPointSamplesVector> GetSamplesEntryPoint(std::string _id) {            
            for (auto& it : samplesEntryPoints) {
                if (it->GetID() == _id) { return it; }
            }
            return nullptr;
        }

        void connectPositionEntryTo(std::shared_ptr<BRTBase::CExitPointTransform> _exitPoint, std::string entryPointID) {
            std::shared_ptr<BRTBase::CEntryPointTransform> _entryPoint = GetPositionEntryPoint(entryPointID);
            if (_entryPoint) { _exitPoint->attach(*_entryPoint.get()); }
        }
             

        void connectHRTFEntryTo(std::shared_ptr<BRTBase::CExitPointHRTFPtr> _exitPoint, std::string entryPointID) {
            std::shared_ptr<BRTBase::CEntryPointHRTFPtr> _entryPoint = GetHRTFPtrEntryPoint(entryPointID);
            if (_entryPoint) { _exitPoint->attach(*_entryPoint.get()); }
        }

        std::shared_ptr<BRTBase::CEntryPointHRTFPtr >  GetHRTFPtrEntryPoint(std::string _id) {
            for (auto& it : hrtfPtrEntryPoints) {
                if (it->GetID() == _id) { return it; }
            }
            return nullptr;
        }


        std::shared_ptr<BRTBase::CEntryPointTransform >  GetPositionEntryPoint(std::string _id) {
            for (auto& it : positionEntryPoints) {
                if (it->GetID() == _id) { return it; }
            }
            return nullptr;
        }

        std::shared_ptr<BRTBase::CExitPointSamplesVector > GetSamplesExitPoint(std::string _id) {
            for (auto& it : samplesExitPoints) {
                if (it->GetID() == _id) { return it; }
            }
            return nullptr;
        }

        // Update callback
        void updateFromEntryPoint(std::string entryPointID) {            
            if (updateStack(entryPointID)) { Update(entryPointID); }
        }


        // Update Management
        void addToUpdateStack(std::string _id, int _multiplicity) {
            if (_multiplicity >= 1) {
                CWaitingEntrypoint temp(_id, _multiplicity);
                entryPointsUpdatingStack.push_back(temp);
            }            
        }

        bool updateStack(std::string _id) {            
            std::vector<CWaitingEntrypoint>::iterator it;
            it = std::find_if(entryPointsUpdatingStack.begin(), entryPointsUpdatingStack.end(), [&_id](CWaitingEntrypoint const& obj) {
                return obj.id == _id;
            });

            if (it != entryPointsUpdatingStack.end())
                if (!it->received) { 
                    it->received = true; 
                    return checkWaitingStack();
                }
                else {
                    // It has been received twice before processing it. // TODO manage multiplicity bigger than 1
                }
            else {
                // TODO error, why are we receibing this?
            }
            return false;
        }                                

        bool checkWaitingStack() {
            bool ready = true;
            typename std::vector<CWaitingEntrypoint>::iterator it;
            for (it = entryPointsUpdatingStack.begin(); it != entryPointsUpdatingStack.end(); it++) {
                if (!(it->received)) { ready = false; }
            }
            return ready;
        }

        void resetUpdatingStack() {
            typename std::vector<CWaitingEntrypoint>::iterator it;
            for (it = entryPointsUpdatingStack.begin(); it != entryPointsUpdatingStack.end(); it++) {
                it->received = false;
            }
        }
        
    private:                        
        std::vector<std::shared_ptr<BRTBase::CEntryPointSamplesVector> > samplesEntryPoints;        
        std::vector<std::shared_ptr<BRTBase::CExitPointSamplesVector >> samplesExitPoints;

        std::vector<std::shared_ptr <BRTBase::CEntryPointTransform > > positionEntryPoints;
        
        std::vector<std::shared_ptr <BRTBase::CEntryPointHRTFPtr>> hrtfPtrEntryPoints;


        std::vector< CWaitingEntrypoint> entryPointsUpdatingStack;

    };
}
#endif