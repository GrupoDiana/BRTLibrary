/**
* \class CExitPointManager
*
* \brief Declaration of CExitPointManager class
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

#ifndef _EXIT_POINT_MANAGER_
#define _EXIT_POINT_MANAGER_

#include <Common/ErrorHandler.hpp>
#include <Base/ExitPoint.hpp>

namespace BRTBase {
    class CExitPointManager {
    public:
        /////////////////////
        // Transform
        /////////////////////
        
        /** \brief Initialises an exit point of type transform
        *	\param [in] Identifier to be given to the exit point
        *	\retval void
        *   \eh On error, NO error code is reported.
        */
        void CreateTransformExitPoint() {
            transformExitPoint = std::make_shared<CExitPointTransform>("moduleTransform");
        }

        /** \brief Returns a pointer to the exit point
        *	\param [in] Exit point identifier to be searched for
        *	\retval A pointer to this type of exit point or null if not found.
        *   \eh On error, an error code is reported to the error handler.
        */
        std::shared_ptr<BRTBase::CExitPointTransform> GetTransformExitPoint() {
            if (transformExitPoint != nullptr)  { return transformExitPoint; }
            ASSERT(false, RESULT_ERROR_NOTINITIALIZED, "The exit point of type Transform has not been initialised.", "Call CExitPointManager::CreateTransformExitPoint() in your constructor.");
            return nullptr;
        }
        
        /////////////////////
        // Samples
        /////////////////////
        /** \brief Creates a new exit point of type samples and saves it
        *	\param [in] Identifier to be given to the exit point
        *	\retval void
        *   \eh On error, an error code is reported to the error handler.
        */
        void CreateSamplesExitPoint(std::string exitPointID) {
            std::shared_ptr<BRTBase::CExitPointSamplesVector> _newExitPoint = std::make_shared<BRTBase::CExitPointSamplesVector>(exitPointID);
            samplesExitPoints.push_back(_newExitPoint);
        }

        /** \brief Returns a pointer to the exit point
        *	\param [in] Exit point identifier to be searched for
        *	\retval A pointer to this type of exit point or null if not found.
        *   \eh On error, an error code is reported to the error handler.
        */
        std::shared_ptr<BRTBase::CExitPointSamplesVector > GetSamplesExitPoint(std::string exitPointID) {
            for (auto& it : samplesExitPoints) {
                if (it->GetID() == exitPointID) { return it; }
            }
            ASSERT(false, RESULT_ERROR_INVALID_PARAM, "No exit point, of type Samples, has been found with this id." + exitPointID, "");
            return nullptr;
        }
        
        /////////////////////
        // IDs 
        /////////////////////
        /** \brief Initialises an exit point of type ID. Used to send the identifier of this module to other modules.
        *	\param [in] Identifier to be given to the exit point
        *	\retval void
        *   \eh On error, an error code is reported to the error handler.
        */
        void CreateIDExitPoint() {
            moduleIDExitPoint = std::make_shared<CExitPointID>("moduleID");
        }
        
        /** \brief Returns a pointer to the exit point
        *	\param [in] Exit point identifier to be searched for
        *	\retval A pointer to this type of exit point or null if not found.
        *   \eh On error, an error code is reported to the error handler.
        */
        std::shared_ptr<BRTBase::CExitPointID> GetIDExitPoint() {
            if (moduleIDExitPoint!=nullptr) { return moduleIDExitPoint; }
            ASSERT(false, RESULT_ERROR_NOTINITIALIZED, "The exit point of type ID has not been initialised.", "Call CExitPointManager::CreateIDExitPoint() in your constructor.");
            return nullptr;
        }

        /////////////////////
       // HRTFs 
       /////////////////////
        void CreateHRTFExitPoint() {
            hrtfExitPoint = std::make_shared<CExitPointHRTFPtr>("moduleHRTF");
        }

        std::shared_ptr<CExitPointHRTFPtr> GetHRTFExitPoint() {
            return hrtfExitPoint;
        }

        /////////////////////
       // ILDs 
       /////////////////////
        void CreateILDExitPoint() {
            ildExitPoint = std::make_shared<CExitPointILDPtr>("listenerILD");
        }

        std::shared_ptr<CExitPointILDPtr> GetILDExitPoint() {
            return ildExitPoint;
        }
    
    private:
        std::shared_ptr<CExitPointTransform> transformExitPoint;
        std::vector<std::shared_ptr<BRTBase::CExitPointSamplesVector >> samplesExitPoints;        
        std::shared_ptr<CExitPointID> moduleIDExitPoint;
        
        std::shared_ptr<CExitPointHRTFPtr>  hrtfExitPoint;
        std::shared_ptr<CExitPointILDPtr>   ildExitPoint;

    };
}
#endif