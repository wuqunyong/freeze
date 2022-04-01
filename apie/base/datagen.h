#pragma once

#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <variant>
#include <vector>


namespace apie {
namespace base {

std::string RandomString(size_t length) {
  constexpr char kCharSet[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  std::random_device rnd_device;
  std::mt19937 mersenne_engine{rnd_device()};  // Generates random integers
  std::uniform_int_distribution<> distrib(0, sizeof(kCharSet) - 1);

  auto randchar = [&]() -> char { return kCharSet[distrib(mersenne_engine)]; };
  std::string str(length, 0);
  std::generate_n(str.begin(), length, randchar);
  return str;
}

}
}
