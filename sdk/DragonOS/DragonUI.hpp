#pragma once

// DragonOS SDK — DragonUI Framework
//
// This header exposes the official DragonUI input controls to SDK consumers.
// Applications include <DragonOS/DragonOS.hpp> which pulls in this header.
//
// All controls live in the DragonOS::DragonUI namespace.
//
// Usage:
//   #include <DragonOS/DragonOS.hpp>
//   using namespace DragonOS::DragonUI;
//
//   auto textBox = std::make_unique<UITextBox>(L"Enter value...");
//   textBox->SetOnTextChanged([](UITextBox& tb) { ... });
//
// Available controls:
//   UITextBox        — Single-line text input with clipboard, undo/redo, validation
//   UIPasswordBox    — Password input with show/hide toggle, strength callback
//   UICheckBox       — Three-state check box (checked, unchecked, indeterminate)
//   UIRadioButton    — Radio button for single-selection groups
//   UIToggleSwitch   — Animated on/off toggle with smooth transition
//
// Also included:
//   RadioGroup       — Groups UIRadioButton instances for exclusive selection
//   Validator        — Base validation class
//   RequiredValidator, MinLengthValidator, MaxLengthValidator, RegexValidator
//   CompositeValidator
//   ValidationState, ValidationResult
//
// These controls integrate with ThemeManager, Input System, Focus Manager,
// Layout Engine, Event System, and the Direct2D Renderer.

#include <DragonUI/DragonUI.hpp>
