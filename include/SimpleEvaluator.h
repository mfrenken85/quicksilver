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

class SimpleEvaluator : public Evaluator {

    std::shared_ptr<SimpleGraph> graph;
    std::shared_ptr<SimpleEstimator> est;

public:

    explicit SimpleEvaluator(std::shared_ptr<SimpleGraph> &g);
    ~SimpleEvaluator() = default;

    void prepare() override ;
    cardStat evaluate(RPQTree *query) override ;

    void attachEstimator(std::shared_ptr<SimpleEstimator> &e);

    std::shared_ptr<SimpleGraph> ll_evaluate_aux(RPQTree *q); // needs to be removed
    std::shared_ptr<SimpleGraph> evaluate_aux(RPQTree *q);

    static std::shared_ptr<SimpleGraph> project(uint32_t label, bool inverse, std::shared_ptr<SimpleGraph> &g);
    static std::shared_ptr<SimpleGraph> v_project(uint32_t label, bool inverse, std::shared_ptr<SimpleGraph> &g);
    static std::shared_ptr<SimpleGraph> ll_project(uint32_t label, bool inverse, std::shared_ptr<SimpleGraph> &g);

    static std::shared_ptr<SimpleGraph> join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right);
    static std::shared_ptr<SimpleGraph> vv_join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right);
    static std::shared_ptr<SimpleGraph> vl_join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right);
    static std::shared_ptr<SimpleGraph> lv_join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right);
    static std::shared_ptr<SimpleGraph> ll_join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right);

    static cardStat computeStats(std::shared_ptr<SimpleGraph> &g);

    static cardStat ll_computeStats(std::shared_ptr<SimpleGraph> &g);

};


#endif //QS_SIMPLEEVALUATOR_H
