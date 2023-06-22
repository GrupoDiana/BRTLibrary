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
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SONICOM ||
* \b Website: https://www.sonicom.eu/
*
* \b Copyright: University of Malaga
*
* \b Licence: This program is free software, you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
*
* \b Acknowledgement: This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement no.101017743
*/

#ifndef _ENTRY_POINT_
#define _ENTRY_POINT_

#include <functional>
#include <Base/ExitPoint.hpp>
#include <Base/ObserverBase.hpp>
#include <Common/Buffer.hpp>
#include <Common/Transform.hpp>
#include <Base/Command.hpp>


namespace BRTBase {
   
    template <class T>
    class CEntryPointBase : public Observer/*, public CEntryExitPointData<T> */{
    public:
        CEntryPointBase(std::function<void(std::string)> _callBack, std::string _id, int _multiplicity) : callBackUpdate{ _callBack }, id{ _id }, multiplicity{ _multiplicity }/*, CEntryExitPointData<T>() */{}
        ~CEntryPointBase() {}
        
        void Update(Subject* subject) {
            CExitPointBase<T>* _subject = (static_cast<CExitPointBase<T>*>(subject));
            Update(_subject);
        };

        void Update(CExitPointBase<T>* subject)
        {            
            this->SetData(subject->GetData());            
            if (multiplicity == 0) { /*Do nothing*/ }
            else if (multiplicity == 1) { callBackUpdate(this->GetID()); }
            else if (multiplicity >1 ) { /*TODO manage this */ callBackUpdate(this->GetID()); }
            
        }
        
        int GetMultiplicity() { return multiplicity; }
        std::string GetID() { return id; };
        void SetData(const T& _data) { data = _data; }
        T GetData() { return data; }

    private:
        // Vars
        std::function<void(std::string)> callBackUpdate;
        std::string id;
        int multiplicity;
 
        T data;
    };
           
    using CEntryPointSamplesVector = CEntryPointBase<CMonoBuffer<float>>;
    using CEntryPointTransform = CEntryPointBase<Common::CTransform>;     
    using CEntryPointCommand = CEntryPointBase<BRTBase::CCommand>;
    using CEntryPointID = CEntryPointBase<std::string>;

    using CEntryPointHRTFPtr = CEntryPointBase< std::weak_ptr<BRTServices::CHRTF> >;
    using CEntryPointILDPtr = CEntryPointBase< std::weak_ptr<BRTServices::CILD> >;
    using CEntryPointSRTFPtr = CEntryPointBase< std::weak_ptr<BRTServices::CSRTF> >;
}
#endif