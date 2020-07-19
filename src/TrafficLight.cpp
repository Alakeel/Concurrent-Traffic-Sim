#include <iostream>
#include <random>
#include "TrafficLight.h"

#include <chrono>
#include <future>

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait()
    // to wait for and receive new messages and pull them from the queue using move semantics.
    // The received object should then be returned by the receive function.

    // perform queue updates under the lock
    std::unique_lock<std::mutex> uLock(_mutex);
    // wait for new messages
    _condition.wait(uLock, [this] { return !_queue.empty(); }); // pass unique lock to condition variable

    // pull message from the queue using move semantics
    // remove first vector element from queue
    T msg = std::move(_queue.front());
    _queue.pop_front();
    return msg;  // will not be copied due to return value optimization (RVO) in C++
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex>
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    // perform vector updates under the lock
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.push_back(std::move(msg)); // add message to queue vector
    _condition.notify_one(); // send notification
    // unlock is initiated as soon as we leave current scope.
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
    _trafficLightMsgsQueue = std::make_shared<MessageQueue<TrafficLightPhase>>();
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop
    // runs and repeatedly calls the receive function on the message queue.
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1)); // good to debug and catch data race
      if(_trafficLightMsgsQueue->receive() == TrafficLightPhase::green){
        return;
      }
    // keep looping until we receive() returns green light.
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles
    // and toggles the current phase of the traffic light between red and green and sends an update method
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds.
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.

    // the cycle duration should be a random value between 4 and 6 seconds
    std::random_device rd;
    std::mt19937 rGenerator(rd());
    std::uniform_int_distribution<int> distribution(4000, 6000);
    int cycleDuration = distribution(rGenerator);

    auto lastUpdateTime = std::chrono::system_clock::now();
    while (true)
    {
          // sleep at every iteration to reduce CPU usage + wait 1ms between two cycles
          std::this_thread::sleep_for(std::chrono::milliseconds(1));

          //  measures the time between two loop cycles
          const auto initUpdateTime = std::chrono::system_clock::now(); // std::chrono::time_point<std::chrono::system_clock>
          const long elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(initUpdateTime - lastUpdateTime).count();

          if(elapsedTime >= cycleDuration)
          {
            // toggles the current phase of traffic light between red/green
            _currentPhase = (getCurrentPhase() == TrafficLightPhase::red)
                            ? TrafficLightPhase::green : TrafficLightPhase::red;

            // sends an update method to the message queue using move semantics
            auto ftr_sent = std::async(std::launch::async,
                                        &MessageQueue<TrafficLightPhase>::send,
                                        _trafficLightMsgsQueue,
                                        std::move(_currentPhase)
                                        );
            ftr_sent.wait();

            cycleDuration = distribution(rGenerator); // generate new random time for next
            lastUpdateTime = std::chrono::system_clock::now();
          }
    }

}
