#pragma once
#include <string>
#include <vector>

enum TaskType
{
    TYPING,
    TRIVIA,
    CODING,
    EXERCISE,
    MATH
};

struct Task
{
    TaskType type;
    std::string prompt;
    std::string answer;
};

class TaskManager
{
public:
    TaskManager();
    Task GetRandomTask();
    bool HasError() const { return m_error; }
    std::string GetErrorMessage() const { return m_errorMessage; }

private:
    void LoadFromJson(const std::string& path);

    std::vector<Task> m_tasks;
    bool m_error = false;
    std::string m_errorMessage;
};