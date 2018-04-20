//
// Created by Nikolay Yakovets on 2018-01-31.
//

#include "SimpleGraph.h"

SimpleGraph::SimpleGraph(uint32_t n)   {
    setNoVertices(n);
}
//SimpleGraph::~SimpleGraph() {
    // clean the memory.
    //delete tableHead;
    //delete reverse_tableHead;
//}

uint32_t SimpleGraph::getNoVertices() const {
    return V;
}

void SimpleGraph::setNoVertices(uint32_t n) {
    if (n > 0) {
        V = n;
        // normal
        adj.resize(V);
        reverse_adj.resize(V);
        dataType = 0;
    } else {
        V = 0;
        //linked list
        dataType = 1;
        tableHead = createTableHead();
        reverse_tableHead = createTableHead();
        //tablePointers[0] = tableHead;
        //reverse_tablePointers[0] = reverse_tableHead;
    }
}

uint32_t SimpleGraph::getNoEdges() const {
    if (dataType == 0) {
        uint32_t sum = 0;
        for (const auto & l : adj)
            sum += l.size();
        return sum;
    } else {
        return tableHead->E;
    }

}

// sort on the second item in the pair, then on the first (ascending order)
bool sortPairs(const std::pair<uint32_t,uint32_t> &a, const std::pair<uint32_t,uint32_t> &b) {
    if (a.second < b.second) return true;
    if (a.second == b.second) return a.first < b.first;
    return false;
}

uint32_t SimpleGraph::getNoDistinctEdges() const {

    if (dataType == 0) {
        uint32_t sum = 0;

        for (auto sourceVec : adj) {

            std::sort(sourceVec.begin(), sourceVec.end(), sortPairs);

            uint32_t prevTarget = 0;
            uint32_t prevLabel = 0;
            bool first = true;

            for (const auto &labelTgtPair : sourceVec) {
                if (first || !(prevTarget == labelTgtPair.second && prevLabel == labelTgtPair.first)) {
                    first = false;
                    sum++;
                    prevTarget = labelTgtPair.second;
                    prevLabel = labelTgtPair.first;
                }
            }
        }

        return sum;
    } else {
        return tableHead->E;
    }
}

uint32_t SimpleGraph::getNoLabels() const {
    return L;
}

void SimpleGraph::setNoLabels(uint32_t noLabels) {
    L = noLabels;
}

std::string SimpleGraph::getQuery() const {
    return dataQuery;
}

void SimpleGraph::setQuery(std::string s) {
    dataQuery = s;
}

void SimpleGraph::addEdge(uint32_t from, uint32_t to, uint32_t edgeLabel) {
    if(from >= V || to >= V || edgeLabel >= L)
        throw std::runtime_error(std::string("Edge data out of bounds: ") +
                                 "(" + std::to_string(from) + "," + std::to_string(to) + "," +
                                 std::to_string(edgeLabel) + ")");
    adj[from].emplace_back(std::make_pair(edgeLabel, to));
    reverse_adj[to].emplace_back(std::make_pair(edgeLabel, from));
}

void SimpleGraph::readFromContiguousFile(const std::string &fileName) {

    std::string line;
    std::ifstream graphFile { fileName };

    std::regex edgePat (R"((\d+)\s(\d+)\s(\d+)\s\.)"); // subject predicate object .
    std::regex headerPat (R"((\d+),(\d+),(\d+))"); // noNodes,noEdges,noLabels

    // parse the header (1st line)
    std::getline(graphFile, line);
    std::smatch matches;
    if(std::regex_search(line, matches, headerPat)) {
        uint32_t noNodes = (uint32_t) std::stoul(matches[1]);
        uint32_t noLabels = (uint32_t) std::stoul(matches[3]);

        setNoVertices(noNodes);
        setNoLabels(noLabels);
    } else {
        throw std::runtime_error(std::string("Invalid graph header!"));
    }

    // parse edge data
    while(std::getline(graphFile, line)) {

        if(std::regex_search(line, matches, edgePat)) {
            uint32_t subject = (uint32_t) std::stoul(matches[1]);
            uint32_t predicate = (uint32_t) std::stoul(matches[2]);
            uint32_t object = (uint32_t) std::stoul(matches[3]);

            addEdge(subject, object, predicate);
        }
    }

    graphFile.close();
}

