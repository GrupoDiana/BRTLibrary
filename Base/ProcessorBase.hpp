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
            
            
            /*BRTBase::CEntryPointWithData<std::vector<float>>* _newEntryPoint2 = new BRTBase::CEntryPointWithData<std::vector<float>>(std::bind(&CProcessorBase::updateFromEntryPoint, this, std::placeholders::_1), entryPointID);            
            _newEntryPoint2->setup(std::bind(&CProcessorBase::updateFromEntryPoint, this, std::placeholders::_1), entryPointID);
            entryPoints.push_back(std::shared_ptr<CEntryPoint2>(_newEntryPoint2));*/

            std::shared_ptr<CEntryPoint2> _newEntryPoint = std::make_shared<BRTBase::CEntryPointWithData<std::vector<float>> >(std::bind(&CProcessorBase::updateFromEntryPoint, this, std::placeholders::_1), entryPointID);            
            entryPoints.push_back(_newEntryPoint);


            /*std::shared_ptr<CEntryPoint2> _newEntryPoint2 = std::make_shared<BRTBase::CEntryPointWithData<int> >(std::bind(&CProcessorBase::updateFromEntryPoint, this, std::placeholders::_1), entryPointID);
            entryPoints.push_back(_newEntryPoint2);*/            
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
            
            //BRTBase::CEntryPointWithData<std::vector<float>>*  _entryPoint = GetEntryPoint2(entryPointID);
            //_exitPoint->attach(*_entryPoint);

            std::shared_ptr<BRTBase::CEntryPointWithData<std::vector<float>> > _entryPoint2 = GetEntryPoint(entryPointID);
            _exitPoint->attach(*_entryPoint2.get());
        }

        
        BRTBase::CEntryPointWithData<std::vector<float>> * GetEntryPoint2(std::string _id) {        
            for (auto& it : entryPoints) {
                if (it->GetID() == _id) {                 
                    CEntryPointWithData<std::vector<float>>* a = dynamic_cast<CEntryPointWithData<std::vector<float>>*>(it.get());                    
                    return a;                                    
                }
            }
            return nullptr;
        }
        
        std::shared_ptr<BRTBase::CEntryPointWithData<std::vector<float>> > GetEntryPoint(std::string _id) {
        for (auto& it : entryPoints) {
            if (it->GetID() == _id) {                
                //std::shared_ptr<BRTBase::CEntryPointWithData<std::vector<float>> >  b = std::dynamic_pointer_cast<BRTBase::CEntryPointWithData<std::vector<float>>>(it);
                //return b;
                return std::dynamic_pointer_cast<BRTBase::CEntryPointWithData<std::vector<float>>>(it); 
            }
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