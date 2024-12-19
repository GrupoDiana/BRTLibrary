/**
* \class CExitPointBase
*
* \brief Declaration of CExitPointBase class
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

#ifndef _EXIT_POINT_
#define _EXIT_POINT_

#include <iostream>
#include <memory>
#include <Connectivity/ObserverBase.hpp>
#include <Common/Buffer.hpp>
#include <Common/Transform.hpp>
#include <Connectivity/Command.hpp>
#include <ServiceModules/AmbisonicBIR.hpp>

namespace BRTServices { class CHRTF; class CSOSFilters; class CDirectivityTF; class CHRBRIR; }

namespace BRTConnectivity {          
    template <class T>
    class CExitPointBase : public Subject/*, public CEntryExitPointData<T>*/
    {
    public:
        CExitPointBase(std::string _id) : id{ _id }/*, CEntryExitPointData<T>() */{ }
        ~CExitPointBase() {}

        std::string GetID() { return id; };
        void SetData(const T& _data) { data = _data; }        
        T GetData() { return data; }

        void sendData(T& _data) {
            this->SetData(_data);
            notify();            
        }       

        void sendDataPtr(T _data) {
            this->SetData(_data);
            notify();
        }
    private:    
        std::string id;
        T data;
    };
    
    using CExitPointSamplesVector = CExitPointBase<CMonoBuffer<float> >;
    using CExitPointMultipleSamplesVector = CExitPointBase<std::vector<CMonoBuffer<float>>>;
    using CExitPointTransform = CExitPointBase<Common::CTransform >;
    using CExitPointCommand = CExitPointBase<BRTConnectivity::CCommand>;
    using CExitPointID = CExitPointBase<std::string>;

    using CExitPointHRTFPtr = CExitPointBase< std::weak_ptr<BRTServices::CHRTF> >;
    using CExitPointHRBRIRPtr = CExitPointBase< std::weak_ptr<BRTServices::CHRBRIR> >;
    using CExitPointILDPtr = CExitPointBase< std::weak_ptr<BRTServices::CSOSFilters> >;
    using CExitPointDirectivityTFPtr = CExitPointBase< std::weak_ptr<BRTServices::CDirectivityTF> >;
    using CExitPointABIRPtr = CExitPointBase< std::weak_ptr<BRTServices::CAmbisonicBIR> >;
}
#endif