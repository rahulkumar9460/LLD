#include<iostream>
#include<vector>
#include<unordered_map>
#include<string>
using namespace std;

/* 
    Proxy Design Pattern 
    🧩 Proxy Pattern — Overview

        Definition:

        The Proxy pattern provides a surrogate or placeholder for another object to control access to it.
        It’s part of the Structural Design Patterns (from the GoF patterns).

        🧱 Core Characteristics

        Surrogate object — A proxy class that represents the real object.

        Control access — The proxy controls access to the real object, adding a layer of indirection.

        Interface conformity — The proxy implements the same interface as the real object.

                | ✅ Use When                                     | 🚫 Avoid When                                          |
        | ---------------------------------------------- | ------------------------------------------------------ |
        | You need to control access to an object         | You have simple objects that don’t require access control |
        | You want to add functionality (e.g., logging,   | You want to avoid added complexity                      |
        | caching) without changing the real object       | You have performance-critical code where overhead is unacceptable |
    
        🧩 The Problem

        Direct access to an object can lead to issues like security vulnerabilities,
        performance bottlenecks, or unwanted side effects.

        🧩 The Solution
        Introduce a proxy class that controls access to the real object.
        The proxy can add additional behavior (like logging, caching, or access control)
        without modifying the real object's code.

        Example:
        Consider a bus ticket booking system where searching for buses can be resource-intensive.
        A ProxySearchService can cache results of previous searches to improve performance and log search activities.

        Benefits:
        - Access Control: Restrict or manage access to the real object.
        - Additional Functionality: Add features like logging or caching transparently.
        - Lazy Initialization: Delay the creation of resource-intensive objects until they are needed.

        Problem with Proxy Pattern:
        - Added Complexity: Introduces additional classes and layers.
        - Performance Overhead: May introduce latency due to the extra layer of indirection.
*/

class Bus {
    string name;
    string pickup;
    string destination;
public:
    Bus(string name, string pickup, string destination)
        : name(name), pickup(pickup), destination(destination) {}
};

class ISearchService {
public:
    virtual vector<Bus> searchBus(string pickup, string destination) = 0;
    virtual ~ISearchService() = default;
};

class RealSearchService: public ISearchService {
public:
    vector<Bus> searchBus(string pickup, string destination) override {
        cout << "Searching buses from " << pickup << " to " << destination << endl;
        return {{"Bus1", pickup, destination}, {"Bus2", pickup, destination}};
    }
};

class ProxySearchService: public ISearchService {
    RealSearchService* realService;
    unordered_map<string, vector<Bus>> cache;
public:
    ProxySearchService(RealSearchService* realService) : realService(realService) {}

    vector<Bus> searchBus(string pickup, string destination) override {
        cout << "Proxy: Logging search request from " << pickup << " to " << destination << endl;

        string cacheKey = pickup + "-" + destination;
        if (cache.find(cacheKey) != cache.end()) {
            cout << "Proxy: Returning cached results for " << cacheKey << endl;
            return cache[cacheKey];
        }

        vector<Bus> results = realService->searchBus(pickup, destination);
        cache[cacheKey] = results;
        return results;
    }
};

// to run the code use command: g++ -std=c++17 main.cpp -o main && ./main
int main() {
    cout << "Proxy Design Pattern in C++\n";
    RealSearchService* realService = new RealSearchService();
    ProxySearchService proxyService(realService);

    // First search (cache miss)
    proxyService.searchBus("LocationA", "LocationB");

    // Second search (cache hit)
    proxyService.searchBus("LocationA", "LocationB");

    return 0;
}