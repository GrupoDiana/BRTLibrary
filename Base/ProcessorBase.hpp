#pragma once

#include "../Base/EntryPoint.hpp"
#include "../Base/EntryPoint2.hpp"
#include <vector>

namespace BRTBase {
    //template <class T>
    class CProcessorBase {
    public:
        CProcessorBase() {}
        virtual ~CProcessorBase() {}
        virtual void Update() = 0;

        
        void CreateEntryPoint(std::string entryPointID) {

            std::shared_ptr<BRTBase::CEntryPoint2 > _newEntryPoint2 = std::make_shared<BRTBase::CEntryPoint2 >(std::bind(&CProcessorBase::updateFromEntryPoint, this, std::placeholders::_1) , entryPointID);
            entryPoints.push_back(_newEntryPoint2);
        }

        void CreateExitPoint(std::string exitPointID) {
            std::shared_ptr<BRTBase::CExitPoint> _newExitPoint = std::make_shared<BRTBase::CExitPoint>(exitPointID);
            exitPoints.push_back(_newExitPoint);
        }

        
        void updateFromEntryPoint(std::string entryPointID) {
            std::cout << "SingleProcessor Updating --> Recibing buffer" << std::endl;

            //TODO Check if we have received an update from all the entry points, then we should call to Update.
            Update();
        }

        void connectEntryTo(std::shared_ptr<BRTBase::CExitPoint> _exitPoint, std::string entryPointID) {
            //std::shared_ptr<BRTBase::CEntryPoint<T> > _entryPoint = GetEntryPoint(entryPointID);
            std::shared_ptr<BRTBase::CEntryPoint2 > _entryPoint = GetEntryPoint(entryPointID);
            _exitPoint->attach(*_entryPoint.get());
        }

                
        std::shared_ptr<BRTBase::CEntryPoint2 > GetEntryPoint(std::string _id) {
            for (auto& it : entryPoints) {
                if (it->GetID() == _id) { return it; }
            }
            return nullptr;
        }

        std::shared_ptr<BRTBase::CExitPoint > GetExitPoint(std::string _id) {
            for (auto& it : exitPoints) {
                if (it->GetID() == _id) { return it; }
            }
            return nullptr;
        }

    private:        
        std::vector<std::shared_ptr<BRTBase::CEntryPoint2> > entryPoints;
        std::vector<std::shared_ptr<BRTBase::CExitPoint >> exitPoints;
    };
}