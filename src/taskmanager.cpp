#include "taskmanager.h"
#include "json.hpp"
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <windows.h>

using json = nlohmann::json;

static std::string GetExeDir()
{
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::string s(path);
    size_t pos = s.find_last_of("\\/");
    return (pos != std::string::npos) ? s.substr(0, pos) : ".";
}

static TaskType CategoryToType(const std::string& cat)
{
    if (cat == "typing") return TYPING;
    if (cat == "trivia") return TRIVIA;
    if (cat == "coding") return CODING;
    if (cat == "exercise") return EXERCISE;
    if (cat == "math") return MATH;
    return TRIVIA;
}

TaskManager::TaskManager()
{
    srand((unsigned int)time(NULL));

    std::string jsonPath = GetExeDir() + "\\tasks.json";
    LoadFromJson(jsonPath);
}

void TaskManager::LoadFromJson(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
       m_error = true;
       m_errorMessage = "tasks.json not found!\n\nPlace tasks.json in the same folder as gauntlet.exe.";
       return;
    }

    json data;
    try { file >> data; }
    catch (...)
    {
        m_error = true;
        m_errorMessage = "tasks.json contains invalid JSON.\n\nCheck your formatting at jsonlint.com.";
        return;
    }

    for (auto& [category, entries] : data["tasks"].items())
    {
        TaskType type = CategoryToType(category);

        for (auto& entry : entries)
        {
            Task t;
            t.type = type;
            t.prompt = entry.value("prompt", "");
            t.answer = entry.value("answer", "");

            if (!t.prompt.empty())
                m_tasks.push_back(t);
        }
    }

    if (m_tasks.empty())
    {
        m_error = true;
        m_errorMessage = "tasks.json is empty!\n\nAdd some tasks and try again.";
    }
}

Task TaskManager::GetRandomTask()
{
    int index = rand() % (int)m_tasks.size();
    return m_tasks[index];
}