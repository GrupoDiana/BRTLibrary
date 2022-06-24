#pragma once

#include "ObserverBase.hpp"
#include <iostream>

namespace BRT_Base {
    class CExitPoint : public Subject<CExitPoint>
    {
    public:
        CExitPoint(std::string _id = "1") : id{_id} {}
        ~CExitPoint() {}

        void sendData(std::vector<float>& _buffer) {
            buffer = _buffer;
            notify();
        }


        std::vector<float> GetBuffer() const { return buffer; };
        std::string GetID() { return id; };

    private:
        std::vector<float> buffer;
        std::string id;
    };
}