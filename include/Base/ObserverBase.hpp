
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