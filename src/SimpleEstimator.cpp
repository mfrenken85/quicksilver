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
                        break;
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
                        break;
                    }
                }
                // otherwise add that new label into the list.
                if(!found) {
                    std::vector<std::pair<uint32_t,uint32_t>> v;
                    v.emplace_back(std::make_pair (i,graph->adj[i][j].second));
                    groupededges.emplace_back( std::make_pair(label ,v) );
                }

                // increase the counter if the label is already in the list.
                found = false;
                for (int k = 0; k < groupededgesinverse.size(); k++) {
                    if(groupededgesinverse[k].first == label){
                        found = true;
                        groupededgesinverse[k].second.emplace_back(std::make_pair(graph->adj[i][j].second, i));
                        break;
                    }
                }
                // otherwise add that new label into the list.
                if(!found) {
                    std::vector<std::pair<uint32_t,uint32_t>> v;
                    v.emplace_back(std::make_pair (graph->adj[i][j].second, i));
                    groupededgesinverse.emplace_back( std::make_pair(label ,v) );
                }
            }
        }
    }

    for (int i = 0; i < labelcounter.size(); i++) {
        std::cout << "label: " << labelcounter[i].first << " encountered times: " << labelcounter[i].second << std::endl;
    }
    for (int i = 0; i < groupededges.size(); i++) {
        std::cout << "label: " << groupededges[i].first  << " encountered times: " << groupededges[i].second.size() << std::endl;
    }
    for (int i = 0; i < groupededgesinverse.size(); i++) {
        std::cout << "label: " << groupededgesinverse[i].first  << " encountered times: " << groupededgesinverse[i].second.size() << std::endl;
    }
}

cardStat SimpleEstimator::computeStats() {
    cardStat stats {};
    stats.noIn = noIn;
    stats.noOut= noOut;
    stats.noPaths = noPaths;
    return stats;
}

//std::shared_ptr<SimpleGraph> SimpleEstimator::calculate(uint32_t cl, bool inverse ,std::shared_ptr<SimpleGraph> &in) {
void SimpleEstimator::calculate(uint32_t cl, bool inverse) {

    // std::cout << "current Label: " << cl << std::endl;

    if(previousLabel!=-1){

        std::vector<std::pair<uint32_t,uint32_t>> current;
        std::vector<std::pair<uint32_t,uint32_t>> previous;
        if(!inverse) {
            for (int i = 0; i < groupededges.size(); i++) {
                if (groupededges[i].first == previousLabel) {
                    previous = groupededges[i].second;
                }
                if (groupededges[i].first == cl) {
                    current = groupededges[i].second;
                }
            }
        }
        else{
            for (int i = 0; i < groupededgesinverse.size(); i++) {
                if (groupededgesinverse[i].first == previousLabel) {
                    previous = groupededgesinverse[i].second;
                }
                if (groupededgesinverse[i].first == cl) {
                    current = groupededgesinverse[i].second;
                }
            }
        }

        std::vector<uint32_t> r;
        std::vector<uint32_t> s;
        for (int i = 0; i < current.size(); i++) {
            s.emplace_back(current[i].first);
        }
        for (int i = 0; i < previous.size(); i++) {
            r.emplace_back(previous[i].second);
        }

        // calculate intersection.
        std::vector<uint32_t> intersection;
        std::sort(r.begin(), r.end());
        std::sort(s.begin(), s.end());
        std::set_intersection(r.begin(),r.end(),s.begin(),s.end(),back_inserter(intersection));

        if(intersection.size()!=0) {
            std::sort( intersection.begin(), intersection.end() );
            intersection.erase( std::unique( intersection.begin(), intersection.end() ), intersection.end() );
            //noPaths = previous.size();
            //noPaths = noPaths * current.size() / intersection.size();
            uint32_t paths = current.size() * previous.size() / intersection.size();
            noPaths += paths;
            std::cout << "# of Distinct intersection vertices: " << intersection.size() << std::endl;
        } else{
            std::cout << "previous Label: " << previousLabel << std::endl;
            std::cout << "current Label: " << cl << std::endl;
            bool stop = true;
        }

        noIn += intersection.size();
        noOut += intersection.size();
    }

    previousLabel = cl;
    // return  in;
}

std::shared_ptr<SimpleGraph> SimpleEstimator::join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right) {

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
/*std::shared_ptr<SimpleGraph>*/
void  SimpleEstimator::estimator_aux(RPQTree *q) {

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
            //return nullptr;
        }
        // return SimpleEstimator::calculate(label, inverse, graph);
        SimpleEstimator::calculate(label, inverse);
    }

    if(q->isConcat()) {

        // evaluate the children
        //auto leftGraph = SimpleEstimator::estimator_aux(q->left);
        //auto rightGraph = SimpleEstimator::estimator_aux(q->right);

        SimpleEstimator::estimator_aux(q->left);
        SimpleEstimator::estimator_aux(q->right);

        // join left with right
        // return SimpleEstimator::join(leftGraph, rightGraph);

    }

    // return nullptr;
}

cardStat SimpleEstimator::estimate(RPQTree *query) {

    noIn = 0;
    noOut = 0;
    noPaths = 1;
    previousLabel = -1;

    // perform your estimation here
    estimator_aux(query);
    return SimpleEstimator::computeStats();
    // return cardStat {0, 0, 0};
}
