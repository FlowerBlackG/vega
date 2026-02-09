// SPDX-License-Identifier: MulanPSL-2.0

#pragma once

#include <stdexcept>
#include <string>

namespace vega::io {

class SocketError : public std::runtime_error {
public:
    SocketError(const std::string& message) : std::runtime_error(message) {}
};


class ConnectError : public SocketError {
public:
    ConnectError(const std::string& message) : SocketError(message) {}
};

class BindError : public SocketError {
public:
    BindError(const std::string& message) : SocketError(message) {}
};

class AcceptError : public SocketError {
public:
    AcceptError(const std::string& message) : SocketError(message) {}
};


}  // namespace vega::io
