#pragma once

#include "ObserverBase.hpp"
#include <iostream>

namespace BRTBase {

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
    
   
    template <class T>
    class CExitPointBase : public Subject, public Attribute<T>
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