#pragma once
#include "ObserverBase.hpp"
#include "ExitPoint.h"
#include "Common/Buffer.h"
#include "Common/Transform.h"
#include <functional>

namespace BRTBase {
   
    template <class T>
    class CEntryPointBase : public Observer, public CEntryExitPointData<T> {
    public:
        CEntryPointBase(std::function<void(std::string)> _callBack, std::string _id, int _multiplicity) : callBackUpdate{ _callBack }, id{ _id }, multiplicity{ _multiplicity }, CEntryExitPointData<T>() {}
        ~CEntryPointBase() {}
        
        void Update(Subject* subject) {
            CExitPointBase<T>* _subject = (static_cast<CExitPointBase<T>*>(subject));
            Update(_subject);
        };

        void Update(CExitPointBase<T>* subject)
        {            
            this->SetData(subject->GetData());            
            if (multiplicity == 0) { /*Do nothing*/ }
            else if (multiplicity == 1) { callBackUpdate(this->GetID()); }
            else if (multiplicity >1 ) { /*TODO manage this */ callBackUpdate(this->GetID()); }
            
        }
        
        int GetMultiplicity() { return multiplicity; }
        std::string GetID() { return id; };

        // Vars
        std::function<void(std::string)> callBackUpdate;
        std::string id;
        int multiplicity;
    };
   
    typedef CEntryPointBase<CMonoBuffer<float>> CEntryPointSamplesVector;
    
    typedef CEntryPointBase<Common::CTransform> CEntryPointTransform;

    typedef CEntryPointBase<Common::CEarsTransforms> CEntryPoinEarsTransform;
}
