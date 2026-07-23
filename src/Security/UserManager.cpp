#include <Security/UserManager.hpp>
#include <Security/CredentialManager.hpp>

#include <Engine/EngineContext.hpp>

#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>

#include <Windows.h>
#include <objbase.h>
#include <Rpc.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "Rpcrt4.lib")

namespace DragonOS::Security {

namespace {

constexpr std::wstring_view UsersFile = L"security\\users.dat";

std::wstring GuidToString()
{
    UUID uuid;
    UuidCreate(&uuid);
    RPC_WSTR str;
    UuidToStringW(&uuid, &str);
    std::wstring result(reinterpret_cast<wchar_t*>(str));
    RpcStringFreeW(&str);
    return result;
}

} // anonymous namespace

bool UserManager::Initialize(Engine::EngineContext& ctx) noexcept
{
    if (m_initialized) return true;
    (void)ctx;

    LoadUsers();

    if (m_users.empty())
    {
        auto adminUid = CreateUser(L"Admin", L"Administrator", L"admin", UserRole::Administrator);
        if (adminUid != InvalidUserId)
        {
            m_nextUserId = FirstCustomUserId;
        }
    }

    m_initialized = true;
    return true;
}

void UserManager::Shutdown() noexcept
{
    if (!m_initialized) return;
    SaveUsers();
    m_users.clear();
    m_currentSession = {};
    m_securityContext = {};
    m_initialized = false;
}

void UserManager::Update(float /*deltaTime*/) noexcept {}
void UserManager::Render(Engine::EngineContext& /*ctx*/) noexcept {}
void UserManager::Resize(float /*width*/, float /*height*/) noexcept {}

UserId UserManager::CreateUser(
    std::wstring_view username,
    std::wstring_view displayName,
    std::wstring_view password,
    UserRole role) noexcept
{
    if (username.empty()) return InvalidUserId;
    if (FindUserByName(username)) return InvalidUserId;

    auto uid = GenerateUserId();
    UserProfile profile;
    profile.uid = uid;
    profile.username = username;
    profile.displayName = displayName.empty() ? std::wstring{ username } : std::wstring{ displayName };
    profile.role = role;
    profile.isGuest = false;
    profile.isLocked = false;
    profile.createdAt = static_cast<uint64_t>(time(nullptr));

    wchar_t homeDir[MAX_PATH];
    GetModuleFileNameW(nullptr, homeDir, MAX_PATH);
    std::wstring exePath = homeDir;
    auto pos = exePath.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
        exePath = exePath.substr(0, pos);
    profile.homeDirectory = exePath + L"\\users\\" + std::wstring{ username };

    m_users[uid] = std::move(profile);

    if (m_pCredentialMgr && !password.empty())
    {
        m_pCredentialMgr->StorePassword(username, password);
    }

    SaveUsers();

    if (m_sessionEventCb)
        m_sessionEventCb(uid, L"created");

    return uid;
}

bool UserManager::DeleteUser(UserId uid) noexcept
{
    if (uid == BuiltinAdminUserId || uid == BuiltinGuestUserId) return false;
    auto it = m_users.find(uid);
    if (it == m_users.end()) return false;

    if (m_currentSession.uid == uid)
        Logout();

    m_users.erase(it);
    SaveUsers();

    if (m_sessionEventCb)
        m_sessionEventCb(uid, L"deleted");

    return true;
}

bool UserManager::RenameUser(UserId uid, std::wstring_view newUsername) noexcept
{
    auto it = m_users.find(uid);
    if (it == m_users.end()) return false;
    if (FindUserByName(newUsername)) return false;

    it->second.username = newUsername;
    SaveUsers();
    return true;
}

UserId UserManager::Authenticate(std::wstring_view username, std::wstring_view password) noexcept
{
    auto* user = FindUserByName(username);
    if (!user) return InvalidUserId;
    if (user->isLocked) return InvalidUserId;

    if (user->role == UserRole::Guest)
    {
        m_currentSession.uid = user->uid;
        m_currentSession.token = GenerateSessionToken();
        m_currentSession.loginTime = static_cast<uint64_t>(time(nullptr));
        m_currentSession.isLocked = false;
        m_lastUsername = username;
        UpdateSecurityContext();
        m_loginScreenVisible = false;

        if (m_sessionEventCb)
            m_sessionEventCb(user->uid, L"login");

        return user->uid;
    }

    if (!m_pCredentialMgr) return InvalidUserId;

    if (!m_pCredentialMgr->VerifyPassword(username, password))
        return InvalidUserId;

    m_currentSession.uid = user->uid;
    m_currentSession.token = GenerateSessionToken();
    m_currentSession.loginTime = static_cast<uint64_t>(time(nullptr));
    m_currentSession.isLocked = false;
    m_lastUsername = username;
    UpdateSecurityContext();
    m_loginScreenVisible = false;

    if (m_sessionEventCb)
        m_sessionEventCb(user->uid, L"login");

    return user->uid;
}

bool UserManager::SwitchUser(UserId uid) noexcept
{
    auto it = m_users.find(uid);
    if (it == m_users.end()) return false;

    m_currentSession.uid = uid;
    m_currentSession.token = GenerateSessionToken();
    m_currentSession.loginTime = static_cast<uint64_t>(time(nullptr));
    m_currentSession.isLocked = false;
    UpdateSecurityContext();
    m_loginScreenVisible = false;

    if (m_sessionEventCb)
        m_sessionEventCb(uid, L"switch");

    return true;
}

bool UserManager::Logout() noexcept
{
    if (m_currentSession.uid == InvalidUserId) return false;

    auto prevUid = m_currentSession.uid;
    m_currentSession = {};
    m_securityContext = {};
    m_loginScreenVisible = true;

    if (m_sessionEventCb)
        m_sessionEventCb(prevUid, L"logout");

    return true;
}

bool UserManager::LockSession() noexcept
{
    if (m_currentSession.uid == InvalidUserId) return false;
    m_currentSession.isLocked = true;
    m_loginScreenVisible = true;

    if (m_sessionEventCb)
        m_sessionEventCb(m_currentSession.uid, L"lock");

    return true;
}

bool UserManager::UnlockSession(std::wstring_view password) noexcept
{
    if (m_currentSession.uid == InvalidUserId) return false;
    auto* user = FindUser(m_currentSession.uid);
    if (!user) return false;

    if (user->role == UserRole::Guest)
    {
        m_currentSession.isLocked = false;
        m_loginScreenVisible = false;
        return true;
    }

    if (!m_pCredentialMgr) return false;
    if (!m_pCredentialMgr->VerifyPassword(user->username, password))
        return false;

    m_currentSession.isLocked = false;
    m_loginScreenVisible = false;

    if (m_sessionEventCb)
        m_sessionEventCb(m_currentSession.uid, L"unlock");

    return true;
}

bool UserManager::CreateGuestSession() noexcept
{
    auto* guest = FindUserByName(L"Guest");
    UserId guestUid;
    if (!guest)
    {
        guestUid = CreateUser(L"Guest", L"Guest Account", L"", UserRole::Guest);
        if (guestUid == InvalidUserId) return false;
        auto* g = FindUser(guestUid);
        if (g) g->isGuest = true;
    }
    else
    {
        guestUid = guest->uid;
    }

    m_currentSession.uid = guestUid;
    m_currentSession.token = GenerateSessionToken();
    m_currentSession.loginTime = static_cast<uint64_t>(time(nullptr));
    m_currentSession.isLocked = false;
    UpdateSecurityContext();
    m_loginScreenVisible = false;

    if (m_sessionEventCb)
        m_sessionEventCb(guestUid, L"guest_login");

    return true;
}

UserProfile* UserManager::FindUser(UserId uid) noexcept
{
    auto it = m_users.find(uid);
    return (it != m_users.end()) ? &it->second : nullptr;
}

UserProfile* UserManager::FindUserByName(std::wstring_view username) noexcept
{
    for (auto& [uid, user] : m_users)
    {
        if (_wcsicmp(user.username.c_str(), username.data()) == 0)
            return &user;
    }
    return nullptr;
}

const UserProfile* UserManager::GetCurrentUser() const noexcept
{
    auto it = m_users.find(m_currentSession.uid);
    return (it != m_users.end()) ? &it->second : nullptr;
}

std::vector<const UserProfile*> UserManager::GetAllUsers() const noexcept
{
    std::vector<const UserProfile*> result;
    result.reserve(m_users.size());
    for (const auto& [uid, user] : m_users)
        result.push_back(&user);
    return result;
}

void UserManager::SetLoginScreenVisible(bool visible) noexcept
{
    m_loginScreenVisible = visible;
}

void UserManager::UpdateSecurityContext() noexcept
{
    auto* user = FindUser(m_currentSession.uid);
    if (!user)
    {
        m_securityContext = {};
        return;
    }

    m_securityContext.user = *user;
    m_securityContext.session = m_currentSession;
    m_securityContext.effectivePermissions = GetDefaultRolePermissions(user->role).permissions;
}

UserId UserManager::GenerateUserId() noexcept
{
    return m_nextUserId++;
}

std::wstring UserManager::GenerateSessionToken() noexcept
{
    return GuidToString();
}

std::wstring UserManager::GetUsersFilePath() const noexcept
{
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring exePath = path;
    auto pos = exePath.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
        exePath = exePath.substr(0, pos);
    return exePath + L"\\" + std::wstring{ UsersFile };
}

bool UserManager::SaveUsers() noexcept
{
    auto filePath = GetUsersFilePath();

    auto dirPos = filePath.find_last_of(L"\\/");
    if (dirPos != std::wstring::npos)
    {
        std::wstring dir = filePath.substr(0, dirPos);
        CreateDirectoryW(dir.c_str(), nullptr);
    }

    std::wofstream file(filePath);
    if (!file.is_open()) return false;

    file << L"[config]\n";
    file << L"autoLogin=" << (m_autoLogin ? L"true" : L"false") << L"\n";
    file << L"lastUser=" << m_lastUsername << L"\n";
    file << L"nextUid=" << std::to_wstring(m_nextUserId) << L"\n\n";

    file << L"[users]\n";
    for (const auto& [uid, user] : m_users)
    {
        file << std::to_wstring(uid) << L"="
             << user.username << L"|"
             << user.displayName << L"|"
             << static_cast<int>(user.role) << L"|"
             << (user.isGuest ? L"1" : L"0") << L"|"
             << (user.isLocked ? L"1" : L"0") << L"|"
             << user.homeDirectory << L"|"
             << std::to_wstring(user.createdAt)
             << L"\n";
    }

    file.close();
    return true;
}

bool UserManager::LoadUsers() noexcept
{
    auto filePath = GetUsersFilePath();
    std::wifstream file(filePath);
    if (!file.is_open()) return false;

    m_users.clear();

    std::wstring line;
    std::wstring currentSection;
    while (std::getline(file, line))
    {
        if (line.empty()) continue;
        if (line.front() == L'[')
        {
            auto end = line.find(L']');
            if (end != std::wstring::npos)
                currentSection = line.substr(1, end - 1);
            continue;
        }

        auto eq = line.find(L'=');
        if (eq == std::wstring::npos) continue;

        auto key = line.substr(0, eq);
        auto val = line.substr(eq + 1);

        if (currentSection == L"config")
        {
            if (key == L"autoLogin") m_autoLogin = (val == L"true");
            else if (key == L"lastUser") m_lastUsername = val;
            else if (key == L"nextUid") { try { m_nextUserId = static_cast<UserId>(std::stoul(val)); } catch (...) {} }
        }
        else if (currentSection == L"users")
        {
            try {
                UserId uid = static_cast<UserId>(std::stoul(key));

                // Parse pipe-delimited fields
                std::vector<std::wstring> fields;
                size_t start = 0;
                while (start < val.size())
                {
                    auto pipe = val.find(L'|', start);
                    if (pipe == std::wstring::npos)
                    {
                        fields.push_back(val.substr(start));
                        break;
                    }
                    fields.push_back(val.substr(start, pipe - start));
                    start = pipe + 1;
                }

                if (fields.size() >= 7)
                {
                    UserProfile user;
                    user.uid = uid;
                    user.username = fields[0];
                    user.displayName = fields[1];
                    user.role = static_cast<UserRole>(std::stoi(fields[2]));
                    user.isGuest = (fields[3] == L"1");
                    user.isLocked = (fields[4] == L"1");
                    user.homeDirectory = fields[5];
                    user.createdAt = std::stoull(fields[6]);
                    m_users[uid] = std::move(user);
                }
            } catch (...) {}
        }
    }

    file.close();
    return true;
}

} // namespace DragonOS::Security
