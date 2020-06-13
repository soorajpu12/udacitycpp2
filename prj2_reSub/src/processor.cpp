#include "processor.h"

// TODO: Return the aggregate CPU utilization

float Processor::Utilization() {
  jiffie_start = LinuxParser::Jiffies();
  jiffie_active_start = LinuxParser::ActiveJiffies();

  usleep(100000);  

  jiffie_end = LinuxParser::Jiffies();
  jiffie_active_end = LinuxParser::ActiveJiffies();

  long total = jiffie_end - jiffie_start;
  long active = jiffie_active_end - jiffie_active_start;

  if (total == 0) {
    return 0.0;
  }

  auto utilization = float(active) / float(total);
  return utilization;
}