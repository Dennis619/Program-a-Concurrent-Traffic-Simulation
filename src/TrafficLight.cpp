#include <iostream>
#include <random>
#include<chrono>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::unique_lock<std::mutex> lock(MessageQueue::uLock);
    MessageQueue::cond.wait(lock, [this] {
        return !MessageQueue::_queue.empty();
    });

    T msg = std::move(MessageQueue::_queue.back());
    MessageQueue::_queue.pop_back();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::lock_guard<std::mutex> lock(MessageQueue::uLock);
    MessageQueue::_queue.push_back(std::move(msg));
    MessageQueue::cond.notify_one();
}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}
void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if(_queue.receive() == TrafficLightPhase::green)
            return;
    }
    
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}


void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    TrafficObject::threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}


// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    //random number
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> distribut(4000, 6000);
    int randomNo = distribut(mt);
    //time now
    std::chrono::time_point<std::chrono::system_clock> timeNow = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> LaterTime;
    while (true)
    {
        //sleep btw each cycle
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        //time later
        LaterTime = std::chrono::system_clock::now();

        if(std::chrono::duration_cast<std::chrono::milliseconds>(LaterTime - timeNow).count() >= randomNo)
        {
            if(TrafficLight::getCurrentPhase() == TrafficLightPhase::red)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                TrafficLight::_currentPhase = TrafficLightPhase::green;
                std::cout<<" Toggle green from red"<<std::endl;
            }
            else
            {
                TrafficLight::_currentPhase = TrafficLightPhase::red;
                std::cout<<" Toggle red from green"<<std::endl;
            }
            //update message queue
            TrafficLight::_queue.send(std::move(TrafficLight::_currentPhase));

            
        }
    }
     
}

