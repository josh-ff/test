#pragma once
#include <string>
#include <chrono>
#include <vector>
#include <deque>
#include <math.h>

void setFIFOPrioSched(pthread_t &thread, int prioPercentage); 
void systemCall(const std::string& cmd);
std::string exec(const std::string& cmd);
std::string tp_to_str(std::chrono::time_point<std::chrono::system_clock> tp = std::chrono::system_clock::now());
std::chrono::time_point<std::chrono::system_clock> str_to_tp(std::string str);
float get_time_elapsed_ms(std::chrono::time_point<std::chrono::system_clock> start, std::chrono::time_point<std::chrono::system_clock> end=std::chrono::system_clock::now());
void delay_ms(uint16_t del);
bool not_csv(std::string const & value);
void get_only_csv(std::vector<std::string>& v);
void read_directory(const std::string& name, std::vector<std::string>& v);

//time:
int uSecSince(const std::chrono::time_point<std::chrono::system_clock>& tp);
int millisSince(const std::chrono::time_point<std::chrono::system_clock>& tp);

// this will approx time weighted rolling avg over 1 sec
template<class T>
void printStatsRarely(
        std::chrono::time_point<std::chrono::system_clock>& tp,
        const int desired_ms_interval,
        std::string txt,
        const T& src, bool reset=true) {
    static bool first{true};
    
    // static memebers for min/max/exp avg
    static auto max = std::numeric_limits<T>::min();
    static auto min = std::numeric_limits<T>::max();
    static auto avg = src;
    static float alpha = 1/(100.0);
    static float one_minus_alpha = 1 - alpha;

    // static members for standard dev reporting
    // assumption of type T being int/uint or float/double 
    static double std_dev = 0;
    static double mean = 0; 
    static int samp_number = 0;
    static std::deque<T> buff;

    if (first) {
        first = false;
        return;
    }
    //debug("printRarely src: %d\n", src);
    max = std::max(max,src);
    min = std::min(min,src);
    avg =  (alpha * src) + one_minus_alpha * avg; // https://en.wikipedia.org/wiki/Moving_average#Exponential_moving_average

    samp_number++;
    mean += src; // need to divide by samp number when desried
    buff.push_back(src);

    // intermitent printing function
    if (uSecSince(tp) >= desired_ms_interval*1000) {
        // this calculation is somewhat intensive
        mean /= (double) samp_number; // get mean for sample set
        for (const auto& val : buff){
            std_dev += std::pow(val - mean,2); // sum distance from mean
        }
        std_dev /= (double) samp_number; // divide by number of samples
        std_dev = std::sqrt(std_dev); // take square root

        //this isn't great because I should really handle the template format
        //I expect this will give someone grief at some pt. sorry ICM
        debug((txt+"\tmin: %d\tavg: %d\tmax: %d\t std_dev: %f\n").c_str(), min, avg, max, std_dev);
        tp = std::chrono::system_clock::now();
        if (reset) {
            first = true;
            max = std::numeric_limits<T>::min();
            min = std::numeric_limits<T>::max();
            avg = src;
            std_dev = 0;
            mean = 0; 
            samp_number = 0;
            buff.clear();
        }
    }
}
