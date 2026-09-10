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
    static constexpr size_t MOCK_BUFFER_SIZE = 256;
    uint8_t inboundBuffer_[MOCK_BUFFER_SIZE];
    size_t  inboundLen_;
    bool    hasInbound_;

    uint8_t outboundBuffer_[MOCK_BUFFER_SIZE];
    size_t  outboundLen_;
    bool    hasOutbound_;
};

// Lightweight, non-blocking MQTT CodeLine transport adapter
// Bridges between MQTT subscribe callbacks and publish calls without third-party library dependencies.
class MqttCodeLine : public CodeLine {
public:
    typedef bool (*PublishCallback)(const char* topic, const uint8_t* payload, size_t length);

    MqttCodeLine(const char* publishTopic = "", PublishCallback publishCb = nullptr)
        : publishTopic_(publishTopic),
          publishCb_(publishCb),
          inboundLen_(0),
          hasInbound_(false) {
        inboundBuffer_[0] = '\0';
    }

    void setPublishCallback(const char* topic, PublishCallback cb) {
        publishTopic_ = topic;
        publishCb_ = cb;
    }

    // Called from MQTT subscribe message callback (e.g. PubSubClient callback)
    void onControlMessage(const uint8_t* payload, size_t length) {
        if (!payload || length == 0) return;
        size_t copyLen = (length < sizeof(inboundBuffer_) - 1) ? length : sizeof(inboundBuffer_) - 1;
        memcpy(inboundBuffer_, payload, copyLen);
        inboundBuffer_[copyLen] = '\0';
        inboundLen_ = copyLen;
        hasInbound_ = true;
    }

    void onControlMessage(const char* message) {
        if (!message) return;
        onControlMessage(reinterpret_cast<const uint8_t*>(message), strlen(message));
    }

    bool receiveControlPacket(uint8_t* buffer, size_t maxLen, size_t& bytesReceived) override {
        if (!hasInbound_) return false;
        bytesReceived = (inboundLen_ < maxLen) ? inboundLen_ : maxLen;
        memcpy(buffer, inboundBuffer_, bytesReceived);
        hasInbound_ = false;
        return true;
    }

    bool transmitIndicationPacket(const uint8_t* buffer, size_t len) override {
        if (!publishCb_ || !buffer || len == 0) return false;
        return publishCb_(publishTopic_, buffer, len);
    }

private:
    const char*     publishTopic_;
    PublishCallback publishCb_;
    uint8_t         inboundBuffer_[512];
    size_t          inboundLen_;
    bool            hasInbound_;
};

#if defined(ARDUINO)
#include <Stream.h>

// Stream-based CodeLine transport for HardwareSerial, SoftwareSerial, or USBSerial
class StreamCodeLine : public CodeLine {
public:
    StreamCodeLine(Stream& stream, char delimiter = '\n')
        : stream_(&stream), delimiter_(delimiter), rxIndex_(0) {}

    bool receiveControlPacket(uint8_t* buffer, size_t maxLen, size_t& bytesReceived) override {
        if (!stream_) return false;

        while (stream_->available() > 0) {
            int c = stream_->read();
            if (c < 0) break;

            if (static_cast<char>(c) == delimiter_ || static_cast<char>(c) == '\r') {
                if (rxIndex_ > 0) {
                    bytesReceived = (rxIndex_ < maxLen) ? rxIndex_ : maxLen;
                    memcpy(buffer, rxBuffer_, bytesReceived);
                    rxIndex_ = 0;
                    return true;
                }
            } else {
                if (rxIndex_ + 1 < sizeof(rxBuffer_)) {
                    rxBuffer_[rxIndex_++] = static_cast<uint8_t>(c);
                }
            }
        }
        return false;
    }

    bool transmitIndicationPacket(const uint8_t* buffer, size_t len) override {
        if (!stream_ || !buffer || len == 0) return false;
        stream_->write(buffer, len);
        stream_->write(delimiter_);
        return true;
    }

private:
    Stream* stream_;
    char    delimiter_;
    uint8_t rxBuffer_[512];
    size_t  rxIndex_;
};
#endif

} // namespace FieldUnit

#endif // FIELDUNIT_CODELINE_H
