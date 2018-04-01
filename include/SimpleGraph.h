//
// Created by Nikolay Yakovets on 2018-01-31.
//

#ifndef QS_SIMPLEGRAPH_H
#define QS_SIMPLEGRAPH_H

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <map>
#include <iostream>
#include <regex>
#include <fstream>
#include "Graph.h"

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

    // linked list


    struct AdjListNode
    {
        uint32_t to;
        struct AdjListNode *next; // pointer
    };

// A structure to represent an adjacency list
    struct AdjList
    {
        uint32_t from;
        struct AdjListNode *head;  // pointer to head node of list
        struct AdjList *next;
    };


    struct AdjTable
    {
        uint32_t label; // table name
        uint32_t V; // no vertices
        uint32_t E; // no edges
        struct AdjList *head;
        struct AdjTable *next;
    };


    void addEdgeToLinkedList(uint32_t from, uint32_t to, uint32_t edgeLabel, AdjTable *table, bool reverse);
    AdjTable* createTableHead();
    AdjTable* getTable(uint32_t label, bool reverse); // get from tablePointers
    void setTable(uint32_t label, AdjTable* table, bool reverse); // set pointers to tablePointers


    // linked list
    AdjTable *tableHead;
    AdjTable *reverse_tableHead;

    // tablePointers[label, std::make_pair[adj, rev_adj]]
    std::map<uint32_t,AdjTable*> tablePointers;
    std::map<uint32_t,AdjTable*> reverse_tablePointers;


};

#endif //QS_SIMPLEGRAPH_H