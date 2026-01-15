#include<iostream>
#include<memory>
#include<vector>
#include<mutex>
using namespace std;

/* Singleton Database Class 
    🧩 Singleton Design Pattern — Overview

        Definition:

        The Singleton pattern ensures that only one instance of a class exists in the entire program,
        and provides a global point of access to that instance.

        It’s part of the Creational Design Patterns (from the GoF patterns).

        🧱 Core Characteristics

        Single instance — Only one object is ever created.

        Global access — Everyone uses the same instance through a static method (like getInstance()).

        Controlled creation — Constructor is private; no one else can new it.

                | ✅ Use When                                     | 🚫 Avoid When                                          |
        | ---------------------------------------------- | ------------------------------------------------------ |
        | You need one instance for shared state         | You might need multiple instances later                |
        | Object creation is expensive (lazy init helps) | You want easy testing and decoupling                   |
        | Central coordination (e.g., logging, DB pool)  | You just want easy access (global variable temptation) |
    
        🧩 The Problem

        Singletons make testing hard because:

        The instance is globally accessible and hard-wired (via getInstance()).

        You can’t easily inject mocks (e.g., mock database or logger).

        The Singleton instance often persists across tests, causing state leakage.

        So while production code may rely on the Singleton, unit tests need control and isolation.

        🧩 The Solution
        Use Dependency Injection (DI) to pass the Singleton instance into classes that need it.
        This way, in tests, you can pass a mock or stub instead of the real Singleton.
        This keeps your code flexible and testable while still using the Singleton pattern in production.

        class IDatabase {
        public:
            virtual void getData(const std::string& sql) = 0;
            virtual ~IDatabase() = default;
        };

        // ---- Mock Implementation for tests ----
        class MockDatabase : public IDatabase {
        public:
            void getData(const std::string& sql) override {
                std::cout << "[MockDB] Pretending to run: " << sql << "\n";
            }
        };

        Now UTs can use MockDatabase instead of the real Singleton.
*/
class Database {
    static Database* instance;
    static mutex mtx;
    Database () {}
    Database (const Database& right) = delete;
    Database& operator=(const Database& right) = delete;

    vector<int> data;

public:
    static Database* getInstance() {
        if (instance == nullptr) {
            mtx.lock();
            if(instance == nullptr) {
                instance = new Database();
            }
            mtx.unlock();
        }
        return instance;
    }

    void addData(int value) {
        data.push_back(value);
    }

    vector<int> getData() {
        return data;
    }
};

// Define static members
Database* Database::instance = nullptr;
mutex Database::mtx;

int main() {
    Database* db1 = Database::getInstance();
    db1->addData(10);
    db1->addData(20);

    Database* db2 = Database::getInstance();
    db2->addData(30);

    vector<int> data = db1->getData();
    cout << "Data in Database: ";
    for (int val : data) {
        cout << val << " ";
    }
    cout << endl;

    return 0;
}