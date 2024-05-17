#pragma once
#include <Windows.h>
#include <memory>
#include <string>
#include <cstdint>
#include <sstream>
#include <vector>
#include <functional>

namespace RX784 {
    enum class Status : uint8_t {
        kSuccess,
        kSerialError,
        kReadFlashError,
        kWriteFlashError,
        kInvalidSize,
        kInvalidCommandPacket,
        kInvalidResponsePacket
    };

    enum class VirtualKeyCode : uint8_t {
        kInvalid = 0,

        kBackspace = 0x08,
        kTab,

        kEnter = 0x0D,

        kShift = 0x10,
        kControl,
        kAlt,
        kPause,
        kCapsLock,

        kEscape = 0x1B,

        kSpace = 0x20,
        kPageUp,
        kPageDown,
        kEnd,
        kHome,

        kArrowLeft,
        kArrowUp,
        kArrowRight,
        kArrowDown,

        kPrintScreen = 0x2C,
        kInsert,
        kDelete,

        kDigit0 = '0', kDigit1,
        kDigit2, kDigit3, kDigit4, kDigit5,
        kDigit6, kDigit7, kDigit8, kDigit9,

        kKeyA = 'A', kKeyB,
        kKeyC, kKeyD, kKeyE, kKeyF, kKeyG, kKeyH,
        kKeyI, kKeyJ, kKeyK, kKeyL, kKeyM, kKeyN,
        kKeyO, kKeyP, kKeyQ, kKeyR, kKeyS, kKeyT,
        kKeyU, kKeyV, kKeyW, kKeyX, kKeyY, kKeyZ,

        kOSLeft,
        kOSRight,
        kContextMenu,

        kNumpad0 = 0x60, kNumpad1,
        kNumpad2, kNumpad3, kNumpad4, kNumpad5,
        kNumpad6, kNumpad7, kNumpad8, kNumpad9,

        kNumpadMultiply,  /* * */
        kNumpadAdd,       /* + */
        kNumpadEnter,
        kNumpadSubtract,  /* - */
        kNumpadDecimal,   /* . */
        kNumpadDivide,    /* / */

        kF1, kF2, kF3, kF4, kF5, kF6,
        kF7, kF8, kF9, kF10, kF11, kF12,

        kNumLock = 0x90,
        kScrollLock,

        kShiftLeft = 0xA0,
        kShiftRight,
        kControlLeft,
        kControlRight,
        kAltLeft,
        kAltRight,

        kSemicolon = 0xBA,    /* ; */
        kEqual,               /* = */
        kComma,               /* , */
        kMinus,               /* - */
        kPeriod,              /* . */
        kSlash,               /* / */
        kBackquote,           /* ` */

        kBracketLeft = 0xDB,  /* [ */
        kBackslash,           /* \ */
        kBracketRight,        /* ] */
        kQuote                /* ' */
    };

    enum class Button : uint8_t {
        kLeft, kRight, kMiddle, kButton4, kButton5
    };

    struct KeyboardLEDsState {
        uint8_t numLock    : 1;
        uint8_t capsLock   : 1;
        uint8_t scrollLock : 1;
        uint8_t compose    : 1;
        uint8_t kana       : 1;
    };

    struct KeyboardState {
        struct ModifierKeys {
            uint8_t controlLeft  : 1;
            uint8_t shiftLeft    : 1;
            uint8_t altLeft      : 1;
            uint8_t OSLeft       : 1;
            uint8_t controlRight : 1;
            uint8_t shiftRight   : 1;
            uint8_t altRight     : 1;
            uint8_t OSRight      : 1;
        };
        ModifierKeys modifierKeys;
        VirtualKeyCode regularKeys[7];
    };

    struct KeyboardStateMask {
        struct ModifierKeys {
            uint8_t controlLeft  : 1;
            uint8_t shiftLeft    : 1;
            uint8_t altLeft      : 1;
            uint8_t OSLeft       : 1;
            uint8_t controlRight : 1;
            uint8_t shiftRight   : 1;
            uint8_t altRight     : 1;
            uint8_t OSRight      : 1;
        };
        ModifierKeys modifierKeys;
        bool regularKeys[7];
    };

#pragma pack(push, 1)
    struct ButtonsState {
        uint8_t left   : 1;
        uint8_t right  : 1;
        uint8_t middle : 1;
        uint8_t button4: 1;
        uint8_t button5: 1;
    };

    struct MouseState {
        struct Buttons {
            uint8_t left   : 1;
            uint8_t right  : 1;
            uint8_t middle : 1;
            uint8_t button4: 1;
            uint8_t button5: 1;
        };
        struct Axes {
            int16_t x;
            int16_t y;
            int16_t w;
        };
        Buttons buttons;
        Axes axes;
    };

    union MouseStateMask {
        struct Buttons {
            uint8_t left   : 1;
            uint8_t right  : 1;
            uint8_t middle : 1;
        };
        struct Axes {
            uint8_t _ : 3;
            uint8_t x : 1;
            uint8_t y : 1;
            uint8_t w : 1;
        };
        Buttons buttons;
        Axes axes;
    };
#pragma pack(pop)

    struct LinearPath {
        double a1;
        double b1;
        double a2;
        double b2;
        double p1x;
        double p1y;
        double p2x;
        double p2y;
    };

    class Device {
    public:
        static constexpr size_t maxManufacturerStringSize() { return 30; }
        static constexpr size_t maxProductStringSize()      { return 30; }

        Device() : hSerial(INVALID_HANDLE_VALUE) {}

        Status open(const std::string& port) {
            return serialOpen(port.c_str(), 250000) ? Status::kSuccess : Status::kSerialError;
        }

        Status close() {
            return serialClose() ? Status::kSuccess : Status::kSerialError;
        }

        Status reboot() {
            Status status, cmdStatus{};
            status = sendPacket(Command::kReboot);
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kReboot, &cmdStatus, sizeof(cmdStatus));
            if (status != Status::kSuccess) return status;

            return cmdStatus;
        }

        Status keyDown(VirtualKeyCode virtualKeyCode) {
            Status status, cmdStatus{};
            HIDKeyCode hidKeyCode = virtualKeyCodeToHIDKeyCode(virtualKeyCode);

            status = sendPacket(Command::kKeyDown, &hidKeyCode, sizeof(hidKeyCode));
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kKeyDown, &cmdStatus, sizeof(cmdStatus));
            if (status != Status::kSuccess) return status;

