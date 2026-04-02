// РК 1 задача 1

#include "Header.h"

// Глобальный мьютекс для синхронизации вывода
mutex coutMutex;

// 1. Класс зоны хранения

class StorageZone {
private:
    string name;
    int stock;
    int targetStock;
    mutable mutex mtx;
    condition_variable cv;
    atomic<bool> isOccupied;

public:
    StorageZone(const string& n, int initial, int target)
        : name(n), stock(initial), targetStock(target), isOccupied(false) {
    }

    // Запрещаем копирование
    StorageZone(const StorageZone&) = delete;
    StorageZone& operator=(const StorageZone&) = delete;

    // Проверка запаса 
    bool checkStock() {
        // Пытаемся захватить зону
        if (isOccupied.exchange(true)) {
            return false;
        }

        unique_lock<mutex> lock(mtx);

        {
            lock_guard<mutex> coutLock(coutMutex);
            cout << "[CHECKER] " << name << ": current stock = " << stock << endl;
        }

        bool needRestock = (stock < targetStock);

        lock.unlock();

        isOccupied = false;
        cv.notify_one();

        return needRestock;
    }

    // Добавление товара 
    bool restock() {
        if (isOccupied.exchange(true)) {
            return false;
        }

        unique_lock<mutex> lock(mtx);

        // Проверяем, нужно ли вообще добавлять
        if (stock >= targetStock) {
            {
                lock_guard<mutex> coutLock(coutMutex);
                cout << "[RESTOCKER] " << name << ": already enough stock ("
                    << stock << " >= " << targetStock << "), skipping" << endl;
            }
            lock.unlock();
            isOccupied = false;
            cv.notify_one();
            return false;
        }

        int oldStock = stock;

        // Добавляем до targetStock
        int needed = targetStock - stock;
        stock = targetStock; 

        {
            lock_guard<mutex> coutLock(coutMutex);
            cout << "[RESTOCKER] " << name << ": was " << oldStock
                << ", added " << needed << ", now " << stock
                << " (target: " << targetStock << ")" << endl;
        }

        lock.unlock();

        isOccupied = false;
        cv.notify_one();

        return true;
    }

    int getStock() const {
        lock_guard<mutex> lock(mtx);
        return stock;
    }

    string getName() const { return name; }
};

// 2. Класс оператора - общий для проверяющего и пополнителя

class Operator {
private:
    string name;
    StorageZone& zone;
    bool isChecker;
    atomic<bool>& activeFlag;

public:
    Operator(const string& n, StorageZone& z, bool checker, atomic<bool>& flag)
        : name(n), zone(z), isChecker(checker), activeFlag(flag) {
    }

    void operator()() {
        while (activeFlag) {
            if (isChecker) {
                // Проверяющий
                if (zone.checkStock()) {
                    {
                        lock_guard<mutex> coutLock(coutMutex);
                        cout << "   >>> " << zone.getName() << " needs restock!" << endl;
                    }
                }
                this_thread::sleep_for(chrono::milliseconds(500));
            }
            else {
                // Пополнитель
                zone.restock();
                this_thread::sleep_for(chrono::milliseconds(800));
            }
        }
    }
};

// 3. Класс менеджера
class WarehouseManager {
private:
    vector<unique_ptr<StorageZone>> zones;
    vector<thread> threads;
    atomic<bool> isRunning{ true };

public:
    WarehouseManager() {
        zones.push_back(make_unique<StorageZone>("Zone A", 50, 100));
        zones.push_back(make_unique<StorageZone>("Zone B", 60, 100));
    }

    void start() {
        // Для каждой зоны создаём проверяющего и пополнителя
        threads.emplace_back(Operator("Checker A", *zones[0], true, isRunning));
        threads.emplace_back(Operator("Restocker A", *zones[0], false, isRunning));

        threads.emplace_back(Operator("Checker B", *zones[1], true, isRunning));
        threads.emplace_back(Operator("Restocker B", *zones[1], false, isRunning));
    }

    void stop() {
        isRunning = false;
        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

    void printReport() {
        cout << endl<<"FINAL REPORT " << endl;
        for (auto& zone : zones) {
            cout << zone->getName() << ": stock = " << zone->getStock() << endl;
        }
    }
};

int main() {
    WarehouseManager warehouse;
    warehouse.start();

    cout << "System running for 2 seconds"<<endl;
    this_thread::sleep_for(chrono::seconds(2));

    warehouse.stop();
    warehouse.printReport();

    return 0;
}