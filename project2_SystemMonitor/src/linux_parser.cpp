#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}
/*
// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { return 0.0; }

// TODO: Read and return the system uptime
long LinuxParser::UpTime() { return 0; }

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return 0; }

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid[[maybe_unused]]) { return 0; }

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { return 0; }

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { return 0; }

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { return {}; }

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() { return 0; }

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() { return 0; }

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid[[maybe_unused]]) { return 0; }
*/

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string line;
  string key;
  string value;
  float totalMemory = 0.0;
  float freeMemory = 0.0;
  std::ifstream file_stream(kProcDirectory + kMeminfoFilename);
  if (file_stream.is_open()) {
    while (std::getline(file_stream, line)) {
      std::istringstream lineStream(line);
      lineStream >> key >> value;
      if (key == "MemTotal:") {
        totalMemory = stof(value);
      }
      if (key == "MemFree:") {
        freeMemory = stof(value);
      }
    }
    return totalMemory - freeMemory;
  }
  return 0.0;
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
  string uptime;
  string line;
  std::ifstream file_stream(kProcDirectory + kUptimeFilename);
  if (file_stream.is_open()) {
    std::getline(file_stream, line);
    std::istringstream line_stream(line);
    line_stream >> uptime;
    return stol(uptime);
  }
  return 0;
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  long total_jiffies = 0;
  auto cpu_utilization = LinuxParser::CpuUtilization();
  for (auto i : cpu_utilization) {
    total_jiffies += stol(i);
  }
  return total_jiffies;
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) {
  long total_active_jiffies = 0;
  string value;
  string line;
  std::ifstream file_stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (file_stream.is_open()) {
    std::getline(file_stream, line);
    std::istringstream line_stream(line);
    int i = 0;
    while (line_stream >> value) {
      if (i > 12 && i < 17) {
        total_active_jiffies += stol(value);
      }
      i += 1;
    }
  }
  return total_active_jiffies;
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  return LinuxParser::Jiffies() - LinuxParser::IdleJiffies();
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  long total = 0;
  auto cpu_utilization = LinuxParser::CpuUtilization();
  for (int i = kIdle_; i <= kIOwait_; i++) {
    total += stol(cpu_utilization[i]);
  }
  return total;
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  string value;
  string line;
  vector<string> cpu_values;
  std::ifstream file_stream(kProcDirectory + kStatFilename);
  if (file_stream.is_open()) {
    std::getline(file_stream, line);
    std::istringstream line_stream(line);
    while (line_stream >> value) {
      if (value != "cpu") {
        cpu_values.push_back(value);
      }
    }
  }
  return cpu_values;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line;
  string key;
  string value;
  std::ifstream file_stream(kProcDirectory + kStatFilename);
  if (file_stream.is_open()) {
    while (std::getline(file_stream, line)) {
      std::istringstream line_stream(line);
      line_stream >> key >> value;
      if (key == "processes") {
        if (value != "") {
          return stoi(value);
        }
        
      }
    }
  }
  return 0;
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line;
  string key;
  string value;
  std::ifstream file_stream(kProcDirectory + kStatFilename);
  if (file_stream.is_open()) {
    while (std::getline(file_stream, line)) {
      std::istringstream line_stream(line);
      line_stream >> key >> value;
      if (key == "procs_running") {
        if (value != "") {
          return stoi(value);
        }
        
      }
    }
  }
  return 0;
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  string command_used;
  std::ifstream file_stream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (file_stream.is_open()) {
    std::getline(file_stream, command_used);
    return command_used;
  }
  return string();
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
  string key;
  string value;
  string line;
  std::stringstream ram;
  std::ifstream file_stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (file_stream.is_open()) {
    while (std::getline(file_stream, line)) {
      std::istringstream line_stream(line);
      line_stream >> key >> value;
      if (key == "VmSize") {
        ram << std::fixed << std::setprecision(3) << stof(value) / 1000;
        return ram.str();
      }
    }
  }

  return string("0");
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  string line;
  string key;
  string value;
  std::ifstream file_stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (file_stream.is_open()) {
    while (std::getline(file_stream, line)) {
      std::istringstream line_stream(line);
      line_stream >> key >> value;
      if (key == "Uid:") {
        return value;
      }
    }
  }
  return string();
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  string uid = LinuxParser::Uid(pid);
  string line;
  string key;
  string x;
  string  value;
  std::ifstream file_stream(kPasswordPath);
  if (file_stream.is_open()) {
    while (std::getline(file_stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream line_stream(line);
      line_stream >> key >> x >> value;
      if (value == uid) {
        return key;
      }
    }
  }
  return string();
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) {

  string line;
  string value;
  std::ifstream file_stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (file_stream.is_open()) {
    std::getline(file_stream, line);
    std::istringstream line_stream(line);
	int i = 0;
    while (line_stream >> value) {
      if (i == 21) {
        return stol(value) / sysconf(_SC_CLK_TCK);
      }
      i++;
    }
  }
  return 0;
 }

