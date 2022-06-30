#pragma once
#include <vector>


namespace BRTBase {

    class Subject2;

    class Observer2
    {
    public:
        Observer2() {}
        virtual ~Observer2() {}
        virtual void Update(Subject2* subject) = 0;
    };

    
    class Subject2
    {
    public:
        Subject2() {}
        virtual ~Subject2() {}
        void attach(Observer2& observer)
        {
            observers.push_back(&observer);
        }
        void detach(Observer2& observer) {
            /*auto it = std::find(observers.begin(), observers.end(), observer);
            if (it != observers.end())
                observers.erase(it);*/
        }
        void notify()
        {
            typename std::vector<Observer2*>::iterator it;
            //for (it = observers.begin(); it != observers.end(); it++) (*it)->Update(static_cast<T*>(this));
            for (it = observers.begin(); it != observers.end(); it++) (*it)->Update(this);                        
        }
    private:
        std::vector<Observer2*> observers;
    };

    template <class T>
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
    };
}