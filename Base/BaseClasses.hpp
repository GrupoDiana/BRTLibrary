#pragma once
#include <vector>


namespace BRT_Base {

    template <class T>
    class Observer
    {
    public:
        Observer() {}
        virtual ~Observer() {}
        virtual void update(T* subject) = 0;
    };

    template <class T>
    class Subject
    {
    public:
        Subject() {}
        virtual ~Subject() {}
        void attach(Observer<T>& observer)
        {
            m_observers.push_back(&observer);
        }
        void detach(Observer<T>& observer) {
            auto it = std::find(m_observers.begin(), m_observers.end(), observer);
            if (it != m_observers.end())
                m_observers.erase(it);
        }
        void notify()
        {
            typename std::vector<Observer<T>*>::iterator it;
            for (it = m_observers.begin(); it != m_observers.end(); it++) (*it)->update(static_cast<T*>(this));
        }
    private:
        std::vector<Observer<T>*> m_observers;
    };
}