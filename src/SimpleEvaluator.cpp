//
// Created by Nikolay Yakovets on 2018-02-02.
//

#include "SimpleEstimator.h"
#include "SimpleEvaluator.h"
#include <chrono>
#include <sstream>

SimpleEvaluator::SimpleEvaluator(std::shared_ptr<SimpleGraph> &g) {

    // works only with SimpleGraph
    graph = g;
    est = nullptr; // estimator not attached by default
}

SimpleEvaluator::~SimpleEvaluator() {
    // clean the memory.
    cachedBestPlans.clear();
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
    g = nullptr;
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

// split the string by a specific character.
std::vector<std::string> split(const std::string &s, char delim) {
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> tokens;
    while (std::getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}

// parse the query into plain text.
std::string queryToString(RPQTree *query) {
    std::string r = "";
    std::string p = "";
    std::string q = "";

    if(query->left == nullptr && query->right == nullptr) {
        r = ' ';
        r+=query->data;
        r += ' ';
    } else {
        if(query->left != nullptr) p = queryToString(query->left);
        if(query->right!= nullptr) q = queryToString(query->right);
        //r = '(' << data << ' ' << p << q << ')';
        r += '(';
        r += p;
        r += query->data;
        r += q;
        r += ')';
    }
    return r;
}

cardStat SimpleEvaluator::evaluate(RPQTree *query) {

    // Since we do not want to change the program's structure,
    // we convert the query -> string
    // optimize the string and convert the string -> query again.
    // In this case, indeed there will be one time useless query to string and string to query transformation.
    // But since this is pretty cheap, and this save much time of programming since this does not change the program's structure.
    // Hence, we implemented it in this way.

    auto start = std::chrono::steady_clock::now();
    auto querypath = queryToString(query);
    querypath = preParse(querypath, graph, est);
    RPQTree *newQuery = RPQTree::strToTree(querypath);
    auto end = std::chrono::steady_clock::now();
    std::cout << "\nTime to select the best execution plan is: "
              << std::chrono::duration<double, std::milli>(end - start).count() << " ms" << std::endl;
    std::cout << "The best execution plan is: " + querypath << std::endl;

    start = std::chrono::steady_clock::now();
    auto res = evaluate_aux(newQuery);
    end = std::chrono::steady_clock::now();
    std::cout << "Time to execute the best execution plan is: "
              << std::chrono::duration<double, std::milli>(end - start).count() << " ms" << std::endl;

    /*
    auto start = std::chrono::steady_clock::now();
    auto res = evaluate_aux(query);
    auto end = std::chrono::steady_clock::now();
    std::cout << "Time to execute the original execution plan (without plan selecton) is: " << std::chrono::duration<double, std::milli>(end - start).count() << " ms" << std::endl;
    */
    return SimpleEvaluator::computeStats(res);
}

// We simply do the estimation on the entire 2 subtrees.
// The ideal case would be that, the estimator will only estimate the join cost of two subtrees.
// i.e, oin the right most element of the left subtree with the left most element of the right subtree.
// Instead of estimating the entire 2 subtrees.
// But since estimation is pretty cheap, and this is easier to implement, it is implemented in this way.
uint32_t estimateCostOfJoin(std::string left, std::string right, std::shared_ptr<SimpleEstimator> &est){
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

// We use dynamic programming to find the best query execution plan.
// For further development, we could also implement the greedy algorithm.
// Since dynamic programming may not be suitable for very larger queries (like number of joins> 15).
// We could implement both greedy algorithm and dynamic programming, when larger queries are encountered.
// We firstly use greedy algorithm to split the query and process the small queries with dynamic programming.
bestPlan SimpleEvaluator::findBestPlan(std::string originalQuery, std::string query, std::shared_ptr<SimpleGraph> &graph, std::shared_ptr<SimpleEstimator> &est){

    if( intermediatePlans.count(originalQuery) !=0 )
        return intermediatePlans[originalQuery]; // best plan already calculated.

    // if same query is already processed.
    if(cachedBestPlans.count(query)!=0){
        intermediatePlans[query].executedQuery = cachedBestPlans[query].executedQuery;
        intermediatePlans[query].cost = cachedBestPlans[query].cost;
    }else{
        auto splits = split(query,'/');
        uint32_t splitsSize = splits.size();
        if ( splitsSize==1 ){
            if (intermediatePlans.count(query) == 0) {
                bestPlan p;
                p.executedQuery = "";
                p.cost = std::numeric_limits<int>::max();
                intermediatePlans[query] = p;
            }
            intermediatePlans[query].executedQuery = query;
            intermediatePlans[query].cost = estimateCostOfJoin(query, "", est);
        } else if( splitsSize == 2){
            if (intermediatePlans.count(query) == 0) {
                bestPlan p;
                p.executedQuery = "";
                p.cost = std::numeric_limits<int>::max();
                intermediatePlans[query] = p;
            }
            std::string leftLabel = splits[0];
            std::string rightLabel = splits[1];
            intermediatePlans[query].executedQuery = "(" + leftLabel + rightLabel + ")";
            intermediatePlans[query].cost = estimateCostOfJoin(leftLabel, rightLabel, est);
        }
        else{
           for (int i = 0; i < splits.size() - 1; ++i) {
                std::string leftSub = "";
                std::string rightSub = "";
                for (int j = 0; j < splits.size(); ++j) {
                    if (j <= i) leftSub += splits[j] + '/';
                    else rightSub += splits[j] + '/';
                }
                // remove the last "/"
                leftSub = leftSub.substr(0, leftSub.size() - 1);
                rightSub = rightSub.substr(0, rightSub.size() - 1);
                bestPlan p1 = SimpleEvaluator::findBestPlan(originalQuery, leftSub, graph, est);
                bestPlan p2 = SimpleEvaluator::findBestPlan(originalQuery, rightSub, graph, est);
                // cost of joining
                std::string leftLabel = p1.executedQuery;
                std::string rightLabel = p2.executedQuery;
                uint32_t totalCost = estimateCostOfJoin(leftLabel, rightLabel, est);
                if (intermediatePlans.count(query) == 0) {
                    bestPlan p;
                    p.executedQuery = "";
                    p.cost = std::numeric_limits<int>::max();
                    intermediatePlans[query] = p;
                }
                if (totalCost < intermediatePlans[query].cost) {
                    std::string leftquery = p1.executedQuery;
                    std::string rightquery = p2.executedQuery;
                    // if there is only 1 relation, then no brackets allowed.
                    uint32_t leftpmCounter = 0;
                    uint32_t rightpmCounter = 0;
                    for (int j = 0; j < leftquery.size(); ++j) {
                        if (leftquery[j] == '+' || leftquery[j] == '-') leftpmCounter++;
                    }
                    for (int j = 0; j < rightquery.size(); ++j) {
                        if (rightquery[j] == '+' || rightquery[j] == '-') rightpmCounter++;
                    }
                    if (leftpmCounter != 1) leftquery = '(' + leftquery + ')';
                    if (rightpmCounter != 1) rightquery = '(' + rightquery + ')';
                    intermediatePlans[query].executedQuery = leftquery + rightquery;
                    intermediatePlans[query].cost = totalCost;
                }
            }
        }
        // cache the intermediate results.
        cachedBestPlans[query] = intermediatePlans[query];
    }
    return intermediatePlans[query];
}

// preparse the input query, select the best query execution plan.
std::string SimpleEvaluator::preParse(std::string str,std::shared_ptr<SimpleGraph> &graph, std::shared_ptr<SimpleEstimator> &est){
    // if this query only contains 1 or 2 relation, no need to find a good plan, since this is only 1 plan possible.
    if(split(str,'/').size() <= 2)
        return  str;

    std::string parsed = "";
    for (int i = 0; i < str.size(); ++i) {
        if(str[i]=='('||str[i]==')') continue;
        else parsed+=str[i];
    }

    std::string temp = "";
    // if same query has already been executed before.
    if(cachedBestPlans.count(parsed)!=0) {
        temp = cachedBestPlans[parsed].executedQuery;
    }
    else {
        temp = SimpleEvaluator::findBestPlan(parsed, parsed, graph, est).executedQuery;
        // cache the best plan.
        cachedBestPlans[parsed] = intermediatePlans[parsed];
    }

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
    // clean the caches.
    intermediatePlans.clear();
    return  queryPath;
}