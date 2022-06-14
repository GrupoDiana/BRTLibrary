#pragma once

#include "BaseClasses.hpp"
#include <iostream>

namespace BRT_Base {
    class CExitPoint : public Subject<CExitPoint>
    {
    public:
        CExitPoint() {}
        ~CExitPoint() {}

        void sendData(std::vector<float>& _buffer) {
            buffer = _buffer;
            notify();
        }

        std::vector<float> GetBuffer() const { return buffer; };

    private:
        std::vector<float> buffer;
    };
}