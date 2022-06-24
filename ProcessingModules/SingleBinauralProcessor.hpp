#pragma once
#include "../brt/Base/ObserverBase.hpp"
#include "../Base/EntryPoint.hpp"

#include <memory>
#include <vector>
#include <algorithm>
    
class CSingleBinauralProcessorNew : public CProcessorBase<CSingleBinauralProcessorNew> {
public:
    CSingleBinauralProcessorNew() : leftGain{ 1.0f }, rightGain{ 1.0f } {
        CreateEntryPoint(*this, "inputSamples");
        CreateExitPoint("leftEar");
        CreateExitPoint("rightEar");
    }
    
    void Update() {
        std::vector<float> temp = GetEntryPoint("inputSamples")->GetData();
        Process(temp);
    }

    void setLeftGain(float _gain) { leftGain = _gain; }
    void setRighttGain(float _gain) { rightGain = _gain; }

private: 
    float leftGain;
    float rightGain;



    void Process(std::vector<float>& _inbuffer) {

        std::vector<float> _leftBuffer = _inbuffer;
        std::vector<float> _rightBuffer = _inbuffer;

        MultiplyVectorByValue(_leftBuffer, leftGain);
        GetExitPoint("leftEar")->sendData(_leftBuffer);

        MultiplyVectorByValue(_rightBuffer, rightGain);
        GetExitPoint("rightEar")->sendData(_rightBuffer);
    }


    void MultiplyVectorByValue(std::vector<float>& v, float k) {
        std::transform(v.begin(), v.end(), v.begin(), [k](float& c) { return c * k; });
    }
};

//class CSingleBinauralProcessor {
//public:
//    CSingleBinauralProcessor() {
//        std::shared_ptr<BRT_Base::CEntryPoint<CSingleBinauralProcessor> > _newEntryPoint = std::make_shared<BRT_Base::CEntryPoint<CSingleBinauralProcessor> >(*this, "inputSamples");
//        entryPoint = _newEntryPoint;
//                
//        leftEarExitPoint = std::make_shared<BRT_Base::CExitPoint>("leftEar");
//        rightEarExitPoint = std::make_shared<BRT_Base::CExitPoint>("rightEar");
//        
//        leftGain = 1.0f;        
//        rightGain = 1.0f;
//    };
//
//    
//    void connectEntryTo(std::shared_ptr<BRT_Base::CExitPoint> _exitPoint, std::string entryPointID) {
//
//        //std::shared_ptr<BRT_Base::CEntryPoint<CSingleProcessor> > _entryPoint = GetEntryPoint(entryPointID);
//        _exitPoint->attach(*entryPoint.get());
//    }
//
//    void updateFromEntryPoint(std::string id) {
//        std::cout << "SingleProcessor Updating --> Recibing buffer" << std::endl;
//        std::vector<float> temp = entryPoint->GetBuffer();
//        Process(temp);
//    }
//
//    std::shared_ptr<BRT_Base::CExitPoint> GetExitPoint(std::string _id = "1") { 
//        if (_id == "leftEar") { return leftEarExitPoint; }
//        else if (_id == "rightEar") { return rightEarExitPoint; }
//        else return nullptr;            
//    }
//
//    void setLeftGain(float _gain) { leftGain = _gain; }
//    void setRighttGain(float _gain) { rightGain = _gain; }
//
//private:
//
//    std::shared_ptr<BRT_Base::CEntryPoint<CSingleBinauralProcessor> > entryPoint;
//    
//    std::shared_ptr<BRT_Base::CExitPoint> leftEarExitPoint;
//    std::shared_ptr<BRT_Base::CExitPoint> rightEarExitPoint;
//    
//    
//    float leftGain;
//    float rightGain;
//
//    
//
//    void Process(std::vector<float>& _inbuffer) {
//        
//        std::vector<float> _leftBuffer = _inbuffer;
//        std::vector<float> _rightBuffer = _inbuffer;
//        
//        MultiplyVectorByValue(_leftBuffer, leftGain);
//        leftEarExitPoint->sendData(_leftBuffer);
//        
//        MultiplyVectorByValue(_rightBuffer, rightGain);
//        rightEarExitPoint->sendData(_rightBuffer);
//    }
//    
//
//
//    void MultiplyVectorByValue(std::vector<float>& v, float k) {
//        std::transform(v.begin(), v.end(), v.begin(), [k](float& c) { return c * k; });
//    }
//
//};
