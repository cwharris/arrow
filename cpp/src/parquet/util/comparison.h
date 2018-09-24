// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#ifndef PARQUET_UTIL_COMPARISON_H
#define PARQUET_UTIL_COMPARISON_H

#include <algorithm>
#include <memory>

#include "parquet/exception.h"
#include "parquet/schema.h"
#include "parquet/types.h"
#include "parquet/util/visibility.h"

namespace parquet {

class PARQUET_EXPORT Comparator {
 public:
  virtual ~Comparator() {}
  static std::shared_ptr<Comparator> Make(const ColumnDescriptor* descr);
};

// The default comparison is SIGNED
template <typename DType>
class PARQUET_TEMPLATE_CLASS_EXPORT CompareDefault : public Comparator {
 public:
  typedef typename DType::c_type T;
  CompareDefault() {}
  virtual bool operator()(const T& a, const T& b) { return a < b; }
};

template <>
class PARQUET_TEMPLATE_CLASS_EXPORT CompareDefault<Int96Type> : public Comparator {
 public:
  CompareDefault() {}
  virtual bool operator()(const Int96& a, const Int96& b) {
    // Only the MSB bit is by Signed comparison
    // For little-endian, this is the last bit of Int96 type
    const int32_t amsb = static_cast<const int32_t>(a.value[2]);
    const int32_t bmsb = static_cast<const int32_t>(b.value[2]);
    if (amsb != bmsb) {
      return (amsb < bmsb);
    } else if (a.value[1] != b.value[1]) {
      return (a.value[1] < b.value[1]);
    }
    return (a.value[0] < b.value[0]);
  }
};

template <>
class PARQUET_TEMPLATE_CLASS_EXPORT CompareDefault<ByteArrayType> : public Comparator {
 public:
  CompareDefault() {}
  virtual bool operator()(const ByteArray& a, const ByteArray& b) {
    const int8_t* aptr = reinterpret_cast<const int8_t*>(a.ptr);
    const int8_t* bptr = reinterpret_cast<const int8_t*>(b.ptr);
    return std::lexicographical_compare(aptr, aptr + a.len, bptr, bptr + b.len);
  }
};

template <>
class PARQUET_TEMPLATE_CLASS_EXPORT CompareDefault<FLBAType> : public Comparator {
 public:
  explicit CompareDefault(int length) : type_length_(length) {}
  virtual bool operator()(const FLBA& a, const FLBA& b) {
    const int8_t* aptr = reinterpret_cast<const int8_t*>(a.ptr);
    const int8_t* bptr = reinterpret_cast<const int8_t*>(b.ptr);
    return std::lexicographical_compare(aptr, aptr + type_length_, bptr,
                                        bptr + type_length_);
  }
  int32_t type_length_;
};

typedef CompareDefault<BooleanType> CompareDefaultBoolean;
typedef CompareDefault<Int32Type> CompareDefaultInt32;
typedef CompareDefault<Int64Type> CompareDefaultInt64;
typedef CompareDefault<Int96Type> CompareDefaultInt96;
typedef CompareDefault<FloatType> CompareDefaultFloat;
typedef CompareDefault<DoubleType> CompareDefaultDouble;
typedef CompareDefault<ByteArrayType> CompareDefaultByteArray;
typedef CompareDefault<FLBAType> CompareDefaultFLBA;

// Define Unsigned Comparators
class PARQUET_EXPORT CompareUnsignedInt32 : public CompareDefaultInt32 {
 public:
  bool operator()(const int32_t& a, const int32_t& b) override;
};

class PARQUET_EXPORT CompareUnsignedInt64 : public CompareDefaultInt64 {
 public:
  bool operator()(const int64_t& a, const int64_t& b) override;
};

class PARQUET_EXPORT CompareUnsignedInt96 : public CompareDefaultInt96 {
 public:
  bool operator()(const Int96& a, const Int96& b) override;
};

class PARQUET_EXPORT CompareUnsignedByteArray : public CompareDefaultByteArray {
 public:
  bool operator()(const ByteArray& a, const ByteArray& b) override;
};

class PARQUET_EXPORT CompareUnsignedFLBA : public CompareDefaultFLBA {
 public:
  explicit CompareUnsignedFLBA(int length);
  bool operator()(const FLBA& a, const FLBA& b) override;
};

PARQUET_EXTERN_TEMPLATE CompareDefault<BooleanType>;
PARQUET_EXTERN_TEMPLATE CompareDefault<Int32Type>;
PARQUET_EXTERN_TEMPLATE CompareDefault<Int64Type>;
PARQUET_EXTERN_TEMPLATE CompareDefault<Int96Type>;
PARQUET_EXTERN_TEMPLATE CompareDefault<FloatType>;
PARQUET_EXTERN_TEMPLATE CompareDefault<DoubleType>;
PARQUET_EXTERN_TEMPLATE CompareDefault<ByteArrayType>;
PARQUET_EXTERN_TEMPLATE CompareDefault<FLBAType>;

}  // namespace parquet

#endif  // PARQUET_UTIL_COMPARISON_H
