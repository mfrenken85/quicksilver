//
// Created by Nikolay Yakovets on 2018-02-02.
//

#include "SimpleEstimator.h"
#include "SimpleEvaluator.h"

SimpleEvaluator::SimpleEvaluator(std::shared_ptr<SimpleGraph> &g) {

    // works only with SimpleGraph
    graph = g;
    est = nullptr; // estimator not attached by default
}

void SimpleEvaluator::attachEstimator(std::shared_ptr<SimpleEstimator> &e) {
    est = e;
}

void SimpleEvaluator::prepare() {

    // if attached, prepare the estimator
    if(est != nullptr) est->prepare();

    // prepare other things here.., if necessary

}

cardStat SimpleEvaluator::computeStats(std::shared_ptr<SimpleGraph> &g) {

    cardStat stats {};

    for(int source = 0; source < g->getNoVertices(); source++) {
        if(!g->adj[source].empty()) stats.noOut++;
    }

    stats.noPaths = g->getNoDistinctEdges();

    for(int target = 0; target < g->getNoVertices(); target++) {
        if(!g->reverse_adj[target].empty()) stats.noIn++;
    }

    return stats;
}

cardStat SimpleEvaluator::ll_computeStats(std::shared_ptr<SimpleGraph> &g) {

    cardStat stats {};
    SimpleGraph::AdjTable *table;
    table = g->getTable(0,false);
    stats.noPaths = table->E;
    stats.noOut = table->V;
    table = g->getTable(0,true);
    stats.noIn = table->V;

    return stats;
}

std::shared_ptr<SimpleGraph> SimpleEvaluator::project(uint32_t projectLabel, bool inverse, std::shared_ptr<SimpleGraph> &in) {

    auto out = std::make_shared<SimpleGraph>(in->getNoVertices());
    out->setNoLabels(in->getNoLabels());

    if(!inverse) {
        // going forward
        for(uint32_t source = 0; source < in->getNoVertices(); source++) {
            for (auto labelTarget : in->adj[source]) {

                auto label = labelTarget.first;
                auto target = labelTarget.second;

                if (label == projectLabel)
                    out->addEdge(source, target, label);
            }
        }
    } else {
        // going backward
        for(uint32_t source = 0; source < in->getNoVertices(); source++) {
            for (auto labelTarget : in->reverse_adj[source]) {

                auto label = labelTarget.first;
                auto target = labelTarget.second;

                if (label == projectLabel)
                    out->addEdge(source, target, label);
            }
        }
    }

    return out;
}




std::shared_ptr<SimpleGraph> SimpleEvaluator::join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right) {

    auto out = std::make_shared<SimpleGraph>(left->getNoVertices());
    out->setNoLabels(1);

    for(uint32_t leftSource = 0; leftSource < left->getNoVertices(); leftSource++) {
        for (auto labelTarget : left->adj[leftSource]) {

            int leftTarget = labelTarget.second;
            // try to join the left target with right source
            for (auto rightLabelTarget : right->adj[leftTarget]) {

                auto rightTarget = rightLabelTarget.second;
                out->addEdge(leftSource, rightTarget, 0);

            }
        }
    }

    return out;
}

/*
 * ????
 */
std::shared_ptr<SimpleGraph> SimpleEvaluator::ll_project(uint32_t projectLabel, bool inverse, std::shared_ptr<SimpleGraph> &in) {
    // create new graph class, will only contain 1 table, hence table is reffered to as label 0
    auto out = std::make_shared<SimpleGraph>(0);
    out->setNoLabels(1);
    //set normal table on label 0
    out->setTable(0, in->getTable(projectLabel,!inverse),true);
    //set inverse table on label 0
    out->setTable(0, in->getTable(projectLabel,inverse),false);
    return out;
}

/*
 * ?????
 */
