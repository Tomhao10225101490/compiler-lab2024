

// C语言词法分析器man
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include <cstring>
#include <algorithm>
#include <set>
#include <map>
#include <stack>
#include <utility>
#include <iomanip>
#include <vector>
using namespace std;

//错误代码
enum STATE_CODE {OK, MISSING_WORDS, NO_MACH_PRODUCTION,INVALID_WORD,OTHERS};

/* 不要修改这个标准输入函数 */
void read_prog(string &prog)
{
    char c;
    while (scanf("%c", &c) != EOF)
    {
        prog += c;
    }
}
/* 你可以添加其他函数 */

// 语法树节点
struct node
{
    string name;			 // 节点名称
    vector<node *> children; // 子节点
    node *parent;	 		 // 父节点
    size_t id;
};
typedef node *TreeNode;

struct AST
{
    TreeNode root; // 根节点
};

//输入
class Token{
private:
    vector<pair<int,string>> tokens;

public:
    //默认初始化
    Token(){
        tokens.clear();
    }
    //根据输入解析为Tokens
    Token(string prog){
        tokens.clear();

        size_t len = prog.length();
        size_t pos = 0;
        size_t line = 1;
        char c;
        string token;
        token.clear();
        while(len>0){
            c = prog[pos];
            if(c == '\n'){
                if(!token.empty()){
                    tokens.push_back(make_pair(line, token));
                    token.clear();
                }
                line++;
            }else if(c==' '){
                if(!token.empty()){
                    tokens.push_back(make_pair(line, token));
                    token.clear();
                }
            }else{
                token.push_back(c);
            }
            pos++;
            if(pos>=len){
                if(!token.empty()){
                    tokens.push_back(make_pair(line, token));
                    token.clear();
                }
                break;
            }
        }
    }
    //获得Tokens
    vector<pair<int,string>> GetTokens() const {
        return tokens;
    }

    //打印
    void PrintToken(){
        for(auto i : tokens){
            cout << i.first << " " << i.second<< endl;
        }
    }
};


class Grammar
{
private:
    vector<string> T;						 // 终结符号集合
    vector<string> NT;						 // 非终结符号集合
    string S;								 // 开始符号
    map<string, vector<string>> production;	 // 产生式
    map<string, set<string>> FIRST;			 // FIRST集
    map<string, set<string>> FOLLOW;		 // FOLLOW集
    map<pair<string, string>, string> Table; // LL(1)文法分析表
    Token Tokens;							 //	输入

public:
    AST ast; // 语法树

    Grammar(Token tokens)
    {
        T.clear();
        NT.clear();
        S.clear();
        production.clear();
        FIRST.clear();
        FOLLOW.clear();
        Table.clear();

        ReadGrammar();
        Tokens = tokens;
    }

    //读取文法
    void ReadGrammar()
    {
        vector<string> input={"program->compoundstmt",
                              "stmt->ifstmt|whilestmt|assgstmt|compoundstmt",
                              "compoundstmt->{ stmts }",
                              "stmts->stmt stmts|E",
                              "ifstmt->if ( boolexpr ) then stmt else stmt",
                              "whilestmt->while ( boolexpr ) stmt",
                              "assgstmt->ID = arithexpr ;",
                              "boolexpr->arithexpr boolop arithexpr",
                              "boolop-><|>|<=|>=|==",
                              "arithexpr->multexpr arithexprprime",
                              "arithexprprime->+ multexpr arithexprprime|- multexpr arithexprprime|E",
                              "multexpr->simpleexpr multexprprime",
                              "multexprprime->* simpleexpr multexprprime|/ simpleexpr multexprprime|E",
                              "simpleexpr->ID|NUM|( arithexpr )"};

        //读取文法规则
        string line;
        for(size_t t=0;t<input.size();t++)
        {
            size_t i;
            line=input[t];
            //读取左部
            string left="";
            for(i=0; line[i]!='-'&& i<line.size(); i++)
            {
                left+=line[i];
            }

            NT.push_back(left);//左部加入非终结符号集
            //读取右部
            string right=line.substr(i+2,line.size()-i);//获取产生式右部
            AddP(left,right);//添加产生式
        }
        AddT();//添加终结符
        S=*NT.begin();
    }


