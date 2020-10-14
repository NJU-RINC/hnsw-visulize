#ifndef __HNSW_H__
#define __HNSW_H__

/// file: hnsw.h
/// author: fengshanliuf@gmail.com
/// date: 2019.6.12

#include <cstdint>
#include <vector>
#include <string>
#include <map>
#include <random>
#include <cmath>

namespace ann
{

class one_data;

class hnsw
{
public:
    using idx_t = std::uint64_t;
    using level_t = std::uint8_t;
    using neb_id_t = std::uint32_t;
    using neb_sz_t = std::uint8_t;

private:
    const one_data* one_data_;
    idx_t size_;
    idx_t data_sz_;
    idx_t used_sz_;
    char* data_;
    
    int m_;
    int m_max_;
    int m_max0_;
    idx_t link_sz_;
    idx_t link_sz0_;
    char** neibor_link_;

    int ef_construction_;   // control quality of graph
    mutable int ef_search_;         // control quality of search result

    idx_t entor_point_;
    int max_level_;

    std::mt19937 gen_;

public:
    hnsw(const one_data*);
    hnsw(const one_data*, idx_t, int, int);
    ~hnsw();

    hnsw(const hnsw&) = delete;
    hnsw& operator=(const hnsw&) = delete;
    
    idx_t insert(const char*);

    // query data, number K
    std::map<double, size_t> search(const char*, int) const;

    // to ballance recall and speed
    void set_ef_search(int) const;

    // Save and load information of the graph, which changes over time
    // Include: cur_pos_, entor_point_, max_level_
    //          data_, neibor_link_ 
    // load will allocate new link memory
    // save will not free memory
    void load(std::string);
    void save(std::string) const;

private:
    // query point, entor point, ef, level
    std::map<double, idx_t> search_layer(idx_t, idx_t, int, int) const;
    std::map<double, idx_t> search_layer(const char*, idx_t, int, int) const;
    // query point, candidate, choose number, level

    std::map<double, idx_t> select_neighbors(
        idx_t, const std::map<double, idx_t>&, int, int,
        bool extend_candidates = false, bool keep_pruned_connection = false) const;
        
    level_t new_level() {
        double level_norm_factor = 1 / std::log(1.0 * m_);
        std::uniform_real_distribution<> uni_d(0.0, 1.0);
        return -std::log(uni_d(gen_)) * level_norm_factor;
    }

    char* new_linklist(level_t level) const {
        char* result = new char[sizeof(level_t) + link_sz0_ + level * link_sz_];
        *((level_t*)result) = level;
        *((neb_sz_t*)(result + sizeof(level_t))) = 0;
        for(level_t i = 0; i < level; ++i) {
            *((neb_sz_t*)(
            result + sizeof(level_t) + link_sz0_ + i * link_sz_)) = 0;
        }
        return result;
    }

    level_t* p2linklist_level(idx_t idx) const {
        return (level_t*)neibor_link_[idx];
    }

    neb_sz_t* p2linklist_nebsz(idx_t idx, level_t level) const {
        if (level == 0) {
            return (neb_sz_t*)(neibor_link_[idx] + sizeof(level_t));
        }
        else {
            return (neb_sz_t*)(neibor_link_[idx] + sizeof(level_t) + 
                link_sz0_ + (level - 1) * link_sz_);
        }
    } 

    neb_id_t* p2linklist_neb(idx_t idx, level_t level) const {
        return (neb_id_t*)((char*)p2linklist_nebsz(idx, level) + sizeof(neb_sz_t));
    }

    int link_len(idx_t idx) const {
        auto level = *p2linklist_level(idx);
        return (sizeof(level_t) + link_sz0_ + link_sz_ * level); 
    }
};

} // namespace ann

#endif // __HNSW_H__
