#ifndef FIELDUNIT_CODELINE_H
#define FIELDUNIT_CODELINE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

namespace FieldUnit {

// Abstract non-blocking transport interface for CodeLine communications
// Implemented by CMRInet serial transport, MQTT transport, or Mock transport
class CodeLine {
public:
    virtual ~CodeLine() = default;

    // Non-blocking poll for incoming Control packet from dispatcher
    // Returns true if a complete new packet was received into buffer
    virtual bool receiveControlPacket(uint8_t* buffer, size_t maxLen, size_t& bytesReceived) = 0;

    // Non-blocking transmission of verified Indication packet to dispatcher
    // Returns true if packet was accepted for transmission
    virtual bool transmitIndicationPacket(const uint8_t* buffer, size_t len) = 0;
};

// Mock CodeLine for desktop simulation, automated test vectors, and replay
class MockCodeLine : public CodeLine {
public:
    MockCodeLine() : inboundLen_(0), hasInbound_(false), outboundLen_(0), hasOutbound_(false) {}

    // Test bench helper: inject raw bytes from dispatcher
    void injectControlPacket(const uint8_t* bytes, size_t len) {
        if (len > sizeof(inboundBuffer_)) len = sizeof(inboundBuffer_);
        for (size_t i = 0; i < len; ++i) {
            inboundBuffer_[i] = bytes[i];
        }
        inboundLen_ = len;
        hasInbound_ = true;
    }

    // Test bench helper: inject symbolic text snapshot from dispatcher
    void injectControlText(const char* text) {
        if (!text) return;
        injectControlPacket(reinterpret_cast<const uint8_t*>(text), strlen(text));
    }

    // Test bench helper: inspect raw indication bytes emitted by sketch
    bool hasOutboundPacket() const { return hasOutbound_; }
    size_t outboundLength() const { return outboundLen_; }
    const uint8_t* outboundPacket() const { return outboundBuffer_; }
    const char* outboundText() const { return reinterpret_cast<const char*>(outboundBuffer_); }
    void clearOutbound() { hasOutbound_ = false; outboundLen_ = 0; outboundBuffer_[0] = '\0'; }

    // CodeLine interface implementation
    bool receiveControlPacket(uint8_t* buffer, size_t maxLen, size_t& bytesReceived) override {
        if (!hasInbound_) {
            return false;
        }
        bytesReceived = (inboundLen_ < maxLen) ? inboundLen_ : maxLen;
        for (size_t i = 0; i < bytesReceived; ++i) {
            buffer[i] = inboundBuffer_[i];
        }
        hasInbound_ = false; // Consumed
        return true;
    }

    bool transmitIndicationPacket(const uint8_t* buffer, size_t len) override {
        outboundLen_ = (len < sizeof(outboundBuffer_) - 1) ? len : sizeof(outboundBuffer_) - 1;
        for (size_t i = 0; i < outboundLen_; ++i) {
            outboundBuffer_[i] = buffer[i];
        }
        outboundBuffer_[outboundLen_] = '\0'; // Safe null termination for text inspection
        hasOutbound_ = true;
        return true;
    }

private:
    uint8_t inboundBuffer_[256];
    size_t  inboundLen_;
    bool    hasInbound_;

    uint8_t outboundBuffer_[256];
    size_t  outboundLen_;
    bool    hasOutbound_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_CODELINE_H
