#pragma once
#include "ObserverBase.hpp"
#include "ExitPoint.h"
#include <functional>

namespace BRTBase {

    //class CEntryPointSamplesVector : public Observer<CExitPointSamplesVector>, public Attribute<std::vector<float>>
    //{
    //public:
    //    CEntryPointSamplesVector(std::function<void(std::string)> _callBack, std::string _id, int _multiplicity = 1) : callBackUpdate{ _callBack }, /*id{ _id },*/ multiplicity{ _multiplicity }, Attribute<std::vector<float>>(_id) {}
    //    ~CEntryPointSamplesVector() {}
    //    
    //    void Update(CExitPointSamplesVector* subject)
    //    {
    //        std::cout << "Entry point recebing Updating from : " << typeid(subject).name() << std::endl;
    //        //std::vector<float> a = subject->getAttr();
    //        //setAttr(a);
    //        setAttr(subject->getAttr());
    //        //TODO Manage multiplicity                    
    //        callBackUpdate(GetID());
    //    }
    //            

    //    //std::string GetID() { return id; }
    //    int GetMultiplicity() { return multiplicity; }                    
    //    
    //    //std::vector<float> GetData() const { return data; }

    //    // Vars
    //    std::function<void(std::string)> callBackUpdate;
    //    //std::string id;
    //    int multiplicity;
    //    //std::vector<float> data;
    //};

  

    template <class T>
    class CEntryPointBase : public Observer2, public Attribute<T> {
    public:
        CEntryPointBase(std::function<void(std::string)> _callBack, std::string _id, int _multiplicity) : callBackUpdate{ _callBack }, multiplicity{ _multiplicity }, Attribute<T>(_id) {}
        ~CEntryPointBase() {}
        
        void Update(Subject2* subject) {
            CExitPointBase<T>* _subject = (static_cast<CExitPointBase<T>*>(subject));
            Update(_subject);
        };

        void Update(CExitPointBase<T>* subject)
        {
            std::cout << "Entry point recebing Updating from : " << typeid(subject).name() << std::endl;            
            this->setAttr(subject->getAttr());
            //TODO Manage multiplicity                    
            callBackUpdate(this->GetID());
        }
        int GetMultiplicity() { return multiplicity; }

        // Vars
        std::function<void(std::string)> callBackUpdate;
        //std::string id;
        int multiplicity;
    };
   
    typedef CEntryPointBase<std::vector<float>> CEntryPointSamplesVector;
    typedef CEntryPointBase<int> CEntryPointInt;
    
    //class CEntryPointBase : public Observer<CExitPoint>
    //{
    //protected:                
    //    CEntryPointBase(std::function<void(std::string)> _callBack, std::string _id, int _multiplicity = 1) : callBackUpdate{ _callBack }, id{ _id }, multiplicity{ _multiplicity } {}
    //public:
    //    
    //    virtual ~CEntryPointBase() {}        
    //    virtual void Update(CExitPoint* subject) =0;
    //            
    //    std::string GetID() { return id; }
    //    int GetMultiplicity() { return multiplicity; }                    
    //    
    //    // Vars
    //    std::function<void(std::string)> callBackUpdate;
    //    std::string id;
    //    int multiplicity;
    //};


    //template <class T>
    //class CEntryPoint2 : public CEntryPointBase {

    //public:

    //    CEntryPoint2(std::function<void(std::string)> _callBack, std::string _id, int _multiplicity = 1) : CEntryPointBase(_callBack, _id, _multiplicity) {}
    //    ~CEntryPoint2() {};
    //
    //    void Update(CExitPoint* subject)
    //    {
    //        std::cout << "Entry point recebing Updating from : " << typeid(subject).name() << std::endl;
    //        data = subject->GetBuffer();
    //        //TODO Manage multiplicity
    //        
    //        callBackUpdate(id);
    //    }

    //    T GetData() const {
    //        return data;
    //    }

    //private:
    //    T data;
    //};     
}
