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
// UNSUPPORTED: c++03, c++11, c++14
// UNSUPPORTED: pre-sm-70

// <mutex>

// template <class ...Mutex> class scoped_lock;

// scoped_lock(adopt_lock_t, Mutex&...);

#include<cuda/std/mutex>
#include<cuda/std/cassert>
#include "test_macros.h"

struct TestMutex {
    bool locked = false;
    TestMutex() = default;

    __host__ __device__ void lock() { assert(!locked); locked = true; }
    __host__ __device__ bool try_lock() { if (locked) return false; locked = true; return true; }
    __host__ __device__ void unlock() { assert(locked); locked = false; }

    TestMutex(TestMutex const&) = delete;
    TestMutex& operator=(TestMutex const&) = delete;
};

int main(int, char**)
{
    {
        using LG = cuda::std::scoped_lock<>;
        LG lg(cuda::std::adopt_lock);
    }
    {
        TestMutex m1;
        using LG = cuda::std::scoped_lock<TestMutex>;
        m1.lock();
        {
            LG lg(cuda::std::adopt_lock, m1);
            assert(m1.locked);
        }
        assert(!m1.locked);
    }
    {
        TestMutex m1, m2;
        using LG = cuda::std::scoped_lock<TestMutex, TestMutex>;
        m1.lock(); m2.lock();
        {
            LG lg(cuda::std::adopt_lock, m1, m2);
            assert(m1.locked && m2.locked);
        }
        assert(!m1.locked && !m2.locked);
    }
    {
        TestMutex m1, m2, m3;
        using LG = cuda::std::scoped_lock<TestMutex, TestMutex, TestMutex>;
        m1.lock(); m2.lock(); m3.lock();
        {
            LG lg(cuda::std::adopt_lock, m1, m2, m3);
            assert(m1.locked && m2.locked && m3.locked);
        }
        assert(!m1.locked && !m2.locked && !m3.locked);
    }


  return 0;
}