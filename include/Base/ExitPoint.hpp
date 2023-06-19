#ifndef _EXIT_POINT_
#define _EXIT_POINT_

#include <Base/ObserverBase.hpp>
#include <Common/Buffer.hpp>
#include <Common/Transform.hpp>
#include <Base/Command.hpp>
#include <iostream>

namespace BRTBase {

    template<typename T> class CEntryExitPointData  {
    public:
        CEntryExitPointData() : mAttr(T()) { }
        CEntryExitPointData(T pAttr) : mAttr(pAttr) { }
        
        void SetData(const T& pAttr) { mAttr = pAttr; }
        T GetData() { return mAttr; }
        
    private:
        T mAttr;        
    };
    
   
    template <class T>
    class CExitPointBase : public Subject, public CEntryExitPointData<T>
    {
    public:
        CExitPointBase(std::string _id) : id{ _id }, CEntryExitPointData<T>() { }
        ~CExitPointBase() {}

        std::string GetID() { return id; };

        void sendData(T& _buffer) {
            this->SetData(_buffer);
            notify();            
        }

    private:    
        std::string id;
    };
    
    using CExitPointSamplesVector = CExitPointBase<CMonoBuffer<float> >;
    using CExitPointTransform = CExitPointBase<Common::CTransform >;
    using CExitPointCommand = CExitPointBase<BRTBase::CCommand>;
    using CExitPointID = CExitPointBase<std::string>;
}
#endif