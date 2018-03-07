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

    cardStat cardStat1{};
    std::shared_ptr<SimpleGraph> graph;

    // for each element, <label, <left distinct vertices count, right distinct vertices count>>.
    std::vector<std::pair<uint32_t,std::pair<uint32_t,uint32_t>>> edgeDistVertCount;
    // for each element, <label, list of edges with this label>.
    std::vector<std::pair<uint32_t,std::vector<std::pair<uint32_t,uint32_t>>>> groupededges;

public:
    explicit SimpleEstimator(std::shared_ptr<SimpleGraph> &g);
    ~SimpleEstimator() = default;

    void prepare() override ;
    cardStat estimate(RPQTree *q) override ;

    void estimator_aux(RPQTree *q);
    void calculate(uint32_t currentLabel, bool inverse);
    std::shared_ptr<SimpleGraph> join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right);
};


#endif //QS_SIMPLEESTIMATOR_H
