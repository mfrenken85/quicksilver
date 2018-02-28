//
// Created by Nikolay Yakovets on 2018-02-01.
//

#ifndef QS_SIMPLEESTIMATOR_H
#define QS_SIMPLEESTIMATOR_H

#include <memory>
#include <cmath>
#include "Estimator.h"
#include "SimpleGraph.h"
#include "RPQTree.h"
#include "Graph.h"

class SimpleEstimator : public Estimator {

    std::uint32_t  previousLabel;
    std::uint32_t  noIn;
    std::uint32_t  noPaths;
    std::uint32_t  noOut;
    // std::vector<std::pair<uint32_t,uint32_t>> estedges;
    std::shared_ptr<SimpleGraph> graph;
    std::vector<std::pair<uint32_t,uint32_t>> labelcounter;
    std::vector<std::pair<uint32_t,std::vector<std::pair<uint32_t,uint32_t>>>> groupededges;
    std::vector<std::pair<uint32_t,std::vector<std::pair<uint32_t,uint32_t>>>> groupededgesinverse;

public:
    explicit SimpleEstimator(std::shared_ptr<SimpleGraph> &g);
    ~SimpleEstimator() = default;

    void prepare() override ;
    cardStat estimate(RPQTree *q) override ;

    //std::shared_ptr<SimpleGraph> estimator_aux(RPQTree *q);
    void estimator_aux(RPQTree *q);
    //std::shared_ptr<SimpleGraph> calculate(uint32_t currentLabel, bool inverse, std::shared_ptr<SimpleGraph> &g);
    void calculate(uint32_t currentLabel, bool inverse);
    std::shared_ptr<SimpleGraph> join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right);

    cardStat computeStats();

};


#endif //QS_SIMPLEESTIMATOR_H