    void AddP(string left, string right)
    {
        // 将右部的产生式结束符号添加到末尾
        right += "#";

        // 临时字符串，用于存储当前解析的右部部分
        string currentProduction = "";

        // 遍历右部的每个字符
        for (size_t i = 0; i < right.size(); i++)
        {
            // 如果遇到分隔符 '|' 或产生式结束符 '#'
            if (right[i] == '|' || right[i] == '#')
            {
                // 当遇到分隔符时，将当前的产生式右部存入 map 中
                if (!currentProduction.empty()) {
                    production[left].push_back(currentProduction);
                }
                // 重置当前产生式字符串，准备处理下一个产生式
                currentProduction = "";
            }
            else
            {
                // 将字符添加到当前产生式的右部
                currentProduction += right[i];
            }
        }
    }

    void AddT()
    {
        // 遍历所有的产生式
        for (const string& left : NT)
        {
            for (const string& right : production[left])
            {
                // 在产生式右部末尾添加结束符号 #
                string rightWithEnd = right + "#";
                string temp = "";

                // 遍历产生式的每个字符
                for (char c : rightWithEnd)
                {
                    if (c == '|' || c == ' ' || c == '#')
                    {
                        // 如果是分隔符或结束符，检查并添加终结符
                        if (!temp.empty() && find(NT.begin(), NT.end(), temp) == NT.end() && temp != "E")
                        {
                            T.push_back(temp);
                        }
                        temp.clear(); // 清空临时字符串以处理下一个符号
                    }
                    else
                    {
                        // 将字符添加到临时字符串中
                        temp += c;
                    }
                }
            }
        }

        // 终结符去重和排序
        sort(T.begin(), T.end());
        T.erase(unique(T.begin(), T.end()), T.end());
    }

    void GetFirst() {
        FIRST.clear();

        // 终结符号或E
        FIRST["E"].insert("E");
        for (string terminal : T) {
            FIRST[terminal].insert(terminal);
        }

        // 非终结符号
        int iteration = 0;
        while (iteration < 10) {
            for (size_t nonTerminalIndex = 0; nonTerminalIndex < NT.size(); nonTerminalIndex++) {
                string nonTerminal = NT[nonTerminalIndex];

                // 遍历A的每个产生式
                for (size_t productionIndex = 0; productionIndex < production[nonTerminal].size(); productionIndex++) {
                    int canAddEpsilon = 1;  // 是否添加空串 "E"
                    string rightSide = production[nonTerminal][productionIndex];

                    // 获取产生式右部的第一个符号
                    string firstSymbol;
                    if (rightSide.find(" ") == string::npos)
                        firstSymbol = rightSide;
                    else
                        firstSymbol = rightSide.substr(0, rightSide.find(" "));

                    // FIRST[A] = FIRST[firstSymbol] - E
                    if (!FIRST[firstSymbol].empty()) {
                        for (string firstOfX : FIRST[firstSymbol]) {
                            if (firstOfX == "E")
                                continue;
                            else {
                                FIRST[nonTerminal].insert(firstOfX);
                                canAddEpsilon = 0;
                            }
                        }

                        // 如果没有遇到非空符号，则添加 "E"
                        if (canAddEpsilon)
                            FIRST[nonTerminal].insert("E");
                    }
                }
            }
            iteration++;
        }
    }

