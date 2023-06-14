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
    
    private:
        std::shared_ptr<CExitPointTransform> transformExitPoint;
        std::vector<std::shared_ptr<BRTBase::CExitPointSamplesVector >> samplesExitPoints;
        std::shared_ptr<CExitPointID> moduleIDExitPoint;
    };
}
#endif