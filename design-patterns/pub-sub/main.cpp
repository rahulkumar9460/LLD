#include<iostream>
using namespace std;
/*
    Implementing a simple Pub-Sub pattern for an e-commerce order processing system.
    Components:
    1. Order: Represents an order with details like order ID, amount, and status.
    2. EventManager: Manages subscriptions and notifications.
    3. OrderManager: Manages orders and interacts with the EventManager.
    4. PaymentService: Processes payments and notifies the EventManager upon completion.

    flow:
    1. User places an order.
    2. OrderManager creates an Order and subscribes it to the EventManager.
    3. PaymentService processes the payment and notifies the EventManager.
    4. EventManager notifies the relevant Order about the payment status.
*/
class Subscriber {
public:
    virtual void notify(string message) = 0;
    virtual string getId() const = 0;
    ~Subscriber() {}
};

enum class OrderStatus {
    PAYMENT_PENDING,
    PAYMENT_COMPLETED,
    SHIPPED,
    DELIVERED
};

class Order : public Subscriber {
    int orderId;
    int amount;
    OrderStatus status;
public:
    Order(int orderId, int amount) : orderId(orderId), amount(amount), status(OrderStatus::PAYMENT_PENDING) {}
    int getOrderId() const { return orderId; }
    int getAmount() const { return amount; }
    OrderStatus getStatus() const { return status; }
    void setStatus(OrderStatus newStatus) { status = newStatus; }   

    void notify(string message) override {
        cout << "Order ID: " << orderId << " received notification: " << message << endl;
        this->setStatus(OrderStatus::PAYMENT_COMPLETED);
    }

    string getId() const override {
        return "Order_" + to_string(orderId);
    }
};

class EventManager {
    unordered_map<string, Subscriber*> subscribers;
public:
    void subscribe(Subscriber* subscriber) {
        subscribers[subscriber->getId()] = subscriber;
    }

    void unsubscribe(Subscriber* subscriber) {
        subscribers.erase(subscriber->getId());
    }

    void notify(string message, string orderId) {
        subscribers[orderId]->notify(message);
    }
};

class OrderManager {
    vector<Order*> orders;
    EventManager& eventManager;
public:
    OrderManager(EventManager& em) : eventManager(em) {}

    void addOrder(Order* order) {
        orders.push_back(order);
        eventManager.subscribe(order);
    }

    void removeOrder(Order* order) {
        orders.erase(remove(orders.begin(), orders.end(), order), orders.end());
        eventManager.unsubscribe(order);
    }
};

class PaymentService {
    EventManager& eventManager;
public:
    PaymentService(EventManager& em) : eventManager(em) {}
    void processPayment(Order& order) {
        cout << "Processing payment for Order ID: " << order.getOrderId() << ", Amount: " << order.getAmount() << endl;
        eventManager.notify("Payment completed for Order ID: " + to_string(order.getOrderId()), order.getId());
    }
};

int main() {
    EventManager eventManager;

    OrderManager orderManager(eventManager);
    PaymentService paymentService(eventManager);

    Order* order1 = new Order(1, 100);
    Order* order2 = new Order(2, 200);

    orderManager.addOrder(order1);
    orderManager.addOrder(order2);

    paymentService.processPayment(*order1);
    paymentService.processPayment(*order2);

    delete order1;
    delete order2;

    return 0;
}