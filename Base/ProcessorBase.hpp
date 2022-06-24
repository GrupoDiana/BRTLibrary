#pragma once

#include "../Base/EntryPoint.hpp"
#include <vector>


template <class T>
class CProcessorBase {
public:
    CProcessorBase() {}
    virtual ~CProcessorBase() {}
    virtual void Update() = 0;


    void CreateEntryPoint(T& owner, std::string entryPointID) {
        std::shared_ptr<BRT_Base::CEntryPoint<T> > _newEntryPoint = std::make_shared<BRT_Base::CEntryPoint<T> >(owner, entryPointID);
        entryPoints.push_back(_newEntryPoint);
    }

    void CreateExitPoint(std::string exitPointID) {
        std::shared_ptr<BRT_Base::CExitPoint> _newExitPoint = std::make_shared<BRT_Base::CExitPoint>(exitPointID);
        exitPoints.push_back(_newExitPoint);
    }


    void updateFromEntryPoint(std::string entryPointID) {
        std::cout << "SingleProcessor Updating --> Recibing buffer" << std::endl;

        //TODO Check if we have received an update from all the entry points, then we should call to Update.
        Update();
    }

    void connectEntryTo(std::shared_ptr<BRT_Base::CExitPoint> _exitPoint, std::string entryPointID) {
        std::shared_ptr<BRT_Base::CEntryPoint<T> > _entryPoint = GetEntryPoint(entryPointID);
        _exitPoint->attach(*_entryPoint.get());
    }

    std::shared_ptr<BRT_Base::CEntryPoint<T> > GetEntryPoint(std::string _id) {
        for (auto& it : entryPoints) {
            if (it->GetID() == _id) { return it; }
        }
        return nullptr;
    }

    std::shared_ptr<BRT_Base::CExitPoint > GetExitPoint(std::string _id) {
        for (auto& it : exitPoints) {
            if (it->GetID() == _id) { return it; }
        }
        return nullptr;
    }

private:
    std::vector<std::shared_ptr<BRT_Base::CEntryPoint<T> >> entryPoints;
    std::vector<std::shared_ptr<BRT_Base::CExitPoint >> exitPoints;
};