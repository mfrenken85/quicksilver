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