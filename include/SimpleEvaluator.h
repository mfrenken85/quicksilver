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

    // the data structure used to find the best plan.
    // the best plan is => intermediatePlans[original query]
    std::map<std::string,bestPlan> intermediatePlans;

    // cache the executed bestPlans (the whole plan).
    std::map<std::string,bestPlan> cachedBestPlans;

public:

    explicit SimpleEvaluator(std::shared_ptr<SimpleGraph> &g);
    ~SimpleEvaluator();

    void prepare() override ;
    cardStat evaluate(RPQTree *query) override;

    bestPlan findBestPlanDynamic(std::string query, std::shared_ptr<SimpleGraph> &graph,
                                 std::shared_ptr<SimpleEstimator> &est);
    bestPlan findBestPlanGreedy(std::string query, std::shared_ptr<SimpleGraph> &graph,
                                std::shared_ptr<SimpleEstimator> &est);
    std::string preParse(std::string str,std::shared_ptr<SimpleGraph> &graph, std::shared_ptr<SimpleEstimator> &est);

    void attachEstimator(std::shared_ptr<SimpleEstimator> &e);

    std::shared_ptr<SimpleGraph> evaluate_aux(RPQTree *q, std::map<std::string,bestPlan> plan, uint32_t noV);


    static std::shared_ptr<SimpleGraph> project(uint32_t label, bool inverse, std::shared_ptr<SimpleGraph> &g, std::map<std::string,bestPlan> plan, uint32_t noV);
    static std::shared_ptr<SimpleGraph> v_project(uint32_t label, bool inverse, std::shared_ptr<SimpleGraph> &g, std::map<std::string,bestPlan> plan, uint32_t noV);
    static std::shared_ptr<SimpleGraph> ll_project(uint32_t label, bool inverse, std::shared_ptr<SimpleGraph> &g, std::map<std::string,bestPlan> plan, uint32_t noV);

    static std::shared_ptr<SimpleGraph> join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right, std::map<std::string,bestPlan> plan, uint32_t noV);
    static std::shared_ptr<SimpleGraph> vv_join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right, std::map<std::string,bestPlan> plan, uint32_t noV);
    static std::shared_ptr<SimpleGraph> vl_join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right, std::map<std::string,bestPlan> plan, uint32_t noV);
    static std::shared_ptr<SimpleGraph> lv_join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right, std::map<std::string,bestPlan> plan, uint32_t noV);
    static std::shared_ptr<SimpleGraph> ll_join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right, std::map<std::string,bestPlan> plan, uint32_t noV);


    static cardStat computeStats(std::shared_ptr<SimpleGraph> &g);
    static cardStat ll_computeStats(std::shared_ptr<SimpleGraph> &g);

};


#endif //QS_SIMPLEEVALUATOR_H
