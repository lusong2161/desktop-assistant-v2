#include "../include/Database.h"
#include <sqlite3.h>
#include <filesystem>
#include <sstream>

namespace SmartAssistant {
namespace Core {

namespace {
    const wchar_t* DEFAULT_DB_NAME = L"SmartAssistant.db";
    sqlite3* db = nullptr;
    std::wstring dbPath;
}

bool Database::Initialize(const std::wstring& path) {
    dbPath = path.empty() ? DEFAULT_DB_NAME : path;
    std::filesystem::path fsPath(dbPath);
    std::filesystem::create_directories(fsPath.parent_path());

    std::string utf8Path(dbPath.begin(), dbPath.end());
    if (sqlite3_open(utf8Path.c_str(), &db) != SQLITE_OK) {
        return false;
    }

    // Create tables
    const char* createRemindersTable = 
        "CREATE TABLE IF NOT EXISTS reminders ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "title TEXT NOT NULL,"
        "message TEXT,"
        "timestamp INTEGER NOT NULL,"
        "recurring INTEGER DEFAULT 0,"
        "interval_minutes INTEGER DEFAULT 0"
        ");";

    const char* createSettingsTable =
        "CREATE TABLE IF NOT EXISTS settings ("
        "key TEXT PRIMARY KEY,"
        "value TEXT NOT NULL"
        ");";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, createRemindersTable, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }

    if (sqlite3_exec(db, createSettingsTable, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}

void Database::Shutdown() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool Database::AddReminder(const Reminder& reminder) {
    const char* sql = 
        "INSERT INTO reminders (title, message, timestamp, recurring, interval_minutes) "
        "VALUES (?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    std::string title(reminder.title.begin(), reminder.title.end());
    std::string message(reminder.message.begin(), reminder.message.end());
    auto timestamp = std::chrono::system_clock::to_time_t(reminder.timestamp);

    sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, message.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, timestamp);
    sqlite3_bind_int(stmt, 4, reminder.recurring ? 1 : 0);
    sqlite3_bind_int(stmt, 5, reminder.intervalMinutes);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool Database::GetReminders(std::vector<Reminder>& reminders, bool includeCompleted) {
    std::string sql = "SELECT * FROM reminders";
    if (!includeCompleted) {
        sql += " WHERE timestamp >= strftime('%s', 'now')";
    }
    sql += " ORDER BY timestamp;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Reminder reminder;
        
        const char* title = (const char*)sqlite3_column_text(stmt, 1);
        const char* message = (const char*)sqlite3_column_text(stmt, 2);
        reminder.title = std::wstring(title, title + strlen(title));
        reminder.message = std::wstring(message, message + strlen(message));
        
        time_t timestamp = sqlite3_column_int64(stmt, 3);
        reminder.timestamp = std::chrono::system_clock::from_time_t(timestamp);
        
        reminder.recurring = sqlite3_column_int(stmt, 4) != 0;
        reminder.intervalMinutes = sqlite3_column_int(stmt, 5);
        
        reminders.push_back(reminder);
    }

    sqlite3_finalize(stmt);
    return true;
}

bool Database::UpdateReminder(const Reminder& reminder) {
    const char* sql = 
        "UPDATE reminders SET message = ?, timestamp = ?, "
        "recurring = ?, interval_minutes = ? WHERE title = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    std::string title(reminder.title.begin(), reminder.title.end());
    std::string message(reminder.message.begin(), reminder.message.end());
    auto timestamp = std::chrono::system_clock::to_time_t(reminder.timestamp);

    sqlite3_bind_text(stmt, 1, message.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, timestamp);
    sqlite3_bind_int(stmt, 3, reminder.recurring ? 1 : 0);
    sqlite3_bind_int(stmt, 4, reminder.intervalMinutes);
    sqlite3_bind_text(stmt, 5, title.c_str(), -1, SQLITE_STATIC);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool Database::RemoveReminder(const std::wstring& title) {
    const char* sql = "DELETE FROM reminders WHERE title = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    std::string titleStr(title.begin(), title.end());
    sqlite3_bind_text(stmt, 1, titleStr.c_str(), -1, SQLITE_STATIC);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool Database::SetSetting(const std::wstring& key, const std::wstring& value) {
    const char* sql = "INSERT OR REPLACE INTO settings (key, value) VALUES (?, ?);";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    std::string keyStr(key.begin(), key.end());
    std::string valueStr(value.begin(), value.end());

    sqlite3_bind_text(stmt, 1, keyStr.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, valueStr.c_str(), -1, SQLITE_STATIC);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool Database::GetSetting(const std::wstring& key, std::wstring& value) {
    const char* sql = "SELECT value FROM settings WHERE key = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    std::string keyStr(key.begin(), key.end());
    sqlite3_bind_text(stmt, 1, keyStr.c_str(), -1, SQLITE_STATIC);

    bool success = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* valueStr = (const char*)sqlite3_column_text(stmt, 0);
        value = std::wstring(valueStr, valueStr + strlen(valueStr));
        success = true;
    }

    sqlite3_finalize(stmt);
    return success;
}

bool Database::BeginTransaction() {
    char* errMsg = nullptr;
    if (sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg) != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Database::CommitTransaction() {
    char* errMsg = nullptr;
    if (sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg) != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Database::RollbackTransaction() {
    char* errMsg = nullptr;
    if (sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, &errMsg) != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

} // namespace Core
} // namespace SmartAssistant
