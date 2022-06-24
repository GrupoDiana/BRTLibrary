#pragma once
#include "../Base/BaseClasses.hpp"
#include "../Base/EntryPoint.hpp"

#include <memory>
#include <vector>
#include <algorithm>
    
template <class T>
class CProcessorBase {
public:
    CProcessorBase() {}
    virtual ~CProcessorBase() {}
    virtual void Update() = 0;


    void CreateEntryPoint(T& owner,std::string entryPointID) {
        std::shared_ptr<BRT_Base::CEntryPoint<T> > _newEntryPoint = std::make_shared<BRT_Base::CEntryPoint<T> >(owner, entryPointID);
        entryPoints.push_back(_newEntryPoint);
    }

    void CreateExitPoint(std::string exitPointID) {
        std::shared_ptr<BRT_Base::CExitPoint> _newExitPoint = std::make_shared<BRT_Base::CExitPoint>(exitPointID);
        exitPoints.push_back(_newExitPoint);
    }


    void updateFromEntryPoint(std::string entryPointID) {
        std::cout << "SingleProcessor Updating --> Recibing buffer" << std::endl;

        //TODO Check if we have received an update from all the entry points, then we should call to Update.
        Update();
    }

    void connectEntryTo(std::shared_ptr<BRT_Base::CExitPoint> _exitPoint, std::string entryPointID) {
        std::shared_ptr<BRT_Base::CEntryPoint<T> > _entryPoint = GetEntryPoint(entryPointID);
        _exitPoint->attach(*_entryPoint.get());
    }

    std::shared_ptr<BRT_Base::CEntryPoint<T> > GetEntryPoint(std::string _id) {
        for (auto& it : entryPoints) {
            if (it->GetID() == _id) { return it; }
        }
        return nullptr;
    }

    std::shared_ptr<BRT_Base::CExitPoint > GetExitPoint(std::string _id) {
        for (auto& it : exitPoints) {
            if (it->GetID() == _id) { return it; }
        }
        return nullptr;
    }

private:
    std::vector<std::shared_ptr<BRT_Base::CEntryPoint<T> >> entryPoints;
    std::vector<std::shared_ptr<BRT_Base::CExitPoint >> exitPoints;
};


class CSingleProcessorNew : public CProcessorBase<CSingleProcessorNew>
{
public:
    CSingleProcessorNew() : gain{ 1.0f } {
        CreateEntryPoint(*this, "inputSamples");
        CreateExitPoint("outputSamples");
        
    }

    void setGain(float _gain) { gain = _gain; }

    void Update() {        
        std::vector<float> temp = GetEntryPoint("inputSamples")->GetData();
        Process(temp);
    }

private:
    float gain;


    // Methods
    void Process(std::vector<float>& inbuffer) {
        MultiplyVectorByValue(inbuffer, gain);
        GetExitPoint("outputSamples")->sendData(inbuffer);        
    }

    void MultiplyVectorByValue(std::vector<float>& v, float k) {
        std::transform(v.begin(), v.end(), v.begin(), [k](float& c) { return c * k; });
    }
};


//class CSingleProcessor {
//public:
//    CSingleProcessor() {        
//        std::shared_ptr<BRT_Base::CEntryPoint<CSingleProcessor> > _newEntryPoint = std::make_shared<BRT_Base::CEntryPoint<CSingleProcessor> >(*this, "inputSamples");
//        //entryPoint = _newEntryPoint;
//        entryPoints.push_back(_newEntryPoint);
//
//        exitPoint = std::make_shared<BRT_Base::CExitPoint>("outputSamples");
//        gain = 1.0f;
//        
//    };
//
//    
//	void connectEntryTo(std::shared_ptr<BRT_Base::CExitPoint> _exitPoint, std::string entryPointID) {
//
//        std::shared_ptr<BRT_Base::CEntryPoint<CSingleProcessor> > _entryPoint = GetEntryPoint(entryPointID);
//        _exitPoint->attach(*_entryPoint.get());        
//	}
//
//    void updateFromEntryPoint(std::string entryPointID) {
//        std::cout << "SingleProcessor Updating --> Recibing buffer" << std::endl;
//
//        std::shared_ptr<BRT_Base::CEntryPoint<CSingleProcessor> > _entryPoint = GetEntryPoint(entryPointID);
//
//        std::vector<float> temp = _entryPoint->GetBuffer();
//        Process(temp);
//    }
//
//    std::shared_ptr<BRT_Base::CExitPoint> GetExitPoint(std::string _id = "1") { return exitPoint; }
//
//    void setGain(float _gain) { gain = _gain; }
//
//private:
//
//    std::vector<std::shared_ptr<BRT_Base::CEntryPoint<CSingleProcessor> >> entryPoints;
//
//    //std::shared_ptr<BRT_Base::CEntryPoint<CSingleProcessor> > entryPoint;
//    std::shared_ptr<BRT_Base::CExitPoint> exitPoint;
//    
//    float gain;
//
//
//
//
//
//    std::shared_ptr<BRT_Base::CEntryPoint<CSingleProcessor> > GetEntryPoint(std::string _id) {                
//        for (auto& it : entryPoints) {
//            if (it->GetID() == _id) { return it; }
//        }
//        return nullptr;                
//    }
//
//    void Process(std::vector<float>& inbuffer) {
//        
//        MultiplyVectorByValue(inbuffer, gain);
//        exitPoint->sendData(inbuffer);
//    }
//    
//
//
//    void MultiplyVectorByValue(std::vector<float>& v, float k) {
//        std::transform(v.begin(), v.end(), v.begin(), [k](float& c) { return c * k; });
//    }
//
//};
