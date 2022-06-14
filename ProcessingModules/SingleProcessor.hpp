#pragma once
#include "../Base/BaseClasses.hpp"
#include "../Base/EntryPoint.hpp"

#include <memory>
#include <vector>
#include <algorithm>

    

class CSingleProcessor {
public:
    CSingleProcessor() {        
        std::shared_ptr<BRT_Base::CEntryPoint<CSingleProcessor> > _newEntryPoint = std::make_shared<BRT_Base::CEntryPoint<CSingleProcessor> >(*this);
        entryPoint = _newEntryPoint;
        exitPoint = std::make_shared<BRT_Base::CExitPoint>();
        gain = 1.0f;
    };

    
	void connectEntryTo(std::shared_ptr<BRT_Base::CExitPoint> _exitPoint) {
        _exitPoint->attach(*entryPoint.get());        
	}

    void updateFromEntryPoint() {
        std::cout << "SingleProcessor Updating --> Recibing buffer" << std::endl;
        std::vector<float> temp = entryPoint->GetBuffer();
        Process(temp);
    }

    std::shared_ptr<BRT_Base::CExitPoint> GetExitPoint() { return exitPoint; }

    void setGain(float _gain) { gain = _gain; }

private:

    std::shared_ptr<BRT_Base::CEntryPoint<CSingleProcessor> > entryPoint;
    std::shared_ptr<BRT_Base::CExitPoint> exitPoint;


    float gain;


    void Process(std::vector<float>& inbuffer) {
        
        MultiplyVectorByValue(inbuffer, gain);
        exitPoint->sendData(inbuffer);
    }
    


    void MultiplyVectorByValue(std::vector<float>& v, float k) {
        std::transform(v.begin(), v.end(), v.begin(), [k](float& c) { return c * k; });
    }

};
