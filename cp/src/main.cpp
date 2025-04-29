#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
struct Job
{
    int id;                        // уникальный индентификатор джобы
    std::vector<int> dependencies; // зависимые джобы, которые должны будут выполнены до данной джобы
    std::vector<int> dependents;   // зависимые джобы, которые должны будут выполнены после данной джобы
    bool hasError = false;         // флаг ошибки
};

// класс, представляющий DAG джобов
class DAG
{
public:
    std::unordered_map<int, Job> jobs;
    // добавление ребра от from к to
    // добавляет зависимость между двумя джобами.
    void addEdge(int from, int to)
    {
        jobs[from].id = from;
        jobs[to].id = to;
        jobs[from].dependents.push_back(to);
        jobs[to].dependencies.push_back(from);
    }

    // этот метод проверяет, есть ли цикл с какой-то джобы
    bool hasCycleUtil(int job_id, std::unordered_set<int> &visited,
                      std::unordered_set<int> &recStack)
    {
        if (visited.find(job_id) == visited.end())
        {
            visited.insert(job_id);
            recStack.insert(job_id);
            for (auto &dependent : jobs[job_id].dependents)
            {
                if (visited.find(dependent) == visited.end() &&
                    hasCycleUtil(dependent, visited, recStack))
                    return true;
                else if (recStack.find(dependent) != recStack.end())
                    return true;
            }
        }
        recStack.erase(job_id);
        return false;
    }
    // для инициализации поиска цикла для всех джобов
    bool hasCycle()
    {
        std::unordered_set<int> visited;
        std::unordered_set<int> recStack;
        for (auto &pair : jobs)
        {
            if (hasCycleUtil(pair.first, visited, recStack))
                return true;
        }
        return false;
    }
    // gпроверка на связность графа
    bool isConnected()
    {
        if (jobs.empty())
            return true;
        std::unordered_set<int> visited;
        std::queue<int> q;
        // начнем с любого узла
        q.push(jobs.begin()->first);
        while (!q.empty())
        {
            int current = q.front();
            q.pop();
            if (visited.find(current) == visited.end())
            {
                visited.insert(current);
                for (auto &dep : jobs[current].dependencies)
                    q.push(dep);
                for (auto &dep : jobs[current].dependents)
                    q.push(dep);
            }
        }
        return visited.size() == jobs.size();
    }
    // получение стартовых джоб без зависимостей
    std::vector<int>
    getStartJobs()
    {
        std::vector<int> starts;
        for (auto &pair : jobs)
        {
            if (pair.second.dependencies.empty())
                starts.push_back(pair.first);
        }
        return starts;
    }
    // получение завершающих джоб без зависимых джоб
    std::vector<int> getEndJobs()
    {
        std::vector<int> ends;
        for (auto &pair : jobs)
        {
            if (pair.second.dependents.empty())
                ends.push_back(pair.first);
        }
        return ends;
    }
};
// класс для парсинга INI
class Parser
{
public:
    static bool parseINI(const std::string &filename, DAG &dag)
    {
        std::ifstream infile(filename);
        if (!infile.is_open())
        {
            std::cerr << "Не удалось открыть файл: " << filename << std::endl;
            return false;
        }
        std::string line;
        while (std::getline(infile, line))
        {
            // Игнорируем пустые строки и комментарии
            if (line.empty() || line[0] == ';' || line[0] == '#')
                continue;
            std::stringstream ss(line);
            std::string token;
            int from = -1, to = -1;

            // Читаем "job_id = X"
            if (!std::getline(ss, token, '='))
                continue;
            // Пропускаем "job_id"
            size_t pos = token.find("job_id");
            if (pos != std::string::npos)
                token = token.substr(pos + 6);
            // Читаем число
            if (!(ss >> from))
                continue;
            // Пропускаем "-> job_id = "
            ss.ignore(std::numeric_limits<std::streamsize>::max(), '=');
            if (!(ss >> to))
                continue;
            if (from != -1 && to != -1)
                dag.addEdge(from, to);
        }
        infile.close();
        return true;
    }
};
// Функция для валидации DAG
bool validateDAG(DAG &dag)
{
    // Проверка на циклы
    if (dag.hasCycle())
    {
        std::cerr << "Ошибка: Обнаружен цикл в DAG." << std::endl;
        return false;
    }
    // Проверка на связность
    if (!dag.isConnected())
    {
        std::cerr << "Ошибка: DAG имеет более одной компоненты связности." << std::endl;
        return false;
    }
    // Проверка наличия стартовых и завершающих джоб
    auto starts = dag.getStartJobs();
    auto ends = dag.getEndJobs();
    if (starts.empty())
    {
        std::cerr << "Ошибка: Отсутствуют стартовые джобы." << std::endl;
        return false;
    }
    if (ends.empty())
    {
        std::cerr << "Ошибка: Отсутствуют завершающие джобы." << std::endl;
        return false;
    }
    std::cout << "DAG прошел валидацию успешно." << std::endl;
    std::cout << "Стартовые джобы: ";
    for (auto id : starts)
        std::cout << id << " ";
    std::cout << "\nЗавершающие джобы: ";
    for (auto id : ends)
        std::cout << id << " ";
    std::cout << std::endl;
    return true;
}
// Класс планировщика джоб
class Scheduler
{
public:
    Scheduler(DAG &dag) : dag(dag), stopFlag(false) {}
    // Запуск выполнения джоб
    void execute()
    {
        // Инициализация счетчиков зависимостей
        std::unordered_map<int, int> dependencyCount;
        for (auto &pair : dag.jobs)
        {
            dependencyCount[pair.first] = pair.second.dependencies.size();
        }
        // Очередь для готовых джоб
        std::queue<int> readyQueue;
        for (auto &pair : dependencyCount)
        {
            if (pair.second == 0)
                readyQueue.push(pair.first);
        }
        // Запуск потоков
        std::vector<std::thread> workers;
        // Mutex для доступа к readyQueue
        std::mutex queueMutex;
        while (true)
        {
            // Блокируем очередь для доступа
            std::unique_lock<std::mutex> lock(queueMutex);
            if (readyQueue.empty() && workers.empty())
            {
                // Если нет готовых джоб и все потоки завершены, выходим
                break;
            }
            // Запуск готовых джоб
            while (!readyQueue.empty())
            {
                int jobId = readyQueue.front();
                readyQueue.pop();
                workers.emplace_back(&Scheduler::runJob, this, jobId,
                                     std::ref(dependencyCount), std::ref(readyQueue), std::ref(queueMutex));
            }
            lock.unlock();
            // Ожидание завершения хотя бы одного потока
            for (auto it = workers.begin(); it != workers.end();)
            {
                if (it->joinable())
                {
                    it->join();
                    it = workers.erase(it);
                }
                else
                {
                    ++it;
                }
            }
            if (stopFlag)
            {
                // Прерываем выполнение
                std::cerr << "Выполнение прервано из-за ошибки." << std::endl;
                break;
            }
        }
    }

private:
    DAG &dag;
    std::atomic<bool> stopFlag;
    std::mutex mtx;

