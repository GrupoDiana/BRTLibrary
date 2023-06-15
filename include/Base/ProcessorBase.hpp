#ifndef _PROCESSOR_BASE_
#define _PROCESSOR_BASE_

//#include <Base/EntryPoint.hpp>
//#include <Base/ExitPoint.hpp>
//#include <Base/EntryPointPtr.hpp>

//#include <vector>
#include <Common/CommonDefinitions.h>
#include <Base/EntryPointManager.hpp>
#include <Base/CommandEntryPointManager.hpp>
#include <Base/ExitPointManager.hpp>


namespace BRTBase {    
    class CWaitingEntrypoint {
    public:
        CWaitingEntrypoint(std::string _id, int _multiplicity) : id{ _id }, multiplicity{ _multiplicity }, received{ false } {}
    
        std::string id;
        int multiplicity;
        bool received;
    };

    class CProcessorBase : public CEntryPointManager, public CCommandEntryPointManager, public CExitPointManager {
    public:
        CProcessorBase() {
            CreateCommandEntryPoint();
        }

        virtual ~CProcessorBase() {}
        virtual void Update(std::string entryPointID) = 0;
        virtual void UpdateCommand() = 0;
       

        // Update callback
        void updateFromEntryPoint(std::string entryPointID) {            
            if (updateStack(entryPointID)) { Update(entryPointID); }
        }

        void updateFromCommandEntryPoint(std::string entryPointID) {
            /*if (updateStack(entryPointID)) { 
                UpdateCommand(); 
            }*/
            //std::string command = GetCommandEntryPoint()->GetData();
            //if (command!="") { UpdateCommand(); }            

            BRTBase::CCommand _command = GetCommandEntryPoint()->GetData();
            if (!_command.isNull()) { 
                UpdateCommand(); 
            }
        }

        // Update Management
        void addToUpdateStack(std::string _id, int _multiplicity) {
            if (_multiplicity >= 1) {
                CWaitingEntrypoint temp(_id, _multiplicity);
                entryPointsUpdatingStack.push_back(temp);
            }            
        }

        bool updateStack(std::string _id) {            
            std::vector<CWaitingEntrypoint>::iterator it;
            it = std::find_if(entryPointsUpdatingStack.begin(), entryPointsUpdatingStack.end(), [&_id](CWaitingEntrypoint const& obj) {
                return obj.id == _id;
            });

            if (it != entryPointsUpdatingStack.end())
                if (!it->received) { 
                    it->received = true; 
                    return checkWaitingStack();
                }
                else {
                    // It has been received twice before processing it. // TODO manage multiplicity bigger than 1
                }
            else {
                // TODO error, why are we receibing this?
            }
            return false;
        }                                

        bool checkWaitingStack() {
            bool ready = true;
            typename std::vector<CWaitingEntrypoint>::iterator it;
            for (it = entryPointsUpdatingStack.begin(); it != entryPointsUpdatingStack.end(); it++) {
                if (!(it->received)) { ready = false; }
            }
            return ready;
        }

        void resetUpdatingStack() {
            typename std::vector<CWaitingEntrypoint>::iterator it;
            for (it = entryPointsUpdatingStack.begin(); it != entryPointsUpdatingStack.end(); it++) {
                it->received = false;
            }
        }
        
    private:                                       
        std::vector< CWaitingEntrypoint> entryPointsUpdatingStack;
    };
}
#endif