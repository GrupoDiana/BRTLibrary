#ifndef _EXIT_POINT_
#define _EXIT_POINT_

#include <Base/ObserverBase.hpp>
#include <Common/Buffer.h>
#include <Common/Transform.h>
#include <Common/EarsTransform.hpp>
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
    //typedef CExitPointBase<CMonoBuffer<float> > CExitPointSamplesVector;     
    //typedef CExitPointBase<Common::CTransform > CExitPointTransform;
    //typedef CExitPointBase<Common::CEarsTransforms> CExitPointEarsTransform;

    using CExitPointSamplesVector = CExitPointBase<CMonoBuffer<float> >;
    using CExitPointTransform = CExitPointBase<Common::CTransform >;
    using CExitPointEarsTransform = CExitPointBase<Common::CEarsTransforms>;
}
#endif