//
// Created by Nikolay Yakovets on 2018-02-01.
//

#ifndef QS_SIMPLEESTIMATOR_H
#define QS_SIMPLEESTIMATOR_H

#include <memory>
#include <cmath>
#include <map>
#include <set>
#include "Estimator.h"
#include "SimpleGraph.h"
#include "RPQTree.h"
#include "Graph.h"

class SimpleEstimator : public Estimator {

    std::shared_ptr<SimpleGraph> graph;

    std::set<uint32_t> setLabels;
    std::map<uint32_t,uint32_t> histLabels;
    std::map<uint32_t,uint32_t> histIn;
    std::map<uint32_t,uint32_t> histOut;

    std::map<uint32_t,cardStat> labelCardStats;
    std::vector<std::pair<uint32_t, char>> parsedQuery;

public:
    explicit SimpleEstimator(std::shared_ptr<SimpleGraph> &g);
    ~SimpleEstimator() = default;

    void prepare() override ;
    cardStat estimate(RPQTree *q) override;
    cardStat estimateQuery(std::vector<std::pair<uint32_t, char>> pq);

    void estimator_aux(RPQTree *q);
    cardStat reverse(cardStat card);

    void prepare_default();
};


#endif //QS_SIMPLEESTIMATOR_H
