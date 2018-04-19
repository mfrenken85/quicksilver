//
// Created by Nikolay Yakovets on 2018-02-01.
//
// Mark:
// See chapter 13.3 of the book for details:
// Create a histogram, evaluate each query
// for its opperations and implement the given
// calculations of the book.

// Jiaqi:
// Several solutions were implemented, the current one gives best results
// with relatively fast execution.

// Current solution,
// Processing the labels from the Left to Right.
// Possible alternative solution.
// Processing the labels from the Right to Left.

// Jiaqi Ni
// 2018 03 19
// Simplified the code.

#include <cmath>
#include "SimpleGraph.h"
#include "SimpleEstimator.h"

SimpleEstimator::SimpleEstimator(std::shared_ptr<SimpleGraph> &g){

    // works only with SimpleGraph
    graph = g;
}

void SimpleEstimator::prepare() {
    prepare_default();

}

void SimpleEstimator::prepare_default() {
    // do your prep here
    for(int i = 0; i < graph->getNoVertices(); i++) {
        if (!graph->adj[i].empty()){
            for (int j = 0; j < graph->adj[i].size(); j++ ) {
                uint32_t label = graph->adj[i][j].first;
                if (setLabels.insert(label).second) {
                    histOut[label]++;
                }
                if (histLabels[label]){
                    histLabels[label]++;
                } else {
                    histLabels[label] = 1;
                }
            }
            setLabels.clear();
        }
        if (!graph->reverse_adj[i].empty()) {
            for (int j = 0; j < graph->reverse_adj[i].size(); j++) {
                uint32_t label = graph->reverse_adj[i][j].first;
                if (setLabels.insert(label).second) {
                    histIn[label]++;
                }
            }
            setLabels.clear();
        }
    }
    for (int i = 0; i < histLabels.size(); ++i) {
        labelCardStats.emplace(i , cardStat { histOut[i], histLabels[i], histIn[i]} );
    }
}

void  SimpleEstimator::estimator_aux(RPQTree *q) {

    if(q->isLeaf()) {
        // project out the label in the AST
        std::regex directLabel (R"((\d+)\+)");
        std::regex inverseLabel (R"((\d+)\-)");
        std::smatch matches;

        uint32_t label;
        char inverse;

        if(std::regex_search(q->data, matches, directLabel)) {
            label = (uint32_t) std::stoul(matches[1]);
            inverse = '+';
        } else if(std::regex_search(q->data, matches, inverseLabel)) {
            label = (uint32_t) std::stoul(matches[1]);
            inverse = '-';
        } else {
            std::cerr << "Label parsing failed!" << std::endl;
        }
        parsedQuery.push_back(std::make_pair(label, inverse));
    }

    if(q->isConcat()) {
        SimpleEstimator::estimator_aux(q->left);
        SimpleEstimator::estimator_aux(q->right);
    }
}

cardStat SimpleEstimator::reverse(cardStat card) {
    return {card.noIn, card.noPaths, card.noOut};
}

cardStat SimpleEstimator::estimate(RPQTree *query) {

    estimator_aux(query);
    cardStat card = estimateQuery(parsedQuery);
    parsedQuery.clear();
    return  card;
}

cardStat SimpleEstimator::estimateQuery(std::vector<std::pair<uint32_t, char>> pq){
    cardStat card{0,0,0};
    if(pq.size()==0)
    {
        return cardStat{0,0,0};
    }
    else if(pq.size()==1)
    {
        if(pq[0].second == '+'){
            card = labelCardStats[pq[0].first];
        }
        else{
            card = reverse(labelCardStats[pq[0].first]);
        }
    }
    else
    {
        if(pq[0].second == '+')
            card = labelCardStats[pq[0].first];
        else card = reverse(labelCardStats[pq[0].first]);

        for(int i = 1; i<pq.size();i++)
        {
            cardStat next;
            if(pq[i].second == '+')
                next = labelCardStats[pq[i].first];
            else next = reverse(labelCardStats[pq[i].first]);

            uint32_t in = card.noIn; // * next.noPaths / graph->getNoEdges();
            uint32_t out = next.noOut; // * card.noPaths / graph->getNoEdges();
            uint32_t divider = std::max(in, out);
            uint32_t noPaths = card.noPaths * next.noPaths / divider;
            card = cardStat{ std::min(card.noOut, noPaths), noPaths, std::min(next.noIn, noPaths) };
        }
    }
    pq.clear();
    return card;
}

