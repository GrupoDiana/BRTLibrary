#pragma once
#include "ObserverBase.hpp"
#include "ExitPoint.h"
#include <functional>

namespace BRTBase {

    class CEntryPoint2 : public Observer<CExitPoint>
    {
    public:

        CEntryPoint2(std::function<void(std::string)> callBack, std::string _id, int _multiplicity = 1);
        ~CEntryPoint2() {}
        void Update(CExitPoint* subject);

        std::vector<float> GetData() const;
        std::string GetID() { return id; };
        
        //std::function<void(int)> callBack;
        
        
    private:

        std::function<void(std::string)> callBackUpdate;

        std::vector<float> data;
        std::string id;
        int multiplicity;
    };

    /////////////////////////////
    //// Methods definition
    ////////////////////////////
    
    CEntryPoint2::CEntryPoint2(std::function<void(std::string)> _callBack, std::string _id, int _multiplicity) : id{ _id }, multiplicity{ _multiplicity } {
        callBackUpdate = _callBack;        
    }

    void CEntryPoint2::Update(CExitPoint* subject)
    {
        std::cout << "Entry point recebing Updating from : " << typeid(subject).name() << std::endl;
        data = subject->GetBuffer();
        //TODO Manage multiplicity
        //ownerModule.updateFromEntryPoint(id);
        callBackUpdate(id);
    }

    std::vector<float> CEntryPoint2::GetData() const {
        return data;
    }

}