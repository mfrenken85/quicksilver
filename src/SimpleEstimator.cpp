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

SimpleEstimator::SimpleEstimator(std::shared_ptr<SimpleGraph> &g){

    // works only with SimpleGraph
    graph = g;
}

void SimpleEstimator::prepare() {

    // do your prep here

    //edgeCountMatrix.resize(graph->getNoLabels());
    //edgeCountMatrixInverse.resize(graph->getNoLabels());

    for(int i = 0; i < graph->getNoVertices(); i++) {
        if (!graph->adj[i].empty()){
            for (int j = 0; j < graph->adj[i].size(); j++ ) {

                uint32_t label = graph->adj[i][j].first;

                // groupe edges based on their labels.
                // <label, <vertices, vertices>>

                // increase the counter if the label is already in the list.
                bool found = false;
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

                //  groupe edges based on their labels (inverse)

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

    // calculate edgeDistVertCount,
    // for each element,
    // <label , <number of distinct vertices, number of distinct vertices>>
    for (int i = 0; i < groupededges.size(); i++) {
        std::vector<uint32_t > left;
        std::vector<uint32_t > right;
        for (int j = 0; j < groupededges[i].second.size(); ++j) {
            // if the vertices has not been added before.
            if ( std::find(left.begin(), left.end(), groupededges[i].second[j].first) == left.end() )
                left.emplace_back(groupededges[i].second[j].first);
            // if the vertices has not been added before.
            if ( std::find(right.begin(), right.end(), groupededges[i].second[j].second) == right.end() )
                right.emplace_back(groupededges[i].second[j].second);
        }
        auto p = std::make_pair(left.size(),right.size());
        edgeDistVertCount.emplace_back(std::make_pair (groupededges[i].first, p));
    }

    groupededges.resize(graph->getNoLabels());
    groupededgesinverse.resize(graph->getNoLabels());
    edgeDistVertCount.resize(graph->getNoLabels());

    // output
    for (int i = 0; i < groupededges.size(); i++) {
        std::cout << "label: " << groupededges[i].first  << " encountered times: " << groupededges[i].second.size() << std::endl;
        std::cout << "label: " << edgeDistVertCount[i].first  << " left distinctVertices times: " << edgeDistVertCount[i].second.first <<  " right distinctVertices times: " << edgeDistVertCount[i].second.second << std::endl;
    }
}

// calculate the intersected vertices of two sets(labels).
// return distinct vertices.
std::vector<uint32_t> calculateIntersection(std::vector<std::pair<uint32_t,uint32_t>> v1, std::vector<std::pair<uint32_t,uint32_t>> v2){
    std::vector<uint32_t > intersection;
    for (int i = 0; i < v1.size(); ++i) {
        for (int j = 0; j < v2.size(); ++j) {
            if(v1[i].second==v2[j].first){
                bool found = false;
                for (int k = 0; k < intersection.size(); ++k) {
                    if(intersection[k]==v2[j].first) found = true;
                }
                if(!found) intersection.emplace_back(v2[j].first);
            }
        }
    }
}

void SimpleEstimator::calculate(uint32_t label, bool inverse) {

    // std::cout << "current Label: " << cl << std::endl;
    if( cardStat1.noPaths!= 0 ){

        // apply the formula.
        // because we are trying to get the min value of (Ts * Tr / divider), so we choose the larger divider,
        // which means, divider = Max(V(R,Y), V(S,Y))
        uint32_t divider = 0;
        // calculate the value of V(S,Y), the current label, or the right part of the join calculation.
        for (int i = 0; i < edgeDistVertCount.size(); i++) {
            if(edgeDistVertCount[i].first==label){
                if(!inverse) {
                    divider = edgeDistVertCount[i].second.first;
                }
                else {
                    divider = edgeDistVertCount[i].second.second;
                }
            }
        }
        // calculate the value of V(R,Y), the previous label, or the left part of the join calculation.
        // meanwhile, get the larger value between V(R,Y) and V(S,Y))
        for (int i = 0; i < edgeDistVertCount.size(); i++) {
            if(edgeDistVertCount[i].first==previousLabel){
                if(!previousInverse) {
                    if(divider<edgeDistVertCount[i].second.second)
                        divider = edgeDistVertCount[i].second.second;
                }
                else{
                    if(divider<edgeDistVertCount[i].second.first)
                        divider = edgeDistVertCount[i].second.first;
                }
            }
        }

        // process the label. Get all edges with this label.
        // Edges.size() is T_R in the formula.
        std::vector<std::pair<uint32_t,uint32_t>> edges;
        if(!inverse) {
            for (int i = 0; i < groupededges.size(); i++) {
                if (groupededges[i].first == label) {
                    edges = groupededges[i].second;
                }
            }
        }
        else{
            for (int i = 0; i < groupededgesinverse.size(); i++) {
                if (groupededgesinverse[i].first == label) {
                    edges = groupededgesinverse[i].second;
                }
            }
        }
        // process the label. Get all edges with previous Label.
        // calculated to update noIn and noOut.
        std::vector<std::pair<uint32_t,uint32_t>> previousEdges;
        if(!previousInverse) {
            for (int i = 0; i < groupededges.size(); i++) {
                if (groupededges[i].first == previousLabel) {
                    previousEdges = groupededges[i].second;
                }
            }
        }
        else{
            for (int i = 0; i < groupededgesinverse.size(); i++) {
                if (groupededgesinverse[i].first == previousLabel) {
                    previousEdges = groupededgesinverse[i].second;
                }
            }
        }

        cardStat1.noPaths = cardStat1.noPaths * edges.size() /  divider;

        std::cout << "current processing label is: " << label << std::endl;
        std::cout << "# of edges with current label is: " << edges.size() << std::endl;
        std::cout << "Divider (V(R,Y) or V(S,Y)) is: " << divider << std::endl;

        // Solution 1.
        // This is fast, but not accurate.
        for (int j = 0; j < edgeDistVertCount.size(); ++j) {
            if(edgeDistVertCount[j].first==label){
                cardStat1.noIn = edgeDistVertCount[j].second.second;
                break;
            }
        }

        // Solution 2.
        // update noIn and noOut.
        // This takes more time, but more accurate for noIn and noOut.
        // We are using solution 1 since noIns and noOuts are not used to measure the accuracy.
        // SimpleEstimator::updateCardStat(edges, previousEdges, isprocessing2ndLabel);
        // isprocessing2ndLabel = false;
    }
    else{
        std::cout << "first processed label is " << label << std::endl;

        if(!inverse) {
            for (int j = 0; j < edgeDistVertCount.size(); ++j) {
                if(edgeDistVertCount[j].first==label){
                    cardStat1.noIn = edgeDistVertCount[j].second.second;
                    cardStat1.noOut = edgeDistVertCount[j].second.first;
                    break;
                }
            }
        }
        else{
            for (int j = 0; j < edgeDistVertCount.size(); ++j) {
                if(edgeDistVertCount[j].first==label){
                    cardStat1.noOut = edgeDistVertCount[j].second.second;
                    cardStat1.noIn = edgeDistVertCount[j].second.first;
                    break;
                }
            }
        }
        for (int i = 0; i < groupededges.size(); ++i) {
            if(groupededges[i].first==label){
                cardStat1.noPaths = groupededges[i].second.size();
                break;
            }
        }
        // when processing the 2nd label, noOut will be updated.
        // isprocessing2ndLabel = true;
    }

    previousLabel=label;
    previousInverse=inverse;
}

// update the noIn and noOut of the cardStat.
void SimpleEstimator::updateCardStat(std::vector<std::pair<uint32_t,uint32_t>> v2, std::vector<std::pair<uint32_t,uint32_t>> v1, bool is2ndLabel){

    //std::cout << "v1 size: " << v1.size() << std::endl;
    //std::cout << "v2 size: " << v2.size() << std::endl;

    // the distinct intersection vertices.
    std::vector<uint32_t> intersection;
    for (int i = 0; i < v1.size(); i++) {
        for (int j = 0; j < v2.size(); j++) {
            if(v1[i].second == v2[j].first){
                if(std::find(intersection.begin(), intersection.end(), v2[j].first) == intersection.end()) {
                    intersection.emplace_back(v2[j].first);
                }
            }
        }
    }

    uint32_t noleftCounter=0;
    uint32_t norightCounter=0;

    //std::cout << "intersection size: " << intersection.size() << std::endl;

    for (int l = 0; l < intersection.size(); l++) {
        for (int i = 0; i < v1.size(); i++) {
            if(v1[i].second==intersection[l]) noleftCounter++;
        }
        for (int j = 0; j < v2.size(); j++) {
            if(v2[j].first==intersection[l]) norightCounter++;
        }
    }

    // only update noOut of the cardStat when processing the 2nd label.
    if(is2ndLabel) cardStat1.noOut=noleftCounter;

    cardStat1.noIn=norightCounter;
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
        SimpleEstimator::estimator_aux(q->left);
        SimpleEstimator::estimator_aux(q->right);
    }
}

cardStat SimpleEstimator::estimate(RPQTree *query) {

    // for processing backwords.
    previousLabel= -1;
    previousInverse = false;
    isprocessing2ndLabel = false;

    cardStat1.noIn = 0;
    cardStat1.noOut= 0;
    cardStat1.noPaths = 0;

    // perform your estimation here
    estimator_aux(query);
    return cardStat1;
}
