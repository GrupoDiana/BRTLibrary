#pragma once
#include "ObserverBase.hpp"
#include "ExitPoint.h"
#include "Common/Buffer.h"
#include "Common/Transform.h"
#include <functional>

namespace BRTBase {
   
    template <class T>
    class CEntryPointBase : public Observer, public Attribute<T> {
    public:
        CEntryPointBase(std::function<void(std::string)> _callBack, std::string _id, int _multiplicity) : callBackUpdate{ _callBack }, multiplicity{ _multiplicity }, Attribute<T>(_id) {}
        ~CEntryPointBase() {}
        
        void Update(Subject* subject) {
            CExitPointBase<T>* _subject = (static_cast<CExitPointBase<T>*>(subject));
            Update(_subject);
        };

        void Update(CExitPointBase<T>* subject)
        {            
            this->setAttr(subject->getAttr());            
            if (multiplicity == 0) { /*Do nothing*/ }
            else if (multiplicity == 1) { callBackUpdate(this->GetID()); }
            else if (multiplicity >1 ) { /*TODO manage this */ callBackUpdate(this->GetID()); }
            
        }
        int GetMultiplicity() { return multiplicity; }

        // Vars
        std::function<void(std::string)> callBackUpdate;
        //std::string id;
        int multiplicity;
    };
   
    typedef CEntryPointBase<CMonoBuffer<float>> CEntryPointSamplesVector;
    //typedef CEntryPointBase<int> CEntryPointInt;        
    typedef CEntryPointBase<Common::CTransform> CEntryPointTransform;
}
