#pragma once
#include "ObserverBase.hpp"
#include "ExitPoint.h"
#include <functional>

namespace BRTBase {

   
    class CEntryPoint2 : public Observer<CExitPoint>
    {
    protected:                
        CEntryPoint2(std::function<void(std::string)> _callBack, std::string _id, int _multiplicity = 1) : callBackUpdate{ _callBack }, id{ _id }, multiplicity{ _multiplicity } {}
    public:
        
        virtual ~CEntryPoint2() {}        
        virtual void Update(CExitPoint* subject) =0;
                
        std::string GetID() { return id; }
        int GetMultiplicity() { return multiplicity; }                    
        
        // Vars
        std::function<void(std::string)> callBackUpdate;
        std::string id;
        int multiplicity;
    };


    template <class T>
    class CEntryPointWithData : public CEntryPoint2 {

    public:

        CEntryPointWithData(std::function<void(std::string)> _callBack, std::string _id, int _multiplicity = 1) : CEntryPoint2(_callBack, _id, _multiplicity) {}
        ~CEntryPointWithData() {};
    
        void Update(CExitPoint* subject)
        {
            std::cout << "Entry point recebing Updating from : " << typeid(subject).name() << std::endl;
            data = subject->GetBuffer();
            //TODO Manage multiplicity
            
            callBackUpdate(id);
        }

        T GetData() const {
            return data;
        }

    private:
        T data;
    };     
}
