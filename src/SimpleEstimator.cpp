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

#include <cmath>
#include "SimpleGraph.h"
#include "SimpleEstimator.h"

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
                if (setInOutLabels.insert(label).second) {
                    histOut[label]++;
                }
                histLabels[label]++;
            }
            setInOutLabels.clear();
        }
        if (!graph->reverse_adj[i].empty()) {
            for (int j = 0; j < graph->reverse_adj[i].size(); j++) {
                uint32_t label = graph->reverse_adj[i][j].first;
                if (setInOutLabels.insert(label).second) {
                    histIn[label]++;
                }
            }
            setInOutLabels.clear();
        }
    }
}

void SimpleEstimator::calculate(uint32_t label, bool inverse) {

    if (cardStat2.noPaths == -1) {
        // case first label checked
        if(!inverse) {
            cardStat2.noIn = histIn[label];
            cardStat2.noOut= histOut[label];
            cardStat2.noPaths = histLabels[label];
        }
        else {
            cardStat2.noIn = histOut[label];
            cardStat2.noOut= histIn[label];
            cardStat2.noPaths = histLabels[label];
        }
    } else {
        if(!inverse) {
            cardStat2.noPaths = cardStat2.noPaths * histLabels[label] / std::max(histOut[label], cardStat2.noIn);
        }
        else {
            cardStat2.noPaths = cardStat2.noPaths * histLabels[label] / std::max(histIn[label], cardStat2.noIn);
        }
        cardStat2.noIn = std::min(cardStat2.noPaths,histIn[label]);
    }
    // apply the formula.
    // because we are trying to get the min value of (Ts * Tr / divider), so we choose the larger divider,
    // which means, divider = Max(V(R,Y), V(S,Y))
    uint32_t divider = 0;
    uint32_t noVIn = histIn[label];
    uint32_t noVOut = histOut[label];
    uint32_t Tr = histLabels[label];
    // calculate the value of divider (V(S,Y)).

    if(!inverse) {
        divider = noVOut;
        // if this is the first label, update noOut.
        if( cardStat1.noPaths== 0 ) cardStat1.noOut = noVOut;
        cardStat1.noIn = noVIn;
    }
    else {
        divider = noVIn;
        // if this is the first label, update noOut.
        if( cardStat1.noPaths== 0 ) cardStat1.noOut = noVIn;
        cardStat1.noIn = noVOut;
    }

    // process the label. Get all edges with this label and calculate Tr, which is the # of edges.

    //uint32_t tempDivider = noIn * Tr / graph->getNoEdges();
    //if( tempDivider > divider) divider = tempDivider;

    //Ts = current cardStat1.noPaths
    // apply the formula: new noPhts = Tr * Ts / Max(V(R,Y), V(S,Y))
    // if this is the first label, update noOut.
    if( cardStat1.noPaths== 0 ) cardStat1.noPaths = Tr;
    else cardStat1.noPaths = cardStat1.noPaths * Tr /  divider;

    //std::cout << std::endl;
    //std::cout << "after processing label: " << label << std::endl;
    //std::cout << "current Ins is: " << cardStat1.noIn << std::endl;
    //std::cout << "current Paths is: " << cardStat1.noPaths << std::endl;
    //std::cout << "current Outs is: " << cardStat1.noOut << std::endl;
}

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
        }
        SimpleEstimator::calculate(label, inverse);
    }

    if(q->isConcat()) {
        // evaluate the children
        // processing the labels from left to right.
        SimpleEstimator::estimator_aux(q->left);
        SimpleEstimator::estimator_aux(q->right);

        // Possible alternative solution.
        // Processing the labels from the Right to Left.
    }
}

cardStat SimpleEstimator::estimate(RPQTree *query) {

    cardStat1.noIn = 0;
    cardStat1.noOut= 0;
    cardStat1.noPaths = 0;

    cardStat2.noIn = -1;
    cardStat2.noOut= -1;
    cardStat2.noPaths = -1;

    // perform your estimation here
    estimator_aux(query);
    return cardStat2;
}
