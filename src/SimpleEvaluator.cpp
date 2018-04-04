//
// Created by Nikolay Yakovets on 2018-02-02.
//

#include "SimpleEstimator.h"
#include "SimpleEvaluator.h"
#include "RPQTree.h"

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

cardStat SimpleEvaluator::evaluate(RPQTree *query) {

    // Since we do not want to change the program's structure,
    // we convert the query -> string
    // optimize the string and convert the string -> query again.
    // In this case, indeed there will be one time useless query to string and string to query transformation.
    // But since this is pretty cheap, and this save much time of programming since this does not change the program's structure.
    // Hence, we implemented it in this way.
    auto querypath = query->toString();
    querypath = preParse(querypath,graph,est);
    auto newQuery = RPQTree::strToTree(querypath);

    auto res = evaluate_aux(newQuery);
    return SimpleEvaluator::computeStats(res);
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> tokens;
    while (getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}

uint32_t estimateCostOfJoin(std::string left, std::string right, std::shared_ptr<SimpleEstimator> est){
    std::string queryPath = "";
    if(right!="") {
        queryPath = "(" + left + ')' + '(' + right + ")";
    }
    else{
        queryPath = left;
    }
    RPQTree *queryTree = RPQTree::strToTree(queryPath);
    //queryTree->print();
    cardStat card = est->estimate(queryTree);
    return card.noPaths;
}

// Dynamic programming to find the best plan.
bestPlan SimpleEvaluator::findBestPlan(std::string originalQuery, std::string query, std::shared_ptr<SimpleGraph> &graph, std::shared_ptr<SimpleEstimator> est){

    if( plans.count(originalQuery) !=0 )
        return plans[originalQuery]; // best plan already calculated.

    auto splits = split(query,'/');
    uint32_t splitsSize = splits.size();
    if ( splitsSize==1 ){
        if(plans.count(query)==0) {
            bestPlan p;
            p.executedQuery = "";
            p.cost = std::numeric_limits<int>::max();
            plans[query] = p;
        }
        plans[query].executedQuery = query;
        plans[query].cost = estimateCostOfJoin(query,"",est);
    } else if( splitsSize == 2){
        if(plans.count(query)==0) {
            bestPlan p;
            p.executedQuery = "";
            p.cost = std::numeric_limits<int>::max();
            plans[query] = p;
        }
        plans[query].executedQuery = "(" + query + ")";
        std::string leftLabel = split(query,'/')[0];
        std::string rightLabel = split(query,'/')[1];
        plans[query].cost = estimateCostOfJoin(leftLabel,rightLabel,est);
    }
    else{
        for (int i = 0; i < splits.size()-1; ++i) {
            std::string left = "";
            std::string right = "";
            for (int j = 0; j < splits.size(); ++j) {
                if(j<=i) left+=splits[j];
                else right+=splits[j];
            }
            bestPlan p1 = findBestPlan(originalQuery,left,graph,est);
            bestPlan p2 = findBestPlan(originalQuery,right,graph,est);
            // cost of joining
            std::string leftLabel = p1.executedQuery;
            std::string rightLabel = p2.executedQuery;
            uint32_t totalCost = estimateCostOfJoin(leftLabel,rightLabel,est);
            if(plans.count(query)==0) {
                bestPlan p;
                p.executedQuery = "";
                p.cost = std::numeric_limits<int>::max();
                plans[query] = p;
            }
            if(totalCost < plans[query].cost) {
                plans[query].cost = totalCost;
                std::string left = p1.executedQuery;
                uint32_t leftpmCounter = 0;
                uint32_t rightpmCounter = 0;
                std::string right = p2.executedQuery;
                for (int j = 0; j < left.size(); ++j) {
                    if(left[j]=='+'||left[j]=='-') leftpmCounter ++;
                }
                for (int j = 0; j < right.size(); ++j) {
                    if(right[j]=='+'||right[j]=='-') rightpmCounter ++;
                }
                if(leftpmCounter !=1) left =  '(' + left + ')';
                if(rightpmCounter!=1) right =  '(' + right + ')';
                plans[query].executedQuery = left + right;
            }
        }
    }
    return plans[query];
}

std::string SimpleEvaluator::preParse(std::string str,std::shared_ptr<SimpleGraph> &graph, std::shared_ptr<SimpleEstimator> est){
    // if this query only contains 1 or 2 relation, no need to find a good plan, since this is only 1 plan possible..
    if(split(str,'/').size() <= 2)
        return  str;

    std::string parsed = "";
    for (int i = 0; i < str.size(); ++i) {
        if(str[i]=='('||str[i]==')') continue;
        else parsed+=str[i];
    }

    std::string temp = SimpleEvaluator::findBestPlan(parsed,parsed,graph,est).executedQuery;
    std::string queryPath = "";
    uint32_t pmCounter = 0;
    for (int j = 0; j < temp.length(); ++j) {
        if(temp[j]=='+' || temp[j]=='-'){
            pmCounter++;
        }
    }
    uint32_t counter = 0;
    for (int j = 0; j < temp.length(); ++j) {
        queryPath += temp[j];
        if(temp[j]=='+' || temp[j]=='-'){
            counter++;
            if(counter!=pmCounter)queryPath += '/';
        }
    }
    return  queryPath;
}