//
// Created by svelja on 26.02.20.
//

#include "../gtest.h"
#include "communication/communicator.hpp"
#include "connection.hpp"
#include <arbor/common_types.hpp>
#include <arbor/domain_decomposition.hpp>
#include <arbor/recipe.hpp>
#include <vector>

using namespace arb;

class TestRecipe : public recipe {
    cell_size_type size;
    std::vector<cell_connection> connections;

public:
    TestRecipe(std::vector<cell_connection> cons) {
        connections = cons;
        size = cons.size();
    }

    cell_size_type num_cells() const override {
        return size;
    }

    cell_kind get_cell_kind(cell_gid_type i) const override {
        return cell_kind::benchmark;
    }

    util::unique_any get_cell_description(cell_gid_type gid) const override {
        return {};
    }

    std::vector<cell_connection> connections_on(cell_gid_type gid){
        std::vector<cell_connection> result_conns;
        for(auto it = connections.begin(); it!= connections.end(); it++){
            if(it->dest.gid == gid) {
                result_conns.push_back(*it);
            }
        }
        return result_conns;
    }
};

bool operator ==(cell_connection a, cell_connection b)
{
    return a.source == b.source && a.dest == b.dest && a.weight == b.weight && a.delay == b.delay;
}

bool operator !=(cell_connection a, cell_connection b)
{
    return !(a==b);
}

bool operator ==(const std::vector<cell_connection> a, const std::vector<cell_connection> b)
{
    if(a.size() != b.size()) return false;
    auto it_b = b.begin();
    for(auto it_a = a.begin(); it_a != a.end(); it_a++)
    {
        if (*it_a != *it_b) return false;
        it_b++;
    }
    return true;
}

TEST(communicator, constructor) {
    /*cell_member_type src[] = {{0, 0},
                              {0, 0},
                              {0, 0},
                              {0, 0},
                              {1, 0},
                              {1, 0},
                              {2, 0},
                              {2, 0}};
    cell_member_type dest[] = {{1, 0},
                               {1, 1},
                               {2, 2},
                               {3, 3},
                               {0, 0},
                               {3, 1},
                               {1, 0},
                               {3, 1}};
    float weight[] = {1, 2, 3, 4, 5, 6, 7, 8};
    time_type delay[] = {60, 10, 22, 16, 30, 14, 12, 20};*/

    std::vector<cell_connection> connections{cell_connection({0, 0}, {1, 0}, 1, 60),
                                             cell_connection({0, 0}, {1, 1}, 2, 10),
                                             cell_connection({0, 0}, {2, 2}, 3, 22),
                                             cell_connection({0, 0}, {3, 3}, 4, 16),
                                             cell_connection({1, 0}, {0, 0}, 5, 30),
                                             cell_connection({1, 0}, {3, 1}, 6, 14),
                                             cell_connection({2, 0}, {1, 0}, 7, 12),
                                             cell_connection({2, 0}, {3, 1}, 8, 20)};
    /*for (int i = 0; i < 8; i++) {
        connections.push_back(cell_connection(src[i], dest[i], weight[i], delay[i]));
    }*/

    domain_decomposition dom_dec;
    dom_dec.num_domains = 1;
    dom_dec.domain_id = 0;
    dom_dec.num_local_cells = 4;
    dom_dec.num_global_cells = 4;

    context ctx = make_context();
    TestRecipe testRecipe(connections);

    communicator comm = communicator(testRecipe, dom_dec, *ctx);

    EXPECT_EQ(comm.connections(), std::vector<cell_connection>({cell_connection({0, 0}, {1, 0}, 1, 60),
                                                                           cell_connection({0, 0}, {1, 1}, 2, 10),
                                                                           cell_connection({0, 0}, {2, 2}, 3, 22),
                                                                           cell_connection({0, 0}, {3, 3}, 4, 16),
                                                                           cell_connection({1, 0}, {0, 0}, 5, 30),
                                                                           cell_connection({1, 0}, {3, 1}, 6, 14),
                                                                           cell_connection({2, 0}, {1, 0}, 7, 12),
                                                                           cell_connection({2, 0}, {3, 1}, 8, 20)}));

}



/*
    TEST(range, subrange) {
    int values[] = {10, 11, 12, 13, 14, 15, 16};

    // `subrange_view` should handle offsets of different integral types sanely.
    auto sub1 = util::subrange_view(values, 1, 6u);
    EXPECT_EQ(11, sub1[0]);
    EXPECT_EQ(15, sub1.back());

    // Should be able to take subranges of subranges, and modify underlying
    // sequence.
    auto sub2 = util::subrange_view(sub1, 3ull, (short)4);
    EXPECT_EQ(1u, sub2.size());

    sub2[0] = 23;
    EXPECT_EQ(23, values[4]);

    // Taking a subrange view of a const range over non-const iterators
    // should still allow modification of underlying sequence.
    const util::range<int*> const_view(values, values+4);
    auto sub3 = util::subrange_view(const_view, std::make_pair(1, 3u));
    sub3[1] = 42;
    EXPECT_EQ(42, values[2]);
}*/