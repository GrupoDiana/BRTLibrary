#ifndef _EXIT_POINT_
#define _EXIT_POINT_

#include <iostream>
#include <memory>
#include <Base/ObserverBase.hpp>
#include <Common/Buffer.hpp>
#include <Common/Transform.hpp>
#include <Base/Command.hpp>


namespace BRTServices { class CHRTF; class CILD; class CSRTF; }

namespace BRTBase {          
    template <class T>
    class CExitPointBase : public Subject/*, public CEntryExitPointData<T>*/
    {
    public:
        CExitPointBase(std::string _id) : id{ _id }/*, CEntryExitPointData<T>() */{ }
        ~CExitPointBase() {}

        std::string GetID() { return id; };
        void SetData(const T& _data) { data = _data; }        
        T GetData() { return data; }

        void sendData(T& _data) {
            this->SetData(_data);
            notify();            
        }       

        void sendDataPtr(T _data) {
            this->SetData(_data);
            notify();
        }
    private:    
        std::string id;
        T data;
    };
    
    using CExitPointSamplesVector = CExitPointBase<CMonoBuffer<float> >;
    using CExitPointTransform = CExitPointBase<Common::CTransform >;
    using CExitPointCommand = CExitPointBase<BRTBase::CCommand>;
    using CExitPointID = CExitPointBase<std::string>;

    using CExitPointHRTFPtr = CExitPointBase< std::weak_ptr<BRTServices::CHRTF> >;
    using CExitPointILDPtr = CExitPointBase< std::weak_ptr<BRTServices::CILD> >;
    using CExitPointSRTFPtr = CExitPointBase< std::weak_ptr<BRTServices::CSRTF> >;
}
#endif