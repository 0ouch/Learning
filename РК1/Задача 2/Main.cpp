#include"Header.h"
using namespace std; 
mutex coutMutex;

void safePrint(const string& msg) {
    lock_guard<mutex> lock(coutMutex);
    cout << msg << endl;
}

void loading(binary_semaphore& sem) {
    safePrint("[1] Loading started");
    this_thread::sleep_for(chrono::milliseconds(1500));
    safePrint("[1] Loading finished");
    sem.release();
}

void processing(binary_semaphore& semIn, binary_semaphore& semOut) {
    safePrint("[2] Processing waiting for signal...");
    semIn.acquire();
    safePrint("[2] Processing started");
    this_thread::sleep_for(chrono::milliseconds(2500));
    safePrint("[2] Processing finished");
    semOut.release();
}

void packaging(binary_semaphore& sem) {
    safePrint("[3] Packaging waiting for signal...");
    sem.acquire();
    safePrint("[3] Packaging started");
    this_thread::sleep_for(chrono::milliseconds(1200));
    safePrint("[3] Packaging finished");
}

int main() {
    binary_semaphore semProcessing(0);
    binary_semaphore semPackaging(0);

    thread t1(loading, ref(semProcessing));
    thread t2(processing, ref(semProcessing), ref(semPackaging));
    thread t3(packaging, ref(semPackaging));

    t1.join();
    t2.join();
    t3.join();

    return 0;
}