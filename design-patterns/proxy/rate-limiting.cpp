#include<iostream>
using namespace std;

/*
    Proxy with Rate Limiting
    This example demonstrates a proxy pattern that adds rate limiting to a booking service. 
    The proxy controls access to the real booking service by limiting the number of requests a user can make.
*/

class IBookingService {
public:                
    virtual void bookTicket(const string& userId) = 0;
    virtual ~IBookingService() = default;
};

class RealBookingService : public IBookingService {
public:
    void bookTicket(const string& userId) override {
        cout << "Booking ticket for user: " << userId << endl;
    }
};

class BookingServiceProxy : public IBookingService {
    RealBookingService realService;
    unordered_map<string, int> userRequestCount;
    const int requestLimit = 5;

public:
    void bookTicket(const string& userId) override {
        if (userRequestCount[userId] < requestLimit) {
            userRequestCount[userId]++;
            realService.bookTicket(userId);
        } else {
            cout << "Booking limit reached for user: " << userId << endl;
        }
    }
};