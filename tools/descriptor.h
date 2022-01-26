#pragma once
#include <stdio.h>
#include <string>
#include <fcntl.h>
//quick wrapper for FDs
class Descriptor {
public:
    Descriptor(std::string filename)
        : filename_(filename) {
            printf("Descriptor file: %s", filename_.c_str());
        }
    void open() {
        printf("opening: %s\r\n", filename_.c_str());
        fd_ = fopen(filename_.c_str(), "w");
        if(fd_==NULL) {
            perror(filename_.c_str());
            printf("fd is null\n");
        } else {
            printf("fd opened\n");
        }

    }
    void close() {
        if (fd_!=NULL) {
            fclose(fd_);
            fd_=NULL;
        }
    }
    void flush() {
        if (fd_ != NULL) {
            fflush(fd_);
        }
    }

    ~Descriptor() { // this should probably have better guards
        close();
    }

    FILE* getFd() {
        if (fcntl(fileno(fd_), F_GETFD)<0) {
            printf("Descriptor fd failed is not open; attempting now\n");
        open();
        if (fd_<0) {
            printf("Descriptor fd failed to reopen\n");
            return NULL;
        }
    }
        return fd_;
    }

private:
    FILE* fd_=NULL;
    std::string filename_;
};
