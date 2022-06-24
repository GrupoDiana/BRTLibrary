#pragma once
#include <vector>


namespace BRTBase {

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