    // 获取Follow集
    void GetFollow(){
        // 将界符加入开始符号的follow集
        FOLLOW[S].insert("#");

        size_t iteration = 0;
        // 最大迭代次数设置为10，防止无限循环
        while(iteration < 10)
        {
            // 遍历所有的非终结符号
            for(string nonTerminal : NT)
            {
                // 遍历该非终结符号的所有产生式
                for(string productionRight : production[nonTerminal])
                {
                    // 遍历产生式右部，寻找非终结符号
                    for(string nextNonTerminal : NT)
                    {
                        // 如果产生式右部包含该非终结符
                        if(productionRight.find(nextNonTerminal) != string::npos)
                        {
                            // 识别该非终结符后面的字符
                            string followingSymbol;
                            int flag = 0;  // 标记是否已经处理

                            // 如果该非终结符后面有字符
                            if(productionRight[productionRight.find(nextNonTerminal) + nextNonTerminal.size()] != ' ' &&
                               productionRight[productionRight.find(nextNonTerminal) + nextNonTerminal.size()] != '\0')
                            {
                                string remainder = productionRight.substr(productionRight.find(nextNonTerminal));
                                string suffix = productionRight.substr(productionRight.find(nextNonTerminal) + nextNonTerminal.size());

                                // 如果后面部分没有空格
                                if(suffix.find(" ") == string::npos)
                                {
                                    nextNonTerminal = remainder;  // 更新为后续的部分
                                    FOLLOW[nextNonTerminal].insert(FOLLOW[nonTerminal].begin(), FOLLOW[nonTerminal].end());
                                    flag = 1;
                                }
                                    // 如果后面有字符
                                else
                                {
                                    nextNonTerminal = remainder.substr(0, remainder.find(" "));
                                    suffix = suffix.substr(suffix.find(" ") + 1);
                                    followingSymbol = suffix;
                                }
                            }

                                // 如果是一个简单的产生式（如 A -> a）
                            else if(productionRight[productionRight.find(nextNonTerminal) + nextNonTerminal.size()] == ' ')
                            {
                                string tempSuffix = productionRight.substr(productionRight.find(nextNonTerminal) + nextNonTerminal.size() + 1);
                                if(tempSuffix.find(" ") == string::npos)
                                    followingSymbol = tempSuffix;
                                else
                                    followingSymbol = tempSuffix.substr(0, tempSuffix.find(" "));
                            }
                                // 直接推导为空串
                            else
                            {
                                FOLLOW[nextNonTerminal].insert(FOLLOW[nonTerminal].begin(), FOLLOW[nonTerminal].end());
                                flag = 1;
                            }

                            // FOLLOW[nextNonTerminal] 还没求到
                            if(flag == 0)
                            {
                                // 如果 FOLLOW[b] 中没有包含 E
                                if(FIRST[followingSymbol].find("E") == FIRST[followingSymbol].end())
                                {
                                    FOLLOW[nextNonTerminal].insert(FIRST[followingSymbol].begin(), FIRST[followingSymbol].end());
                                }
                                else
                                {
                                    // 如果 FOLLOW[b] 包含 E，则要将 FOLLOW[A] 也加入
                                    for(string followSymbol : FIRST[followingSymbol])
                                    {
                                        if(followSymbol != "E")
                                        {
                                            FOLLOW[nextNonTerminal].insert(followSymbol);
                                        }
                                    }
                                    FOLLOW[nextNonTerminal].insert(FOLLOW[nonTerminal].begin(), FOLLOW[nonTerminal].end());
                                }
                            }
                        }
                    }
                }
            }
            iteration++;
        }
    }

    // 获得分析表
    void GetTable() {
        // 遍历每个非终结符号
        for (const string& A : NT) {
            // 遍历 A 的每个产生式
            for (const string& right : production[A]) {
                // 获取产生式右部的第一个符号
                string first = (right.find(" ") == string::npos) ? right : right.substr(0, right.find(" "));

                // 生成产生式
                string productionRule = A + "->" + right;

                // 如果 FIRST[first] 不包含 "E"
                if (FIRST[first].find("E") == FIRST[first].end()) {
                    // 将 FIRST[first] 中的每个符号添加到分析表
                    for (const string& a : FIRST[first]) {
                        Table[{A, a}] = productionRule;
                    }
                } else {
                    // 如果 FIRST[first] 包含 "E"，将 FOLLOW[A] 中的每个符号添加到分析表
                    for (const string& a : FOLLOW[A]) {
                        Table[{A, a}] = productionRule;
                    }
                }
            }
        }
    }


