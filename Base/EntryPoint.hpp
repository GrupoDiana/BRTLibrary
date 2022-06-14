#pragma once
#include "BaseClasses.hpp"
#include "ExitPoint.h"

namespace BRT_Base {

    template <class U>
    class CEntryPoint : public Observer<CExitPoint>
    {
    public:

        CEntryPoint(U& _myOwner);
        ~CEntryPoint() {}
        void update(CExitPoint* subject);

        std::vector<float> GetBuffer() const;

    private:
        U& ownerModule;

        std::vector<float> buffer;
    };


    template <typename U>
    CEntryPoint<U>::CEntryPoint(U& _myOwner) : ownerModule{ _myOwner } {}

    template <typename U>
    void CEntryPoint<U>::update(CExitPoint* subject)
    {
        std::cout << "Entry point recebing Updating from : " << typeid(subject).name() << std::endl;
        buffer = subject->GetBuffer();
        ownerModule.updateFromEntryPoint();
    }

    template <typename U>
    std::vector<float> CEntryPoint<U>::GetBuffer() const {
        return buffer;
    }

}