#include<iostream>
using namespace std;

/* Factory Method Design Pattern 
    🧩 Factory Method Pattern — Overview

        Definition:

        The Factory Method pattern defines an interface for creating an object,
        but lets subclasses alter the type of objects that will be created.

        It’s part of the Creational Design Patterns (from the GoF patterns).

        🧱 Core Characteristics

        Interface for creation — A common interface for creating objects.

        Subclass responsibility — Subclasses decide which class to instantiate.

        Decoupling — Client code is decoupled from the actual object creation logic.

                | ✅ Use When                                     | 🚫 Avoid When                                          |
        | ---------------------------------------------- | ------------------------------------------------------ |
        | You have a superclass with multiple subclasses  | You have a simple, one-off object creation              |
        | You want to localize the knowledge of which     | You want to avoid subclass proliferation                |
        | subclass is instantiated                        | You need more flexibility than a single method can provide |
    
        🧩 The Problem

        Directly instantiating classes in client code leads to tight coupling.
        This makes it hard to change or extend the system with new types.

        🧩 The Solution
        Define a factory method in a base class that returns an instance of a product.
        Subclasses override this method to create specific product instances.
        This way, client code calls the factory method without knowing the exact class being instantiated.

        Example:
        Consider a payment processing system that supports multiple payment methods (CreditCard, UPI, DebitCard, Wallet).
        Instead of directly instantiating these classes in client code, we use a PaymentFactory to create them based on input.

        Benefits:
        - Flexibility: Easily add new product types without changing client code.
        - Maintainability: Centralizes object creation logic.
        - Adherence to Open/Closed Principle: New products can be added by creating new subclasses without modifying existing code.

        Problem with Factory Method Pattern:
        - Increased number of classes: Each product type requires its own subclass.
        - Complexity: The pattern can introduce unnecessary complexity if not needed.
*/

enum class PaymentType {
    CREDIT_CARD,
    UPI,
    DEBIT_CARD,
    WALLET
};

class Payment {
public:
    virtual void processPayment(double amount) = 0;
    virtual ~Payment() = default;
};

class CreditCardPayment : public Payment {
public:                 
    void processPayment(double amount) override {
        cout << "Processing credit card payment of amount: " << amount << endl;
    }
};

class UpiPayment : public Payment {
public:
    void processPayment(double amount) override {
        cout << "Processing UPI payment of amount: " << amount << endl;
    }
};

class DebitCardPayment : public Payment {
public:
    void processPayment(double amount) override {
        cout << "Processing debit card payment of amount: " << amount << endl;
    }
};

class WalletPayment : public Payment {
public:
    void processPayment(double amount) override {
        cout << "Processing wallet payment of amount: " << amount << endl;
    }
};

class PaymentFactory {
public:
    static Payment* createPayment(PaymentType type) {
        switch(type) {
            case PaymentType::CREDIT_CARD:
                return new CreditCardPayment();
            case PaymentType::UPI:
                return new UpiPayment();        
            case PaymentType::DEBIT_CARD:
                return new DebitCardPayment();
            case PaymentType::WALLET:
                return new WalletPayment();
            default:
                return nullptr;
        }
    }
};

int main() {
    cout << "Factory Design Patterns in C++\n";

    Payment* payment1 = PaymentFactory::createPayment(PaymentType::CREDIT_CARD);
    payment1->processPayment(100.0);
    delete payment1;

    Payment* payment2 = PaymentFactory::createPayment(PaymentType::UPI);
    payment2->processPayment(200.0);
    delete payment2;

    Payment* payment3 = PaymentFactory::createPayment(PaymentType::DEBIT_CARD);
    payment3->processPayment(300.0);
    delete payment3;

    Payment* payment4 = PaymentFactory::createPayment(PaymentType::WALLET);
    payment4->processPayment(400.0);
    delete payment4;

    return 0;
}