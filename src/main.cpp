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

struct bestPlan {
    std::string executedQuery;
    uint32_t cost;
};

std::vector<std::string> split(const std::string &s, char delim) {
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> tokens;
    while (getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}

// this part will be changed later.
uint32_t estimateCostOfJoin(std::string left, std::string right, std::shared_ptr<SimpleEstimator> est){
    std::vector<std::pair<uint32_t, char>> parsedQuery;
    parsedQuery.emplace_back(std::stoi(left.substr(0,left.size()-1)),left.at(1));
    parsedQuery.emplace_back(std::stoi(left.substr(0,left.size()-1)),right.at(1));
    cardStat card = est->estimateQuery(parsedQuery);
    return card.noPaths;
}

bestPlan plan;
// use dynamic programming to find the best plan.
bestPlan findBestPlan(std::string query, std::shared_ptr<SimpleGraph> &graph, std::shared_ptr<SimpleEstimator> est){

    std::string temp = "";
    for (int j = 0; j < plan.executedQuery.size(); ++j) {
        if( plan.executedQuery[j] != '(' && plan.executedQuery[j] != ')' && plan.executedQuery[j]!= '/') temp += plan.executedQuery[j];
    }
    if( temp==query ) return plan; // best plan already calculated.

    uint32_t relationCounter =0;
    for (int k = 0; k < query.size(); ++k) {
        if( query[k] == '/') relationCounter++;
    }
    if(relationCounter == 1){
        plan.executedQuery = "(" + query + ")";
        std::string leftLabel = split(query,'/')[0];
        std::string rightLabel = split(query,'/')[1];
        plan.cost = estimateCostOfJoin(leftLabel,rightLabel,est);
    }
    else{
        for (int i = 2; i < query.size()-1; ++i) {
            if( i%2 != 0 ) continue;

            // TODO, change the estimator, such taht it accept cardStatus input.
            std::string left = split(query,'/')[0];
            std::string right = split(query,'/')[split(query,'/').size()-1];

            bestPlan p1 = findBestPlan(left,graph,est);
            bestPlan p2 = findBestPlan(right,graph,est);
            // cost of joining p1 and p2.
            std::string leftLabel = p1.executedQuery.substr(p1.executedQuery.size()-3,2);
            std::string rightLabel = p2.executedQuery.substr(1,2);
            uint32_t costOfJoin = estimateCostOfJoin(leftLabel,rightLabel,est);
            uint32_t totalCost = costOfJoin + p1.cost + p2.cost;
            if(totalCost < plan.cost) {
                plan.cost = totalCost;
                plan.executedQuery = "(" + p1.executedQuery + p2.executedQuery +")";
            }
        }
    }
    return plan;
}

// remove the brackets of the input.
std::string preParse(std::string str,std::shared_ptr<SimpleGraph> &graph, std::shared_ptr<SimpleEstimator> est){

    // if this query only contains 1 or 2 relation, no need to find a good plan, since this is only 1 plan possible..
    uint32_t relationCounter =0;
    for (int k = 0; k < str.size(); ++k) {
        if( str[k] == '/') relationCounter++;
    }
    if(relationCounter <= 2)
        return  str;

    std::string parsed = "";
    for (int i = 0; i < str.size(); ++i) {
        if(str[i]=='('||str[i]==')') continue;
        else parsed+=str[i];
    }
    return  findBestPlan(parsed,graph,est).executedQuery;
}

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

        std::string preParsedQuery = preParse(queryPath,g,est);
        std::cout << "preparsed plan is:: " <<preParsedQuery;

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
        std::cout << "Time to evaluate: " << std::chrono::duration<double, std::milli>(end - start).count() << " ms" << std::endl;

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

    estimatorBench(graphFile, queriesFile);
    evaluatorBench(graphFile, queriesFile);

    return 0;
}


