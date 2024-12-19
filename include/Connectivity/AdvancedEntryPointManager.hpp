/**
* \class CAdvancedEntryPointManager
*
* \brief Declaration of CAdvancedEntryPointManager class
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

#ifndef _CADVANCED_ENTRY_POINT_MANAGER_
#define _CADVANCED_ENTRY_POINT_MANAGER_

#include <Common/CommonDefinitions.hpp>
#include <Connectivity/EntryPointManager.hpp>
#include <Connectivity/CommandEntryPointManager.hpp>
#include <Connectivity/ExitPointManager.hpp>


namespace BRTConnectivity {    
    class CDataWaitingEntryPoint {
    public:
        CDataWaitingEntryPoint(std::string _id) : id{ _id }, connections{ 0 }, received{ false }, timesReceived { 0} {}
    
        std::string id;
        int connections;
        int timesReceived;
        bool received;
    };

    class CAdvancedEntryPointManager : public CEntryPointManager {
    public:
        CAdvancedEntryPointManager() {        
        }

        virtual ~CAdvancedEntryPointManager() {}
        
        /**
         * @brief This method will be called when data has been received at all input points with notification.
         * @param entryPointID 
        */
		virtual void AllEntryPointsAllDataReady() { };
        
        /**
         * @brief This method will be called when all expected data is present at an entry point. 
         * For example, if its multiplicity is two, this method will be called when the second of the data is received.         
         * @param entryPointID 
        */
        virtual void OneEntryPointAllDataReady(std::string entryPointID) {};
        /**
         * @brief This method shall be called whenever data is received at an entry point, with non-zero multiplicity. 
         * @param entryPointID 
        */
        virtual void OneEntryPointOneDataReceived(std::string entryPointID) {};                      
                
       
    private:                                                 
        /// Implementation of EntryPointManager virtual methods                

        /**
         * @brief In this method, notification is received that new data has been received at any entry point with multiplicity greater than zero.
         * @param entryPointID 
        */
        void UpdateEntryPointData(std::string entryPointID) override {                        
            UpdateEntryPointWaitingList(entryPointID);                
        }

        /**
         * @brief A call to this method is received when a new entry point is created.
         * @param _id  EntryPoint ID
         * @param _multiplicity EntryPoint multiplicity
        */
        void EntryPointCreated(std::string _entryPointID, bool _notify) override {
            if (_notify) {
                CDataWaitingEntryPoint temp(_entryPointID);
                entryPointsWaitingList.push_back(temp);
            }
        }
        
        /**
         * @brief A call to this method is received when an entry point connection is added
         * @param _entryPointID Entry point referred to.
         * @param _numberOfConnections New number of connections to this entry point
         */
        void UpdateEntryPointConnections(std::string _entryPointID, int _numberOfConnections) override {
            std::vector<CDataWaitingEntryPoint>::iterator it;
            it = std::find_if(entryPointsWaitingList.begin(), entryPointsWaitingList.end(), [&_entryPointID](CDataWaitingEntryPoint const& obj) {
                return obj.id == _entryPointID;
            });
            if (it != entryPointsWaitingList.end()) {
                it->connections = _numberOfConnections;
            }
        };
                
                    
        /// Waiting list mangement methods
		/**
		 * @brief Update the waiting list of entry points
		 * @param _entryPointID 
		*/
        void UpdateEntryPointWaitingList(std::string _entryPointID) {
            std::vector<CDataWaitingEntryPoint>::iterator it;
            it = std::find_if(entryPointsWaitingList.begin(), entryPointsWaitingList.end(), [&_entryPointID](CDataWaitingEntryPoint const& obj) {
                return obj.id == _entryPointID;
            });

            if (it != entryPointsWaitingList.end()) {                                               
                
                if (it->connections == 0) { return; }
                it->timesReceived++;
                OneEntryPointOneDataReceived(_entryPointID);

                if ((it->timesReceived) >= (it->connections)) {
                    it->received = true;
                    OneEntryPointAllDataReady(_entryPointID);
                    if (AreAllEntryPointsReady()) {
                        AllEntryPointsAllDataReady();
                        ResetEntryPointWaitingList();
                    }
                }               
            }
            else {
                // TODO error, why are we receiving this?
                SET_RESULT(RESULT_ERROR_INVALID_PARAM, "There is no entre point registered with this ID: " + _entryPointID);
            }            
        }

        /**
         * @brief Check whether data has already been received at all entry points.
         * @return 
        */
        bool AreAllEntryPointsReady() {
            bool ready = true;
            typename std::vector<CDataWaitingEntryPoint>::iterator it;
            for (it = entryPointsWaitingList.begin(); it != entryPointsWaitingList.end(); it++) {
                if (!(it->received)) { ready = false; }
            }
            return ready;
        }

        /**
		 * @brief Reset the waiting list of entry points
		*/
        void ResetEntryPointWaitingList() {
            typename std::vector<CDataWaitingEntryPoint>::iterator it;
            for (it = entryPointsWaitingList.begin(); it != entryPointsWaitingList.end(); it++) {
                it->received = false;
                it->timesReceived = 0;
            }
        }
		
        // Attributes
		std::vector<CDataWaitingEntryPoint> entryPointsWaitingList;    

    };
}
#endif