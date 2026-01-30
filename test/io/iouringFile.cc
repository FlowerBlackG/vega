// SPDX-License-Identifier: MulanPSL-2.0
// Mostly by Claude 4.5 Opus.

#include <cassert>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>

#include <print>
#include <iostream>

#include <vega/vega.h>


using namespace std;
using namespace vega;

const char* TEST_FILE_PATH = "./__test_iouring_tmp.txt";


static void assertFileType(vega::io::File& file) {
    if (file.backendType() != vega::io::FileBackendType::IoUring) {
        println("[FAIL] File backend type mismatch: expected IoUring, got {}", (std::int64_t) file.backendType());
        std::cout.flush();
        assert(false);
    }
}


// Test 1: Basic write and read back
Promise<bool> testBasicWriteRead() {


    println("=== Test 1: Basic Write and Read ===");
    
    vega::io::File file;

    if (!file.open(TEST_FILE_PATH, vega::io::FileOpenMode::ReadWrite | vega::io::FileOpenMode::Truncate)) {
        println("[FAIL] Failed to open file for write/read");
        co_return false;
    }


    assertFileType(file);


    // Write test data
    const string testData = "Hello, IoUring! This is a test message.\n";
    vector<char> writeBuf(testData.begin(), testData.end());

    size_t written = co_await file.write(writeBuf);
    println("  Written {} bytes", written);
    if (written != writeBuf.size()) {
        println("[FAIL] Write size mismatch: expected {}, got {}", writeBuf.size(), written);
        co_return false;
    }

    // Read back from position 0
    vector<char> readBuf(writeBuf.size(), 0);
    size_t bytesRead = co_await file.read(readBuf, 0);
    println("  Read {} bytes", bytesRead);

    if (bytesRead != writeBuf.size()) {    
        println("[FAIL] Read size mismatch: expected {}, got {}", writeBuf.size(), bytesRead);
        co_return false;
    }

    // Verify content
    if (memcmp(writeBuf.data(), readBuf.data(), writeBuf.size()) != 0) {
        println("[FAIL] Content mismatch!");
        println("  Expected: {}", string(writeBuf.begin(), writeBuf.end()));
        println("  Got: {}", string(readBuf.begin(), readBuf.end()));
        co_return false;
    }

    println("[PASS] Basic write and read test passed");
    co_return true;
}

// Test 2: Multiple sequential writes
Promise<bool> testSequentialWrites() {
    println("\n=== Test 2: Sequential Writes ===");
    
    vega::io::File file;
    if (!file.open(TEST_FILE_PATH, vega::io::FileOpenMode::ReadWrite)) {
        println("[FAIL] Failed to open file");
        co_return false;
    }

    assertFileType(file);

    // Write multiple chunks
    vector<string> chunks = {
        "First chunk of data\n",
        "Second chunk of data\n", 
        "Third chunk of data\n"
    };

    size_t totalWritten = 0;
    for (const auto& chunk : chunks) {
        vector<char> buf(chunk.begin(), chunk.end());
        size_t written = co_await file.write(buf);
        println("  Written chunk: {} bytes", written);
        totalWritten += written;
    }

    println("  Total written: {} bytes", totalWritten);

    // Read all content back from beginning
    vector<char> readBuf(totalWritten, 0);
    size_t bytesRead = co_await file.read(readBuf, 0);
    
    println("  Read back: {} bytes", bytesRead);
    println("  Content:\n{}", string(readBuf.begin(), readBuf.end()));

    if (bytesRead != totalWritten) {
        println("[FAIL] Total read size mismatch");
        co_return false;
    }

    
    // Verify content
    string expectedContent;
    for (const auto& chunk : chunks) {
        expectedContent += chunk;
    }
    
    if (memcmp(expectedContent.data(), readBuf.data(), totalWritten) != 0) {
        println("[FAIL] Content mismatch!");
        println("  Expected: {}", expectedContent);
        println("  Got: {}", string(readBuf.begin(), readBuf.end()));
        co_return false;
    }


    println("[PASS] Sequential writes test passed");
    co_return true;
}

