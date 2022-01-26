#include "common_helpers.h"
#include "debug.h"
#include <algorithm>
#include <string>
#include <iostream>
#include <array>
#include <memory>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

void setFIFOPrioSched(pthread_t &thread, int prioPercentage) {
  prioPercentage = prioPercentage > 100 ? 100 : prioPercentage;
  prioPercentage = prioPercentage < 0 ? 0 : prioPercentage;

  int prioMin = sched_get_priority_min(SCHED_FIFO);
  int prioMax = sched_get_priority_max(SCHED_FIFO);
  int prioLevel = prioMin + ((prioMax - prioMin)*prioPercentage)/100;

  struct sched_param params;
  params.sched_priority = prioLevel;

  // debug("Trying to set thread realtime prio = %d, policy = %d\n", params.sched_priority, SCHED_FIFO);
  int ret = pthread_setschedparam(thread, SCHED_FIFO, &params);
  if ( ret != 0) {
    debug("Unsuccessful in setting thread realtime prio\n");
  }
  int policy = 0;
  ret = pthread_getschedparam(thread, &policy, &params);
  if (ret != 0 ) {
    debug("couldn't retrieve real-time scheduling parameters\n");
  }
  if (policy != SCHED_FIFO) {
    debug("Scheduling is not SCHED_FIFO!\n");
  }

}

void systemCall(const std::string& cmd) {
    auto r = system(cmd.c_str());
    if (r < 0) {
        debug("system(%s) failed with code %d", cmd, r);
    }
}

std::string exec(const std::string& cmd){
  std::array<char, 128> buff;
  std::string res;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
  while(fgets(buff.data(), buff.size(), pipe.get()) != nullptr){
    res += buff.data();
  }
  return res;
}

// produces a string of time format used in MongoDb from a chrono time point
std::string tp_to_str(std::chrono::time_point<std::chrono::system_clock> tp){
  using namespace std;

  chrono::milliseconds ms = chrono::duration_cast<chrono::milliseconds>(tp.time_since_epoch());
  chrono::seconds s = chrono::duration_cast<chrono::seconds>(ms);
  std::time_t t = s.count();
  uint16_t frac_sec = ms.count() % 1000;

  struct tm * info;
  info = gmtime(&t);

  char buffer[100];

  sprintf(buffer, "%d-%02d-%02dT%02d:%02d:%02d:%03u",
      info->tm_year + 1900, info->tm_mon +1, info->tm_mday, info->tm_hour, info->tm_min, info->tm_sec, frac_sec); // i like printf format specs more

  return string(buffer);
}

// blocking delay function in ms
void delay_ms(uint16_t del){
  usleep(del * 1000);
  return;
}

// for parsings strings of form %Y-%m-%dT%H:%M:%S:%f
std::chrono::time_point<std::chrono::system_clock> str_to_tp(std::string str){
  using namespace std;
  auto tmstmp = str.substr(0, str.find_last_of(':'));
  auto tmstmp_ms = stoi(str.substr(str.find_last_of(':') + 1));
  struct tm tm;
	strptime(tmstmp.c_str(), "%Y-%m-%dT%H:%M:%S", &tm);
  auto tp = chrono::system_clock::from_time_t(std::mktime(&tm));
  tp += chrono::milliseconds(tmstmp_ms);
  return tp;
}

float get_time_elapsed_ms(std::chrono::time_point<std::chrono::system_clock> start, std::chrono::time_point<std::chrono::system_clock> end) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

bool not_csv(std::string const & value) {
  std::string ending = ".csv";
    if (ending.size() > value.size()) return true;
    return !std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void get_only_csv(std::vector<std::string>& v) {
  v.erase(std::remove_if(v.begin(), v.end(), not_csv), v.end());
}

void read_directory(const std::string& name, std::vector<std::string>& v) {
    DIR* dirp = opendir(name.c_str());
    struct dirent * dp;
    while ((dp = readdir(dirp)) != NULL) {
        v.push_back(dp->d_name);
        debug("filename %s\n", dp->d_name);
    }
    closedir(dirp);
}

int millisSince(const std::chrono::time_point<std::chrono::system_clock>& tp) {
    auto duration = std::chrono::system_clock::now() - tp;
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}
int uSecSince(const std::chrono::time_point<std::chrono::system_clock>& tp) {
    auto duration = std::chrono::system_clock::now() - tp;
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}


