//
// Created by Nikolay Yakovets on 2018-02-01.
//
// Mark:
// See chapter 13.3 of the book for details:
// Create a histogram, evaluate each query
// for its opperations and implement the given
// calculations of the book.

#include <cmath>
#include "SimpleGraph.h"
#include "SimpleEstimator.h"

// 简单的做一个overeastimate
// 也就是说，根据label分类，然后再做一个就是说，比如说label1，右边所有的node，所可能连接label1，2，3，4，5.。。的数量也要知道，比如说是n1，n2，n3...
// 那么如果我们的path是label1，label1，label2，label3，那么就是n1*n1*n2*n3.。。

SimpleEstimator::SimpleEstimator(std::shared_ptr<SimpleGraph> &g){

    // works only with SimpleGraph
    graph = g;
}

void SimpleEstimator::prepare() {

    // do your prep here

    noIn = 0;
    noOut = 0;
    noPaths = 0;
    previousLabel = -1;

    for(int i = 0; i < graph->getNoVertices(); i++) {
        if (!graph->adj[i].empty()){
            for (int j = 0; j < graph->adj[i].size(); j++ ) {
                uint32_t label = graph->adj[i][j].first;

                // increase the counter if the label is already in the list.
                bool found = false;
                for (int k = 0; k < labelcounter.size(); k++) {
                    if(labelcounter[k].first == label){
                        found = true;
                        labelcounter[k].second ++;
                    }
                }
                // otherwise add that new label into the list.
                if(!found) {
                    labelcounter.emplace_back(std::make_pair(label, 1));
                }

                // increase the counter if the label is already in the list.
                found = false;
                for (int k = 0; k < groupededges.size(); k++) {
                    if(groupededges[k].first == label){
                        found = true;
                        groupededges[k].second.emplace_back(std::make_pair(i,graph->adj[i][j].second));
                    }
                }
                // otherwise add that new label into the list.
                if(!found) {
                    std::vector<std::pair<uint32_t,uint32_t>> v;
                    v.emplace_back(std::make_pair (i,graph->adj[i][j].second));
                    groupededges.emplace_back( std::make_pair(label ,v) );
                }
            }
        }
    }

    for (int i = 0; i < labelcounter.size(); i++) {
        std::cout << "label: " << labelcounter[i].first << " encountered times: " << labelcounter[i].second << std::endl;
    }
    for (int i = 0; i < groupededges.size(); i++) {
        std::cout << "label: " << groupededges[i].first  << " encountered times: " << groupededges[i].second.size() << std::endl;
        //for (int j = 0; j < groupededges[i].second.size(); ++j) {
        //    std::cout << "left: " << groupededges[i].second[j].first  << " right: " << groupededges[i].second[j].second << std::endl;
        //}
    }
}

cardStat SimpleEstimator::computeStats() {
    cardStat stats {};
    stats.noIn = noIn;
    stats.noOut= noOut;
    stats.noPaths = noPaths;
    return stats;
}

<<<<<<< HEAD
std::shared_ptr<SimpleGraph> SimpleEstimator::calculate(uint32_t cl, bool inverse ,std::shared_ptr<SimpleGraph> &in) {

    if(previousLabel!=-1){

        std::vector<std::pair<uint32_t,uint32_t>> current;
        std::vector<std::pair<uint32_t,uint32_t>> previous;
        for (int i = 0; i < groupededges.size(); i++) {
            if(groupededges[i].first==previousLabel){
                previous = groupededges[i].second;
            }
            if(groupededges[i].first==cl){
                current = groupededges[i].second;
            }
        }

        std::vector<uint32_t> r;
        std::vector<uint32_t> s;
        if(!inverse){
            for (int i = 0; i < current.size(); i++) {
                s.emplace_back(current[i].first);
            }
            for (int i = 0; i < previous.size(); i++) {
                r.emplace_back(previous[i].second);
            }
        }
        else{
            for (int i = 0; i < current.size(); i++) {
                s.emplace_back(current[i].second);
            }
            for (int i = 0; i < previous.size(); i++) {
                r.emplace_back(previous[i].first);
            }
        }

        std::vector<uint32_t> intersection;
        for (int i = 0; i < r.size(); ++i) {
            for (int j = 0; j < s.size(); ++j) {
                if(r[i]==s[j]){
                    intersection.emplace_back(s[j]);
                }
            }
        }

        if(intersection.size()!=0) {
            uint32_t paths = current.size() * previous.size() / intersection.size();
            noPaths *= paths;
            std::cout << "# of intersection vertices: " << intersection.size() << std::endl;
        }
    }

    previousLabel = cl;
    return  in;
}

std::shared_ptr<SimpleGraph> SimpleEstimator::join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right) {

    auto out = std::make_shared<SimpleGraph>(left->getNoVertices());
    out->setNoLabels(1);

    for(uint32_t leftSource = 0; leftSource < left->getNoVertices(); leftSource++) {
        for (auto labelTarget : left->adj[leftSource]) {

            int leftTarget = labelTarget.second;
            // try to join the left target with right source
            for (auto rightLabelTarget : right->adj[leftTarget]) {
=======
    // perform your estimation here
    std::cout<< std::endl;
    std::cout << "testing left: ";
    q->left->print();
    std::cout<< std::endl;
    std::cout << "testing right: ";
    q->right->print();
>>>>>>> e7ee6c05043230a2f59df0ddc7d7d083a58802e3

                auto rightTarget = rightLabelTarget.second;
                out->addEdge(leftSource, rightTarget, 0);
            }
        }
    }

    return out;
}

std::shared_ptr<SimpleGraph> SimpleEstimator::estimator_aux(RPQTree *q) {

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
        return SimpleEstimator::calculate(label, inverse, graph);
    }

    if(q->isConcat()) {

        // evaluate the children
        auto leftGraph = SimpleEstimator::estimator_aux(q->left);
        auto rightGraph = SimpleEstimator::estimator_aux(q->right);

        // join left with right
        return SimpleEstimator::join(leftGraph, rightGraph);

    }

    return nullptr;
}

cardStat SimpleEstimator::estimate(RPQTree *query) {

    // perform your estimation here
    estimator_aux(query);
    return SimpleEstimator::computeStats();
    // return cardStat {0, 0, 0};
}
