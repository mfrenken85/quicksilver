//
// Created by Nikolay Yakovets on 2018-01-31.
//

#ifndef QS_SIMPLEGRAPH_H
#define QS_SIMPLEGRAPH_H

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iostream>
#include <regex>
#include <fstream>
#include "Graph.h"
#include <bitset>
#include <map>


class SimpleGraph : public Graph {
public:
    std::vector<std::vector<std::pair<uint32_t,uint32_t>>> adj;
    std::vector<std::vector<std::pair<uint32_t,uint32_t>>> reverse_adj; // vertex adjacency list


protected:
    uint32_t V;
    uint32_t L;

public:

    SimpleGraph() : V(0), L(0) {};
    ~SimpleGraph() = default;
    explicit SimpleGraph(uint32_t n);

    uint32_t getNoVertices() const override ;
    uint32_t getNoEdges() const override ;
    uint32_t getNoDistinctEdges() const override ;
    uint32_t getNoLabels() const override ;

    void addEdge(uint32_t from, uint32_t to, uint32_t edgeLabel) override ;
    void readFromContiguousFile(const std::string &fileName) override ;

    void setNoVertices(uint32_t n);
    void setNoLabels(uint32_t noLabels);

    // k2tree methods (just for memory test)
    // create all the maps needed to store all data per label
    void createToK2TreeMaps(uint32_t count_labels, uint32_t count_V);
    // add label to k2 table
    void addEdgeToK2Tree(uint32_t from, uint32_t to, uint32_t edgeLabel);
    bool findEdgeToK2Tree(uint32_t from, uint32_t to, uint32_t edgeLabel, bool should);

    // table to store k2 trees by label
    std::map<uint32_t,std::bitset<100000000>> adj_k2table_bitset;

};

#endif //QS_SIMPLEGRAPH_H