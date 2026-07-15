/**
 * @file    KeyCodes.hpp
 * @brief   Virtual key-code enumeration for DragonOS input.
 *
 * Every key on a standard PC keyboard is represented by a
 * KeyCode value.  The numeric values match Win32 virtual-key
 * codes so they can be used directly with WM_KEYDOWN / WM_KEYUP.
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace DragonOS::Input {

/**
 * @brief  Identifies a physical keyboard key.
 *
 * Values correspond to Win32 VK_* constants for seamless
 * integration with the Windows message loop.
 */
enum class KeyCode : uint32_t {
    // ── Mouse simulation (used by Windows, not physical keys) ────────────
    LButton = 0x01,
    RButton = 0x02,
    MButton = 0x04,

    // ── Navigation ──────────────────────────────────────────────────────
    Back        = 0x08,
    Tab         = 0x09,
    Clear       = 0x0C,
    Return      = 0x0D,  // Enter
    Enter       = 0x0D,  // alias

    // ── Modifiers ───────────────────────────────────────────────────────
    Shift       = 0x10,
    Control     = 0x11,
    Alt         = 0x12,  // Menu / Alt
    Pause       = 0x13,
    CapsLock    = 0x14,

    // ── IME ─────────────────────────────────────────────────────────────
    Kana        = 0x15,
    ImeOn       = 0x16,
    Junja       = 0x17,
    Final       = 0x18,
    Hanja       = 0x19,
    ImeOff      = 0x1A,

    // ── Escape ──────────────────────────────────────────────────────────
    Escape = 0x1B,

    // ── IME (continued) ─────────────────────────────────────────────────
    Convert     = 0x1C,
    NonConvert  = 0x1D,
    Accept      = 0x1E,
    ModeChange  = 0x1F,

    // ── Space ───────────────────────────────────────────────────────────
    Space = 0x20,

    // ── Page navigation ─────────────────────────────────────────────────
    PageUp   = 0x21,
    PageDown = 0x22,
    End      = 0x23,
    Home     = 0x24,

    // ── Arrow keys ──────────────────────────────────────────────────────
    Left  = 0x25,
    Up    = 0x26,
    Right = 0x27,
    Down  = 0x28,

    // ── Editing ─────────────────────────────────────────────────────────
    Select  = 0x29,
    Print   = 0x2A,
    Execute = 0x2B,
    PrintScreen = 0x2C,
    Insert      = 0x2D,
    Delete      = 0x2E,
    Help        = 0x2F,

    // ── Digits (0–9) ────────────────────────────────────────────────────
    D0 = 0x30,
    D1 = 0x31,
    D2 = 0x32,
    D3 = 0x33,
    D4 = 0x34,
    D5 = 0x35,
    D6 = 0x36,
    D7 = 0x37,
    D8 = 0x38,
    D9 = 0x39,

    // ── Letters (A–Z) ───────────────────────────────────────────────────
    A = 0x41,
    B = 0x42,
    C = 0x43,
    D = 0x44,
    E = 0x45,
    F = 0x46,
    G = 0x47,
    H = 0x48,
    I = 0x49,
    J = 0x4A,
    K = 0x4B,
    L = 0x4C,
    M = 0x4D,
    N = 0x4E,
    O = 0x4F,
    P = 0x50,
    Q = 0x51,
    R = 0x52,
    S = 0x53,
    T = 0x54,
    U = 0x55,
    V = 0x56,
    W = 0x57,
    X = 0x58,
    Y = 0x59,
    Z = 0x5A,

    // ── Left Windows / Application ──────────────────────────────────────
    LWin    = 0x5B,
    RWin    = 0x5C,
    Apps    = 0x5D,

    // ── Sleep ───────────────────────────────────────────────────────────
    Sleep = 0x5F,

    // ── Numpad ──────────────────────────────────────────────────────────
    Numpad0      = 0x60,
    Numpad1      = 0x61,
    Numpad2      = 0x62,
    Numpad3      = 0x63,
    Numpad4      = 0x64,
    Numpad5      = 0x65,
    Numpad6      = 0x66,
    Numpad7      = 0x67,
    Numpad8      = 0x68,
    Numpad9      = 0x69,
    Multiply     = 0x6A,
    Add          = 0x6B,
    Separator    = 0x6C,
    Subtract     = 0x6D,
    Decimal      = 0x6E,
    Divide       = 0x6F,

