// Copyright 2022 The Chromium Authors.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(WIN32)
#include <pthread.h>
#endif

#include <fstream>
#include <iostream>
#include <queue>
#include <string>

#include "content_analysis/sdk/analysis_agent.h"
#include "demo/handler.h"
#include "demo/request_queue.h"

// An AgentEventHandler that dumps requests information to stdout and blocks
// any requests that have the keyword "block" in their data
class QueuingHandler : public Handler {
 public:
  QueuingHandler() {
    StartBackgroundThread();
  }

  ~QueuingHandler() override {
    // Abort background process and wait for it to finish.
    request_queue_.abort();
    WaitForBackgroundThread();
  }

 private:
  void OnAnalysisRequested(std::unique_ptr<Event> event) override {
    request_queue_.push(std::move(event));
  }

#if defined(WIN32)
  static unsigned __stdcall ProcessRequests(void* qh) {
#else
  static void* ProcessRequests(void* qh) {
#endif
    QueuingHandler* handler = reinterpret_cast<QueuingHandler*>(qh);

    while (true) {
      auto event = handler->request_queue_.pop();
      if (!event)
        break;

      handler->AnalyzeContent(std::move(event));
    }

    return 0;
  }

  // A list of outstanding content analysis requests.
  RequestQueue request_queue_;

  void StartBackgroundThread() {
#if defined(WIN32)
    // Start a background thread to process the queue.  This demo starts one
    // thread but any number would work.
    unsigned tid;
    thread_ = reinterpret_cast<HANDLE>(_beginthreadex(
        nullptr, 0, ProcessRequests, this, 0, &tid));
#else
    // Start a background thread to process the queue.  This demo starts one
    // thread but any number would work.
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&tid_, &attr, ProcessRequests, this);
    pthread_attr_destroy(&attr);
#endif
  }

  void WaitForBackgroundThread() {
#if defined(WIN32)
    WaitForSingleObject(thread_, INFINITE);
    CloseHandle(thread_);
#else
    void* res;
    pthread_join(tid_, &res);
#endif
  }

  // Thread id of backgrond thread.
#if defined(WIN32)
  HANDLE thread_;
#else
  pthread_t tid_;
#endif
};

int main(int argc, char* argv[]) {
  // Each agent uses a unique URI to identify itself with Google Chrome.
  auto agent = content_analysis::sdk::Agent::Create({"content_analysis_sdk"},
      std::make_unique<QueuingHandler>());
  if (!agent) {
    std::cout << "[Demo] Error starting agent" << std::endl;
    return 1;
  };

  // Blocks, sending events to the handler until agent->Stop() is called.
  agent->HandleEvents();

  return 0;
};
