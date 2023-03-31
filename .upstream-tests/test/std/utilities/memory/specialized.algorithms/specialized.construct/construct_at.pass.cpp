//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// SPDX-FileCopyrightText: Copyright (c) 2023 NVIDIA CORPORATION & AFFILIATES
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14

// <memory>

// template <class T, class ...Args>
// constexpr T* construct_at(T* location, Args&& ...args);

#include <cuda/std/ranges> // #include <cuda/std/utility>
#include <cuda/std/cassert>
#include <cuda/std/utility>

#include "test_iterators.h"

struct Foo {
    __host__ __device__ constexpr Foo() { }
    __host__ __device__ constexpr Foo(int a, char b, double c) : a_(a), b_(b), c_(c) { }
    __host__ __device__ constexpr Foo(int a, char b, double c, int* count) : Foo(a, b, c) { *count += 1; }
    __host__ __device__ constexpr bool operator==(Foo const& other) const {
        return a_ == other.a_ && b_ == other.b_ && c_ == other.c_;
    }
#if TEST_STD_VER < 20
    __host__ __device__ constexpr bool operator!=(Foo const& other) const {
        return !(*this == other);
    }
#endif

private:
    int a_{};
    char b_{};
    double c_{};
};

struct Counted {
    int& count_;
    __host__ __device__ constexpr Counted(int& count) : count_(count) { ++count; }
    __host__ __device__ constexpr Counted(Counted const& that) : count_(that.count_) { ++count_; }
    __host__ __device__ TEST_CONSTEXPR_CXX20 ~Counted() { --count_; }
};

__host__ __device__ TEST_CONSTEXPR_CXX20 bool test()
{
    {
        int i = 99;
        int* res = cuda::std::construct_at(&i);
        assert(res == &i);
        assert(*res == 0);
    }

    {
        int i = 0;
        int* res = cuda::std::construct_at(&i, 42);
        assert(res == &i);
        assert(*res == 42);
    }

    {
        Foo foo = {};
        int count = 0;
        Foo* res = cuda::std::construct_at(&foo, 42, 'x', 123.89, &count);
        assert(res == &foo);
        assert(*res == Foo(42, 'x', 123.89));
        assert(count == 1);
    }

#if defined(_LIBCUDACXX_HAS_ALLOCATOR)
    {
        cuda::std::allocator<Counted> a;
        Counted* p = a.allocate(2);
        int count = 0;
        cuda::std::construct_at(p, count);
        assert(count == 1);
        cuda::std::construct_at(p+1, count);
        assert(count == 2);
        (p+1)->~Counted();
        assert(count == 1);
        p->~Counted();
        assert(count == 0);
        a.deallocate(p, 2);
    }

    {
        cuda::std::allocator<Counted const> a;
        Counted const* p = a.allocate(2);
        int count = 0;
        cuda::std::construct_at(p, count);
        assert(count == 1);
        cuda::std::construct_at(p+1, count);
        assert(count == 2);
        (p+1)->~Counted();
        assert(count == 1);
        p->~Counted();
        assert(count == 0);
        a.deallocate(p, 2);
    }
#endif

    return true;
}

template <class ...Args, class = decltype(cuda::std::construct_at(cuda::std::declval<Args>()...))>
__host__ __device__ constexpr bool can_construct_at(Args&&...) { return true; }

template <class ...Args>
__host__ __device__ constexpr bool can_construct_at(...) { return false; }

// Check that SFINAE works.
static_assert( can_construct_at((int*)nullptr, 42));
static_assert( can_construct_at((Foo*)nullptr, 1, '2', 3.0));
static_assert(!can_construct_at((Foo*)nullptr, 1, '2'));
static_assert(!can_construct_at((Foo*)nullptr, 1, '2', 3.0, 4));
static_assert(!can_construct_at(nullptr, 1, '2', 3.0));
static_assert(!can_construct_at((int*)nullptr, 1, '2', 3.0));
static_assert(!can_construct_at(contiguous_iterator<Foo*>(), 1, '2', 3.0));
// Can't construct function pointers.
static_assert(!can_construct_at((int(*)())nullptr));
static_assert(!can_construct_at((int(*)())nullptr, nullptr));

int main(int, char**)
{
    test();
#if TEST_STD_VER > 17
    static_assert(test());
#endif
    return 0;
}
