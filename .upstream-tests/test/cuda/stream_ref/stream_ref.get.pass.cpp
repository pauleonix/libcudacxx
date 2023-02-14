//===----------------------------------------------------------------------===//
//
// Part of libcu++, the C++ Standard Library for your entire system,
// under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// SPDX-FileCopyrightText: Copyright (c) 2022 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
//===----------------------------------------------------------------------===//

#include <cuda/stream_ref>
#include <cuda/std/cassert>

int main(int argc, char** argv) {
#ifndef __CUDA_ARCH__
  cudaStream_t stream = reinterpret_cast<cudaStream_t>(42);
  cuda::stream_ref ref{stream};
  assert(ref.get() == stream);
#endif
  return 0;
}