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

    uint32_t previousLabel;
    cardStat cardStat1{};
    std::shared_ptr<SimpleGraph> graph;
    std::vector<std::pair<uint32_t,std::pair<uint32_t,uint32_t>>> edgeDistVertCount;
    std::vector<std::pair<std::pair<uint32_t,uint32_t>, std::vector<uint32_t>>> edgeCountMatrix;
    std::vector<std::pair<std::pair<uint32_t,uint32_t>, std::vector<uint32_t>>> edgeCountMatrixInverse;
    std::vector<std::pair<uint32_t,std::vector<std::pair<uint32_t,uint32_t>>>> groupededges;
    std::vector<std::pair<uint32_t,std::vector<std::pair<uint32_t,uint32_t>>>> groupededgesinverse;

public:
    explicit SimpleEstimator(std::shared_ptr<SimpleGraph> &g);
    ~SimpleEstimator() = default;

    void prepare() override ;
    cardStat estimate(RPQTree *q) override ;

    std::vector<uint32_t> calculateIntersection(std::vector<std::pair<uint32_t,uint32_t>> v1, std::vector<std::pair<uint32_t,uint32_t>> v2);
    void estimator_aux(RPQTree *q);
    void calculate(uint32_t currentLabel, bool inverse);
    std::shared_ptr<SimpleGraph> join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right);
};


#endif //QS_SIMPLEESTIMATOR_H
