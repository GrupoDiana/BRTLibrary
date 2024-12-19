/**
* \class CEntryPointBase
*
* \brief Declaration of CEntryPointBase class
* \date	June 2023
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco, F. Morales-Benitez ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga)||
* \b Contact: areyes@uma.es
*
* \b Copyright: University of Malaga
* 
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Acknowledgement: This project has received funding from the European Union�s Horizon 2020 research and innovation programme under grant agreement no.101017743
* 
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*/

#ifndef _ENTRY_POINT_
#define _ENTRY_POINT_

#include <functional>
#include <Connectivity/ExitPoint.hpp>
#include <Connectivity/ObserverBase.hpp>
#include <Common/Buffer.hpp>
#include <Common/Transform.hpp>
#include <Connectivity/Command.hpp>
#include <ServiceModules/AmbisonicBIR.hpp>


namespace BRTConnectivity {
   
    template <class T>
    class CEntryPointBase : public Observer {
    public:
        CEntryPointBase(std::function<void(std::string)> _callBack, std::string _id, bool _notify) : callBackUpdate{ _callBack }, id{ _id }, notify{ _notify }, connections{ 0 } {}
        ~CEntryPointBase() {}

        void Update(Subject* subject) {
            CExitPointBase<T>* _subject = (static_cast<CExitPointBase<T>*>(subject));
            Update(_subject);
        };

        void Update(CExitPointBase<T>* subject)
        {
            this->SetData(subject->GetData());
            if (notify) { callBackUpdate(this->GetID()); }
        }
        
        int AddConnection() { 
            connections++; 
            return connections;
        }
        int RemoveConnection() { 
            if (connections > 0) { connections--; }
            return connections;
        }
        int GetConnections() { return connections; }
        
        std::string GetID() { return id; };

        void SetData(const T& _data) { data = _data; }
        T GetData() { return data; }

    private:
        // Vars
        std::function<void(std::string)> callBackUpdate;
        std::string id;
        int connections;          
        bool notify;
        
        T data;
    };
           
    using CEntryPointSamplesVector = CEntryPointBase<CMonoBuffer<float>>;
    using CEntryPointMultipleSamplesVector = CEntryPointBase<std::vector<CMonoBuffer<float>>>;
    using CEntryPointTransform = CEntryPointBase<Common::CTransform>;     
    using CEntryPointCommand = CEntryPointBase<BRTConnectivity::CCommand>;
    using CEntryPointID = CEntryPointBase<std::string>;

    using CEntryPointHRTFPtr = CEntryPointBase< std::weak_ptr<BRTServices::CHRTF> >;
    using CEntryPointHRBRIRPtr = CEntryPointBase< std::weak_ptr<BRTServices::CHRBRIR> >;
    using CEntryPointILDPtr = CEntryPointBase< std::weak_ptr<BRTServices::CSOSFilters> >;
    using CEntryPointDirectivityTFPtr = CEntryPointBase< std::weak_ptr<BRTServices::CDirectivityTF> >;
    using CEntryPointABIRPtr = CEntryPointBase< std::weak_ptr<BRTServices::CAmbisonicBIR> >;
}
#endif