std::shared_ptr<SimpleGraph> SimpleEvaluator::ll_join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right) {
    auto out = std::make_shared<SimpleGraph>(0);
    out->setNoLabels(1);

    SimpleGraph::AdjTable *leftTable;  // This will point to each node as it traverses the list
    SimpleGraph::AdjList *leftList;  // This will point to each node as it traverses the list
    SimpleGraph::AdjListNode *leftNode;  // This will point to each node as it traverses the list
    // previous
    SimpleGraph::AdjTable *rightTable;
    SimpleGraph::AdjList *rightList;
    SimpleGraph::AdjListNode *rightNode;

    leftTable = left->getTable(0,true); //take reverse of left table for easy itteration
    rightTable = right->getTable(0, false);

    if (leftTable->head == 0 || rightTable->head == 0) {
        return out;
    }
    leftList = leftTable->head;
    rightList = rightTable->head;
    bool exhausted = false;
    bool exhausted1;
    bool exhausted2;
    while (!exhausted) { // match inverse right from to normal left from
        if (leftList->from == rightList->from) {
            leftNode = leftList->head;
            // make cartesian product
            exhausted1 = false;
            while (!exhausted1) { // loop inverse left to
                rightNode = rightList->head;
                exhausted2 = false;
                while (!exhausted2) { // loop normal right to
                    // always store as label 0
                    out->addEdgeToLinkedList(leftNode->to, rightNode->to, 0, out->tableHead, false);
                    out->addEdgeToLinkedList(rightNode->to, leftNode->to, 0, out->reverse_tableHead, true);
                    if (rightNode->next != 0) {
                        rightNode = rightNode->next;
                    } else {
                        exhausted2 = true;
                        //break;
                    }
                }
                if (leftNode->next != 0) {
                    leftNode = leftNode->next;
                } else {
                    exhausted1 = true;
                    //break;
                }
            }
            if (rightList->next != 0) {
                rightList = rightList->next;
            } else {
                exhausted = true;
                //break;
            }
            if (leftList->next != 0) {
                leftList = leftList->next;
            } else {
                exhausted = true;
                //break;
            }
        }
        if (leftList->from < rightList->from) {
            if (leftList->next != 0) {
                leftList = leftList->next;
            } else {
                exhausted = true;
                //break;
            }
        } else if (leftList->from > rightList->from) {
            if (rightList->next != 0) {
                rightList = rightList->next;
            } else {
                exhausted = true;
                //break;
            }
        }
    }

    return out;
}

std::shared_ptr<SimpleGraph> SimpleEvaluator::evaluate_aux(RPQTree *q) {

    // evaluate according to the AST bottom-up

    if(q->isLeaf()) {
        // project out the label in the AST
        std::regex directLabel (R"((\d+)\+)");
        std::regex inverseLabel (R"((\d+)\-)");

        std::smatch matches;

        uint32_t label;
        bool inverse;

        if(std::regex_search(q->data, matches, directLabel)) {
            label = (uint32_t) std::stoul(matches[1]);
            inverse = false;
        } else if(std::regex_search(q->data, matches, inverseLabel)) {
            label = (uint32_t) std::stoul(matches[1]);
            inverse = true;
        } else {
            std::cerr << "Label parsing failed!" << std::endl;
            return nullptr;
        }

        return SimpleEvaluator::project(label, inverse, graph);
    }

    if(q->isConcat()) {

        // evaluate the children
        auto leftGraph = SimpleEvaluator::evaluate_aux(q->left);
        auto rightGraph = SimpleEvaluator::evaluate_aux(q->right);

        // join left with right
        return SimpleEvaluator::join(leftGraph, rightGraph);

    }

    return nullptr;
}

std::shared_ptr<SimpleGraph> SimpleEvaluator::ll_evaluate_aux(RPQTree *q) {

    // evaluate according to the AST bottom-up

    if(q->isLeaf()) {
        // project out the label in the AST
        std::regex directLabel (R"((\d+)\+)");
        std::regex inverseLabel (R"((\d+)\-)");

        std::smatch matches;

        uint32_t label;
        bool inverse;

        if(std::regex_search(q->data, matches, directLabel)) {
            label = (uint32_t) std::stoul(matches[1]);
            inverse = false;
        } else if(std::regex_search(q->data, matches, inverseLabel)) {
            label = (uint32_t) std::stoul(matches[1]);
            inverse = true;
        } else {
            std::cerr << "Label parsing failed!" << std::endl;
            return nullptr;
        }

        return SimpleEvaluator::ll_project(label, inverse, graph);
    }

    if(q->isConcat()) {

        // evaluate the children
        auto leftGraph = SimpleEvaluator::ll_evaluate_aux(q->left);
        auto rightGraph = SimpleEvaluator::ll_evaluate_aux(q->right);

        // join left with right
        return SimpleEvaluator::ll_join(leftGraph, rightGraph);

    }

    return nullptr;
}

cardStat SimpleEvaluator::evaluate(RPQTree *query) {
    auto res = ll_evaluate_aux(query);
    return SimpleEvaluator::ll_computeStats(res);
    //auto res = evaluate_aux(query);
    //return SimpleEvaluator::computeStats(res);
}