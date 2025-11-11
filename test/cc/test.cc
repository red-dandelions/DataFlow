#include <chrono>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue> // std::priority_queue
#include <algorithm>
#include <memory>

#include "glog/logging.h"
#include "glog/stl_logging.h"

template <typename T>
struct Node {
    T value;
    std::shared_ptr<Node> prev = nullptr;
    std::shared_ptr<Node> next = nullptr;

    Node(T v) : value(v), prev(nullptr), next(nullptr) {}
    Node(T v, std::shared_ptr<Node> p, std::shared_ptr<Node> n) : value(v), prev(p), next(n) {}
};

struct Function {
    std::string operator()(std::string h) {
        return h;
    }
};

int main() {
    Function func;
    std::vector<std::string> strs = {};
    std::vector<int> arr = {};

   auto ans = func("hello");
   LOG(INFO) << "ans: " << ans;
    return 0;
}
