#include "gtest/gtest.h"
#include "gmock/gmock.h"
#define DEBUG //not sure how to get around defining here and doing a seperate file for the other code path
#include "../common_helpers.h"
#include <string>
#include <iostream>
#include <vector>
using namespace std;


void test_dir();

TEST(csv_parse_test, DEBUG_) {
    vector<string> file_vec = {"0.txt"};
    get_only_csv(file_vec);
    ASSERT_EQ(file_vec.size(),0);

    file_vec = {"0.csv"};
    get_only_csv(file_vec);
    ASSERT_EQ(file_vec.size(),1);

    file_vec = {"0.csv", "0.txt"};
    get_only_csv(file_vec);
    ASSERT_EQ(file_vec.size(),1);

    file_vec = {"0.csv", "0.txt", "liftload_data.csv"};
    get_only_csv(file_vec);
    ASSERT_EQ(file_vec.size(),2);

    file_vec = {};
    get_only_csv(file_vec);
    ASSERT_EQ(file_vec.size(),0);

    file_vec = {"a.csv.txt"};
    get_only_csv(file_vec);
    ASSERT_EQ(file_vec.size(),0);


    // TODO: reimplement the stuff below - arni 4/20/21

    // std::string path = "/tmp/csvparse/0and1";
    // mkdir(path);
    // std::string path = "../csvparse/0and1";
    // std::string base = "liftload_data.csv";
    // vector< std::string > file_vec;
    // file_vec.clear();
    // read_directory(path, file_vec);
    // get_only_csv(file_vec);
    // ASSERT_EQ(file_vec.size(),1);

    // path = "../csvparse/1and0";
    // file_vec.clear();
    // read_directory(path, file_vec);
    // get_only_csv(file_vec);
    // ASSERT_EQ(file_vec.size(),0);

    // path = "../csvparse/1and1";
    // file_vec.clear();
    // read_directory(path, file_vec);
    // get_only_csv(file_vec);
    // ASSERT_EQ(file_vec.size(),1);

    // path = "../csvparse/1and1andll";
    // file_vec.clear();
    // read_directory(path, file_vec);
    // get_only_csv(file_vec);
    // ASSERT_EQ(file_vec.size(),1);

    // path = "../csvparse/empty";
    // file_vec.clear();
    // read_directory(path, file_vec);
    // get_only_csv(file_vec);
    // ASSERT_EQ(file_vec.size(),0);
}
