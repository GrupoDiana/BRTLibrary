#ifndef _EXIT_POINT_PTR_
#define _EXIT_POINT_PTR_

//#include <Base/ObserverBase.hpp>
//#include <ServiceModules/HRTF.hpp>
//#include <ServiceModules/ILD.hpp>
//#include <ServiceModules/SRTF.hpp>
//
//namespace BRTServices { class CHRTF; }
//
////using chrtf = BRTServices::CHRTF;
//
//namespace BRTBase {
//    template <class T>
//    class CExitPointPtrBase : public Subject
//    {
//
//    public:        
//        CExitPointPtrBase(std::string _id) : id(_id) {}
//        
//        const std::weak_ptr<T>& GetData() const { return myData; }
//        void SetData(const std::weak_ptr<T>& value) { myData = value; }
//        
//        std::string GetID() { return id; };
//
//        void sendData(const std::weak_ptr<T>& _value) {
//            SetData(_value);
//            notify();
//        }
//    private:
//        std::weak_ptr<T> myData;
//        std::string id;
//    };
//
//    //using CExitPointHRTFPtr = CExitPointPtrBase<BRTServices::CHRTF>;
//    //using CExitPointILDPtr = CExitPointPtrBase<BRTServices::CILD>;
//    //using CExitPointSRTFPtr = CExitPointPtrBase<BRTServices::CSRTF>;
//}

#endif