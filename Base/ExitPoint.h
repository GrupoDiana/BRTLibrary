#pragma once

#include "ObserverBase.hpp"
#include <iostream>

namespace BRTBase {
    //class CExitPoint : public Subject<CExitPoint>
    //{
    //public:
    //    CExitPoint(std::string _id = "1") : id{_id} {}
    //    ~CExitPoint() {}

    //    void sendData(std::vector<float>& _buffer) {
    //        buffer = _buffer;
    //        notify();
    //    }


    //    std::vector<float> GetBuffer() const { return buffer; };
    //    std::string GetID() { return id; };

    //private:
    //    std::vector<float> buffer;
    //    std::string id;
    //};



    template<typename T> class Attribute  {
    public:
        Attribute(std::string _id) : mAttr(T()), id{ _id } { }
        Attribute(std::string _id, T pAttr) : mAttr(pAttr) { }
        
        void setAttr(const T& pAttr) { mAttr = pAttr; }
        T getAttr() { return mAttr; }
        std::string GetID() { return id; };
    private:
        T mAttr;
        std::string id;
    };
    
    //class CExitPointSamplesVector : public Subject<CExitPointSamplesVector>, public Attribute<std::vector<float>>
    //{
    //public:
    //    CExitPointSamplesVector(std::string _id) :/* id{ _id },*/ Attribute<std::vector<float>>(_id) { }                                
    //    ~CExitPointSamplesVector() {}

    //    //std::string GetID() { return id; };
    //    void sendData(std::vector<float>& _buffer) {
    //        setAttr(_buffer);
    //        notify();
    //    }
    //    
    //private:
    //    //std::string id;
    //};


    template <class T>
    class CExitPointBase : public Subject2, public Attribute<T>
    {
    public:
        CExitPointBase(std::string _id) : Attribute<T>(_id) { }
        ~CExitPointBase() {}

        
        void sendData(T& _buffer) {
            this->setAttr(_buffer);
            notify();            
        }

    private:        
    };

    typedef CExitPointBase<std::vector<float> > CExitPointSamplesVector;
    typedef CExitPointBase<int > CExitPointInt;
}