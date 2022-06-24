#pragma once
#include "ObserverBase.hpp"
#include "ExitPoint.h"

namespace BRT_Base {

    template <class U>
    class CEntryPoint : public Observer<CExitPoint>
    {
    public:

        CEntryPoint(U& _myOwner, std::string _id, int _multiplicity = 1);
        ~CEntryPoint() {}
        void Update(CExitPoint* subject);

        std::vector<float> GetData() const;
        std::string GetID() { return id; };

    private:
        U& ownerModule;

        std::vector<float> data;
        std::string id;
        int multiplicity;
    };

    /////////////////////////////
    //// Methods definition
    ////////////////////////////

    template <typename U>
    CEntryPoint<U>::CEntryPoint(U& _myOwner, std::string _id, int _multiplicity) : ownerModule{ _myOwner }, id{ _id }, multiplicity{ _multiplicity } {}

    template <typename U>
    void CEntryPoint<U>::Update(CExitPoint* subject)
    {
        std::cout << "Entry point recebing Updating from : " << typeid(subject).name() << std::endl;
        data = subject->GetBuffer();
        //TODO Manage multiplicity
        ownerModule.updateFromEntryPoint(id);
    }

    template <typename U>
    std::vector<float> CEntryPoint<U>::GetData() const {
        return data;
    }

}