#pragma once

#include <string>
#include <string_view>
#include <functional>
#include <optional>
#include <regex>
#include <memory>
#include <vector>

namespace DragonOS::DragonUI {

enum class ValidationState : uint8_t {
    Valid,
    Invalid,
    Pending,
};

struct ValidationResult {
    ValidationState state{ValidationState::Valid};
    std::wstring errorMessage;
};

using AsyncValidator = std::function<void(std::wstring_view, std::function<void(ValidationResult)>)>;

class Validator {
public:
    Validator() = default;
    virtual ~Validator() = default;

    virtual ValidationResult Validate(std::wstring_view input) const noexcept {
        (void)input;
        return {ValidationState::Valid};
    }

    [[nodiscard]] std::wstring GetErrorMessage() const noexcept { return m_errorMessage; }
    void SetErrorMessage(std::wstring_view msg) noexcept { m_errorMessage = msg; }

private:
    std::wstring m_errorMessage;
};

class RequiredValidator final : public Validator {
public:
    explicit RequiredValidator(std::wstring_view errorMsg = L"This field is required.") noexcept {
        SetErrorMessage(errorMsg);
    }

    ValidationResult Validate(std::wstring_view input) const noexcept override {
        if (input.empty())
            return {ValidationState::Invalid, GetErrorMessage()};
        return {ValidationState::Valid};
    }
};

class MinLengthValidator final : public Validator {
public:
    explicit MinLengthValidator(size_t minLen, std::wstring_view errorMsg = {}) noexcept
        : m_minLength(minLen) {
        if (errorMsg.empty())
            SetErrorMessage(L"Minimum length is " + std::to_wstring(minLen) + L" characters.");
        else
            SetErrorMessage(errorMsg);
    }

    ValidationResult Validate(std::wstring_view input) const noexcept override {
        if (input.size() < m_minLength)
            return {ValidationState::Invalid, GetErrorMessage()};
        return {ValidationState::Valid};
    }

private:
    size_t m_minLength{};
};

class MaxLengthValidator final : public Validator {
public:
    explicit MaxLengthValidator(size_t maxLen, std::wstring_view errorMsg = {}) noexcept
        : m_maxLength(maxLen) {
        if (errorMsg.empty())
            SetErrorMessage(L"Maximum length is " + std::to_wstring(maxLen) + L" characters.");
        else
            SetErrorMessage(errorMsg);
    }

    ValidationResult Validate(std::wstring_view input) const noexcept override {
        if (input.size() > m_maxLength)
            return {ValidationState::Invalid, GetErrorMessage()};
        return {ValidationState::Valid};
    }

private:
    size_t m_maxLength{};
};

class RegexValidator final : public Validator {
public:
    explicit RegexValidator(std::wstring_view pattern, std::wstring_view errorMsg = L"Invalid format.") noexcept
        : m_pattern(pattern) {
        SetErrorMessage(errorMsg);
    }

    ValidationResult Validate(std::wstring_view input) const noexcept override {
        try {
            std::wregex re(m_pattern.data(), m_pattern.size());
            if (!std::regex_match(input.data(), input.data() + input.size(), re))
                return {ValidationState::Invalid, GetErrorMessage()};
        } catch (...) {
            return {ValidationState::Invalid, L"Regex error."};
        }
        return {ValidationState::Valid};
    }

private:
    std::wstring m_pattern;
};

class CompositeValidator final : public Validator {
public:
    void AddValidator(std::shared_ptr<Validator> validator) noexcept {
        m_validators.push_back(std::move(validator));
    }

    ValidationResult Validate(std::wstring_view input) const noexcept override {
        for (const auto& v : m_validators) {
            auto result = v->Validate(input);
            if (result.state != ValidationState::Valid)
                return result;
        }
        return {ValidationState::Valid};
    }

private:
    std::vector<std::shared_ptr<Validator>> m_validators;
};

} // namespace
