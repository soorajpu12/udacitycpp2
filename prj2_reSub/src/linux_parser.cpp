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
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
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

float LinuxParser::MemoryUtilization() { 
  std::ifstream fileStreamer(kProcDirectory + kMeminfoFilename);
  std::istringstream sstream;
  std::string line;
  std::string tag, value;
  float memTotal = 0, memFree = 0, cached = 0, buffers = 0, sreclaim = 0, shmem = 0;
  if (fileStreamer.is_open()) {
    while(std::getline(fileStreamer, line)){
      sstream.clear();
      sstream.str(line);
      sstream >> tag >> value;
      if (tag == "MemTotal:") {
      memTotal = std::stof(value);
      }
        else if (tag == "MemFree:") {
        memFree = std::stof(value);
        }
          else if (tag == "Cached:") {
            cached = std::stof(value);
          }
            else if (tag == "Buffers:") {
              buffers = std::stof(value);
            }
              else if (tag == "SReclaimable:" ) {
                sreclaim = std::stof(value);
              }
                else if (tag == "Shmem:") {
                  shmem = std::stof(value);
                }
    }
    const float totalUsed = memTotal - memFree; 
    const float cachedMemory = cached + sreclaim - shmem;
    const float util = totalUsed - (buffers + cachedMemory);
    return util/memTotal;
  }
  return 10;
}

long LinuxParser::UpTime() { 
  std::ifstream stream(kProcDirectory + kUptimeFilename); 
  std::string line, value;
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream stringstream(line); 
    stringstream >> value;
    return std::stol(value);
  }
  return 0;
 }

vector<string> LinuxParser::CpuUtilization() { 
  std::ifstream fileStream(kProcDirectory + kStatFilename);
  std::string line;
  if (fileStream.is_open()) {
    std::getline(fileStream, line);
    std::istringstream lineStream(line);
    std::vector<std::string> cpu_util_info{std::istream_iterator<string>{lineStream}, 
    std::istream_iterator<string>{}};
    cpu_util_info.erase(cpu_util_info.begin());
    return cpu_util_info; 
  }
  return {};
}

int LinuxParser::TotalProcesses() { 
  std::string line;
  std::ifstream inputFileStream(kProcDirectory + kStatFilename);
  std::string key, value;
  if (inputFileStream.is_open()) {
    while(std::getline(inputFileStream, line)){
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "processes") {
        return std::stoi(value);
      }
    }
  }
  return 0;
 }

int LinuxParser::RunningProcesses() { 
  std::string line;
  std::ifstream inputFileStream(kProcDirectory + kStatFilename);
  std::string key, value;
  if (inputFileStream.is_open()) {
    while(std::getline(inputFileStream, line)){
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "procs_running") {
        return std::stoi(value);
      }
    }
  }
  return 120;
 }

string LinuxParser::Command(const int pid) { 
  std::ifstream fileStream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  string line;
  string tag, value;
  if (fileStream.is_open()) {
    std::getline(fileStream, line); 
    return line;
  }
  return string(""); 
}

string LinuxParser::Ram(const int pid) { 
  std::ifstream fileStream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  std::string line;
  std::string tag, value;
  if (fileStream.is_open()) {
    while (std::getline(fileStream, line)) {
      std::istringstream lineStream(line);
      lineStream >> tag >> value;
      if (tag == "VmSize:") {
        const long kb_ram = std::stol(value);
        const auto mb_ram = kb_ram / 1000;
        return std::to_string(mb_ram);
      }
    }
  }
  return string(""); }

string LinuxParser::Uid(const int pid) {
  std::ifstream fileStream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  string line;
  string tag, value;
  if (fileStream.is_open()) {
    while(std::getline(fileStream, line)) {
      std::istringstream lineStream(line);
      lineStream >> tag >> value;
      if (tag == "Uid:")
        return value;
    }
  }
  return string(""); 
}

string LinuxParser::User(const int pid) { 
  std::ifstream fileStream(LinuxParser::kPasswordPath);
  std::string line;
  std::string username, x, user_id;
  if (fileStream.is_open()){
    while(std::getline(fileStream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream lineStream(line);
      lineStream >> username >> x >> user_id; 
      if (user_id == LinuxParser::Uid(pid)) {
        return username;
      }
    }
  }
  return string("");
}

long LinuxParser::UpTime(const int pid) { 
  std::ifstream fileStream(kProcDirectory + std::to_string(pid) + kStatFilename);
  std::string line;
  if (fileStream.is_open()) {
    std::getline(fileStream, line);
    std::istringstream lineStream(line);
    std::vector<std::string> data{std::istream_iterator<string>{lineStream}, std::istream_iterator<string>{}};
    return LinuxParser::UpTime() - std::stol(data[21])/sysconf(_SC_CLK_TCK);
  }
  return 0;
}

std::vector<long int> LinuxParser::Cpu(const int pid) {
  std::vector<long int> processInfo = {};
  std::ifstream fileStream(kProcDirectory + std::to_string(pid) + kStatFilename);
  std::string line;
  if (fileStream.is_open()) {
    std::getline(fileStream, line);
    std::istringstream lineStream(line);
    const std::vector<std::string> data {std::istream_iterator<string>{lineStream}, std::istream_iterator<string>{}};
    processInfo.push_back(std::stol(data[13]));
    processInfo.push_back(std::stol(data[14]));
    processInfo.push_back(std::stol(data[15]));
    processInfo.push_back(std::stol(data[16]));
    processInfo.push_back(std::stol(data[21]));
    return processInfo;
  }
  return {};
}
