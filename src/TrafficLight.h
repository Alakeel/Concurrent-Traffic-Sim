#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include "TrafficObject.h"

// forward declarations to avoid include cycle
class Vehicle;


// FP.3 Define a class „MessageQueue“ which has the public methods send and receive.
// Send should take an rvalue reference of type TrafficLightPhase whereas receive should return this type.
// Also, the class should define an std::dequeue called _queue, which stores objects of type TrafficLightPhase.
// Also, there should be an std::condition_variable as well as an std::mutex as private members.

template <class T>
class MessageQueue
{
public:
    void send(T &&msg); // take an rvalue reference of type T
    T receive(); // should return type T

private:
    std::deque<T> _queue; // stores objects of type T - performance of addition and deletion at start of deque is better than vector
    std::condition_variable _condition;
    std::mutex _mutex;
};

// FP.1 : Define a class „TrafficLight“ which is a child class of TrafficObject.
// The class shall have the public methods „void waitForGreen()“ and „void simulate()“
// as well as „TrafficLightPhase getCurrentPhase()“, where TrafficLightPhase is an enum that
// can be either „red“ or „green“. Also, add the private method „void cycleThroughPhases()“.
// Furthermore, there shall be the private member _currentPhase which can take „red“ or „green“ as its value.

// define enum for Traffic Lights
enum class TrafficLightPhase
{
  red,
  green
};

class TrafficLight : TrafficObject
{
public:
    // constructor / desctructor
    TrafficLight();
    // ~TrafficLight() = default;

    // getters / setters
    TrafficLightPhase getCurrentPhase();

    // typical behaviour methods
    void waitForGreen();
    void simulate();


private:
    // typical behaviour methods
    void cycleThroughPhases();

    // FP.4b : create a private member of type MessageQueue for messages of type TrafficLightPhase
    // and use it within the infinite loop to push each new TrafficLightPhase into it by calling
    // send in conjunction with move semantics.

    TrafficLightPhase _currentPhase;
    std::shared_ptr<MessageQueue<TrafficLightPhase>> _trafficLightMsgsQueue;
    std::condition_variable _condition;
    std::mutex _mutex;
};

#endif
