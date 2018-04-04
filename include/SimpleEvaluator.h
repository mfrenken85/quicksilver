//
// Created by Nikolay Yakovets on 2018-02-02.
//

#ifndef QS_SIMPLEEVALUATOR_H
#define QS_SIMPLEEVALUATOR_H


#include <memory>
#include <cmath>
#include "SimpleGraph.h"
#include "RPQTree.h"
#include "Evaluator.h"
#include "Graph.h"

struct bestPlan {
    std::string executedQuery;
    uint32_t cost;
};

class SimpleEvaluator : public Evaluator {

    std::shared_ptr<SimpleGraph> graph;
    std::shared_ptr<SimpleEstimator> est;
    std::map<std::string,bestPlan> plans;

public:

    explicit SimpleEvaluator(std::shared_ptr<SimpleGraph> &g);
    ~SimpleEvaluator() = default;

    void prepare() override ;
    cardStat evaluate(RPQTree *query) override;

    bestPlan findBestPlan(std::string originalQuery, std::string query, std::shared_ptr<SimpleGraph> &graph, std::shared_ptr<SimpleEstimator> est);
    std::string preParse(std::string str,std::shared_ptr<SimpleGraph> &graph, std::shared_ptr<SimpleEstimator> est);

    void attachEstimator(std::shared_ptr<SimpleEstimator> &e);

    std::shared_ptr<SimpleGraph> evaluate_aux(RPQTree *q);
    static std::shared_ptr<SimpleGraph> project(uint32_t label, bool inverse, std::shared_ptr<SimpleGraph> &g);
    static std::shared_ptr<SimpleGraph> join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right);

    static cardStat computeStats(std::shared_ptr<SimpleGraph> &g);

};


#endif //QS_SIMPLEEVALUATOR_H
