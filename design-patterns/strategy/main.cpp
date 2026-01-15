#include<iostream>
using namespace std;
/*
    Implementing a simple Strategy pattern for a delivery service matching system.
    Components:
    1. DeliveryPartner: Represents a delivery partner with details like name, location, and vehicle type.
    2. PartnerManager: Manages a list of delivery partners.
    3. IMatchingStrategy: Interface for different matching strategies.
    4. NearestNeighborStrategy, QuickestDeliveryStrategy, CheapestOptionStrategy: Concrete strategies implementing the matching logic.
    5. DeliveryService: Uses a strategy to find matching delivery partners based on order details.

    flow:
    1. PartnerManager maintains a list of delivery partners.
    2. Different strategies implement the IMatchingStrategy interface to provide various matching algorithms.
    3. DeliveryService uses a selected strategy to find suitable delivery partners for an order.

    strategy pattern is used when we have multiple algorithms for a specific task and want to switch between them easily.
    Benefits:
    - Flexibility: Easily switch between different strategies at runtime.
    - Maintainability: Each strategy is encapsulated in its own class, making it easier to manage and extend.
    - Open/Closed Principle: New strategies can be added without modifying existing code.

    Problem with strategy pattern:
    - Increased number of classes: Each strategy requires its own class, which can lead to a proliferation of classes.
    - Client awareness: Clients must be aware of different strategies and choose the appropriate one.
    - Overhead: There may be a slight performance overhead due to the indirection of strategy selection.    
*/
class DeliveryPartner {
    string name;
    string location;
    string vehicleType;
public:
    DeliveryPartner(string name, string location, string vehicleType) 
        : name(name), location(location), vehicleType(vehicleType) {}
};

class PartnerManager {
    vector<DeliveryPartner*> partners;
public:
    void addPartner(DeliveryPartner* partner) {
        partners.push_back(partner);
    }

    vector<DeliveryPartner*> getPartners() const {
        return partners;
    }
    // Other management functions...
};

class IMatchingStrategy {
public:
    virtual vector<DeliveryPartner*> getMatchingPartners(const string& location, const string& orderInfo,
                                            const vector<DeliveryPartner*>& partners) = 0;
    virtual ~IMatchingStrategy() {}
};

class NearestNeighborStrategy : public IMatchingStrategy {
public:
    vector<DeliveryPartner*> getMatchingPartners(const string& location, const string& orderInfo,
                                            const vector<DeliveryPartner*>& partners) override {
        // Dummy implementation: return all partners for simplicity
        cout << "Using NearestNeighborStrategy\n";
        return partners;
    }
};

class QuickestDeliveryStrategy : public IMatchingStrategy {
public:
    vector<DeliveryPartner*> getMatchingPartners(const string& location, const string& orderInfo,
                                            const vector<DeliveryPartner*>& partners) override {
        // Dummy implementation: return all partners for simplicity
        cout << "Using QuickestDeliveryStrategy\n";
        return partners;
    }
};

class CheapestOptionStrategy : public IMatchingStrategy {
public:
    vector<DeliveryPartner*> getMatchingPartners(const string& location, const string& orderInfo,
                                            const vector<DeliveryPartner*>& partners) override {
        // Dummy implementation: return all partners for simplicity
        cout << "Using CheapestOptionStrategy\n";
        return partners;
    }
};

class DeliveryService {
    PartnerManager& partnerManager;
    IMatchingStrategy* strategy;
public:
    DeliveryService(PartnerManager& pm, IMatchingStrategy* strat) 
        : partnerManager(pm), strategy(strat) {}

    vector<DeliveryPartner*> findMatchingPartners(const string& location, const string& orderInfo) {
        return strategy->getMatchingPartners(location, orderInfo, partnerManager.getPartners());
    }
};

int main () {
    PartnerManager pm;
    pm.addPartner(new DeliveryPartner("Alice", "Downtown", "Bike"));
    pm.addPartner(new DeliveryPartner("Bob", "Uptown", "Car"));
    pm.addPartner(new DeliveryPartner("Charlie", "Midtown", "Scooter"));

    IMatchingStrategy* strategy = new NearestNeighborStrategy();
    DeliveryService service(pm, strategy);

    vector<DeliveryPartner*> matches = service.findMatchingPartners("Downtown", "Order123");
    cout << "Matched Partners: " << matches.size() << endl;

    delete strategy;
    return 0;
}
