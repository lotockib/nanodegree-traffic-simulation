#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait()
    // to wait for and receive new messages and pull them from the queue using move semantics.
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> uLock(_mtx);
    _con_var.wait(uLock, [this] { return !_queue.empty(); }); // pass unique lock to condition variable
    T msg = std::move(_queue.back());
    _queue.pop_back();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex>
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> uLock(_mtx);
    _queue.push_back(std::move(msg));
    _con_var.notify_one();
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
      auto msg = _messages.receive();
      if (msg == TrafficLightPhase::green) {break; };
      std::this_thread::sleep_for(1ms);
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    // Launch thread with cycleThroughPhases()
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));

}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles
    // and toggles the current phase of the traffic light between red and green and sends an update method
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds.
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.

    bool reset_timer = true;
    std::chrono::duration<double, std::milli> elapsed, limit;
    std::chrono::_V2::system_clock::time_point start;
    while (true)
    {
      if (reset_timer)
      {
        // generate random duration from 4000-6000
        limit = (std::chrono::duration<double, std::milli>) ( (std::rand()) % 2001 + 4000 );

        // reset start time
        start = std::chrono::high_resolution_clock::now();

        // set start_timer to false
        reset_timer = false;
      } else if (elapsed >= limit)
      {
        // switch color
        if (_currentPhase == TrafficLightPhase::red) {
          _currentPhase = TrafficLightPhase::green;
        } else {
          _currentPhase = TrafficLightPhase::red;
        }

        // reset timer
        reset_timer = true;

        // Send update method
        _messages.send(std::move(_currentPhase));
      }

      // sleep and update timer
      std::this_thread::sleep_for(1ms);
      elapsed = std::chrono::high_resolution_clock::now() - start;
    }
}
