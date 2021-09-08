// https://www.youtube.com/watch?v=jJS6G7irZSc&t=818s&ab_channel=CodingTech

#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>

#include <iostream>
#include <array>
#include <string>
#include <random>

// mutex just for printing
std::mutex g_lockprint;
const int num_orders = 5; 

struct Grinder {
  std::mutex mutex; 
};
// TODO: make this multiple parts so we actually need multiple mutexes at once to operate this press
struct Press {
  std::mutex mutex;
};
struct Steamer {
  std::mutex mutex;
};

struct Latte {
  int num;
  bool grind_complete = false;
  bool expresso_complete = false;
  bool milk_complete = false;
  Grinder& grinder_ref;
  Press& press_ref;
  Steamer& steamer_ref;
  std::thread lifethread;
  std::mt19937 rng { std::random_device{}() }; 

  Latte(int n, Grinder& g, Press& p, Steamer& s) : num(n), grinder_ref(g), press_ref(p), steamer_ref(s), lifethread(&Latte::work, this) {
  }

  ~Latte() {
    lifethread.join();
  }

  void print(std::string text) {
    std::lock_guard<std::mutex> cout_lock(g_lockprint);
    std::cout << std::left << std::setw(10) << std::setfill(' ') << "latte " << num << text << std::endl;
  }
   
  void work() {
    do {
     if(!grind_complete) use_grinder();
     if(!expresso_complete) use_press();
     if(!milk_complete) use_steamer();
     sleep();
    } while(!grind_complete || !expresso_complete || !milk_complete);
  }

  void use_grinder() {
    // If grinder mutex already locked, skip this (move onto the use_press() step so we can try again later)
    if(grinder_ref.mutex.try_lock()) {
      // The try_lock above was successful, this thread already has ownership of the grinder_ref.mutex
      // **still** want to use lock_guard
      // This is the use case for adopt_lock => "assume the calling thread already has ownership of the mutex"
      std::lock_guard<std::mutex> grinder_lock(grinder_ref.mutex, std::adopt_lock);
      print(" is using grinder");
      // other threads continue their execution, this only blocks current thread for the time specified
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<> distr(1, 6);
      std::this_thread::sleep_for (std::chrono::seconds(distr(gen)));  
      grind_complete = true;
    }
    // mutex would be unlocked here if manually locked mutex instead of using lock_guard
  }

  void use_press() {
    if(press_ref.mutex.try_lock()) {
      std::lock_guard<std::mutex> press_lock(press_ref.mutex, std::adopt_lock);
      print(" is using press");
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<> distr(1, 6);
      std::this_thread::sleep_for (std::chrono::seconds(distr(gen)));  
      expresso_complete = true;
    }
  }

  void use_steamer() {
    if(steamer_ref.mutex.try_lock()) {
      std::lock_guard<std::mutex> steamer_lock(steamer_ref.mutex, std::adopt_lock);
      print(" using using steamer");
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<> distr(1, 6);
      std::this_thread::sleep_for (std::chrono::seconds(distr(gen)));  
      milk_complete = true;
    }
  }

  void sleep() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(1, 6);
    std::this_thread::sleep_for (std::chrono::seconds(distr(gen)));  
    print(" is waiting");
  }
};

// creates Grinder, Press, Steamer
// creates 5 Latte orders
void shop_start() {
  std::cout << "starting shop" << std::endl;
  Grinder g; // Grinder/Press/Steamer all have default constructors and calling them here
  Press p;
  Steamer s;
  {
    // initialize a std::array with Latte structs
    std::array<Latte, num_orders> orders {{
        { 0, g, p, s},
        { 1, g, p, s },
        { 2, g, p, s },
        { 3, g, p, s },
        { 4, g, p, s },
    }};
  }
  std::cout << "job finished" << std::endl;
}

int main() {
  shop_start();
  return 0;
}
