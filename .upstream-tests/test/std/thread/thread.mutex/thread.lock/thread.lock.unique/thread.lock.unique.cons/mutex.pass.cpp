//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// SPDX-FileCopyrightText: Copyright (c) 2023 NVIDIA CORPORATION & AFFILIATES
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: no-threads
// UNSUPPORTED: pre-sm-70

// FLAKY_TEST.

// <mutex>

// template <class Mutex> class unique_lock;

// explicit unique_lock(mutex_type& m);

// template<class _Mutex> unique_lock(unique_lock<_Mutex>)
//     -> unique_lock<_Mutex>;  // C++17

#include<cuda/std/mutex>
#include<cuda/std/cassert>
#include<cuda/std/chrono>

#include "test_macros.h"

#include "cuda_space_selector.h"
#include "heterogeneous_thread_handler.h"

typedef cuda::std::chrono::system_clock Clock;
typedef Clock::time_point time_point;
typedef Clock::duration duration;
typedef cuda::std::chrono::milliseconds ms;
typedef cuda::std::chrono::nanoseconds ns;

template<class Mutex>
__host__ __device__ void f(Mutex* m)
{
    time_point t0 = Clock::now();
    time_point t1;
    {
        cuda::std::unique_lock<Mutex> ul(*m);
        t1 = Clock::now();
    }
    assert(t1 - t0 >= ms(250));
}

template<class Mutex>
__host__ __device__
void check_ctad(Mutex* m)
{
#if TEST_STD_VER >= 17
    cuda::std::unique_lock ul(*m);
    static_assert((cuda::std::is_same<decltype(ul), cuda::std::unique_lock<Mutex>>::value), "" );
    unused(ul);
#endif
}

template<class Mutex,
    template<typename, typename> typename Selector,
    typename Initializer = constructor_initializer>
__host__ __device__ void test() {
    Selector<Mutex, Initializer> sel;
    SHARED Mutex* m;
    m = sel.construct();

    heterogeneous_thread_handler handler;
    handler.run_on_first_thread(&Mutex::lock, m);
    handler.syncthreads();

    handler.run_on_second_thread(f<Mutex>, m);
    handler.sleep_first_thread(ms(250));
    handler.run_on_first_thread(&Mutex::unlock, m);
    handler.join_test_thread();

    handler.syncthreads();
    handler.run_on_first_thread(check_ctad<Mutex>, m);
}

int main(int, char**)
{
#ifndef __CUDA_ARCH__
    cuda_thread_count = 2;

    test<cuda::std::mutex, local_memory_selector>();
#else
    test<cuda::std::mutex, shared_memory_selector>();
    test<cuda::std::mutex, global_memory_selector>();
#endif
    return 0;
}