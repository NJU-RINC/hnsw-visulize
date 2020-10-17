#include "hnsw.h"

#include <cstring>
#include <cmath>
#include <random>
#include <cassert>
#include <map>
#include <unordered_set>
#include <vector>
#include <fstream>

#include "data_interface.h"
#include "utils.h"

namespace ann
{

hnsw::hnsw(const one_data* d) {
    one_data_ = d;
    data_sz_ = one_data_->data_size();
    data_ = nullptr;
    neibor_link_ = nullptr;

    gen_.seed(100);
}

hnsw::hnsw(const one_data* d, idx_t max_sz, int m, int ef_con) :
        one_data_(d), size_(max_sz), data_sz_(d->data_size()),
        graph_ofs("graph.log") {
    used_sz_ = 0;
    data_ = new char[size_ * data_sz_];

    m_ = m;
    m_max_ = m_;
    m_max0_ = 2 * m_;
    link_sz_ = sizeof(neb_sz_t) + sizeof(neb_id_t) * m_max_;
    link_sz0_ = sizeof(neb_sz_t) + sizeof(neb_id_t) * m_max0_;
    // total size will be stored ahead of link_sz list
    neibor_link_ = (char**)new char[size_ * sizeof(void*)];
    std::memset(neibor_link_, 0, sizeof(void*) * size_);

    ef_construction_ = ef_con;
    ef_search_ = 10;

    entor_point_ = 0;
    max_level_ = -1;

    //std::random_device rd;
    //gen_.seed(rd());
    gen_.seed(100);
}

hnsw::~hnsw() {
    delete [] data_;
    for (idx_t i = 0; i < size_; ++i) {
        if (neibor_link_[i] != nullptr) delete [] neibor_link_[i];
    }
    delete [] neibor_link_; 
}

hnsw::idx_t hnsw::insert(const char* data) {
    if (used_sz_ ==  size_) {
        idx_t new_sz = 1;
        while (new_sz <= size_) new_sz <<= 1;
        char* new_data = new char[new_sz * data_sz_];
        char** new_link = (char**)new char[new_sz * sizeof(void*)];
        std::memcpy(new_data, data_, size_ * data_sz_);
        std::memset(new_data + size_ * data_sz_, 0, (new_sz - size_) * data_sz_);
        std::memcpy(new_link, neibor_link_, size_ * sizeof(void*));
        std::memset(new_link + size_, 0, (new_sz - size_) * sizeof(void*));
        delete [] data_;
        delete [] neibor_link_;
        data_ = new_data;
        neibor_link_ = new_link;
        size_ = new_sz;
    }
    assert(used_sz_ < size_);
    auto data_pos = used_sz_++;
    std::memcpy(data_ + data_pos * data_sz_, data, data_sz_);

    int level = new_level();
    debug_print("insert level ", level);

    {
        graph_ofs << "add " << data_pos << " " << level;
        float* pf = (float*)(data);
        graph_ofs << " " << *pf++;
        graph_ofs << "," << *pf;
    }

    assert(neibor_link_[data_pos] == 0);
    neibor_link_[data_pos] = new_linklist(level);

    if (max_level_ == -1) {
        max_level_ = level;
        entor_point_ = data_pos;
        graph_ofs << std::endl;
        return data_pos;
    }
    
    auto ep = entor_point_;
    debug_print("search high level");
    for (int i = max_level_; i >= level + 1; --i) {
        auto neb = search_layer(data_pos, ep, 1, i);
        ep = neb.begin()->second;
    }
    debug_print("search low level");
    std::vector<std::string> line_logs;
    for (auto i = std::min(max_level_, level); i >= 0; --i) {
        auto m_max = i == 0 ? m_max0_ : m_max_;
        auto neb = search_layer(data_pos, ep, ef_construction_, i);
        ep = neb.begin()->second;
        auto neb_choosen = select_neighbors(data_pos, neb, m_, i);
        debug_print("level ", i, ", number of neb ", neb.size(),
            ", neb choosen ", neb_choosen.size());
        *p2linklist_nebsz(data_pos, i) = neb_choosen.size();
        auto p_idx = p2linklist_neb(data_pos, i);
        std::string one_log(" ");
        for (const auto& each : neb_choosen) {
            one_log += std::to_string(each.second) + ",";
            *p_idx++ = each.second;
            auto p_neb = p2linklist_neb(each.second, i);
            neb_sz_t* p_nebsz = p2linklist_nebsz(each.second, i);
            if (*p_nebsz < m_max) {
                *(p_neb + *p_nebsz) = data_pos;
                ++(*p_nebsz);
            }
            else {
                std::map<double, idx_t> his_neb_dist;
                double dist = 0.0;
                {
                    const char* p1 = data_ + each.second * data_sz_;
                    const char* p2 = data_ + data_pos * data_sz_;
                    dist = one_data_->dist(p1, p2);
                }
                his_neb_dist.insert(std::make_pair(dist, data_pos));
                const char* p1 = data_ + each.second * data_sz_;
                for (size_t k = 0; k < m_max; ++k) {
                    const char* p2 = data_ + (*p_neb) * data_sz_;
                    double d = one_data_->dist(p1, p2);
                    his_neb_dist.insert(std::make_pair(d, *p_neb++)); 
                }
                auto his_neb_choosen = select_neighbors(each.second, his_neb_dist, m_max, i);
                p_neb = p2linklist_neb(each.second, i);
                auto ct = *p_nebsz - *p_nebsz;  
                for (const auto& his_each : his_neb_choosen) {
                    *p_neb++ = his_each.second;
                    ++ct;
                }
                *p_nebsz = ct;
            }
        }
        line_logs.emplace_back(std::move(one_log));
    } // end for

    if (level > max_level_) {
        max_level_ = level;
        entor_point_ = data_pos;
    }

    {
        for (auto p = line_logs.rbegin(); p != line_logs.rend(); ++p) {
            graph_ofs << *p;
        }
        graph_ofs << std::endl;
    }

    return data_pos;
}

std::map<double, size_t> hnsw::search(const char* query_data, int k) const {
    std::map<double, size_t> result;
    if (used_sz_ == 0) return result;
    int ef = std::max(k, ef_search_);
    auto entor_point = entor_point_;
    for (int i = max_level_; i > 0; --i) {
        auto neb = search_layer(query_data, entor_point, 1, i);
        entor_point = neb.begin()->second;
    }
    return search_layer(query_data, entor_point, ef, 0);
}

void hnsw::set_ef_search(int val) const {
    assert(val > 0);
    ef_search_ = val;
}

void hnsw::load(std::string fname) {
    std::ifstream file(fname, std::ios::binary);
    file.read((char*)&size_, sizeof(size_));
    file.read((char*)&data_sz_, sizeof(data_sz_));
    file.read((char*)&used_sz_, sizeof(used_sz_));
    file.read((char*)&m_, sizeof(m_));
    file.read((char*)&m_max_, sizeof(m_max_));
    file.read((char*)&m_max0_, sizeof(m_max0_));
    file.read((char*)&link_sz_, sizeof(link_sz_));
    file.read((char*)&link_sz0_, sizeof(link_sz0_));
    file.read((char*)&ef_construction_, sizeof(ef_construction_));
    file.read((char*)&ef_search_, sizeof(ef_search_));
    file.read((char*)&entor_point_, sizeof(entor_point_));
    file.read((char*)&max_level_, sizeof(max_level_));

    assert(data_sz_ == one_data_->data_size());
    if (data_) delete data_;
    if (neibor_link_) delete neibor_link_;
    data_ = new char[size_ * data_sz_];
    neibor_link_ = (char**)new char[size_ * sizeof(void*)];

    file.read(data_, used_sz_ * data_sz_);
    for (idx_t i = 0; i < used_sz_; ++i) {
        level_t level = 0;
        file.read((char*)&level, sizeof(level_t));
        neibor_link_[i] = new_linklist(level);
        auto left_size = link_sz0_ + level * link_sz_;
        file.read(neibor_link_[i] + sizeof(level_t), left_size);
    }
    file.close();
}

void hnsw::save(std::string fname) const {
    std::ofstream file(fname, std::ios::binary);
    file.write((const char*)&size_, sizeof(size_));
    file.write((const char*)&data_sz_, sizeof(data_sz_));
    file.write((const char*)&used_sz_, sizeof(used_sz_));
    file.write((const char*)&m_, sizeof(m_));
    file.write((const char*)&m_max_, sizeof(m_max_));
    file.write((const char*)&m_max0_, sizeof(m_max0_));
    file.write((const char*)&link_sz_, sizeof(link_sz_));
    file.write((const char*)&link_sz0_, sizeof(link_sz0_));
    file.write((const char*)&ef_construction_, sizeof(ef_construction_));
    file.write((const char*)&ef_search_, sizeof(ef_search_));
    file.write((const char*)&entor_point_, sizeof(entor_point_));
    file.write((const char*)&max_level_, sizeof(max_level_));
    file.write(data_, used_sz_ * data_sz_);
    for (idx_t i = 0; i < used_sz_; ++i) {
        file.write(neibor_link_[i], link_len(i));
    }
    file.close();
}

std::map<double, hnsw::idx_t> hnsw::search_layer(
    idx_t query_point, idx_t enter_point, int ef, int level) const {
    debug_print("query point ", query_point, " enter_point ", enter_point);
    const char* query_data = data_ + data_sz_ * query_point;
    return std::move(search_layer(query_data, enter_point, ef, level));
}

std::map<double, hnsw::idx_t> hnsw::search_layer(
    const char* query_data, idx_t enter_point, int ef, int level) const {
    //TODO make sure that the graph is not empty
    debug_print("search at level ", level);
    std::unordered_set<idx_t> visited;
    std::map<double, idx_t> candidates;
    std::map<double, idx_t> result;
    double dist = one_data_->dist(query_data, data_ + enter_point * data_sz_);
    candidates.insert(std::make_pair(dist, enter_point));
    result.insert(std::make_pair(dist, enter_point));
    auto ans = result;
    visited.insert(enter_point);
    while (!candidates.empty()) {
        auto p = candidates.begin();
        debug_print("candidate size, first element, ", candidates.size(), ", ", p->second);
        debug_print("candi dist min, result dist max, ", p->first, ", ", result.rbegin()->first);
        if (p->first > result.rbegin()->first) break;
        int neb_count = *p2linklist_nebsz(p->second, level);
        debug_print("number of neighbor ", neb_count);
        auto p_neb = p2linklist_neb(p->second, level);
        candidates.erase(p);
        for (int i = 0; i < neb_count; ++i) {
            debug_print("neb all, now, ", neb_count, ", ", i);
            idx_t neb = *(p_neb + i);
            if (visited.find(neb) == visited.end()) {
                double d = one_data_->dist(query_data, data_ + neb * data_sz_);                
                if (d < result.rbegin()->first || result.size() < ef) {
                    result.insert(std::make_pair(d, neb));
                    candidates.insert(std::make_pair(d, neb));
                    //ans.insert(std::make_pair(d, neb));
                    if (result.size() > ef) result.erase(result.rbegin()->first);
                }
                visited.insert(neb);
            }
        }
    }
    return result;
    //return ans;
}

std::map<double, hnsw::idx_t> hnsw::select_neighbors(
        idx_t query_point, const std::map<double, idx_t>& candidates, int num, int level,
        bool extend_candidates, bool keep_pruned_connection) const {
    std::map<double, idx_t> result;
    std::map<double, idx_t> left_candidates;
    if (extend_candidates) {
        for (const auto& each : candidates) {
            left_candidates.insert(each);
            int neb_count = *p2linklist_nebsz(each.second, level);
            auto p_neb = p2linklist_neb(each.second, level);
            for (int i = 0; i < neb_count; ++i) {
                double dist = one_data_->dist(data_ + query_point * data_sz_,
                    data_ + *(p_neb + i) * data_sz_);
                left_candidates.insert(std::make_pair(dist, *(p_neb + i)));
            }
        }
    }
    else {
        left_candidates = std::move(candidates);
    }

    std::map<double, idx_t> pruned_connection;
    while (!left_candidates.empty() && result.size() < num) {
        auto ptr = left_candidates.begin();
        bool ptr_good = true;
        for (auto& each : result) {
            double dist = one_data_->dist(
                data_ + data_sz_ * ptr->second, data_ + data_sz_ * each.second);
            if (dist < ptr->first) {
                ptr_good = false;
                break;
            }
        }
        if (ptr_good) {
            result.insert(*ptr);
        }
        else {
            pruned_connection.insert(*ptr);
        }
        left_candidates.erase(ptr);
    }

    if (keep_pruned_connection) {
        while (result.size() < num && pruned_connection.size()) {
            result.insert(*pruned_connection.begin());
            pruned_connection.erase(pruned_connection.begin());
        }
    }
    
    return result;
}

} // namespace ann
