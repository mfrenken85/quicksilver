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

    cardStat cardStat1{};
    cardStat cardStat2{};
    std::shared_ptr<SimpleGraph> graph;

    std::set<uint32_t> setLabels;
    // for each element, it has the following format: <label, <left distinct vertices count, right distinct vertices count>>.
    std::vector<std::pair<uint32_t,std::pair<uint32_t,uint32_t>>> edgeDistVertCount;
    // for each element, it has the following format: <label, list of edges with this label>.
    std::vector<std::pair<uint32_t,std::vector<std::pair<uint32_t,uint32_t>>>> groupededges;

    std::set<uint32_t> setInOutLabels;
    std::map<uint32_t,uint32_t> histLabels;
    std::map<uint32_t,uint32_t> histIn;
    std::map<uint32_t,uint32_t> histOut;

    std::map<uint32_t,cardStat> labelCardStats;
    std::vector<std::pair<uint32_t, char>> parsedQuery;

public:
    explicit SimpleEstimator(std::shared_ptr<SimpleGraph> &g);
    ~SimpleEstimator() = default;

    void prepare() override ;
    cardStat estimate(RPQTree *q) override ;

    void estimator_aux(RPQTree *q);
    void calculate(uint32_t currentLabel, bool inverse);
    cardStat reverse(cardStat c);
};


#endif //QS_SIMPLEESTIMATOR_H
