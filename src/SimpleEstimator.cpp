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

void SimpleEstimator::calculate(uint32_t label, bool inverse) {

    // std::cout << "current Label: " << cl << std::endl;
    if( cardStat1.noPaths!= 0 ){

        // apply the formula.
        // because we are trying to get the min value of (Ts * Tr / divider), so we choose the larger divider,
        // which means, divider = Max(V(R,Y), V(S,Y))
        uint32_t divider = 0;
        // calculate the value of V(S,Y).
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
        // the value of V(R,Y) = noIns.
        // Get the larger value between V(R,Y) and V(S,Y))
        if(cardStat1.noIn > divider) divider = cardStat1.noIn;

        // process the label. Get all edges with this label.
        // Edges.size() = Tr.
        std::vector<std::pair<uint32_t,uint32_t>> edges;
        if(!inverse) {
            for (int i = 0; i < groupededges.size(); i++) {
                if (groupededges[i].first == label) {
                    edges = groupededges[i].second;
                    break;
                }
            }
        }
        else{
            for (int i = 0; i < groupededgesinverse.size(); i++) {
                if (groupededgesinverse[i].first == label) {
                    edges = groupededgesinverse[i].second;
                    break;
                }
            }
        }

        //Ts = current cardStat1.noPaths
        // apply the formula: new noPhts = Tr * Ts / Max(V(R,Y), V(S,Y))
        cardStat1.noPaths = cardStat1.noPaths * edges.size() /  divider;

        std::cout << "current processing label is: " << label << std::endl;
        std::cout << "# of edges with current label is: " << edges.size() << std::endl;
        std::cout << "Divider (V(R,Y) or V(S,Y)) is: " << divider << std::endl;

        for (int j = 0; j < edgeDistVertCount.size(); ++j) {
            if(edgeDistVertCount[j].first==label){
                cardStat1.noIn = edgeDistVertCount[j].second.second;
                break;
            }
        }
    }
    else{

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

        std::cout << "first processed label is " << label << std::endl;
        std::cout << "# of edges with current label is: " << cardStat1.noPaths << std::endl;
        std::cout << "noIn is: " << cardStat1.noIn << std::endl;
        std::cout << "noOut is: " << cardStat1.noOut << std::endl;
    }
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

    cardStat1.noIn = 0;
    cardStat1.noOut= 0;
    cardStat1.noPaths = 0;

    // perform your estimation here
    estimator_aux(query);
    return cardStat1;
}
