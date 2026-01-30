// SPDX-License-Identifier: MulanPSL-2.0


#include <memory>

#include <vega/io/file/File.h>

#include <vega/Promise.h>
#include <vega/io/file/backend/StreamFile.h>
#include <vega/io/file/backend/IoUringFile.h>


namespace vega::io {

File::File() {

}


File::~File() {
    this->close();
}


bool File::createBackend() {
    if (this->backend_ != nullptr) {
        return true;
    }

#if defined (__linux__)
    {
        this->backend_ = std::make_unique<IoUringFile>();
        return true;
    }
#endif


    this->backend_ = std::make_unique<StreamFile>();
    return true;
}


FileBackendType File::backendType() const {
    if (this->backend_ == nullptr) {
        return FileBackendType::None;
    }
    return this->backend_->type();
}


bool File::open(const std::string& path, FileOpenMode mode) {
    this->close();

    if (!this->createBackend()) {
        return false;
    }

    this->backend_->open(path, mode);

    return this->backend_->isOpen();
}


bool File::isOpen() const {
    return this->backend_ != nullptr && this->backend_->isOpen();
}


void File::close() {
    if (this->backend_ != nullptr) {
        this->backend_->close();
        this->backend_ = nullptr;
    }
}


Promise<std::size_t> File::read(void* buffer, std::size_t size, long offset) {
    if (this->backend_ == nullptr) {
        return Promise<std::size_t>::reject("File not open");
    }
    return this->backend_->read(buffer, size, offset);
}


Promise<std::size_t> File::read(std::vector<char>& buf, long offset) {
    return this->read(buf.data(), buf.size(), offset);
}


Promise<std::size_t> File::write(const void* buffer, std::size_t size, long offset) {
    if (this->backend_ == nullptr) {
        return Promise<std::size_t>::reject("File not open");
    }
    return this->backend_->write(buffer, size, offset);
}


Promise<std::size_t> File::write(const std::vector<char>& buf, long offset) {
    return this->write(buf.data(), buf.size(), offset);
}


File::operator bool() const {
    return this->isOpen();
}


}  // namespace vega::io
