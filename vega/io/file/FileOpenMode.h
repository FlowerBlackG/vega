// SPDX-License-Identifier: MulanPSL-2.0

#pragma once

#include <cstdint>

namespace vega::io {


class FileOpenMode {
protected:
    uint32_t mode_ = 0;
    
    constexpr FileOpenMode(const uint32_t mode) : mode_(mode) {}

public:
    constexpr FileOpenMode() = default;

    constexpr FileOpenMode operator | (const FileOpenMode& other) const {
        return FileOpenMode(this->mode_ | other.mode_);
    }

    constexpr FileOpenMode operator & (const FileOpenMode& other) const {
        return FileOpenMode(this->mode_ & other.mode_);
    }

    constexpr bool operator == (const FileOpenMode& other) const {
        return this->mode_ == other.mode_;
    }

    constexpr bool operator != (const FileOpenMode& other) const {
        return this->mode_ != other.mode_;
    }

    constexpr operator uint32_t() const {
        return this->mode_;
    }

    constexpr operator bool() const {
        return this->mode_ != 0;
    }

public:
    static const FileOpenMode Read;
    static const FileOpenMode Write;
    static const FileOpenMode ReadWrite;
    static const FileOpenMode Truncate;
};


inline constexpr FileOpenMode FileOpenMode::Read { 1 << 0 };
inline constexpr FileOpenMode FileOpenMode::Write { 1 << 1 };
inline constexpr FileOpenMode FileOpenMode::ReadWrite { FileOpenMode::Read | FileOpenMode::Write };

inline constexpr FileOpenMode FileOpenMode::Truncate { 1 << 2 };



}  // namespace vega::io
