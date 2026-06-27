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

private:
    void LoadFromJson(const std::string& path);

    std::vector<Task> m_tasks;
};