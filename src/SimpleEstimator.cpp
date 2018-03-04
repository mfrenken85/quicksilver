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
            if ( std::find(left.begin(), left.end(), groupededges[i].second[j].first) == left.end() )
                left.emplace_back(groupededges[i].second[j].first);
            if ( std::find(right.begin(), right.end(), groupededges[i].second[j].second) == right.end() )
                right.emplace_back(groupededges[i].second[j].second);
        }
        auto p = std::make_pair(left.size(),right.size());
        edgeDistVertCount.emplace_back(std::make_pair (groupededges[i].first, p));
    }

    // calculate edgeCountMatrix,
    // for each element,
    // <<left label, right label>, list of intersection vertices.>
    for (int i = 0; i < groupededges.size(); i++) {
        for (int j = 0; j < groupededges.size(); j++) {
            auto pair = std::make_pair(groupededges[i],groupededges[j]);
            edgeCountMatrix.emplace_back(std::make_pair(pair,calculateIntersection(groupededges[i].second, groupededges[j].second)));
        }
    }
    // for intersection vertices (inverse).
    for (int i = 0; i < groupededgesinverse.size(); i++) {
        for (int j = 0; j < groupededgesinverse.size(); j++) {
            auto pair = std::make_pair(groupededgesinverse[i],groupededgesinverse[j]);
            edgeCountMatrix.emplace_back(std::make_pair(pair,calculateIntersection(groupededgesinverse[i].second, groupededgesinverse[j].second)));
        }
    }

    // output
    for (int i = 0; i < groupededges.size(); i++) {
        std::cout << "label: " << groupededges[i].first  << " encountered times: " << groupededges[i].second.size() << std::endl;
        std::cout << "label: " << edgeDistVertCount[i].first  << " left distinctVertices times: " << edgeDistVertCount[i].second.first <<  " right distinctVertices times: " << edgeDistVertCount[i].second.second << std::endl;
    }
    for (int i = 0; i < edgeCountMatrix.size(); i++) {
        std::cout << "label: " << edgeCountMatrix[i].first.first  << " to label: " <<  edgeCountMatrix[i].first.second << " has" <<  edgeCountMatrix[i].second.size() <<" intersection vertices: " << std::endl;
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

        // process the label. Get all edges with this label.
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

        // calculate the distinct vertices which is connected with the edges we got.
        std::vector<uint32_t> distinctVertices;
        for (int i = 0; i < edges.size(); i++) {
            distinctVertices.emplace_back(edges[i].first);
        }
        // remove duplicated vertices.
        std::sort( distinctVertices.begin(), distinctVertices.end() );
        distinctVertices.erase( std::unique( distinctVertices.begin(), distinctVertices.end() ), distinctVertices.end() );

        uint32_t divider = distinctVertices.size();
        if(divider < cardStat1.noIn ) divider = cardStat1.noIn;

        auto key = std::make_pair(previousLabel,label);
        if(!inverse) {
            for (int i = 0; i < edgeCountMatrix.size(); ++i) {
                if(edgeCountMatrix[i].first.first==key.first && edgeCountMatrix[i].first.second==key.second){
                    divider = edgeCountMatrix[i].first.second;
                    break;
                }
            }
        }
        else{
            for (int i = 0; i < edgeCountMatrixInverse.size(); ++i) {
                if(edgeCountMatrixInverse[i].first.first==key.first && edgeCountMatrixInverse[i].first.second==key.second){
                    divider = edgeCountMatrixInverse[i].first.second;
                    break;
                }
            }
        }

        cardStat1.noPaths = cardStat1.noPaths * edges.size() /  divider;

        std::cout << "current processing label is " << label << std::endl;
        std::cout << "# of distinctVertices intersection vertices: " << distinctVertices.size() << std::endl;

        std::cout << "Divider is: " << divider << std::endl;

        cardStat1.noIn = edgeDistVertCount[label].second.second;
    }
    else{

        std::cout << "first processed label is " << label << std::endl;

        cardStat1.noIn = edgeDistVertCount[label].second.second;
        cardStat1.noOut = edgeDistVertCount[label].second.first;
        for (int i = 0; i < groupededges.size(); ++i) {
            if(groupededges[i].first==label){
                cardStat1.noPaths = groupededges[i].second.size();
                break;
            }
        }
        previousLabel=label;
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

    previousLabel= -1;
    cardStat1.noIn = 0;
    cardStat1.noOut= 0;
    cardStat1.noPaths = 0;

    // perform your estimation here
    estimator_aux(query);
    return cardStat1;
}
