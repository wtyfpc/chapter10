#pragma once

#include <algorithm>
#include <map>
#include <string>
#include <vector>

class Percentile { // ==wty== 统计PCT99水平的协程数量，辅助释放多余的空闲协程栈空间
 public:
  Percentile() = default;
  Percentile(size_t maxStatDataLen) : max_stat_data_len_(maxStatDataLen) {}
  void Stat(std::string key, int64_t value) {
    auto iter = origin_stat_data_.find(key);
    if (iter == origin_stat_data_.end()) {
      origin_stat_data_index_[key] = 0;
      origin_stat_data_[key] = std::vector<int64_t>();
      iter = origin_stat_data_.find(key);
      iter->second.reserve(max_stat_data_len_);
    }
    if (iter->second.size() < max_stat_data_len_) {
      iter->second.push_back(value);
      return;
    }
    iter->second[origin_stat_data_index_[key]] = value;                                      // 循环数组
    origin_stat_data_index_[key] = (origin_stat_data_index_[key] + 1) % max_stat_data_len_;  // 更新下标
  }
  bool GetPercentile(std::string key, double pct, double &pctValue) {
    auto iter = origin_stat_data_.find(key);
    if (iter == origin_stat_data_.end()) {
      return false;
    }
    if (iter->second.size() < max_stat_data_len_) {
      return false;
    }
    std::vector<int64_t> temp = iter->second; 
    std::sort(temp.begin(), temp.end()); // ==wty== 升序排列
    double x = (temp.size() - 1) * pct; // ==wty== 假设有100个数据，衡量标准取pct99。那么需要找到第99个数据的下标idx=98：99*0.99 = 98.01
    int32_t i = (int32_t)x; // ==wty== 取整数部分，即为要找的数据的下标
    double j = x - i; // ==wty== 得到小数部分
    pctValue = (1 - j) * temp[i] + j * temp[i + 1]; // ==wty== 为什么不直接返回 temp[i]?
    return true;
  }

 private:
  size_t max_stat_data_len_{1024};                                // 原始统计数据最大长度
  std::map<std::string, std::vector<int64_t>> origin_stat_data_;  // 原始统计数据
  std::map<std::string, int32_t> origin_stat_data_index_;         // 原始统计数据索引
};