// Test 3: Read at specific offset
Promise<bool> testOffsetRead() {
    println("\n=== Test 3: Read at Specific Offset ===");
    
    vega::io::File file;
    if (!file.open(TEST_FILE_PATH, vega::io::FileOpenMode::ReadWrite)) {
        println("[FAIL] Failed to open file");
        co_return false;
    }
    
    assertFileType(file);

    // Write known pattern
    const string pattern = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    vector<char> writeBuf(pattern.begin(), pattern.end());
    co_await file.write(writeBuf);

    // Read from offset 10 (should get "KLMNOPQRSTUVWXYZ")
    vector<char> readBuf(10, 0);
    size_t bytesRead = co_await file.read(readBuf, 10);
    
    string result(readBuf.begin(), readBuf.begin() + bytesRead);
    println("  Read from offset 10: '{}'", result);

    if (result.substr(0, 10) != "KLMNOPQRST") {
        println("[FAIL] Offset read content mismatch");
        co_return false;
    }

    println("[PASS] Offset read test passed");
    co_return true;
}

// Test 4: Write at specific offset
Promise<bool> testOffsetWrite() {
    println("\n=== Test 4: Write at Specific Offset ===");
    
    vega::io::File file;
    if (!file.open(TEST_FILE_PATH, vega::io::FileOpenMode::ReadWrite)) {
        println("[FAIL] Failed to open file");
        co_return false;
    }
    
    assertFileType(file);

    // Write initial pattern
    const string initial = "AAAAAAAAAA";  // 10 A's
    vector<char> initBuf(initial.begin(), initial.end());
    co_await file.write(initBuf);

    // Overwrite at offset 3 with "BBB"
    const string overwrite = "BBB";
    vector<char> overwriteBuf(overwrite.begin(), overwrite.end());
    co_await file.write(overwriteBuf, 3);

    // Read back and verify
    vector<char> readBuf(10, 0);
    co_await file.read(readBuf, 0);
    
    string result(readBuf.begin(), readBuf.end());
    println("  After overwrite at offset 3: '{}'", result);

    if (result != "AAABBBAAAA") {
        println("[FAIL] Expected 'AAABBBAAAA', got '{}'", result);
        co_return false;
    }

    println("[PASS] Offset write test passed");
    co_return true;
}

// Test 5: Large buffer write/read
Promise<bool> testLargeBuffer() {
    println("\n=== Test 5: Large Buffer Write/Read ===");
    
    vega::io::File file;
    if (!file.open(TEST_FILE_PATH, vega::io::FileOpenMode::ReadWrite)) {
        println("[FAIL] Failed to open file");
        co_return false;
    }
    
    assertFileType(file);

    // Create large buffer (64KB)
    const size_t bufSize = 64 * 1024;
    vector<char> largeBuf(bufSize);
    for (size_t i = 0; i < bufSize; ++i) {
        largeBuf[i] = 'A' + (i % 26);
    }

    size_t written = co_await file.write(largeBuf);
    println("  Written {} bytes", written);

    if (written != bufSize) {
        println("[FAIL] Large write size mismatch");
        co_return false;
    }

    // Read back
    vector<char> readBuf(bufSize, 0);
    size_t bytesRead = co_await file.read(readBuf, 0);
    println("  Read {} bytes", bytesRead);

    if (bytesRead != bufSize) {
        println("[FAIL] Large read size mismatch");
        co_return false;
    }

    // Verify content
    if (memcmp(largeBuf.data(), readBuf.data(), bufSize) != 0) {
        println("[FAIL] Large buffer content mismatch");
        co_return false;
    }

    println("[PASS] Large buffer test passed");
    co_return true;
}

// Cleanup function
void cleanupTestFile() {
    println("\n=== Cleanup ===");
    if (std::remove(TEST_FILE_PATH) == 0) {
        println("  Deleted test file: {}", TEST_FILE_PATH);
    } else {
        println("  Warning: Could not delete test file: {} (errno: {})", 
                TEST_FILE_PATH, strerror(errno));
    }
}


Promise<> blockingMain() {
    println("========================================");
    println("    IoUring File Read/Write Tests");
    println("========================================\n");


    // Run all tests
    assert(co_await testBasicWriteRead());
    assert(co_await testSequentialWrites());
    assert(co_await testOffsetRead());
    assert(co_await testOffsetWrite());
    assert(co_await testLargeBuffer());
}


int main() {
    Scheduler::get().runBlocking(blockingMain);
    cleanupTestFile();
    return 0;
}
