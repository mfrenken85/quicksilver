//
// Created by Nikolay Yakovets on 2018-02-01.
//
// Mark:
// See chapter 13.3 of the book for details:
// Create a histogram, evaluate each query
// for its opperations and implement the given
// calculations of the book.

#include "SimpleGraph.h"
#include "SimpleEstimator.h"

SimpleEstimator::SimpleEstimator(std::shared_ptr<SimpleGraph> &g){

    // works only with SimpleGraph
    graph = g;
}

void SimpleEstimator::prepare() {

    // do your prep here
    /*
     * nr,the number of tuples in the relation r.
     * br,the number of blocks containing tuples of relation r.
     * lr,the size of a tuple of relation r in bytes.
     * fr , the blocking factor of relation r —
     *   that is, the number of tuples of relation r that fit into one block.
     * V(A,r),the number of distinct values that appear in the relation r for attribute A.
     *   This value is the same as the size of A(r).If A is a key for relation r,V(A,r) is nr .
     */

    auto nr = graph->getNoEdges();
    auto V = graph->getNoVertices();
    std::cout<< std::endl;
    std::cout << "no of tuples in relation: " << nr;
    std::cout<< std::endl;
    std::cout << "no of V in relation: " << V;
    std::cout<< std::endl;

}

cardStat SimpleEstimator::estimate(RPQTree *q) {

    // perform your estimation here
    std::cout<< std::endl;
    std::cout << "testing left: ";
    q->left->print();
    std::cout<< std::endl;
    std::cout << "testing right: ";
    q->right->print();

    return cardStat {0, 0, 0};
}