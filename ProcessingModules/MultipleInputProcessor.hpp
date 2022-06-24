#pragma once
#include "../Base/BaseClasses.hpp"
#include "../Base/EntryPoint.hpp"

#include <memory>
#include <vector>
#include <algorithm>
#include <string>
    

class CMultipleInputProcessor {
public:
    CSingleProcessor(int numberOfEntryPoints) {        
        std::shared_ptr<BRT_Base::CEntryPoint<CSingleProcessor> > _newEntryPoint = std::make_shared<BRT_Base::CEntryPoint<CSingleProcessor> >(*this, "1");
        entryPoint = _newEntryPoint;
        exitPoint = std::make_shared<BRT_Base::CExitPoint>();
        gain = 1.0f;


        for (int i = 0; i < numberOfEntryPoints; i++) {            
            entryPoints.push_back(std::make_shared<BRT_Base::CEntryPoint<CSingleProcessor> >(*this,  std::to_string(i)));
        }
    };

    
	void connectEntryTo(std::shared_ptr<BRT_Base::CExitPoint> _exitPoint) {
        _exitPoint->attach(*entryPoint.get());        
	}

    void updateFromEntryPoint(std::string id) {
        std::cout << "SingleProcessor Updating --> Recibing buffer" << std::endl;
        std::vector<float> temp = entryPoint->GetBuffer();
        Process(temp);
    }

    std::shared_ptr<BRT_Base::CExitPoint> GetExitPoint() { return exitPoint; }

    void setGain(float _gain) { gain = _gain; }

private:

    std::shared_ptr<BRT_Base::CEntryPoint<CSingleProcessor> > entryPoint;
    std::shared_ptr<BRT_Base::CExitPoint> exitPoint;



    std::vector < std::shared_ptr<BRT_Base::CEntryPoint<CSingleProcessor> >> entryPoints;

    float gain;


    void Process(std::vector<float>& inbuffer) {
        
        MultiplyVectorByValue(inbuffer, gain);
        exitPoint->sendData(inbuffer);
    }
    


    void MultiplyVectorByValue(std::vector<float>& v, float k) {
        std::transform(v.begin(), v.end(), v.begin(), [k](float& c) { return c * k; });
    }

};
