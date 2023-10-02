/**
* \class Observer
*
* \brief Declaration of Observer class
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

#ifndef _OBSERVER_BASE_
#define _OBSERVER_BASE_

#include <vector>
#include <iostream>

namespace BRTBase {

    class Subject;

    class Observer
    {
    public:
        Observer() {}
        virtual ~Observer() {}
        virtual void Update(Subject* subject) = 0;
    };

    
    class Subject
    {
    public:
        Subject() {}
        virtual ~Subject() {}
        void attach(Observer& observer)
        {
            observers.push_back(&observer);
            notify(observer);
        }
        void detach(Observer *observer) {           
            auto it = (std::find(observers.begin(), observers.end(), observer));
            if (it != observers.end())
                observers.erase(it);            
        }
        void notify()
        {
            typename std::vector<Observer*>::iterator it;
            //for (it = observers.begin(); it != observers.end(); it++) (*it)->Update(static_cast<T*>(this));
            for (it = observers.begin(); it != observers.end(); it++) (*it)->Update(this);                        
        }
      
    private:
        std::vector<Observer*> observers;

        void notify(Observer& observer) { observer.Update(this); }
    };

    /*template <class T>
    class Observer
    {
    public:
        Observer() {}
        virtual ~Observer() {}
        virtual void Update(T* subject) = 0;
    };

    template <class T>
    class Subject
    {
    public:
        Subject() {}
        virtual ~Subject() {}
        void attach(Observer<T>& observer)
        {
            observers.push_back(&observer);
        }
        void detach(Observer<T>& observer) {
            auto it = std::find(observers.begin(), observers.end(), observer);
            if (it != observers.end())
                observers.erase(it);
        }
        void notify()
        {
            typename std::vector<Observer<T>*>::iterator it;
            for (it = observers.begin(); it != observers.end(); it++) (*it)->Update(static_cast<T*>(this));
        }
    private:
        std::vector<Observer<T>*> observers;
    };*/
}
#endif