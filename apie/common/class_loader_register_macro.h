#pragma once

#include <iostream>


#define CLASS_LOADER_REGISTER_CLASS_INTERNAL(Derived, Base, UniqueID)         \
  namespace {                                                                 \
  struct ProxyType##UniqueID {                                                \
    ProxyType##UniqueID() {                                                   \
        std::cout << "ProxyType:" << #Derived  << " " << #Base << " " << UniqueID << std::endl;  \
    }                                                                         \
  };                                                                          \
  static ProxyType##UniqueID g_register_class_##UniqueID;                     \
  }

#define CLASS_LOADER_REGISTER_CLASS_INTERNAL_1(Derived, Base, UniqueID) \
  CLASS_LOADER_REGISTER_CLASS_INTERNAL(Derived, Base, UniqueID)

// register class macro
#define CLASS_LOADER_REGISTER_CLASS(Derived, Base) \
  CLASS_LOADER_REGISTER_CLASS_INTERNAL_1(Derived, Base, __COUNTER__)

#define CYBER_REGISTER_COMPONENT(name) \
  CLASS_LOADER_REGISTER_CLASS(name, apollo::cyber::ComponentBase)