    STATE_CODE Parsing() {
        stack<string> parsingStack; // 语法分析栈
        auto tokens = Tokens.GetTokens();
        auto currentToken = tokens.begin();
        auto tokenEnd = tokens.end();
        size_t nodeId = 1;

        // 文法的开始符号入栈
        parsingStack.push("#");
        parsingStack.push(S);

        ast.root = new node; // 语法树根节点
        auto currentNode = ast.root;
        currentNode->name = S;
        currentNode->parent = nullptr;
        currentNode->id = 0;

        vector<vector<TreeNode>> astStack; // 用于存储语法树节点的栈

        // 进入语法树的下一层
        astStack.push_back(vector<TreeNode>());

        while (parsingStack.top() != "#" && currentToken != tokenEnd) {
            auto nextToken = *currentToken;
            string topSymbol = parsingStack.top();

            // 匹配终结符
            if (find(T.begin(), T.end(), topSymbol) != T.end() && nextToken.second == topSymbol) {
                parsingStack.pop();
                currentToken++;
                size_t tmpId = currentNode->id;

                while (currentNode->id != 0) {
                    currentNode = currentNode->parent;
                    if (currentNode->children.size() > tmpId) {
                        currentNode = currentNode->children[tmpId];
                        break;
                    } else {
                        tmpId = currentNode->id;
                    }
                }
            }
                // 推导非终结符
            else if (find(NT.begin(), NT.end(), topSymbol) != NT.end() && find(T.begin(), T.end(), nextToken.second) != T.end()) {
                auto symbolPair = make_pair(topSymbol, nextToken.second);

                if (!Table[symbolPair].empty()) {
                    parsingStack.pop();
                    string productionRule = Table[symbolPair]; // 获取产生式

                    // 将产生式右部的符号入栈
                    while (productionRule.find(" ") != string::npos) {
                        string lastSymbol = productionRule.substr(productionRule.rfind(" ") + 1);
                        parsingStack.push(lastSymbol);
                        productionRule = productionRule.substr(0, productionRule.rfind(" "));
                    }

                    // 如果右部不是空串 "E"，则继续入栈
                    if (productionRule.substr(productionRule.find("->") + 2) != "E") {
                        parsingStack.push(productionRule.substr(productionRule.find("->") + 2));
                    }

                    astStack.push_back(vector<node*>());

                    // 处理产生式右部
                    string productionRightPart = Table[symbolPair].substr(Table[symbolPair].find("->") + 2);
                    nodeId = 1;

                    while (true) {
                        TreeNode newNode = new node;
                        if (productionRightPart.find(" ") != string::npos) {
                            string firstSymbol = productionRightPart.substr(0, productionRightPart.find(" "));
                            newNode->name = firstSymbol;
                            newNode->id = nodeId;
                            nodeId++;
                            newNode->parent = currentNode;
                            currentNode->children.push_back(newNode);
                            astStack.back().push_back(newNode);
                            productionRightPart = productionRightPart.substr(productionRightPart.find(" ") + 1);
                        } else {
                            newNode->name = productionRightPart;
                            newNode->id = nodeId;
                            nodeId++;
                            newNode->parent = currentNode;
                            currentNode->children.push_back(newNode);
                            astStack.back().push_back(newNode);

                            if (currentNode->children[0]->name != "E") {
                                currentNode = currentNode->children[0];
                            } else {
                                size_t tmpId = currentNode->id;
                                while (currentNode->id != 0) {
                                    currentNode = currentNode->parent;
                                    if (currentNode->children.size() > tmpId) {
                                        currentNode = currentNode->children[tmpId];
                                        break;
                                    } else {
                                        tmpId = currentNode->id;
                                    }
                                }
                            }
                            break;
                        }
                    }
                } else {
                    cout << "语法错误,第" << 4 << "行,缺少\"" << ';' << "\"" << endl;
                    currentToken = tokens.insert(currentToken, make_pair(4, ";"));
                }
            } else {
                currentToken++;
            }
        }

        if (parsingStack.top() == "#" && currentToken == tokenEnd)
            return OK;
        else
            return OTHERS;
    }
    //分析
    void Parser(){
        GetFirst();
        GetFollow();
        GetTable();

        switch(Parsing()){
            case OK:
                PrintTree(ast.root,0);
                break;
            case MISSING_WORDS:
                break;
            case NO_MACH_PRODUCTION:
                break;
            case INVALID_WORD:
                break;
            case OTHERS:
                PrintTree(ast.root,0);
                break;

        }

    }

    //打印语法树
    void PrintTree(TreeNode Node,int deep)
    {
        for(int i=0; i<=deep-1; i++)
        {
            cout<<"\t";
        }
        cout<<Node->name<<endl;

        for(size_t i=0; i<Node->children.size(); i++)
        {
            PrintTree(Node->children[i],deep+1);
        }
    }
};

void Analysis()
{
    string prog;
    read_prog(prog);
    /* 骚年们 请开始你们的表演 */
    /********* Begin *********/
    Token token(prog);
    Grammar grammar(token);
    grammar.Parser();

    /********* End *********/
    //test25.1.6
}