            return cmdStatus;
        }

        Status keyUp(VirtualKeyCode virtualKeyCode) {
            Status status, cmdStatus{};
            HIDKeyCode hidKeyCode = virtualKeyCodeToHIDKeyCode(virtualKeyCode);

            status = sendPacket(Command::kKeyUp, &hidKeyCode, sizeof(hidKeyCode));
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kKeyUp, &cmdStatus, sizeof(cmdStatus));
            if (status != Status::kSuccess) return status;

            return cmdStatus;
        }

        Status releaseAllKeys() {
            Status status, cmdStatus{};

            status = sendPacket(Command::kReleaseAllKeys);
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kReleaseAllKeys, &cmdStatus, sizeof(cmdStatus));
            if (status != Status::kSuccess) return status;

            return cmdStatus;
        }

        Status getKeyState(VirtualKeyCode key, bool& isDown) {
            HIDKeyCode hidKeyCode = virtualKeyCodeToHIDKeyCode(key);
            Status status = sendPacket(Command::kGetKeyState, &hidKeyCode, sizeof(hidKeyCode));
            if (status != Status::kSuccess) return status;

            return recvPacket(Command::kGetKeyState, &isDown, sizeof(isDown));
        }

        Status getKeyboardLEDsState(KeyboardLEDsState& keyboardLEDsState) {
            Status status;

            status = sendPacket(Command::kGetKeyboardLEDsState);
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kGetKeyboardLEDsState, &keyboardLEDsState, sizeof(keyboardLEDsState));
            if (status != Status::kSuccess) return status;

            return Status::kSuccess;
        }

        Status getKeyboardState(KeyboardState& keyboardState) {
            Status status;

#pragma pack(push, 1)
            struct {
                KeyboardState::ModifierKeys modifierKeys;
                HIDKeyCode regularKeys[sizeof(keyboardState.regularKeys)];
            } state{};
#pragma pack(pop)

            status = sendPacket(Command::kGetKeyboardState);
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kGetKeyboardState, &state, sizeof(state));
            if (status != Status::kSuccess) return status;

            keyboardState.modifierKeys = state.modifierKeys;
            for (size_t i = 0; i < sizeof(keyboardState.regularKeys); ++i) {
                keyboardState.regularKeys[i] = HIDKeyCodeToVirtualKeyCode(state.regularKeys[i]);
            }

            return Status::kSuccess;
        }

        Status sendKeyboardState(const KeyboardState& keyboardState, const KeyboardStateMask& keyboardStateMask) {
            Status status, cmdStatus{};
            uint8_t regularKeysMask = 0;
            for (size_t i = 0; i < sizeof(keyboardStateMask.regularKeys); ++i) {
                regularKeysMask |= keyboardStateMask.regularKeys[i] << i;
            }

#pragma pack(push, 1)
            struct {
                KeyboardStateMask::ModifierKeys modifierKeysMask;
                uint8_t regularKeysMask;
                KeyboardState::ModifierKeys modifierKeys;
                HIDKeyCode regularKeys[sizeof(keyboardState.regularKeys)];
            } state = { keyboardStateMask.modifierKeys,
                        regularKeysMask,
                        keyboardState.modifierKeys };
#pragma pack(pop)

            for (size_t i = 0; i < sizeof(state.regularKeys); ++i) {
                state.regularKeys[i] = virtualKeyCodeToHIDKeyCode(keyboardState.regularKeys[i]);
            }

            status = sendPacket(Command::kSendKeyboardState, &state, sizeof(state));
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kSendKeyboardState, &cmdStatus, sizeof(cmdStatus));
            if (status != Status::kSuccess) return status;

            return cmdStatus;
        }

        Status buttonDown(Button button) {
            Status status, cmdStatus{};

            status = sendPacket(Command::kButtonDown, &button, sizeof(button));
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kButtonDown, &cmdStatus, sizeof(cmdStatus));
            if (status != Status::kSuccess) return status;

            return cmdStatus;
        }

        Status buttonUp(Button button) {
            Status status, cmdStatus{};

            status = sendPacket(Command::kButtonUp, &button, sizeof(button));
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kButtonUp, &cmdStatus, sizeof(cmdStatus));
            if (status != Status::kSuccess) return status;

            return cmdStatus;
        }

        Status releaseAllButtons() {
            Status status, cmdStatus{};

            status = sendPacket(Command::kReleaseAllButtons);
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kReleaseAllButtons, &cmdStatus, sizeof(cmdStatus));
            if (status != Status::kSuccess) return status;

            return cmdStatus;
        }

        Status getButtonsState(ButtonsState& buttonsState) {
            Status status;

            status = sendPacket(Command::kGetButtonsState);
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kGetButtonsState, &buttonsState, sizeof(buttonsState));
            if (status != Status::kSuccess) return status;

            return Status::kSuccess;
        }

        Status moveRel(int16_t x, int16_t y) {
            Status status, cmdStatus{};
            int16_t pos[2] = {x, y};

            status = sendPacket(Command::kMoveRel, pos, sizeof(pos));
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kMoveRel, &cmdStatus, sizeof(cmdStatus));
            if (status != Status::kSuccess) return status;

            return cmdStatus;
        }

        Status movePathRel(int16_t x, int16_t y, uint32_t duration, uint32_t pollingRate, bool isIgnoreErrors,
                           const LinearPath& path,
                           std::function<void()> callback = []{});

        Status movePathRel(int16_t x, int16_t y, uint32_t duration, uint32_t pollingRate,
                           const LinearPath& path,
                           std::function<void()> callback = []{}) {
            return movePathRel(x, y, duration, pollingRate, false, path, callback);
        }

        Status movePathRel(int16_t x, int16_t y, uint32_t duration, bool isIgnoreErrors,
                           const LinearPath& path,
                           std::function<void()> callback = []{}) {
            return movePathRel(x, y, duration, 250, isIgnoreErrors, path, callback);
        }

        Status movePathRel(int16_t x, int16_t y, uint32_t duration,
                           const LinearPath& path,
                           std::function<void()> callback = []{}) {
            return movePathRel(x, y, duration, 250, false, path, callback);
        }

        Status scrollRel(int16_t w) {
            Status status, cmdStatus{};

            status = sendPacket(Command::kScrollRel, &w, sizeof(w));
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kScrollRel, &cmdStatus, sizeof(cmdStatus));
            if (status != Status::kSuccess) return status;

            return cmdStatus;
        }

        Status getRelMouseState(MouseState& mouseState) {
            Status status;

            status = sendPacket(Command::kGetRelMouseState);
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kGetRelMouseState, &mouseState.buttons, sizeof(mouseState.buttons));
            if (status != Status::kSuccess) return status;

            mouseState.axes = { 0, 0, 0 };

            return Status::kSuccess;
        }

        Status sendRelMouseState(const MouseState& mouseState, MouseStateMask mouseStateMask) {
            Status status, cmdStatus{};

#pragma pack(push, 1)
            struct { MouseStateMask mask; MouseState mouseState; }
                state = { mouseStateMask, mouseState };
#pragma pack(pop)

            status = sendPacket(Command::kSendRelMouseState, &state, sizeof(state));
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kSendRelMouseState, &cmdStatus, sizeof(cmdStatus));
            if (status != Status::kSuccess) return status;

            return cmdStatus;
        }

        Status initAbsSystem(int16_t screenWidth, int16_t screenHeight) {
            Status status, cmdStatus{};
            int16_t screenResolution[2] = { screenWidth, screenHeight };

            status = sendPacket(Command::kInitAbsSystem, screenResolution, sizeof(screenResolution));
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kInitAbsSystem, &cmdStatus, sizeof(cmdStatus));
            if (status != Status::kSuccess) return status;

            return cmdStatus;
        }

        Status moveAbs(int16_t x, int16_t y) {
            Status status, cmdStatus{};
            int16_t pos[2] = { x, y };

            status = sendPacket(Command::kMoveAbs, pos, sizeof(pos));
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kMoveAbs, &cmdStatus, sizeof(cmdStatus));
            if (status != Status::kSuccess) return status;

            return cmdStatus;
        }

        Status movePathAbs(int16_t x, int16_t y, uint32_t duration, uint32_t pollingRate, bool isIgnoreErrors,
                           const LinearPath& path,
                           std::function<void()> callback = []{});

        Status movePathAbs(int16_t x, int16_t y, uint32_t duration, uint32_t pollingRate,
                           const LinearPath& path,
                           std::function<void()> callback = []{}) {
            return movePathAbs(x, y, duration, pollingRate, false, path, callback);
        }

        Status movePathAbs(int16_t x, int16_t y, uint32_t duration, bool isIgnoreErrors,
                           const LinearPath& path,
                           std::function<void()> callback = []{}) {
            return movePathAbs(x, y, duration, 250, isIgnoreErrors, path, callback);
        }

        Status movePathAbs(int16_t x, int16_t y, uint32_t duration,
                           const LinearPath& path,
                           std::function<void()> callback = []{}) {
            return movePathAbs(x, y, duration, 250, false, path, callback);
        }

        Status scrollAbs(int16_t w) {
            Status status, cmdStatus{};

            status = sendPacket(Command::kScrollAbs, &w, sizeof(w));
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kScrollAbs, &cmdStatus, sizeof(cmdStatus));
            if (status != Status::kSuccess) return status;

            return cmdStatus;
        }

        Status getPos(int16_t& x, int16_t& y) {
            Status status;
            int16_t pos[2]{};

            status = sendPacket(Command::kGetPos);
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kGetPos, pos, sizeof(pos));
            if (status != Status::kSuccess) return status;

            x = pos[0];
            y = pos[1];
            return Status::kSuccess;
        }

        Status setPos(int16_t x, int16_t y) {
            Status status, cmdStatus{};
            int16_t pos[2] = { x, y };

            status = sendPacket(Command::kSetPos, pos, sizeof(pos));
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kSetPos, &cmdStatus, sizeof(cmdStatus));
            if (status != Status::kSuccess) return status;

            return cmdStatus;
        }

        Status getWheelAxis(int16_t& w) {
            Status status;

            status = sendPacket(Command::kGetWheelAxis);
            if (status != Status::kSuccess) return status;

            return recvPacket(Command::kGetWheelAxis, &w, sizeof(w));
        }

        Status setWheelAxis(int16_t w) {
            Status status, cmdStatus{};

            status = sendPacket(Command::kSetWheelAxis, &w, sizeof(w));
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kSetWheelAxis, &cmdStatus, sizeof(cmdStatus));
            if (status != Status::kSuccess) return status;

            return cmdStatus;
        }

        Status getAxes(int16_t& x, int16_t& y, int16_t& w) {
            Status status;
            int16_t axes[3]{};

            status = sendPacket(Command::kGetAxes);
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kGetAxes, axes, sizeof(axes));
            if (status != Status::kSuccess) return status;

            x = axes[0];
            y = axes[1];
            w = axes[2];
            return Status::kSuccess;
        }

        Status setAxes(int16_t x, int16_t y, int16_t w) {
            Status status, cmdStatus{};
            int16_t axes[3] = { x, y, w };

            status = sendPacket(Command::kSetAxes, axes, sizeof(axes));
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kSetAxes, &cmdStatus, sizeof(cmdStatus));
            if (status != Status::kSuccess) return status;

            return cmdStatus;
        }

        Status getAbsMouseState(MouseState& mouseState) {
            Status status;

            status = sendPacket(Command::kGetAbsMouseState);
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kGetAbsMouseState, &mouseState, sizeof(mouseState));
            if (status != Status::kSuccess) return status;

            return Status::kSuccess;
        }

        Status sendAbsMouseState(const MouseState& mouseState, MouseStateMask mouseStateMask) {
            Status status, cmdStatus{};

#pragma pack(push, 1)
            struct { MouseStateMask mouseStateMask; MouseState mouseState; }
                state = { mouseStateMask, mouseState };
#pragma pack(pop)

            status = sendPacket(Command::kSendAbsMouseState, &state, sizeof(state));
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kSendAbsMouseState, &cmdStatus, sizeof(cmdStatus));
            if (status != Status::kSuccess) return status;

            return cmdStatus;
        }

        Status configHIDVendorID(uint16_t vendorID) {
            Status status, cmdStatus{};

            status = sendPacket(Command::kConfigVendorID, &vendorID, sizeof(vendorID));
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kConfigVendorID, &cmdStatus, sizeof(cmdStatus));
            if (status != Status::kSuccess) return status;

            return cmdStatus;
        }

        Status configHIDProductID(uint16_t productID) {
            Status status, cmdStatus{};

            status = sendPacket(Command::kConfigProductID, &productID, sizeof(productID));
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kConfigProductID, &cmdStatus, sizeof(cmdStatus));
            if (status != Status::kSuccess) return status;

            return cmdStatus;
        }

        Status configHIDVersionNumber(uint16_t versionNumber) {
            Status status, cmdStatus{};

            status = sendPacket(Command::kConfigVersionNumber, &versionNumber, sizeof(versionNumber));
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kConfigVersionNumber, &cmdStatus, sizeof(cmdStatus));
            if (status != Status::kSuccess) return status;

            return cmdStatus;
        }

        Status configHIDManufacturerString(const std::string& manufacturerString) {
            Status status, cmdStatus{};
            std::wstring data = strToWstr(manufacturerString);
            if (data.size() * sizeof(wchar_t) > UINT8_MAX) return Status::kInvalidSize;

            status = sendPacket(Command::kConfigManufacturerString,
                                data.c_str(),
                                static_cast<uint8_t>(data.size() * sizeof(wchar_t)));
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kConfigManufacturerString, &cmdStatus, sizeof(cmdStatus));
            if (status != Status::kSuccess) return status;

            return cmdStatus;
        }

        Status configHIDProductString(const std::string& productString) {
            Status status, cmdStatus{};
            std::wstring data = strToWstr(productString);
            if (data.size() * sizeof(wchar_t) > UINT8_MAX) return Status::kInvalidSize;

            status = sendPacket(Command::kConfigProductString,
                                data.c_str(),
                                static_cast<uint8_t>(data.size() * sizeof(wchar_t)));
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kConfigProductString, &cmdStatus, sizeof(cmdStatus));
            if (status != Status::kSuccess) return status;

            return cmdStatus;
        }

        Status getHIDVendorID(uint16_t& vendorID) {
            Status status;
            uint8_t dataSize;

#pragma pack(push, 1)
            struct { Status status; uint16_t vendorID; } data{};
#pragma pack(pop)

            status = sendPacket(Command::kGetVendorID);
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kGetVendorID, &data, sizeof(data), &dataSize);
            if (status != Status::kSuccess) return status;

            if (dataSize == 1 && data.status != Status::kSuccess) return data.status;
            if (dataSize != 3 || data.status != Status::kSuccess) return Status::kInvalidResponsePacket;

            vendorID = data.vendorID;
            return Status::kSuccess;
        }

        Status getHIDProductID(uint16_t& productID) {
            Status status;
            uint8_t dataSize;

#pragma pack(push, 1)
            struct { Status status; uint16_t productID; } data{};
#pragma pack(pop)

            status = sendPacket(Command::kGetProductID);
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kGetProductID, &data, sizeof(data), &dataSize);
            if (status != Status::kSuccess) return status;

            if (dataSize == 1 && data.status != Status::kSuccess) return data.status;
            if (dataSize != 3 || data.status != Status::kSuccess) return Status::kInvalidResponsePacket;

            productID = data.productID;
            return Status::kSuccess;
        }

        Status getHIDVersionNumber(uint16_t& versionNumber) {
            Status status;
            uint8_t dataSize;

#pragma pack(push, 1)
            struct { Status status; uint16_t versionNumber; } data{};
#pragma pack(pop)

            status = sendPacket(Command::kGetVersionNumber);
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kGetVersionNumber, &data, sizeof(data), &dataSize);
            if (status != Status::kSuccess) return status;

            if (dataSize == 1 && data.status != Status::kSuccess) return data.status;
            if (dataSize != 3 || data.status != Status::kSuccess) return Status::kInvalidResponsePacket;

            versionNumber = data.versionNumber;
            return Status::kSuccess;
        }

        Status getHIDManufacturerString(std::string& manufacturerString) {
            Status status;
            uint8_t dataSize;

#pragma pack(push, 1)
            struct { Status status; wchar_t manufacturerString[maxManufacturerStringSize()+1]; } data{};
#pragma pack(pop)

            status = sendPacket(Command::kGetManufacturerString);
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kGetManufacturerString, &data, sizeof(data), &dataSize);
            if (status != Status::kSuccess) return status;

            if (dataSize == 1) return data.status;
            if (data.status != Status::kSuccess) return Status::kInvalidResponsePacket;

            manufacturerString = wstrToStr(data.manufacturerString);
            return Status::kSuccess;
        }

        Status getHIDProductString(std::string& productString) {
            Status status;
            uint8_t dataSize;

#pragma pack(push, 1)
            struct { Status status; wchar_t productString[maxProductStringSize() + 1]; } data{};
#pragma pack(pop)

            status = sendPacket(Command::kGetProductString);
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kGetProductString, &data, sizeof(data), &dataSize);
            if (status != Status::kSuccess) return status;

            if (dataSize == 1) return data.status;
            if (data.status != Status::kSuccess) return Status::kInvalidResponsePacket;

            productString = wstrToStr(data.productString);
            return Status::kSuccess;
        }

        Status getDeviceID(uint16_t& deviceID) {
            Status status = sendPacket(Command::kGetDeviceID);
            if (status != Status::kSuccess) return status;

            return recvPacket(Command::kGetDeviceID, &deviceID, sizeof(deviceID));
        }

        Status getFirmwareVersion(uint16_t& firmwareVersion) {
            Status status = sendPacket(Command::kGetFirmwareVersion);
            if (status != Status::kSuccess) return status;

            return recvPacket(Command::kGetFirmwareVersion, &firmwareVersion, sizeof(firmwareVersion));
        }

        Status getDeviceSerialNumber(std::vector<uint8_t>& deviceSerialNumber) {
            Status status;
            std::vector<uint8_t> data(20);

            status = sendPacket(Command::kGetDeviceSerialNumber);
            if (status != Status::kSuccess) return status;

            status = recvPacket(Command::kGetDeviceSerialNumber, data.data(), data.size());
            if (status != Status::kSuccess) return status;

            deviceSerialNumber = data;
            return Status::kSuccess;
        }

    private:
        enum class Command : uint8_t {
            kAny = 0,
            kReboot = 1,

            kKeyDown = 11,
            kKeyUp,
            kReleaseAllKeys,
            kGetKeyState,
            kGetKeyboardLEDsState,
            kGetKeyboardState,
            kSendKeyboardState,

            kButtonDown = 31,
            kButtonUp,
            kReleaseAllButtons,
            kGetButtonsState,

            kMoveRel = 51,
            kScrollRel,
            kGetRelMouseState,
            kSendRelMouseState,

            kInitAbsSystem = 71,
            kMoveAbs,
            kScrollAbs,
            kGetPos,
            kSetPos,
            kGetWheelAxis,
            kSetWheelAxis,
            kGetAxes,
            kSetAxes,
            kGetAbsMouseState,
            kSendAbsMouseState,

            kGetVendorID = 91,
            kGetProductID,
            kGetVersionNumber,
            kGetManufacturerString,
            kGetProductString,

            kConfigVendorID = 111,
            kConfigProductID,
            kConfigVersionNumber,
            kConfigManufacturerString,
            kConfigProductString,

            kGetDeviceID = 131,
            kGetDeviceSerialNumber,
            kGetFirmwareVersion
        };

        enum class HIDKeyCode : uint8_t {
            kInvalid = 0,

            kKeyA = 0x04, kKeyB,
            kKeyC, kKeyD, kKeyE, kKeyF, kKeyG, kKeyH,
            kKeyI, kKeyJ, kKeyK, kKeyL, kKeyM, kKeyN,
            kKeyO, kKeyP, kKeyQ, kKeyR, kKeyS, kKeyT,
            kKeyU, kKeyV, kKeyW, kKeyX, kKeyY, kKeyZ,

            kDigit1, kDigit2, kDigit3, kDigit4, kDigit5,
            kDigit6, kDigit7, kDigit8, kDigit9, kDigit0,

            kEnter, kEscape, kBackspace,
            kTab, kSpace,

            kMinus,             /* - */
            kEqual,             /* + */
            kBracketLeft,       /* [ */
            kBracketRight,      /* ] */
            kBackslash,         /* \ */
            kSemicolon = 0x33,  /* ; */
            kQuote,             /* ' */
            kBackquote,         /* ` */
            kComma,             /* , */
            kPeriod,            /* . */
            kSlash,             /* / */

            kCapsLock,

            kF1, kF2, kF3, kF4, kF5, kF6,
            kF7, kF8, kF9, kF10, kF11, kF12,

            kPrintScreen, kScrollLock, kPause,
            kInsert, kHome, kPageUp,
            kDelete, kEnd, kPageDown,

            kArrowRight,
            kArrowLeft,
            kArrowDown,
            kArrowUp,

            kNumLock,
            kNumpadDivide,    /* / */
            kNumpadMultiply,  /* * */
            kNumpadSubtract,  /* - */
            kNumpadAdd,       /* + */
            kNumpadEnter,

            kNumpad1, kNumpad2, kNumpad3, kNumpad4, kNumpad5,
            kNumpad6, kNumpad7, kNumpad8, kNumpad9, kNumpad0,

            kNumpadDecimal,  /* . */
            kContextMenu = 0x65,

            kControlLeft = 0xE0,
            kShiftLeft,
            kAltLeft,
            kOSLeft,
            kControlRight,
            kShiftRight,
            kAltRight,
            kOSRight
        };

        HANDLE hSerial;

        bool serialOpen(LPCSTR port, DWORD baudRate) {
            DCB dcb{};
            COMMTIMEOUTS timeouts{};

            hSerial = CreateFileA(port,
                                  GENERIC_READ | GENERIC_WRITE,
                                  0,
                                  NULL,
                                  OPEN_EXISTING,
                                  0,
                                  NULL);
            if (hSerial == INVALID_HANDLE_VALUE) goto Error;

            dcb.DCBlength = sizeof(DCB);
            if (!GetCommState(hSerial, &dcb)) goto Error;

            dcb.BaudRate = baudRate;
            dcb.ByteSize = 8;
            dcb.StopBits = ONESTOPBIT;
            dcb.Parity   = NOPARITY;
            if (!SetCommState(hSerial, &dcb)) goto Error;

            timeouts.ReadIntervalTimeout         = 50;
            timeouts.ReadTotalTimeoutConstant    = 50;
            timeouts.ReadTotalTimeoutMultiplier  = 10;
            timeouts.WriteTotalTimeoutConstant   = 50;
            timeouts.WriteTotalTimeoutMultiplier = 10;
            if (!SetCommTimeouts(hSerial, &timeouts)) goto Error;

            return true;
        Error:
            CloseHandle(hSerial);
            hSerial = INVALID_HANDLE_VALUE;
            return false;
        }

        bool serialClose() {
            bool ok = CloseHandle(hSerial) == TRUE;
            if (ok) hSerial = INVALID_HANDLE_VALUE;
            return ok;
        }

        bool serialSend(LPCVOID buffer, DWORD bufferSize) {
            DWORD writeSize;
            return WriteFile(hSerial, buffer, bufferSize, &writeSize, NULL) && writeSize == bufferSize;
        }

        bool serialRecv(LPVOID buffer, DWORD bufferSize) {
            DWORD readSize;
            return ReadFile(hSerial, buffer, bufferSize, &readSize, NULL) && readSize == bufferSize;
        }

        Status sendPacket(Command cmd, LPCVOID data = NULL, uint8_t dataSize = 0) {
            DWORD packetSize = 4u + dataSize;  // 0xBE cmd size [data] 0xED
            std::unique_ptr<uint8_t[]> packet(new uint8_t[packetSize]);

            packet[0] = 0xBE;
            packet[1] = static_cast<uint8_t>(cmd);
            packet[2] = dataSize;
            memcpy(&packet[3], data, dataSize);
            packet[static_cast<size_t>(packetSize) - 1] = 0xED;

            return serialSend(packet.get(), packetSize) ? Status::kSuccess : Status::kSerialError;
        }

        Status recvPacketHead() {
            uint8_t packetHead = 0;
            for (size_t i = 0; i < 64; ++i)
            {
                if (!serialRecv(&packetHead, 1)) return Status::kSerialError;
                if (packetHead == 0xBE) return Status::kSuccess;
            }
            return Status::kInvalidResponsePacket;
        }

        Status recvPacket(Command cmd, LPVOID buffer, size_t bufferSize, uint8_t* dataSize = NULL) {
            uint8_t packetDataSize = 0, packetTail = 0;
            Command packetCmd{};
            Status status;

            status = recvPacketHead();
            if (status != Status::kSuccess) return status;

            if (!serialRecv(&packetCmd, 1)) return Status::kSerialError;
            if (packetCmd != cmd && packetCmd != Command::kAny) return Status::kInvalidResponsePacket;

            if (!serialRecv(&packetDataSize, 1)) return Status::kSerialError;
            if (packetDataSize > bufferSize) return Status::kSerialError;

            if (packetDataSize != 0 && !serialRecv(buffer, packetDataSize)) return Status::kSerialError;

            if (!serialRecv(&packetTail, 1)) return Status::kSerialError;
            if (packetTail != 0xED) return Status::kInvalidResponsePacket;

            if (dataSize) *dataSize = packetDataSize;
            else if (packetDataSize != bufferSize) return Status::kInvalidResponsePacket;

            return Status::kSuccess;
        }

        VirtualKeyCode HIDKeyCodeToVirtualKeyCode(HIDKeyCode code) {
            switch (code)
            {
            case HIDKeyCode::kKeyA:           return VirtualKeyCode::kKeyA;
            case HIDKeyCode::kKeyB:           return VirtualKeyCode::kKeyB;
            case HIDKeyCode::kKeyC:           return VirtualKeyCode::kKeyC;
            case HIDKeyCode::kKeyD:           return VirtualKeyCode::kKeyD;
            case HIDKeyCode::kKeyE:           return VirtualKeyCode::kKeyE;
            case HIDKeyCode::kKeyF:           return VirtualKeyCode::kKeyF;
            case HIDKeyCode::kKeyG:           return VirtualKeyCode::kKeyG;
            case HIDKeyCode::kKeyH:           return VirtualKeyCode::kKeyH;
            case HIDKeyCode::kKeyI:           return VirtualKeyCode::kKeyI;
            case HIDKeyCode::kKeyJ:           return VirtualKeyCode::kKeyJ;
            case HIDKeyCode::kKeyK:           return VirtualKeyCode::kKeyK;
            case HIDKeyCode::kKeyL:           return VirtualKeyCode::kKeyL;
            case HIDKeyCode::kKeyM:           return VirtualKeyCode::kKeyM;
            case HIDKeyCode::kKeyN:           return VirtualKeyCode::kKeyN;
            case HIDKeyCode::kKeyO:           return VirtualKeyCode::kKeyO;
            case HIDKeyCode::kKeyP:           return VirtualKeyCode::kKeyP;
            case HIDKeyCode::kKeyQ:           return VirtualKeyCode::kKeyQ;
            case HIDKeyCode::kKeyR:           return VirtualKeyCode::kKeyR;
            case HIDKeyCode::kKeyS:           return VirtualKeyCode::kKeyS;
            case HIDKeyCode::kKeyT:           return VirtualKeyCode::kKeyT;
            case HIDKeyCode::kKeyU:           return VirtualKeyCode::kKeyU;
            case HIDKeyCode::kKeyV:           return VirtualKeyCode::kKeyV;
            case HIDKeyCode::kKeyW:           return VirtualKeyCode::kKeyW;
            case HIDKeyCode::kKeyX:           return VirtualKeyCode::kKeyX;
            case HIDKeyCode::kKeyY:           return VirtualKeyCode::kKeyY;
            case HIDKeyCode::kKeyZ:           return VirtualKeyCode::kKeyZ;
            case HIDKeyCode::kDigit1:         return VirtualKeyCode::kDigit1;
            case HIDKeyCode::kDigit2:         return VirtualKeyCode::kDigit2;
            case HIDKeyCode::kDigit3:         return VirtualKeyCode::kDigit3;
            case HIDKeyCode::kDigit4:         return VirtualKeyCode::kDigit4;
            case HIDKeyCode::kDigit5:         return VirtualKeyCode::kDigit5;
            case HIDKeyCode::kDigit6:         return VirtualKeyCode::kDigit6;
            case HIDKeyCode::kDigit7:         return VirtualKeyCode::kDigit7;
            case HIDKeyCode::kDigit8:         return VirtualKeyCode::kDigit8;
            case HIDKeyCode::kDigit9:         return VirtualKeyCode::kDigit9;
            case HIDKeyCode::kDigit0:         return VirtualKeyCode::kDigit0;
            case HIDKeyCode::kEnter:          return VirtualKeyCode::kEnter;
            case HIDKeyCode::kEscape:         return VirtualKeyCode::kEscape;
            case HIDKeyCode::kBackspace:      return VirtualKeyCode::kBackspace;
            case HIDKeyCode::kTab:            return VirtualKeyCode::kTab;
            case HIDKeyCode::kSpace:          return VirtualKeyCode::kSpace;
            case HIDKeyCode::kMinus:          return VirtualKeyCode::kMinus;
            case HIDKeyCode::kEqual:          return VirtualKeyCode::kEqual;
            case HIDKeyCode::kBracketLeft:    return VirtualKeyCode::kBracketLeft;
            case HIDKeyCode::kBracketRight:   return VirtualKeyCode::kBracketRight;
            case HIDKeyCode::kBackslash:      return VirtualKeyCode::kBackslash;
            case HIDKeyCode::kSemicolon:      return VirtualKeyCode::kSemicolon;
            case HIDKeyCode::kQuote:          return VirtualKeyCode::kQuote;
            case HIDKeyCode::kBackquote:      return VirtualKeyCode::kBackquote;
            case HIDKeyCode::kComma:          return VirtualKeyCode::kComma;
            case HIDKeyCode::kPeriod:         return VirtualKeyCode::kPeriod;
            case HIDKeyCode::kSlash:          return VirtualKeyCode::kSlash;
            case HIDKeyCode::kCapsLock:       return VirtualKeyCode::kCapsLock;
            case HIDKeyCode::kF1:             return VirtualKeyCode::kF1;
            case HIDKeyCode::kF2:             return VirtualKeyCode::kF2;
            case HIDKeyCode::kF3:             return VirtualKeyCode::kF3;
            case HIDKeyCode::kF4:             return VirtualKeyCode::kF4;
            case HIDKeyCode::kF5:             return VirtualKeyCode::kF5;
            case HIDKeyCode::kF6:             return VirtualKeyCode::kF6;
            case HIDKeyCode::kF7:             return VirtualKeyCode::kF7;
            case HIDKeyCode::kF8:             return VirtualKeyCode::kF8;
            case HIDKeyCode::kF9:             return VirtualKeyCode::kF9;
            case HIDKeyCode::kF10:            return VirtualKeyCode::kF10;
            case HIDKeyCode::kF11:            return VirtualKeyCode::kF11;
            case HIDKeyCode::kF12:            return VirtualKeyCode::kF12;
            case HIDKeyCode::kPrintScreen:    return VirtualKeyCode::kPrintScreen;
            case HIDKeyCode::kScrollLock:     return VirtualKeyCode::kScrollLock;
            case HIDKeyCode::kPause:          return VirtualKeyCode::kPause;
            case HIDKeyCode::kInsert:         return VirtualKeyCode::kInsert;
            case HIDKeyCode::kHome:           return VirtualKeyCode::kHome;
            case HIDKeyCode::kPageUp:         return VirtualKeyCode::kPageUp;
            case HIDKeyCode::kDelete:         return VirtualKeyCode::kDelete;
            case HIDKeyCode::kEnd:            return VirtualKeyCode::kEnd;
            case HIDKeyCode::kPageDown:       return VirtualKeyCode::kPageDown;
            case HIDKeyCode::kArrowRight:     return VirtualKeyCode::kArrowRight;
            case HIDKeyCode::kArrowLeft:      return VirtualKeyCode::kArrowLeft;
            case HIDKeyCode::kArrowDown:      return VirtualKeyCode::kArrowDown;
            case HIDKeyCode::kArrowUp:        return VirtualKeyCode::kArrowUp;
            case HIDKeyCode::kNumLock:        return VirtualKeyCode::kNumLock;
            case HIDKeyCode::kNumpadDivide:   return VirtualKeyCode::kNumpadDivide;
            case HIDKeyCode::kNumpadMultiply: return VirtualKeyCode::kNumpadMultiply;
            case HIDKeyCode::kNumpadSubtract: return VirtualKeyCode::kNumpadSubtract;
            case HIDKeyCode::kNumpadAdd:      return VirtualKeyCode::kNumpadAdd;
            case HIDKeyCode::kNumpadEnter:    return VirtualKeyCode::kNumpadEnter;
            case HIDKeyCode::kNumpad1:        return VirtualKeyCode::kNumpad1;
            case HIDKeyCode::kNumpad2:        return VirtualKeyCode::kNumpad2;
            case HIDKeyCode::kNumpad3:        return VirtualKeyCode::kNumpad3;
            case HIDKeyCode::kNumpad4:        return VirtualKeyCode::kNumpad4;
            case HIDKeyCode::kNumpad5:        return VirtualKeyCode::kNumpad5;
            case HIDKeyCode::kNumpad6:        return VirtualKeyCode::kNumpad6;
            case HIDKeyCode::kNumpad7:        return VirtualKeyCode::kNumpad7;
            case HIDKeyCode::kNumpad8:        return VirtualKeyCode::kNumpad8;
            case HIDKeyCode::kNumpad9:        return VirtualKeyCode::kNumpad9;
            case HIDKeyCode::kNumpad0:        return VirtualKeyCode::kNumpad0;
            case HIDKeyCode::kNumpadDecimal:  return VirtualKeyCode::kNumpadDecimal;
            case HIDKeyCode::kContextMenu:    return VirtualKeyCode::kContextMenu;
            case HIDKeyCode::kControlLeft:    return VirtualKeyCode::kControlLeft;
            case HIDKeyCode::kShiftLeft:      return VirtualKeyCode::kShiftLeft;
            case HIDKeyCode::kAltLeft:        return VirtualKeyCode::kAltLeft;
            case HIDKeyCode::kOSLeft:         return VirtualKeyCode::kOSLeft;
            case HIDKeyCode::kControlRight:   return VirtualKeyCode::kControlRight;
            case HIDKeyCode::kShiftRight:     return VirtualKeyCode::kShiftRight;
            case HIDKeyCode::kAltRight:       return VirtualKeyCode::kAltRight;
            case HIDKeyCode::kOSRight:        return VirtualKeyCode::kOSRight;
            case HIDKeyCode::kInvalid:        return VirtualKeyCode::kInvalid;
            default:                          return VirtualKeyCode::kInvalid;
            }
        }

        HIDKeyCode virtualKeyCodeToHIDKeyCode(VirtualKeyCode code) {
            switch (code)
            {
            case VirtualKeyCode::kBackspace:      return HIDKeyCode::kBackspace;
            case VirtualKeyCode::kTab:            return HIDKeyCode::kTab;
            case VirtualKeyCode::kEnter:          return HIDKeyCode::kEnter;
            case VirtualKeyCode::kShift:          return HIDKeyCode::kShiftLeft;
            case VirtualKeyCode::kControl:        return HIDKeyCode::kControlLeft;
            case VirtualKeyCode::kAlt:            return HIDKeyCode::kAltLeft;
            case VirtualKeyCode::kPause:          return HIDKeyCode::kPause;
            case VirtualKeyCode::kCapsLock:       return HIDKeyCode::kCapsLock;
            case VirtualKeyCode::kEscape:         return HIDKeyCode::kEscape;
            case VirtualKeyCode::kSpace:          return HIDKeyCode::kSpace;
            case VirtualKeyCode::kPageUp:         return HIDKeyCode::kPageUp;
            case VirtualKeyCode::kPageDown:       return HIDKeyCode::kPageDown;
            case VirtualKeyCode::kEnd:            return HIDKeyCode::kEnd;
            case VirtualKeyCode::kHome:           return HIDKeyCode::kHome;
            case VirtualKeyCode::kArrowLeft:      return HIDKeyCode::kArrowLeft;
            case VirtualKeyCode::kArrowUp:        return HIDKeyCode::kArrowUp;
            case VirtualKeyCode::kArrowRight:     return HIDKeyCode::kArrowRight;
            case VirtualKeyCode::kArrowDown:      return HIDKeyCode::kArrowDown;
            case VirtualKeyCode::kPrintScreen:    return HIDKeyCode::kPrintScreen;
            case VirtualKeyCode::kInsert:         return HIDKeyCode::kInsert;
            case VirtualKeyCode::kDelete:         return HIDKeyCode::kDelete;
            case VirtualKeyCode::kDigit0:         return HIDKeyCode::kDigit0;
            case VirtualKeyCode::kDigit1:         return HIDKeyCode::kDigit1;
            case VirtualKeyCode::kDigit2:         return HIDKeyCode::kDigit2;
            case VirtualKeyCode::kDigit3:         return HIDKeyCode::kDigit3;
            case VirtualKeyCode::kDigit4:         return HIDKeyCode::kDigit4;
            case VirtualKeyCode::kDigit5:         return HIDKeyCode::kDigit5;
            case VirtualKeyCode::kDigit6:         return HIDKeyCode::kDigit6;
            case VirtualKeyCode::kDigit7:         return HIDKeyCode::kDigit7;
            case VirtualKeyCode::kDigit8:         return HIDKeyCode::kDigit8;
            case VirtualKeyCode::kDigit9:         return HIDKeyCode::kDigit9;
            case VirtualKeyCode::kKeyA:           return HIDKeyCode::kKeyA;
            case VirtualKeyCode::kKeyB:           return HIDKeyCode::kKeyB;
            case VirtualKeyCode::kKeyC:           return HIDKeyCode::kKeyC;
            case VirtualKeyCode::kKeyD:           return HIDKeyCode::kKeyD;
            case VirtualKeyCode::kKeyE:           return HIDKeyCode::kKeyE;
            case VirtualKeyCode::kKeyF:           return HIDKeyCode::kKeyF;
            case VirtualKeyCode::kKeyG:           return HIDKeyCode::kKeyG;
            case VirtualKeyCode::kKeyH:           return HIDKeyCode::kKeyH;
            case VirtualKeyCode::kKeyI:           return HIDKeyCode::kKeyI;
            case VirtualKeyCode::kKeyJ:           return HIDKeyCode::kKeyJ;
            case VirtualKeyCode::kKeyK:           return HIDKeyCode::kKeyK;
            case VirtualKeyCode::kKeyL:           return HIDKeyCode::kKeyL;
            case VirtualKeyCode::kKeyM:           return HIDKeyCode::kKeyM;
            case VirtualKeyCode::kKeyN:           return HIDKeyCode::kKeyN;
            case VirtualKeyCode::kKeyO:           return HIDKeyCode::kKeyO;
            case VirtualKeyCode::kKeyP:           return HIDKeyCode::kKeyP;
            case VirtualKeyCode::kKeyQ:           return HIDKeyCode::kKeyQ;
            case VirtualKeyCode::kKeyR:           return HIDKeyCode::kKeyR;
            case VirtualKeyCode::kKeyS:           return HIDKeyCode::kKeyS;
            case VirtualKeyCode::kKeyT:           return HIDKeyCode::kKeyT;
            case VirtualKeyCode::kKeyU:           return HIDKeyCode::kKeyU;
            case VirtualKeyCode::kKeyV:           return HIDKeyCode::kKeyV;
            case VirtualKeyCode::kKeyW:           return HIDKeyCode::kKeyW;
            case VirtualKeyCode::kKeyX:           return HIDKeyCode::kKeyX;
            case VirtualKeyCode::kKeyY:           return HIDKeyCode::kKeyY;
            case VirtualKeyCode::kKeyZ:           return HIDKeyCode::kKeyZ;
            case VirtualKeyCode::kOSLeft:         return HIDKeyCode::kOSLeft;
            case VirtualKeyCode::kOSRight:        return HIDKeyCode::kOSRight;
            case VirtualKeyCode::kContextMenu:    return HIDKeyCode::kContextMenu;
            case VirtualKeyCode::kNumpad0:        return HIDKeyCode::kNumpad0;
            case VirtualKeyCode::kNumpad1:        return HIDKeyCode::kNumpad1;
            case VirtualKeyCode::kNumpad2:        return HIDKeyCode::kNumpad2;
            case VirtualKeyCode::kNumpad3:        return HIDKeyCode::kNumpad3;
            case VirtualKeyCode::kNumpad4:        return HIDKeyCode::kNumpad4;
            case VirtualKeyCode::kNumpad5:        return HIDKeyCode::kNumpad5;
            case VirtualKeyCode::kNumpad6:        return HIDKeyCode::kNumpad6;
            case VirtualKeyCode::kNumpad7:        return HIDKeyCode::kNumpad7;
            case VirtualKeyCode::kNumpad8:        return HIDKeyCode::kNumpad8;
            case VirtualKeyCode::kNumpad9:        return HIDKeyCode::kNumpad9;
            case VirtualKeyCode::kNumpadMultiply: return HIDKeyCode::kNumpadMultiply;
            case VirtualKeyCode::kNumpadAdd:      return HIDKeyCode::kNumpadAdd;
            case VirtualKeyCode::kNumpadEnter:    return HIDKeyCode::kNumpadEnter;
            case VirtualKeyCode::kNumpadSubtract: return HIDKeyCode::kNumpadSubtract;
            case VirtualKeyCode::kNumpadDecimal:  return HIDKeyCode::kNumpadDecimal;
            case VirtualKeyCode::kNumpadDivide:   return HIDKeyCode::kNumpadDivide;
            case VirtualKeyCode::kF1:             return HIDKeyCode::kF1;
            case VirtualKeyCode::kF2:             return HIDKeyCode::kF2;
            case VirtualKeyCode::kF3:             return HIDKeyCode::kF3;
            case VirtualKeyCode::kF4:             return HIDKeyCode::kF4;
            case VirtualKeyCode::kF5:             return HIDKeyCode::kF5;
            case VirtualKeyCode::kF6:             return HIDKeyCode::kF6;
            case VirtualKeyCode::kF7:             return HIDKeyCode::kF7;
            case VirtualKeyCode::kF8:             return HIDKeyCode::kF8;
            case VirtualKeyCode::kF9:             return HIDKeyCode::kF9;
            case VirtualKeyCode::kF10:            return HIDKeyCode::kF10;
            case VirtualKeyCode::kF11:            return HIDKeyCode::kF11;
            case VirtualKeyCode::kF12:            return HIDKeyCode::kF12;
            case VirtualKeyCode::kNumLock:        return HIDKeyCode::kNumLock;
            case VirtualKeyCode::kScrollLock:     return HIDKeyCode::kScrollLock;
            case VirtualKeyCode::kShiftLeft:      return HIDKeyCode::kShiftLeft;
            case VirtualKeyCode::kShiftRight:     return HIDKeyCode::kShiftRight;
            case VirtualKeyCode::kControlLeft:    return HIDKeyCode::kControlLeft;
            case VirtualKeyCode::kControlRight:   return HIDKeyCode::kControlRight;
            case VirtualKeyCode::kAltLeft:        return HIDKeyCode::kAltLeft;
            case VirtualKeyCode::kAltRight:       return HIDKeyCode::kAltRight;
            case VirtualKeyCode::kSemicolon:      return HIDKeyCode::kSemicolon;
            case VirtualKeyCode::kEqual:          return HIDKeyCode::kEqual;
            case VirtualKeyCode::kComma:          return HIDKeyCode::kComma;
            case VirtualKeyCode::kMinus:          return HIDKeyCode::kMinus;
            case VirtualKeyCode::kPeriod:         return HIDKeyCode::kPeriod;
            case VirtualKeyCode::kSlash:          return HIDKeyCode::kSlash;
            case VirtualKeyCode::kBackquote:      return HIDKeyCode::kBackquote;
            case VirtualKeyCode::kBracketLeft:    return HIDKeyCode::kBracketLeft;
            case VirtualKeyCode::kBackslash:      return HIDKeyCode::kBackslash;
            case VirtualKeyCode::kBracketRight:   return HIDKeyCode::kBracketRight;
            case VirtualKeyCode::kQuote:          return HIDKeyCode::kQuote;
            case VirtualKeyCode::kInvalid:        return HIDKeyCode::kInvalid;
            default:                              return HIDKeyCode::kInvalid;
            }
        }

        std::wstring strToWstr(const std::string& str) {
            int size = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
            std::unique_ptr<wchar_t[]> wstr(new wchar_t[static_cast<size_t>(size)]);
            MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wstr.get(), size);
            return std::wstring(wstr.get());
        }

        std::string wstrToStr(const std::wstring& wstr) {
            int size = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
            std::unique_ptr<char[]> str(new char[static_cast<size_t>(size)]);
            WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, str.get(), size, NULL, NULL);
            return std::string(str.get());
        }
    };

    static std::string statusToString(Status status) {
        std::ostringstream errorMessage;

        switch (status)
        {
        case RX784::Status::kSuccess:               return "RX784::Status::kSuccess";
        case RX784::Status::kSerialError:           return "RX784::Status::kSerialError";
        case RX784::Status::kReadFlashError:        return "RX784::Status::kReadFlashError";
        case RX784::Status::kWriteFlashError:       return "RX784::Status::kWriteFlashError";
        case RX784::Status::kInvalidSize:           return "RX784::Status::kInvalidSize";
        case RX784::Status::kInvalidCommandPacket:  return "RX784::Status::kInvalidCommandPacket";
        case RX784::Status::kInvalidResponsePacket: return "RX784::Status::kInvalidResponsePacket";
        default:
            errorMessage << static_cast<int8_t>(status) << " is not a valid RX784::Status";
            throw std::out_of_range(errorMessage.str());
        }
    }

    static std::ostream& operator<<(std::ostream& stream, Status status)
    {
        return stream << statusToString(status);
    }
};
