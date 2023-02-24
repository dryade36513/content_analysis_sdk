// Copyright 2022 The Chromium Authors.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_ANALYSIS_BROWSER_SRC_CLIENT_WIN_H_
#define CONTENT_ANALYSIS_BROWSER_SRC_CLIENT_WIN_H_

#include <string>

#include "client_base.h"

namespace content_analysis {
namespace sdk {

// Client implementaton for Windows.
class ClientWin : public ClientBase {
 public:
   ClientWin(Config config, int* rc);
   ~ClientWin() override;

  // Client:
  int Send(ContentAnalysisRequest request,
           ContentAnalysisResponse* response) override;
  int Acknowledge(const ContentAnalysisAcknowledgement& ack) override;
  int CancelRequests(const ContentAnalysisCancelRequests& cancel) override;

 private:
  static DWORD ConnectToPipe(const std::string& pipename, HANDLE* handle);

  // Reads the next message from the pipe and returns a buffer of chars.
  // Can read any length of message.
  static std::vector<char> ReadNextMessageFromPipe(HANDLE pipe);

  // Writes a string to the pipe. Returns True if successful, else returns False.
  static bool WriteMessageToPipe(HANDLE pipe, const std::string& message);

  // Get a duplicate handle for printed data using DuplicateHandle.
  // See https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-duplicatehandle
  // for details. The target process is the one corresponding to `hPipe_`, and
  // errors or an invalid `hPipe_` will result in this function returning null.
  HANDLE CreateDuplicatePrintDataHandle(HANDLE print_data);

  // Performs a clean shutdown of the client.
  void Shutdown();

  HANDLE hPipe_ = INVALID_HANDLE_VALUE;
};

}  // namespace sdk
}  // namespace content_analysis

#endif  // CONTENT_ANALYSIS_BROWSER_SRC_CLIENT_WIN_H_
