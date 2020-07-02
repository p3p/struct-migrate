#include "struct_migrate.h"

#include <cstdint>
#include <iostream>
#include <fstream>

/*********************** Test storage implementations ******************************************/

constexpr std::size_t storage_size = 256;

namespace test_storage_ram {
[[gnu::aligned(4)]] char memory[storage_size]{}; // storage initialised constand [0xAB, 0xCD] and version 1 [0x01, 0x00]
struct StorageMedium {
  static bool init() {
    struct_store_migrate::storage_header header{0xCDAB, 1};
    write(0, (char *)&header, sizeof(header));
    struct_store_migrate::layout_header lheader{};
    write(sizeof(struct_store_migrate::storage_header), (char *)&lheader, sizeof(lheader));
    return true; // cant go wrong, right?
  }
  static void read(std::size_t offset, char* buffer, const std::size_t size) {
    memcpy(buffer, memory + offset, size);
  }
  static void write(std::size_t offset, char* buffer, const std::size_t size) {
    memcpy(memory + offset, buffer, size);
  }
  static constexpr std::size_t capacity() {
    return storage_size;
  }
};
}

namespace test_storage_file {
std::fstream file_memory;
struct StorageMedium {
  static bool init() {
    file_memory.open("memory.dat", std::ios::out|std::ios::in|std::ios::binary);
    if (!file_memory.is_open()) {
      file_memory.open("memory.dat", std::ios::out|std::ios::in|std::ios::binary|std::ios::trunc);
      for (std::size_t i = 0; i < storage_size; i ++) {
        file_memory.write("\377", 1);
      }
      file_memory.seekp(0, std::ios::beg);
      struct_store_migrate::storage_header header{0xCDAB, 1};
      file_memory.write((char *)&header, sizeof(header));
      struct_store_migrate::layout_header lheader{};
      file_memory.write((char *)&lheader, sizeof(lheader));
      file_memory.seekp(0, std::ios::beg);
      file_memory.flush();
    }
    return true; // cant go wrong, right?
  }
  static void read(std::size_t offset, char* buffer, const std::size_t size) {
    file_memory.seekg(offset);
    file_memory.read(buffer, size);
  }
  static void write(std::size_t offset, char* buffer, const std::size_t size) {
    file_memory.seekp(offset);
    file_memory.write(buffer, size);
  }
  static constexpr std::size_t capacity() {
    return storage_size;
  }
};
}

using namespace test_storage_file;

// helper function to save "T::Config config" properties
template <typename T>
inline bool save_config(T& feature) {
  return struct_store_migrate::save<StorageMedium, typename T::Config>(feature.config);
}

// helper function to load "T::Config config" properties
template <typename T>
inline bool load_config(T* feature) {
  return struct_store_migrate::load<StorageMedium, typename T::Config>(feature->config);
}



class FeatureFour {
public:
  struct Config {
    struct Layout_1 {
      using NextRevision = Layout_1;
      static constexpr std::size_t version = 1;
      NextRevision migrate() {return NextRevision{};}

      uint8_t value1;
      uint32_t value2;
      double value3;
    };
    using Layout = Layout_1;
  };
  Config::Layout config;

  FeatureFour() {
    load_config(this);
  }
};

class FeatureThree {
public:
  struct  Config {
    struct Layout_2 {
      using NextRevision = Layout_2;
      static constexpr std::size_t version = 2;
      NextRevision migrate() {return NextRevision{};}

      float value1;
      uint32_t value2;
      double value3;
    };
    struct Layout_1 {
      using NextRevision = Layout_2;
      static constexpr std::size_t version = 1;
      NextRevision migrate() {return NextRevision{(float)value1, value2, value3};}

      uint8_t value1;
      uint32_t value2;
      double value3;
    };
    using Layout = Layout_2;
  };
  Config::Layout config;

  FeatureThree() {
    load_config(this);
  }
};

class FeatureTwo {
public:
  struct Config {
    struct Layout_1 {
      using NextRevision = Layout_1;
      static constexpr std::size_t version = 1;
      NextRevision migrate() {return NextRevision{};}

      uint8_t value1;
      uint32_t value2;
      double value3;
    };
    using Layout = Layout_1;
  };
  Config::Layout config;

  FeatureTwo() {
    load_config(this);
  }
};


class Feature {
public:
  struct Config {
    struct Layout_5 {
      using NextRevision = Layout_5;
      static constexpr std::size_t version = 5;
      NextRevision migrate() { return NextRevision{}; }
      float value1;
      //uint32_t value2;
      //double value3;
    };
    struct Layout_4 {
      using NextRevision = Layout_5;
      static constexpr std::size_t version = 4;
      NextRevision migrate() { return NextRevision{value1/3.0f};/*, value2 + 10, 5.12345};*/ }
      float value1;
      uint32_t value2;
    };
    struct Layout_3 {
      using NextRevision = Layout_4;
      static constexpr std::size_t version = 3;
      NextRevision migrate() { return NextRevision{value1, value3}; }
      float value1;
      uint32_t value3;
    };
    struct Layout_2 {
      using NextRevision = Layout_3;
      static constexpr std::size_t version = 2;
      NextRevision migrate() { return NextRevision{static_cast<float>(value1), value3}; }
      uint16_t value1;
      uint8_t value2;
      uint32_t value3;
      float value4;
    };
    struct Layout_1 {
      using NextRevision = Layout_2;
      static constexpr std::size_t version = 1;
      NextRevision migrate() { return NextRevision{static_cast<uint16_t>(value1 * 2), value2, value3, value4}; }
      uint8_t value1;
      uint8_t value2;
      uint32_t value3;
      float value4;
    };
    using Layout = Layout_5;
  };
  Config::Layout config;

  Feature() {
    load_config(this);
  }
};


int main() {
  StorageMedium::init();

  Feature feature;
  FeatureTwo feature2;
  FeatureThree feature3;
  FeatureFour feature4;

  feature.config.value1 = 10.0f;

  save_config(feature);
  save_config(feature2);
  save_config(feature3);
  save_config(feature4);

  feature2.config.value2 = 10000;
  save_config(feature2);

  FeatureTwo reload;
  Feature reload2;
  std::cout << reload.config.value2 << std::endl;

  /*
  *  Save old version and try a default load
  */
  Feature::Config::Layout_2 test{};
  struct_store_migrate::save<StorageMedium, Feature::Config>(test);

  Feature feature_test;
  save_config(feature);

  return 0;
}