    // Функция выполнения одной джобы
    void
    runJob(int jobId, std::unordered_map<int, int> &dependencyCount,
           std::queue<int> &readyQueue, std::mutex &queueMutex)
    {
        if (stopFlag)
            return;
        std::cout << "Запуск джобы " << jobId << std::endl;
        // Симуляция выполнения джобы
        bool success = executeJob(jobId);
        if (!success)
        {
            std::cerr << "Джоба " << jobId << " завершилась с ошибкой." << std::endl;
            stopFlag = true;
            return;
        }
        std::cout << "Джоба " << jobId << " завершена успешно." << std::endl;
        // Обновление зависимых джоб
        std::lock_guard<std::mutex> lock(queueMutex);
        for (auto &dependent : dag.jobs[jobId].dependents)
        {
            dependencyCount[dependent]--;
            if (dependencyCount[dependent] == 0)
                readyQueue.push(dependent);
        }
    }
    // Симуляция выполнения джобы
    bool executeJob(int jobId)
    {
        // Здесь должна быть реальная логика выполнения джобы.
        // Для примера будем симулировать время выполнения и ошибку для
        // определенных джоб.
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Симуляция времени выполнения

        return true;
    }
};
int main(int argc, char *argv[])
{
    if (argc < 2)

    {
        std::cerr << "Использование: " << argv[0] << " <config.ini>" << std::endl;
        return 1;
    }
    std::string configFile = argv[1];
    DAG dag;
    // Парсинг конфигурационного файла
    if (!Parser::parseINI(configFile, dag))
    {
        return 1;
    }
    // Валидация DAG
    if (!validateDAG(dag))
    {
        return 1;
    }
    // Создание и запуск планировщика
    Scheduler scheduler(dag);
    scheduler.execute();
    return 0;
}