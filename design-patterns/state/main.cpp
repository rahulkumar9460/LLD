#include<iostream>
using namespace std;

/*
    State Design Pattern 
    🧩 State Pattern — Overview

        Definition:

        The State pattern allows an object to alter its behavior when its internal state changes.
        The object will appear to change its class.
        It’s part of the Behavioral Design Patterns (from the GoF patterns).    
        🧱 Core Characteristics
        State objects — Different classes representing various states of the context.   
        Context — The main class that maintains a reference to a state object and delegates state-specific behavior to it.
        State transitions — The context can change its state, leading to different behaviors.

                | ✅ Use When                                     | 🚫 Avoid When                                          |
                | - You need to manage state-specific behavior.  | - You have a simple state management scenario.       |
                | - State transitions are complex and varied.    | - You want to avoid the overhead of multiple classes. |
*/

class Ride; // Forward declaration

class IRideState {
public:
    virtual void assignDriver(Ride* ride) {
        cout << "Not allowed in current state\n";
    }
    virtual void startRide(Ride* ride) {
        cout << "Not allowed in current state\n";
    }
    virtual void completeRide(Ride* ride) {
        cout << "Not allowed in current state\n";
    }
    virtual void cancelRide(Ride* ride) {
        cout << "Not allowed in current state\n";
    }
    
    virtual string getStateName() = 0;
};

class CreatedState : public IRideState {
public:
    void assignDriver(Ride* ride) override;
    void cancelRide(Ride* ride) override;
    string getStateName() override {
        return "Created";
    }
};

class AssignedDriverState : public IRideState {
public:
    void startRide(Ride* ride) override;
    void cancelRide(Ride* ride) override;
    string getStateName() override {
        return "AssignedDriver";
    }
};

class StartedState : public IRideState {
public:
    void completeRide(Ride* ride) override;
    string getStateName() override {
        return "Started";
    }
};

class CompletedState : public IRideState {
public:
    string getStateName() override {
        return "Completed";
    }
};

class CancelledState : public IRideState {
public:
    string getStateName() override {
        return "Cancelled";
    }
};

class Ride {
private:
    string rideId;
    shared_ptr<IRideState> state;  // current state
public:
    Ride(string id) : rideId(id) {}
    
    void setState(shared_ptr<IRideState> newState) {
        state = newState;
        cout << "Ride [" << rideId << "] transitioned to state: " << state->getStateName() << endl;
    }

    shared_ptr<IRideState> getState() { return state; }

    // delegate behavior to state
    void assignDriver()  { state->assignDriver(this); }
    void startRide()     { state->startRide(this); }
    void completeRide()  { state->completeRide(this); }
    void cancelRide()    { state->cancelRide(this); }
};

//define state transition methods
void CreatedState::assignDriver(Ride* ride) {
    cout << "Assigning driver to ride\n";
    ride->setState(make_shared<AssignedDriverState>());
}

void CreatedState::cancelRide(Ride* ride) {
    ride->setState(make_shared<CancelledState>());
} 

void AssignedDriverState::startRide(Ride* ride) {
    cout << "Starting ride\n";
    ride->setState(make_shared<StartedState>());
}

void StartedState::completeRide(Ride* ride) {
    cout << "Completing ride\n";
    ride->setState(make_shared<CompletedState>());
}

void AssignedDriverState::cancelRide(Ride* ride) {
    ride->setState(make_shared<CancelledState>());
}

// to run the code use command: g++ -std=c++17 main.cpp -o main && ./main
int main() {
    cout << "State Design Pattern in C++\n";
    Ride ride("RIDE123");
    ride.setState(make_shared<CreatedState>());

    ride.assignDriver();  // Transition to AssignedDriver
    ride.startRide();     // Transition to Started
    ride.completeRide();  // Transition to Completed

    // Uncommenting the next line will show that you cannot cancel a completed ride
    // ride.cancelRide(); 

    return 0;
}
    