// SPDX-License-Identifier: MIT
// Copyright (c) 2023 University of Washington

#include "DrvAPIThread.hpp"
#include "DrvAPIMain.hpp"
#include "DrvAPIGlobal.hpp"
#include "DrvAPIAllocator.hpp"
#include <iostream>

using namespace DrvAPI;

DrvAPIThread::DrvAPIThread()
    : thread_context_(nullptr)
    , state_(new DrvAPIThreadIdle)
    , main_(nullptr)
    , argc_(0)
    , argv_(nullptr) {
}

void DrvAPIThread::start() {
    auto coro_function = [this](coro_t::push_type &sink) {
        this->main_context_ = &sink;
        this->yield();
        while (true) {
            if (this->main_) {
                this->main_(argc_, argv_);
                this->main_ = nullptr;
                this->state_ = std::make_shared<DrvAPITerminate>();
            }
            this->yield();
        }
    };
    if (stack_in_modeled_memory_) {
        modeled_memory_stack_allocator allocator;
        thread_context_ = std::make_unique<coro_t::pull_type>(allocator, coro_function);
    } else {
        thread_context_ = std::make_unique<coro_t::pull_type>(coro_function);
    }
}

/* should only be called from the thread context */
void DrvAPIThread::yield(DrvAPIThreadState *state) {
    state_ = std::make_shared<DrvAPIThreadState>(*state);
    yield();
}

/* should only be called from the thread context */
void DrvAPIThread::yield() {
    (*main_context_)();
}

/* should only be called from the main context */
void DrvAPIThread::resume() {
    (*thread_context_)();
}

thread_local DrvAPIThread *DrvAPIThread::g_current_thread = nullptr;


DrvAPIThread *DrvAPIGetCurrentContext() {
    return DrvAPIThread::g_current_thread;
}

void DrvAPISetCurrentContext(DrvAPI::DrvAPIThread *thread) {
    DrvAPIThread::g_current_thread = thread;
}
