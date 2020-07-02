// MIT License

// Copyright (c) 2020 Chris Pepper

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <cstdint>
#include <cstring>
#include <utility>

#include <iostream>

namespace struct_store_migrate {

/******************* Compile time repeat with numberic iteration value <source: https://vittorioromeo.info/> ******************************/
namespace vittorioromeo {
namespace detail {
    template <auto... Is> struct seq_type_impl : std::common_type<decltype(Is)...> {};
    template <>           struct seq_type_impl<> { using type = int; };
    template <auto... Is> using seq_type = typename seq_type_impl<Is...>::type;
}

template <auto... Is> using sequence      = std::integer_sequence<detail::seq_type<Is...>, Is...>;
template <auto I>     using constant      = std::integral_constant<decltype(I), I>;
template <auto I>     using make_sequence = std::make_integer_sequence<decltype(I), I>;

namespace detail {
    template <typename F, auto... Is>
    void repeat_impl(F&& f, sequence<Is...>) {
        if constexpr((std::is_invocable_v<F&&, constant<Is>> && ...)) {
            ( f(constant<Is>{}), ... );
        } else {
            ( ((void)Is, f()), ... );
        }
    }
}

template <auto N, typename F> void repeat(F&& f) { detail::repeat_impl(f, make_sequence<N>{}); }

}

// Compile time struct version migration

template <typename T>
constexpr bool can_migrate() {
  return !std::is_same<T, typename T::NextRevision>::value;
}

template <typename T, std::size_t N> struct select_layout { using type = typename select_layout<T, N - 1>::type::NextRevision; };
template <typename T> struct select_layout<T, 1> { using type = typename T::Layout_1; };

template <typename T, typename U>
void migrate_to(T layout, U& target ) {
    if constexpr (can_migrate<T>() && !std::is_same<T, U>::value ) {
      migrate_to(layout.migrate(), target);
    } else {
      target = layout;
    }
}

//  Compile time hashing implementation
namespace checksum_lookup {
constexpr uint32_t crc32Table[256] = {
	0x00000000L, 0xF26B8303L, 0xE13B70F7L, 0x1350F3F4L,
	0xC79A971FL, 0x35F1141CL, 0x26A1E7E8L, 0xD4CA64EBL,
	0x8AD958CFL, 0x78B2DBCCL, 0x6BE22838L, 0x9989AB3BL,
	0x4D43CFD0L, 0xBF284CD3L, 0xAC78BF27L, 0x5E133C24L,
	0x105EC76FL, 0xE235446CL, 0xF165B798L, 0x030E349BL,
	0xD7C45070L, 0x25AFD373L, 0x36FF2087L, 0xC494A384L,
	0x9A879FA0L, 0x68EC1CA3L, 0x7BBCEF57L, 0x89D76C54L,
	0x5D1D08BFL, 0xAF768BBCL, 0xBC267848L, 0x4E4DFB4BL,
	0x20BD8EDEL, 0xD2D60DDDL, 0xC186FE29L, 0x33ED7D2AL,
	0xE72719C1L, 0x154C9AC2L, 0x061C6936L, 0xF477EA35L,
	0xAA64D611L, 0x580F5512L, 0x4B5FA6E6L, 0xB93425E5L,
	0x6DFE410EL, 0x9F95C20DL, 0x8CC531F9L, 0x7EAEB2FAL,
	0x30E349B1L, 0xC288CAB2L, 0xD1D83946L, 0x23B3BA45L,
	0xF779DEAEL, 0x05125DADL, 0x1642AE59L, 0xE4292D5AL,
	0xBA3A117EL, 0x4851927DL, 0x5B016189L, 0xA96AE28AL,
	0x7DA08661L, 0x8FCB0562L, 0x9C9BF696L, 0x6EF07595L,
	0x417B1DBCL, 0xB3109EBFL, 0xA0406D4BL, 0x522BEE48L,
	0x86E18AA3L, 0x748A09A0L, 0x67DAFA54L, 0x95B17957L,
	0xCBA24573L, 0x39C9C670L, 0x2A993584L, 0xD8F2B687L,
	0x0C38D26CL, 0xFE53516FL, 0xED03A29BL, 0x1F682198L,
	0x5125DAD3L, 0xA34E59D0L, 0xB01EAA24L, 0x42752927L,
	0x96BF4DCCL, 0x64D4CECFL, 0x77843D3BL, 0x85EFBE38L,
	0xDBFC821CL, 0x2997011FL, 0x3AC7F2EBL, 0xC8AC71E8L,
	0x1C661503L, 0xEE0D9600L, 0xFD5D65F4L, 0x0F36E6F7L,
	0x61C69362L, 0x93AD1061L, 0x80FDE395L, 0x72966096L,
	0xA65C047DL, 0x5437877EL, 0x4767748AL, 0xB50CF789L,
	0xEB1FCBADL, 0x197448AEL, 0x0A24BB5AL, 0xF84F3859L,
	0x2C855CB2L, 0xDEEEDFB1L, 0xCDBE2C45L, 0x3FD5AF46L,
	0x7198540DL, 0x83F3D70EL, 0x90A324FAL, 0x62C8A7F9L,
	0xB602C312L, 0x44694011L, 0x5739B3E5L, 0xA55230E6L,
	0xFB410CC2L, 0x092A8FC1L, 0x1A7A7C35L, 0xE811FF36L,
	0x3CDB9BDDL, 0xCEB018DEL, 0xDDE0EB2AL, 0x2F8B6829L,
	0x82F63B78L, 0x709DB87BL, 0x63CD4B8FL, 0x91A6C88CL,
	0x456CAC67L, 0xB7072F64L, 0xA457DC90L, 0x563C5F93L,
	0x082F63B7L, 0xFA44E0B4L, 0xE9141340L, 0x1B7F9043L,
	0xCFB5F4A8L, 0x3DDE77ABL, 0x2E8E845FL, 0xDCE5075CL,
	0x92A8FC17L, 0x60C37F14L, 0x73938CE0L, 0x81F80FE3L,
	0x55326B08L, 0xA759E80BL, 0xB4091BFFL, 0x466298FCL,
	0x1871A4D8L, 0xEA1A27DBL, 0xF94AD42FL, 0x0B21572CL,
	0xDFEB33C7L, 0x2D80B0C4L, 0x3ED04330L, 0xCCBBC033L,
	0xA24BB5A6L, 0x502036A5L, 0x4370C551L, 0xB11B4652L,
	0x65D122B9L, 0x97BAA1BAL, 0x84EA524EL, 0x7681D14DL,
	0x2892ED69L, 0xDAF96E6AL, 0xC9A99D9EL, 0x3BC21E9DL,
	0xEF087A76L, 0x1D63F975L, 0x0E330A81L, 0xFC588982L,
	0xB21572C9L, 0x407EF1CAL, 0x532E023EL, 0xA145813DL,
	0x758FE5D6L, 0x87E466D5L, 0x94B49521L, 0x66DF1622L,
	0x38CC2A06L, 0xCAA7A905L, 0xD9F75AF1L, 0x2B9CD9F2L,
	0xFF56BD19L, 0x0D3D3E1AL, 0x1E6DCDEEL, 0xEC064EEDL,
	0xC38D26C4L, 0x31E6A5C7L, 0x22B65633L, 0xD0DDD530L,
	0x0417B1DBL, 0xF67C32D8L, 0xE52CC12CL, 0x1747422FL,
	0x49547E0BL, 0xBB3FFD08L, 0xA86F0EFCL, 0x5A048DFFL,
	0x8ECEE914L, 0x7CA56A17L, 0x6FF599E3L, 0x9D9E1AE0L,
	0xD3D3E1ABL, 0x21B862A8L, 0x32E8915CL, 0xC083125FL,
	0x144976B4L, 0xE622F5B7L, 0xF5720643L, 0x07198540L,
	0x590AB964L, 0xAB613A67L, 0xB831C993L, 0x4A5A4A90L,
	0x9E902E7BL, 0x6CFBAD78L, 0x7FAB5E8CL, 0x8DC0DD8FL,
	0xE330A81AL, 0x115B2B19L, 0x020BD8EDL, 0xF0605BEEL,
	0x24AA3F05L, 0xD6C1BC06L, 0xC5914FF2L, 0x37FACCF1L,
	0x69E9F0D5L, 0x9B8273D6L, 0x88D28022L, 0x7AB90321L,
	0xAE7367CAL, 0x5C18E4C9L, 0x4F48173DL, 0xBD23943EL,
	0xF36E6F75L, 0x0105EC76L, 0x12551F82L, 0xE03E9C81L,
	0x34F4F86AL, 0xC69F7B69L, 0xD5CF889DL, 0x27A40B9EL,
	0x79B737BAL, 0x8BDCB4B9L, 0x988C474DL, 0x6AE7C44EL,
	0xBE2DA0A5L, 0x4C4623A6L, 0x5F16D052L, 0xAD7D5351L
};

constexpr uint32_t crc32(const char *message) {
  int32_t i = 0, crc = 0;
  const uint8_t *p = (uint8_t*)message;
  while (message[i++] != 0) crc = crc32Table[(crc ^ *p++) & 0xff] ^ (crc >> 8);
  return crc;
}
constexpr uint32_t crc32(const char *message, std::size_t length) {
  int32_t i = 0, crc = 0;
  const uint8_t *p = (uint8_t*)message;
  while (length--) crc = crc32Table[(crc ^ *p++) & 0xff] ^ (crc >> 8);
  return crc;
}
}

//seclect the crc function to use
using checksum_lookup::crc32;

template <typename T> constexpr std::size_t Hash() { return crc32(/*typeid(T).name());}//*/__PRETTY_FUNCTION__); } // typeid not constexpr
template <typename T> std::size_t constexpr_hash = Hash<T>();

template <typename T> constexpr uint32_t struct_checksum(const T& data) {
  return crc32((char*)&data, sizeof(T));
}

// struct binary serialisation

template <typename Storage, typename T>
std::size_t read_struct(const std::size_t offset, T* object) {
  static_assert(std::is_trivial_v<T>, "Struct not trivial");
  Storage::read(offset, (char *)object, sizeof(T));
  return sizeof(T);
}

template <typename Storage, typename T>
std::size_t write_struct(const std::size_t offset, T* object) {
  static_assert(std::is_trivial_v<T>, "Struct not trivial");
  Storage::write(offset, (char *)object, sizeof(T));
  return sizeof(T);
}

template <typename Storage>
void storage_resize_block(const std::size_t offset, const std::size_t current_size, const std::size_t new_size, const std::size_t data_end) {
  constexpr std::size_t buffer_size = 32;
  char buffer[buffer_size];
  int32_t delta = new_size - current_size;
  if (current_size < new_size) {
    int32_t move_block = (offset + current_size);
    for(int32_t i = data_end; i > move_block; i-= buffer_size ) {
      std::size_t block_start = ((i - buffer_size) < move_block ? move_block : (i - buffer_size));
      Storage::read(block_start, buffer, i - block_start);
      Storage::write(block_start + delta, buffer, i - block_start);
    }
  } else {
    for(int32_t i = offset + current_size; i < data_end; i+= buffer_size ) {
      std::size_t block_end = i + buffer_size < data_end ? i + buffer_size : data_end;
      Storage::read(i, buffer, block_end - i);
      Storage::write(i + delta, buffer, block_end - i);
    }
  }
}

//*********** struct storage system *************//

struct storage_header {
  uint16_t token;
  uint16_t version;
};
struct layout_header {
  uint32_t typename_crc;
  uint16_t version, size;
};
struct layout_descriptor {
  uint16_t version, size;
  std::size_t offset;
};
using layout_checksum = uint32_t;

template <typename T, typename Storage>
layout_descriptor read_layout_descriptor() {
  // read structure header
  std::size_t offset = 0;
  storage_header layout;
  offset += read_struct<Storage>(offset, &layout);

  if (layout.token != 52651 || layout.version != 1) {
    return layout_descriptor{};
  }

  // search for slot
  layout_header header;
  offset += read_struct<Storage>(offset, &header);

  while(header.typename_crc !=  constexpr_hash<T> && header.typename_crc != 0 && header.size != 0 && offset < Storage::capacity()) {
    offset += header.size + sizeof(layout_checksum);
    offset += read_struct<Storage>(offset, &header);
  }

  return layout_descriptor{T::Layout::version >= header.version ? header.version : (uint16_t)0, header.size, offset};
}

template <typename Storage>
layout_descriptor new_layout_descriptor() {
  // read structure header
  std::size_t offset = 0;
  storage_header layout;
  offset += read_struct<Storage>(offset, &layout);

  // storage is not formatted correctly? error out?
  if (layout.token != 52651 || layout.version != 1) {
    return layout_descriptor{};
  }

  // search for new slot
  layout_header header;
  offset += read_struct<Storage>(offset, &header);
  while(header.typename_crc != 0 && offset < Storage::capacity()) {
    offset += header.size + sizeof(layout_checksum);
    offset += read_struct<Storage>(offset, &header);
  }

  return layout_descriptor{(uint16_t)0, 0, offset};
}

template <typename T, typename Storage, std::size_t N, typename U>
void load_layout(layout_descriptor& descriptor, U& layout) {
  using Layout_N = typename select_layout<T, N>::type;
  Layout_N layout_N;

  std::size_t bytes = read_struct<Storage>(descriptor.offset, &layout_N);
  uint32_t stored_checksum;
  read_struct<Storage>(descriptor.offset + bytes, &stored_checksum);
  uint32_t current_checksum = struct_checksum(layout_N);

  if (stored_checksum != current_checksum) {
    std::cout << "Checksum mismatch loading defaults for struct version(" << N << ")\n";
    layout_N = Layout_N{};
  }

  if constexpr (Layout_N::version != U::version) {
    migrate_to(layout_N, layout);
    std::cout << "Migrated struct from version(" << layout_N.version << ") to version(" << layout.version << ")\n";
  } else {
    layout = layout_N;
    std::cout << "Loaded " << typeid(T).name() <<" version(" << layout.version << ")\n";
  }
}

template <typename Storage, typename T, typename U>
bool load(U& layout) {
  auto layout_desc = read_layout_descriptor<T, Storage>();
  if (layout_desc.version <= 0 || layout_desc.version > U::version) {
    std::cout << "Failed to migrate from version(" << layout_desc.version << "), loaded defaults\n";
    layout = U{}; //default initialise
    return false;
  }
  vittorioromeo::repeat<U::version>([&layout_desc, &layout](auto i) {
    if (i + 1 == layout_desc.version) { load_layout<T, Storage, i + 1>(layout_desc, layout); return; }
  });
  return true;
}

template <typename Storage, typename T, typename U>
bool save(U& layout) {
  auto layout_desc = read_layout_descriptor<T, Storage>();
  layout_header header{static_cast<uint32_t>(constexpr_hash<T>), static_cast<uint16_t>(U::version), static_cast<uint16_t>(sizeof(U))};

  std::cout << "Saving " << typeid(T).name() << " version(" << layout.version << ")\n";
  if (!(layout_desc.version == 0 || layout_desc.size == header.size)) {
    std::cout << "Saving " << typeid(T).name() <<" version(" << U::version << ") to a version(" << layout_desc.version << ") slot, size mismatch, Resizing\n";
    storage_resize_block<Storage>(layout_desc.offset, layout_desc.size,  header.size, new_layout_descriptor<Storage>().offset);
  }

  write_struct<Storage>(layout_desc.offset - sizeof(layout_header), &header);
  std::size_t bytes = write_struct<Storage>(layout_desc.offset, &layout);
  uint32_t checksum = struct_checksum(layout);
  bytes += write_struct<Storage>(layout_desc.offset + bytes, &checksum);
  if (layout_desc.version == 0) {
    std::cout << "Creating new block for " << typeid(T).name() <<" version(" << U::version << ")\n";
    layout_header terminator{};
    write_struct<Storage>(layout_desc.offset + bytes, &terminator); // write the "null termination" of the linked list
  }
  return true;
}

} // namespace struct_store_migrate
