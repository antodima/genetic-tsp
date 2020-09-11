#include <iostream>
#include <chrono>
#include <thread>
using namespace std;


class utimer {
  std::chrono::system_clock::time_point start;
  std::chrono::system_clock::time_point stop;
  string message;
  using nsecs = std::chrono::nanoseconds;
  using usecs = std::chrono::microseconds;
  using msecs = std::chrono::milliseconds;

public:

  utimer(const std::string m) : message(m) {
    start = std::chrono::high_resolution_clock::now();
  }

  ~utimer() {
    auto elapsed = std::chrono::duration_cast<usecs>(std::chrono::high_resolution_clock::now() - start).count();
    cout<<message<<" computed in "<<elapsed<<" usecs"<<endl;
  }
};
