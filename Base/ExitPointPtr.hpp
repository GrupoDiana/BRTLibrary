#ifndef _EXIT_POINT_PTR_
#define _EXIT_POINT_PTR_

#include <Base/ObserverBase.hpp>
#include <ServiceModules/HRTF.h>

namespace BRTServices { class CHRTF; }

//using chrtf = BRTServices::CHRTF;

namespace BRTBase {
    template <class T>
    class CExitPointPtrBase : public Subject
    {

    public:
        //CExitPointPtrBase(const std::shared_ptr<T>& value) : myData(value) {}
        CExitPointPtrBase(std::string _id) : id(_id) {}
        
        const std::shared_ptr<T>& GetData() const { return myData; }
        void SetData(const std::shared_ptr<T>& value) { myData = value; }
        
        std::string GetID() { return id; };

        void sendData(const std::shared_ptr<T>& _value) {
            SetData(_value);
            notify();
        }
    private:
        std::shared_ptr<T> myData;
        std::string id;
    };

    using CExitPointHRTFPtr = CExitPointPtrBase<BRTServices::CHRTF>;
}

#endif