// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/task/sequence_manager/thread_controller_power_monitor.h"

#include "base/feature_list.h"
#include "base/power_monitor/power_monitor.h"
#include "base/trace_event/trace_event.h"

namespace base {
namespace sequence_manager {
namespace internal {

namespace {

// Activate the power management events that affect task scheduling.
const Feature kUsePowerMonitorWithThreadController{
    "UsePowerMonitorWithThreadController", FEATURE_DISABLED_BY_DEFAULT};

// TODO(1074332): Remove this when the experiment becomes the default.
bool g_use_thread_controller_power_monitor_ = false;

}  // namespace

ThreadControllerPowerMonitor::ThreadControllerPowerMonitor() = default;

ThreadControllerPowerMonitor::~ThreadControllerPowerMonitor() {
  PowerMonitor::RemoveObserver(this);
}

void ThreadControllerPowerMonitor::BindToCurrentThread() {
#if DCHECK_IS_ON()
  DCHECK(!is_observer_registered_);
  is_observer_registered_ = true;
#endif

  // Register the observer to deliver notifications on the current thread.
  PowerMonitor::AddObserver(this);
}

bool ThreadControllerPowerMonitor::IsProcessInPowerSuspendState() {
  return is_power_suspended_;
}

// static
void ThreadControllerPowerMonitor::InitializeOnMainThread() {
  DCHECK(!g_use_thread_controller_power_monitor_);
  g_use_thread_controller_power_monitor_ =
      FeatureList::IsEnabled(kUsePowerMonitorWithThreadController);
}

// static
void ThreadControllerPowerMonitor::OverrideUsePowerMonitorForTesting(
    bool use_power_monitor) {
  g_use_thread_controller_power_monitor_ = use_power_monitor;
}

// static
void ThreadControllerPowerMonitor::ResetForTesting() {
  g_use_thread_controller_power_monitor_ = false;
}

void ThreadControllerPowerMonitor::OnSuspend() {
  if (!g_use_thread_controller_power_monitor_)
    return;
  DCHECK(!is_power_suspended_);

  TRACE_EVENT_NESTABLE_ASYNC_BEGIN0("base", "ThreadController::Suspended",
                                    this);
  is_power_suspended_ = true;
}

void ThreadControllerPowerMonitor::OnResume() {
  if (!g_use_thread_controller_power_monitor_)
    return;

  // It is possible a suspend was already happening before the observer was
  // added to the power monitor. Ignoring the resume notification in that case.
  if (is_power_suspended_) {
    TRACE_EVENT_NESTABLE_ASYNC_END0("base", "ThreadController::Suspended",
                                    this);
    is_power_suspended_ = false;
  }
}

}  // namespace internal
}  // namespace sequence_manager
}  // namespace base