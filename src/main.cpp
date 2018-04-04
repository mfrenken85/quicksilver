#include <iostream>
#include <chrono>
#include <SimpleGraph.h>
#include <Estimator.h>
#include <SimpleEstimator.h>
#include <SimpleEvaluator.h>


struct query {
    std::string s;
    std::string path;
    std::string t;

    void print() {
        std::cout << s << ", " << path << ", " << t << std::endl;
    }
};

/*
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

std::map<std::string,bestPlan> plans;
// Dynamic programming to find the best plan.
bestPlan findBestPlan(std::string originalQuery, std::string query, std::shared_ptr<SimpleGraph> &graph, std::shared_ptr<SimpleEstimator> est){

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

// remove the brackets of the input.
std::string preParse(std::string str,std::shared_ptr<SimpleGraph> &graph, std::shared_ptr<SimpleEstimator> est){
    // if this query only contains 1 or 2 relation, no need to find a good plan, since this is only 1 plan possible..
    if(split(str,'/').size() <= 2)
        return  str;

    std::string parsed = "";
    for (int i = 0; i < str.size(); ++i) {
        if(str[i]=='('||str[i]==')') continue;
        else parsed+=str[i];
    }

    std::string temp = findBestPlan(parsed,parsed,graph,est).executedQuery;
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
*/

std::vector<query> parseQueries(std::string &fileName) {

    std::vector<query> queries {};

    std::string line;
    std::ifstream graphFile { fileName };

    std::regex edgePat (R"((.+),(.+),(.+))");

    while(std::getline(graphFile, line)) {
        std::smatch matches;

        // match edge data
        if(std::regex_search(line, matches, edgePat)) {
            auto s = matches[1];
            auto path = matches[2];
            auto t = matches[3];

            queries.emplace_back(query{s, path, t});
        }
    }

    graphFile.close();

    if(queries.size() == 0) std::cout << "Did not parse any queries... Check query file." << std::endl;

    return queries;
}

int estimatorBench(std::string &graphFile, std::string &queriesFile) {

    std::cout << "\n(1) Reading the graph into memory and preparing the estimator...\n" << std::endl;

    // read the graph
    auto g = std::make_shared<SimpleGraph>();

    auto start = std::chrono::steady_clock::now();
    try {
        g->readFromContiguousFile(graphFile);
    } catch (std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    auto end = std::chrono::steady_clock::now();
    std::cout << "Time to read the graph into memory: " << std::chrono::duration<double, std::milli>(end - start).count() << " ms" << std::endl;

    // prepare the estimator
    auto est = std::make_unique<SimpleEstimator>(g);
    start = std::chrono::steady_clock::now();
    est->prepare();
    end = std::chrono::steady_clock::now();
    std::cout << "Time to prepare the estimator: " << std::chrono::duration<double, std::milli>(end - start).count() << " ms" << std::endl;

    std::cout << "\n(2) Running the query workload..." << std::endl;

    for(auto query : parseQueries(queriesFile)) {

        // perform estimation
        // parse the query into an AST
        std::cout << "\nProcessing query: ";
        query.print();

        RPQTree *queryTree = RPQTree::strToTree(query.path);
        std::cout << "Parsed query tree: ";
        queryTree->print();

        start = std::chrono::steady_clock::now();
        auto estimate = est->estimate(queryTree);
        end = std::chrono::steady_clock::now();

        std::cout << "\nEstimation (noOut, noPaths, noIn) : ";
        estimate.print();
        std::cout << "Time to estimate: " << std::chrono::duration<double, std::milli>(end - start).count() << " ms" << std::endl;

        // perform evaluation
        auto ev = std::make_unique<SimpleEvaluator>(g);
        ev->prepare();
        start = std::chrono::steady_clock::now();
        auto actual = ev->evaluate(queryTree);
        end = std::chrono::steady_clock::now();

        std::cout << "Actual (noOut, noPaths, noIn) : ";
        actual.print();
        std::cout << "Time to evaluate: " << std::chrono::duration<double, std::milli>(end - start).count() << " ms" << std::endl;

        // clean-up
        delete(queryTree);

    }

    return 0;
}

int evaluatorBench(std::string &graphFile, std::string &queriesFile) {

    std::cout << "\n(1) Reading the graph into memory and preparing the evaluator...\n" << std::endl;

    // read the graph
    auto g = std::make_shared<SimpleGraph>();

    auto start = std::chrono::steady_clock::now();
    try {
        g->readFromContiguousFile(graphFile);
    } catch (std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    auto end = std::chrono::steady_clock::now();
    std::cout << "Time to read the graph into memory: " << std::chrono::duration<double, std::milli>(end - start).count() << " ms" << std::endl;

    // prepare the evaluator
    auto est = std::make_shared<SimpleEstimator>(g);
    auto ev = std::make_unique<SimpleEvaluator>(g);
    ev->attachEstimator(est);

    start = std::chrono::steady_clock::now();
    ev->prepare();
    end = std::chrono::steady_clock::now();
    std::cout << "Time to prepare the evaluator: " << std::chrono::duration<double, std::milli>(end - start).count() << " ms" << std::endl;

    std::cout << "\n(2) Running the query workload..." << std::endl;

    for(auto query : parseQueries(queriesFile)) {

        // choose the best plan.
        std::string queryPath = query.path;

        // TODO: Move the preparser to evaluator.cpp.
        //std::string preParsedQuery = preParse(queryPath,g,est);
        //std::cout << "preparsed plan is:: " <<preParsedQuery;

        // parse the query into an AST
        std::cout << "\nProcessing query: ";
        query.print();
        RPQTree *queryTree = RPQTree::strToTree(queryPath);
        std::cout << "Parsed query tree: ";
        queryTree->print();

        // perform the evaluation
        start = std::chrono::steady_clock::now();
        auto actual = ev->evaluate(queryTree);
        end = std::chrono::steady_clock::now();

        std::cout << "\nActual (noOut, noPaths, noIn) : ";
        actual.print();
        std::cout << "Time to evaluate: " << std::chrono::duration<double, std::milli>(end - start).count() << " ms\n" << std::endl;

        // clean-up
        delete(queryTree);

    }

    return 0;
}

int main(int argc, char *argv[]) {

    if(argc < 3) {
        std::cout << "Usage: quicksilver <graphFile> <queriesFile>" << std::endl;
        return 0;
    }

    // args
    std::string graphFile {argv[1]};
    std::string queriesFile {argv[2]};

    //estimatorBench(graphFile, queriesFile);
    evaluatorBench(graphFile, queriesFile);

    return 0;
}


