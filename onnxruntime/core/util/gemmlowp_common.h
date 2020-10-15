// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#pragma once
#include "core/util/gemmlowp_common_wrapper.h"
#include "core/platform/threadpool.h"
#include "onnxruntime_config.h"
#ifdef __GNUC__
#pragma GCC diagnostic push
#ifdef HAS_DEPRECATED_COPY
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#endif
#endif

namespace onnxruntime {

void inline QuantizeMultiplier(float fp_multiplier, std::int32_t* integer_multiplier, int* right_shift) {
  auto* fp_as_bits = reinterpret_cast<uint32_t*>(&fp_multiplier);
  auto current_exponent = (*fp_as_bits >> 23);
  // bring multiplier in [.5,1) range and calculate the shift
  auto bumped_multiplier_as_bits =
      (*fp_as_bits & UINT32_C(0x007fffff)) | UINT32_C(0x3f000000);
  auto* bumped_multiplier = reinterpret_cast<float*>(&bumped_multiplier_as_bits);
  auto shift = 126 - current_exponent;
  // convert to fixed point number
  auto int_multiplier = static_cast<std::int64_t>(std::round(*bumped_multiplier * (1ll << 31)));

  *integer_multiplier = static_cast<int32_t>(int_multiplier);
  *right_shift = shift;
}

void GemmlowpMultiplyu8u8_u8(const uint8_t* lhs_data, const uint8_t* rhs_data, uint8_t* result_data,
                             const int lhs_offset, const int rhs_offset, const int result_offset,
                             int m, int n, int k, int32_t int_multiplier, int32_t right_shift, const int32_t* bias = nullptr);

void GemmlowpMultiplyu8u8_s32(const uint8_t* lhs_data, const uint8_t* rhs_data, int32_t* result_data,
                              const int lhs_offset, const int rhs_offset,
                              int m, int n, int k, int lda, int ldb, int ldc, concurrency::ThreadPool*);

}  // namespace onnxruntime

inline void GemmlowpDebug(const uint8_t* lhs_data, const uint8_t* rhs_data, uint8_t* result_data,
                        const int lhs_offset, const int rhs_offset, const int result_offset,
                        int m, int n, int k, int32_t int_multiplier, int32_t right_shift, const int32_t* bias) {

  printf("lhs matrix\n");
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < k; j++) {
      printf("%d ", (int) lhs_data[i * k + j]);
    }
    printf("\n");
  }

  printf("rhs matrix\n");
  for (int i = 0; i < k; i++) {
    for (int j = 0; j < n; j++) {
      printf("%d ", (int) rhs_data[i * n + j]);
    }
    printf("\n");
  }

  printf("out matrix\n");
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      printf("%d ", (int) result_data[i * n + j]);
    }
    printf("\n");
  }

  printf("m, n, k: %d %d %d\n", m, n, k);
  printf("lhs_offset: %d\n", lhs_offset);
  printf("rhs_offset: %d\n", rhs_offset);
  printf("result_offset: %d\n", result_offset);

  printf("int_multiplier: %d\n", int_multiplier);
  printf("right_shift: %d\n", right_shift);
  if (bias) {
    printf("bias:\n");
    for (int i = 0; i < m; i++) {
      printf("%d ", (int) bias[i]);
    }
  }

}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
