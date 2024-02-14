#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <stdexcept>

namespace storage {
struct Repository {
  std::string local_path;
  std::string remote_path;
  std::string target_path;
};
} // namespace storage

namespace parser {
auto fromFile(const std::string &path) -> std::list<storage::Repository> {
  std::list<storage::Repository> result;

  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << path << std::endl;
    return result;
  }

  std::string line;
  int line_number = 0;
  while (std::getline(file, line)) {
    line_number++;
    std::istringstream iss(line);
    std::string target, local, remote;

    if (iss >> local >> remote >> target) {
      result.push_back(storage::Repository{local, remote, target});
    } else {
      std::cerr << "Configuration file is incorrect: line does not contain "
                   "enough tokens:"
                << std::endl;
      std::cerr << "Line " << line_number << ": " << line << std::endl;
      std::cerr << "Expected format: [local_path] [remote_path] [target]"
                << std::endl;
    }
  }

  return result;
}
} // namespace parser

namespace worker {
auto setupLocalDir(const std::string &local_path) -> bool {
  std::filesystem::create_directories(local_path);
  return std::filesystem::exists(local_path);
}

auto checkGitRepoExists(const std::string &local_path) -> bool {
  std::filesystem::path git_dir = std::filesystem::path(local_path) / ".git";
  return std::filesystem::exists(git_dir) &&
         std::filesystem::is_directory(git_dir);
}

void setupGitRepo(const std::string &local_path,
                  const std::string &remote_path) {
  int result = 0;
  std::string init_repo = "git -C " + local_path + " init";
  result = std::system(init_repo.c_str());
  if (result != 0) {
    throw std::runtime_error("Failed to initialize Git repository at: " +
                             local_path);
  }

  std::string add_remote =
      "git -C " + local_path + " remote add origin " + remote_path;
  result = std::system(add_remote.c_str());
  if (result != 0) {
    std::string undo_init = "rm -rf " + local_path;
    std::system(undo_init.c_str());

    throw std::runtime_error("Failed to add remote to Git repository at: " +
                             remote_path);
  }
}

void updateFiles(const std::string &source, const std::string &destination) {
  std::filesystem::copy(source, destination,
                        std::filesystem::copy_options::recursive |
                            std::filesystem::copy_options::overwrite_existing);
}

void updateLocalRepo(const std::string &local_path) {
  std::string git_add = "git -C " + local_path + " add -A";
  int result = std::system(git_add.c_str());
  if (result != 0) {
    throw std::runtime_error("Failed to push repository to origin master: " +
                             local_path);
  }
}

void updateRemoteRepo(const std::string &local_path,
                      const std::string &commit_message) {
  int result = 0;
  std::string git_commit =
      "git -C " + local_path + " commit -m " + "\"" + commit_message + "\"";
  result = std::system(git_commit.c_str());
  if (result != 0) {
    throw std::runtime_error("Failed to add a new commit to the repository: " +
                             local_path);
  }

  std::string git_push = "git -C " + local_path + " push origin master";
  result = std::system(git_push.c_str());
  if (result != 0) {
    throw std::runtime_error("Failed to add new files to the repository: " +
                             local_path);
  }
}

void processTasks(const std::list<storage::Repository> &tasks) {
  auto now = std::chrono::system_clock::now();
  std::time_t time = std::chrono::system_clock::to_time_t(now);
  std::tm *tm_time = std::localtime(&time);
  std::stringstream ss;
  ss << std::put_time(tm_time, "%Y/%m/%d - %H:%M:%S");

  const std::string commit_message = ss.str();

  int task_id = 0;
  for (const auto &task : tasks) {
    task_id++;

    try {
      if (!setupLocalDir(task.local_path)) {
        throw std::runtime_error("Error creating local directory");
      }

      if (!checkGitRepoExists(task.local_path)) {
        setupGitRepo(task.local_path, task.remote_path);
      }

      updateFiles(task.target_path, task.local_path);
      updateLocalRepo(task.local_path);
      updateRemoteRepo(task.local_path, commit_message);

      std::cout << "Task " << task_id << " executed successfully." << std::endl;

    } catch (const std::exception &e) {
      std::cerr << "Error in task " << task_id << ": " << e.what() << std::endl;
    }
  }
}
} // namespace worker

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Configuration file should be provided" << std::endl;
    return 1;
  }

  const auto tasks = parser::fromFile(argv[1]);
  worker::processTasks(tasks);

  return 0;
}