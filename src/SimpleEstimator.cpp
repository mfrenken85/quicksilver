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
                if (setLabels.insert(label).second) {
                    histOut[label]++;
                }
                if (histLabels[label]){
                    histLabels[label]++;
                } else {
                    histLabels[label] = 1;
                }
                // histLabels[label]++;
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
        labelCardStats.emplace(i , cardStat {histOut[i], histLabels[i], histIn[i]});
    }
}

void SimpleEstimator::calculate(uint32_t label, bool inverse) {
    /*
    uint32_t noIn = histIn[label];
    uint32_t noOut = histOut[label];
    uint32_t noLabels = histLabels[label];
    if (cardStat2.noPaths == -1) {
        // case first label checked
        if(!inverse) {
            cardStat2.noIn = noIn;
            cardStat2.noOut= noOut;
            cardStat2.noPaths = noLabels;
        }
        else {
            cardStat2.noIn = noOut;
            cardStat2.noOut= noIn;
            cardStat2.noPaths = noLabels;
        }
    } else {
        if(!inverse) {
            cardStat2.noPaths = cardStat2.noPaths * noLabels / std::max(noOut, cardStat2.noIn);
            cardStat2.noIn = std::min(cardStat2.noPaths,noIn);
        }
        else {
            cardStat2.noPaths = cardStat2.noPaths * noLabels / std::max(noIn, cardStat2.noIn);
            cardStat2.noIn = std::min(cardStat2.noPaths,noOut);
        }
        cardStat2.noOut= std::min(cardStat2.noPaths,cardStat2.noOut);
    }
    /*
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
     */
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
        //SimpleEstimator::calculate(label, inverse);
    }

    if(q->isConcat()) {
        SimpleEstimator::estimator_aux(q->left);
        SimpleEstimator::estimator_aux(q->right);
    }
}

cardStat SimpleEstimator::reverse(cardStat c) {
    return {c.noIn, c.noPaths, c.noOut};
}

cardStat SimpleEstimator::estimate(RPQTree *query) {

    //cardStat1.noIn = 0;
    //cardStat1.noOut= 0;
    //cardStat1.noPaths = 0;

    //cardStat2.noIn = -1;
    //cardStat2.noOut= -1;
    //cardStat2.noPaths = -1;

    // perform your estimation here
    estimator_aux(query);
    //return cardStat1;

    if(parsedQuery.size()==0)
    {
        parsedQuery.clear();
        return cardStat{0,0,0};
    }
    else if(parsedQuery.size()==1)
    {
        cardStat card;
        if(parsedQuery[0].second == '+'){
            card = labelCardStats[parsedQuery[0].first];
        }
        else{
            card = reverse(labelCardStats[parsedQuery[0].first]);
        }
        parsedQuery.clear();
        return  card;
    }
    else
    {
        cardStat card;
        if(parsedQuery[0].second == '+')
            card = labelCardStats[parsedQuery[0].first];
        else card = reverse(labelCardStats[parsedQuery[0].first]);

        for(int i=1; i<parsedQuery.size();i++)
        {
            cardStat next;
            if(parsedQuery[i].second == '+')
                next = labelCardStats[parsedQuery[i].first];
            else next = reverse(labelCardStats[parsedQuery[i].first]);

            uint32_t in = card.noIn / 2; // * next.noPaths / graph->getNoEdges();
            uint32_t out = next.noOut / 2; // * card.noPaths / graph->getNoEdges();
            uint32_t divider = std::max(in, out);
            uint32_t noPaths = card.noPaths * next.noPaths / divider;
            card = cardStat{ next.noOut, noPaths, card.noIn };
        }
        parsedQuery.clear();
        return card;
    }
}
