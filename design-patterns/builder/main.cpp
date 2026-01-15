#include<iostream>
using namespace std;

class Booking {
public:
    string bookingId;
    string userId;
    string itineraryId;
    string createdAt;
    vector<string> seatIds;
    string paymentStatus;

    void showBooking() {
        cout << "Booking Details: \n";
        cout << "Booking ID: " << bookingId << "\n";
        cout << "User ID: " << userId << "\n";
        cout << "Itinerary ID: " << itineraryId << "\n";
        cout << "Created At: " << createdAt << "\n";
        cout << "Seats: ";
        for (const auto& seat : seatIds) {
            cout << seat << " ";
        }
        cout << "\nPayment Status: " << paymentStatus << "\n";
    }
};

class IBookingBuilder {
public:
    virtual IBookingBuilder& setBookingId(const string& bookingId) = 0;
    virtual IBookingBuilder& setUserId(const string& userId) = 0;
    virtual IBookingBuilder& setItineraryId(const string& itineraryId) = 0;
    virtual IBookingBuilder& setCreatedAt(const string& createdAt) = 0;
    virtual IBookingBuilder& setSeatIds(const vector<string>& seatIds) = 0;
    virtual IBookingBuilder& setPaymentStatus(const string& paymentStatus) = 0;
    virtual Booking build() = 0;
    virtual ~IBookingBuilder() = default;
};

class BookingBuilder : public IBookingBuilder {
    Booking booking;

public:
    IBookingBuilder& setBookingId(const string& bookingId) override {
        booking.bookingId = bookingId;
        return *this;
    }

    IBookingBuilder& setUserId(const string& userId) override {
        booking.userId = userId;
        return *this;
    }

    IBookingBuilder& setItineraryId(const string& itineraryId) override {
        booking.itineraryId = itineraryId;
        return *this;
    }

    IBookingBuilder& setCreatedAt(const string& createdAt) override {
        booking.createdAt = createdAt;
        return *this;
    }

    IBookingBuilder& setSeatIds(const vector<string>& seatIds) override {
        booking.seatIds = seatIds;
        return *this;
    }

    IBookingBuilder& setPaymentStatus(const string& paymentStatus) override {
        booking.paymentStatus = paymentStatus;
        return *this;
    }

    Booking build() override {
        return booking;
    }
};  

// to run the code use command: g++ -std=c++17 main.cpp -o main && ./main
int main() {
    cout << "Builder Design Pattern in C++\n";

    BookingBuilder builder;
    Booking booking = builder.setBookingId("B123")
                             .setUserId("U456")
                             .setItineraryId("I789")                
                             .setCreatedAt("2023-10-01")
                             .setSeatIds({"S1", "S2"})
                             .setPaymentStatus("Paid")
                             .build();

    booking.showBooking();

    return 0;
}