// A C Program to demonstrate adjacency list representation of graphs


SimpleGraph::AdjTable* SimpleGraph::createTableHead() {
    AdjTable *table;
    table = new AdjTable;
    table->next = 0;
    table->label = 0;
    table->V = 0;
    table->E = 0;
    table->head = 0;
    return table;
}
void SimpleGraph::addEdgeLL(uint32_t from, uint32_t to, uint32_t edgeLabel) {
    addEdgeToLinkedList(from, to, tableHead);
    addEdgeToLinkedList(to, from, reverse_tableHead);
}

void SimpleGraph::addEdgeToLinkedList(uint32_t from, uint32_t to, AdjTable* table) // add v to linked list
{
    AdjTable *conductorTable;  // This will point to each node as it traverses the list
    AdjList *conductorList;  // This will point to each node as it traverses the list
    AdjListNode *conductorNode;  // This will point to each node as it traverses the list
    // previous
    AdjList *previousList;
    AdjListNode *previousNode;

    //labels//////////////////////////////

    conductorTable = table; // The conductor points to the first node

    bool first_itter;
    // from /////////////////////////////////////////////
    if (conductorTable->head == 0) { //head empty, create new AdjList
        conductorTable->head = new AdjList;
        conductorList = conductorTable->head;
        conductorList->from = from;
        conductorList->next = 0;
        conductorList->head = 0;
        conductorTable->V++;
    } else { // not empty
        first_itter = true;
        conductorList = conductorTable->head;
        previousList = conductorTable->head;
        if ( conductorList != 0 ) {
            while ( conductorList->next != 0 && conductorList->from < from) {
                conductorList = conductorList->next;
                if (!first_itter) {  //dont increase previousTable on the first itter of the while loop
                    previousList = previousList->next;
                } else {
                    first_itter = false; // first itteration done
                }

            }
        }
        if (conductorList->from < from) {    // Label not found, create new label
            conductorList->next = new AdjList; // Creates a node at the end of the list
            conductorList = conductorList->next;    // Points to that node
            conductorList->next = 0;            // Prevents it from going any further
            conductorList->from = from;
            conductorList->head = 0;
            conductorTable->V++;
        }

        if (conductorList->from > from) {    // Label not found, create new label
            if (previousList != conductorList) {
                previousList->next = new AdjList; //
                previousList = previousList->next;
            } else {
                conductorTable->head = new AdjList; //
                previousList = conductorTable->head;
            }
            previousList->next = conductorList;            // glues the linked list back together
            previousList->from = from;
            previousList->head = 0;
            conductorList = previousList;
            conductorTable->V++;
        }
    }

    // to /////////////////////////////////////////////

    if (conductorList->head == 0) { //head empty, create new AdjList
        conductorList->head = new AdjListNode;
        conductorNode = conductorList->head;
        conductorNode->to = to;
        conductorNode->next = 0;
        conductorTable->E++;
    } else { // not empty
        first_itter = true;
        conductorNode = conductorList->head;
        previousNode = conductorList->head;
        if ( conductorNode != 0 ) {
            while ( conductorNode->next != 0 && conductorNode->to != to) {
                conductorNode = conductorNode->next;
                if (!first_itter) {  //dont increase previousTable on the first itter of the while loop
                    previousNode = previousNode->next;
                } else {
                    first_itter = false; // first itteration done
                }
            }
        }
        if (conductorNode->to < to) {    // Label not found, create new label
            conductorNode->next = new AdjListNode; // Creates a node at the end of the list
            conductorNode = conductorNode->next;    // Points to that node
            conductorNode->next = 0;            // Prevents it from going any further
            conductorNode->to = to;
            conductorTable->E++;
        }

        if (conductorNode->to > to) {    // Label not found, create new label
            if (previousNode != conductorNode) {
                previousNode->next = new AdjListNode; //
                previousNode = previousNode->next;
            } else {
                conductorList->head = new AdjListNode; //
                previousNode = conductorList->head;
            }
            previousNode->next = conductorNode;            // glues the linked list back together
            previousNode->to = to;
            conductorTable->E++;
        }
    }

}



// Driver program to test above functions