    // ── Function keys (F1–F24) ──────────────────────────────────────────
    F1  = 0x70,
    F2  = 0x71,
    F3  = 0x72,
    F4  = 0x73,
    F5  = 0x74,
    F6  = 0x75,
    F7  = 0x76,
    F8  = 0x77,
    F9  = 0x78,
    F10 = 0x79,
    F11 = 0x7A,
    F12 = 0x7B,
    F13 = 0x7C,
    F14 = 0x7D,
    F15 = 0x7E,
    F16 = 0x7F,
    F17 = 0x80,
    F18 = 0x81,
    F19 = 0x82,
    F20 = 0x83,
    F21 = 0x84,
    F22 = 0x85,
    F23 = 0x86,
    F24 = 0x87,

    // ── Extended navigation ─────────────────────────────────────────────
    NumLock     = 0x90,
    ScrollLock  = 0x91,

    // ── OEM keys ────────────────────────────────────────────────────────
    LShift     = 0xA0,
    RShift     = 0xA1,
    LControl   = 0xA2,
    RControl   = 0xA3,
    LAlt       = 0xA4,
    RAlt       = 0xA5,

    // ── Browser / Media (modern keyboards) ──────────────────────────────
    BrowserBack       = 0xA6,
    BrowserForward    = 0xA7,
    BrowserRefresh    = 0xA8,
    BrowserStop       = 0xA9,
    BrowserSearch     = 0xAA,
    BrowserFavorites  = 0xAB,
    BrowserHome       = 0xAC,
    VolumeMute        = 0xAD,
    VolumeDown        = 0xAE,
    VolumeUp          = 0xAF,
    NextTrack         = 0xB0,
    PrevTrack         = 0xB1,
    StopMedia         = 0xB2,
    PlayPause         = 0xB3,
    LaunchMail        = 0xB4,
    LaunchMediaSelect = 0xB5,
    LaunchApp1        = 0xB6,
    LaunchApp2        = 0xB7,

    // ── Additional OEM keys ─────────────────────────────────────────────
    Oem1      = 0xBA,  // ;:
    OemPlus   = 0xBB,  // =+
    OemComma  = 0xBC,  // ,<
    OemMinus  = 0xBD,  // -_
    OemPeriod = 0xBE,  // .>
    Oem2      = 0xBF,  // /?
    Oem3      = 0xC0,  // `~

    Oem4 = 0xDB,  // [{
    Oem5 = 0xDC,  // \|
    Oem6 = 0xDD,  // ]}
    Oem7 = 0xDE,  // '"
    Oem8 = 0xDF,

    // ── Miscellaneous ───────────────────────────────────────────────────
    OemClear    = 0xFE,
    Attn        = 0xF6,
    CrSel       = 0xF7,
    ExSel       = 0xF8,
    EraseEof    = 0xF9,
    Play        = 0xFA,
    Zoom        = 0xFB,
    NoName      = 0xFC,
    Pa1         = 0xFD,

    // ── Sentinel ──────────────────────────────────────────────────────
    Unknown = 0,
};

/**
 * @brief  Number of distinct virtual-key values (0–255).
 */
inline constexpr std::size_t KeyCodeCount = 256;

/**
 * @brief  Convert a KeyCode to its underlying integer.
 */
[[nodiscard]] inline constexpr std::uint32_t KeyCodeValue(KeyCode code) noexcept
{
    return static_cast<std::uint32_t>(code);
}

/**
 * @brief  Convert an integer to a KeyCode, clamping to Unknown on invalid
 *         values.
 */
[[nodiscard]] inline constexpr KeyCode ToKeyCode(std::uint32_t value) noexcept
{
    return (value < KeyCodeCount)
               ? static_cast<KeyCode>(value)
               : KeyCode::Unknown;
}

} // namespace DragonOS::Input
