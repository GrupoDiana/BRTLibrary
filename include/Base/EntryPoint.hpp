#ifndef _ENTRY_POINT_
#define _ENTRY_POINT_

#include <functional>
#include <Base/ExitPoint.hpp>
#include <Base/ObserverBase.hpp>
#include <Common/Buffer.h>
#include <Common/Transform.h>
#include <Common/EarsTransform.hpp>
#include <Base/Command.hpp>


namespace BRTBase {
   
    template <class T>
    class CEntryPointBase : public Observer/*, public CEntryExitPointData<T> */{
    public:
        CEntryPointBase(std::function<void(std::string)> _callBack, std::string _id, int _multiplicity) : callBackUpdate{ _callBack }, id{ _id }, multiplicity{ _multiplicity }/*, CEntryExitPointData<T>() */{}
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
        void SetData(const T& _data) { data = _data; }
        T GetData() { return data; }

    private:
        // Vars
        std::function<void(std::string)> callBackUpdate;
        std::string id;
        int multiplicity;
 
        T data;
    };
           
    using CEntryPointSamplesVector = CEntryPointBase<CMonoBuffer<float>>;
    using CEntryPointTransform = CEntryPointBase<Common::CTransform>;     
    using CEntryPointCommand = CEntryPointBase<BRTBase::CCommand>;
    using CEntryPointID = CEntryPointBase<std::string>;

    using CEntryPointHRTFPtr = CEntryPointBase< std::weak_ptr<BRTServices::CHRTF> >;
    using CEntryPointILDPtr = CEntryPointBase< std::weak_ptr<BRTServices::CILD> >;
    using CEntryPointSRTFPtr = CEntryPointBase< std::weak_ptr<BRTServices::CSRTF> >;
}
#endif