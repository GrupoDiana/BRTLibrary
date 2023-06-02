#ifndef _ENTRY_POINT_PTR_
#define _ENTRY_POINT_PTR_

#include "ExitPointPtr.hpp"


namespace BRTBase {

    template <class T>
    class CEntryPointPtrBase : public Observer 
    {
    public:
        CEntryPointPtrBase(std::function<void(std::string)> _callBack, std::string _id, int _multiplicity) : callBackUpdate{ _callBack }, id{ _id }, multiplicity{ _multiplicity } {}
        ~CEntryPointPtrBase() {}

        void Update(Subject* subject) {
            CExitPointPtrBase<T>* _subject = (static_cast<CExitPointPtrBase<T>*>(subject));
            Update(_subject);
        }

        void Update(CExitPointPtrBase<T>* subject)
        {
            this->SetData(subject->GetData());
            if (multiplicity == 0) { /*Do nothing*/ }
            else if (multiplicity == 1) { callBackUpdate(this->GetID()); }
            else if (multiplicity > 1) { /*TODO manage this */ callBackUpdate(this->GetID()); }

        }
        int GetMultiplicity() { return multiplicity; }
        std::string GetID() { return id; };
        void SetData(const std::weak_ptr<T>& value) { myData = value; }
        const std::weak_ptr<T>& GetData() const { return myData; }
    private:

        // Vars
        std::function<void(std::string)> callBackUpdate;
        std::string id;
        int multiplicity;

   
        std::weak_ptr<T> myData;
    };

    using CEntryPointHRTFPtr = CEntryPointPtrBase<BRTServices::CHRTF>;
    using CEntryPointILDPtr = CEntryPointPtrBase<BRTServices::CILD>;
}

#endif
