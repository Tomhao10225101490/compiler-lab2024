#pragma once

#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <algorithm>

const int maxn = 100 + 10;

// 全局变量
std::string token[maxn], prog;
int cnt = 0;

// 文法分析相关结构
struct Gram {
    std::string leftSide;
    std::vector<std::string> rightSide;
    int dotPosition;
};

Gram gram[100];

struct Set {
    std::vector<std::string> leftSymbols;
    std::vector<std::string> rightSymbols;
    int canBeEmpty = 0;
} First[30], Follow[30];

// 全局变量
int productionCount = 0;
std::string tempSymbols[30];
int symbolCount = 0;
int emptyFlag = 0;
int firstSetCount = 0;
int followSetCount = 0;
bool isused_gram[maxn];

// LR分析相关变量
Gram I[100][100];
int Inum[100];
int statenum;

bool used_state[110];
bool get_state[110];
std::string trans_state_table[110][110];
bool is_trans_state_table[110][110];

int update_state[100], updatenum, topnum = 1;

// SLR表相关变量
int slt_table[maxn][maxn];
int MorS[maxn][maxn];
std::string T[maxn], NT[maxn];
int numT = 0;

// 分析栈相关变量
struct action_LR_Parse_stack {
    int state;
    std::string symbol;
} parseStack[maxn];

std::string input[maxn];
int output[maxn];
int stackTop = 0;
int inputTop = 0;
int outputTop = 0;
std::string printSymbols[maxn];
int printTop = 0;

// 函数声明部分
void read_prog(std::string& prog);
int gettoken(int currentPos);
void token_divide();
void gram_divide(std::string input[], int num);
bool isend(const std::string& symbol);
void Get_First(const std::string& s);
void deal_first();
void get_follow();
void deal_follow();
void deal_automaton();
void Grammatical_Analysis();
void slr_analysis();
void print_pris(int stepNumber);
void print_max_right();
void Analysis();

// 函数实现部分
void read_prog(std::string& prog) {
    char c;
    while(scanf("%c", &c) != EOF) {
        prog += c;
    }
}

// 获取单个token的函数
int gettoken(int currentPos) {
    std::string tokenStr;
    int beginPos = currentPos;
    int finalPos;
    
    // 读取直到遇到空格或换行符
    while(currentPos < prog.length()) {
        char currChar = prog[currentPos];
        
        // 检查是否遇到分隔符
        if(currChar == ' ' || currChar == '\n') {
            finalPos = currentPos;
            break;
        }
        
        // 累积字符
        tokenStr += currChar;
        currentPos++;
    }
    
    // 如果到达程序末尾
    if(currentPos >= prog.length()) {
        finalPos = currentPos;
    }
    
    // 存储token
    if(!tokenStr.empty()) {
        cnt++;
        token[cnt] = tokenStr;
        
        // 调试输出
        // std::cout << "Token " << cnt << ": " << tokenStr << std::endl;
    }
    
    return finalPos;
}

// 词法分析相关函数
void token_divide() {
    size_t currentIndex = 0;
    size_t programLength = prog.length();
    
    // 遍历整个输入程序
    while(currentIndex < programLength) {
        char currentChar = prog[currentIndex];
        
        // 跳过空白字符
        if(currentChar == ' ' || currentChar == '\n' || currentChar == '\t' || currentChar == '\r') {
            currentIndex++;
            continue;
        }
        
        // 获取下一个token
        size_t nextIndex = gettoken(currentIndex);
        
        // 检查是否成功获取token
        if(nextIndex == currentIndex) {
            // 如果位置没有改变，说明可能有问题
            currentIndex++;
        } else {
            currentIndex = nextIndex;
        }
    }
}

// 文法分析相关函数
void gram_divide(std::string input[], int num) {
    // 遍历每条产生式
    for(int i = 0; i < num; i++) {
        std::string current_production = input[i];
        size_t arrow_pos = current_production.find("->");
        
        // 确保找到了箭头
        if(arrow_pos != std::string::npos) {
            // 提取并处理左部
            std::string left_side = current_production.substr(0, arrow_pos);
            // 去除可能的空白
            left_side.erase(0, left_side.find_first_not_of(" \t\n\r"));
            left_side.erase(left_side.find_last_not_of(" \t\n\r") + 1);
            gram[productionCount].leftSide = left_side;
            
            // 处理右部
            std::string right_side = current_production.substr(arrow_pos + 2);
            size_t pos = 0;
            std::string token;
            bool in_token = false;
            
            // 逐字符处理右部
            while(pos < right_side.length()) {
                char c = right_side[pos];
                
                // 处理空白字符
                if(c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                    if(in_token) {
                        // 结束当前token
                        if(!token.empty()) {
                            gram[productionCount].rightSide.push_back(token);
                            token.clear();
                        }
                        in_token = false;
                    }
                }
                // 处理或符号
                else if(c == '|') {
                    if(in_token) {
                        if(!token.empty()) {
                            gram[productionCount].rightSide.push_back(token);
                            token.clear();
                        }
                        in_token = false;
                    }
                    // 创建新的产生式
                    productionCount++;
                    gram[productionCount].leftSide = gram[productionCount-1].leftSide;
                }
                // 处理普通字符
                else {
                    token += c;
                    in_token = true;
                }
                pos++;
            }
            
            // 处理最后一个token
            if(!token.empty()) {
                gram[productionCount].rightSide.push_back(token);
            }
            
            // 移动到下一个产生式
            productionCount++;
        }
    }
}

// 使用不同的查找方式
bool isend(const std::string& symbol) {
    for(const auto& productionRule : std::vector<Gram>(gram, gram + productionCount)) {
        if(symbol == productionRule.leftSide) return false;
    }
    return true;
}

// First集合计算相关函数
void Get_First(const std::string& s) {
    // 遍历所有产生式
    for(int i = 0; i < productionCount; i++) {
        // 找到以s为左部且未使用过的产生式
        if(s == gram[i].leftSide && !isused_gram[i]) {
            isused_gram[i] = true;
            
            // 获取产生式右部的第一个符号
            const auto& first_symbol = *gram[i].rightSide.begin();
            
            if(isend(first_symbol)) {
                // 如果是终结符，直接加入First集
                bool already_exists = false;
                for(int j = 1; j <= symbolCount; j++) {
                    if(first_symbol == tempSymbols[j]) {
                        already_exists = true;
                        break;
                    }
                }
                
                if(!already_exists) {
                    tempSymbols[++symbolCount] = first_symbol;
                    // 如果是空串，设置标记
                    if(first_symbol == "E") emptyFlag = 1;
                }
            }
            else {
                // 如果是非终结符，递归计算其First集
                Get_First(first_symbol);
            }
        }
    }
}

// 扩充First集合相关函数
void deal_first() {
    // 对于每个产生式右侧，进行遍历递归求First集
    for(int i = 0; i < productionCount; i++) {
        // 将当前产生式右部复制到First集合的左部
        First[++firstSetCount].leftSymbols.assign(gram[i].rightSide.begin(), gram[i].rightSide.end());
        
        // 获取右部第一个符号
        auto first_symbol = First[firstSetCount].leftSymbols.begin();
        
        // 如果是终结符，直接加入First集
        if(isend(*first_symbol)) {
            // 检查是否为空串
            if(*first_symbol == "E") {
                First[firstSetCount].canBeEmpty = 1;  // 标记可以推导出空串
            }
            First[firstSetCount].rightSymbols.push_back(*first_symbol);
        }
        else {
            // 如果是非终结符，需要递归计算First集
            symbolCount = 0;
            emptyFlag = 0;
            
            // 清空使用标记数组
            std::memset(isused_gram, 0, sizeof(isused_gram));
            
            // 递归计算First集
            Get_First(*first_symbol);
            
            // 如果可以推导出空串，设置标记
            if(emptyFlag == 1) {
                First[firstSetCount].canBeEmpty = emptyFlag;
            }
            
            // 将计算得到的First集加入结果
            for(int j = 1; j <= symbolCount; j++) {
                First[firstSetCount].rightSymbols.push_back(tempSymbols[j]);
            }
        }
        
        // 调试输出
        /*
        std::cout << "First set for production " << i << ": ";
        std::cout << gram[i].leftSide << " -> ";
        for(const auto& symbol : gram[i].rightSide) {
            std::cout << symbol << " ";
        }
        std::cout << "\nFirst set: ";
        for(const auto& symbol : First[firstSetCount].rightSymbols) {
            std::cout << symbol << " ";
        }
        if(First[firstSetCount].canBeEmpty) {
            std::cout << "(can derive empty)";
        }
        std::cout << std::endl;
        */
    }
}

// 扩充Follow集合计算函数
void get_follow() {
    // 初始化第一个Follow集（开始符号）
    if(Follow[1].rightSymbols.empty()) {
        Follow[1].rightSymbols.push_back("$");
    }
    
    // 遍历所有非终结符
    for(int j = 1; j <= followSetCount; j++) {
        std::string current = *Follow[j].leftSymbols.begin();
        
        // 遍历所有产生式寻找当前非终结符的出现位置
        for(int i = 0; i < productionCount; i++) {
            for(auto it = gram[i].rightSide.begin(); it != gram[i].rightSide.end(); ++it) {
                if(*it == current) {
                    auto next = std::next(it);
                    
                    // 情况1：A -> αBβ
                    if(next != gram[i].rightSide.end()) {
                        // β是终结符
                        if(isend(*next)) {
                            if(*next != "E") {
                                // 检查是否已存在
                                bool exists = false;
                                for(const auto& symbol : Follow[j].rightSymbols) {
                                    if(symbol == *next) {
                                        exists = true;
                                        break;
                                    }
                                }
                                if(!exists) {
                                    Follow[j].rightSymbols.push_back(*next);
                                }
                            }
                        }
                        // β是非终结符
                        else {
                            symbolCount = 0;
                            emptyFlag = 0;
                            std::memset(isused_gram, 0, sizeof(isused_gram));
                            Get_First(*next);
                            
                            // 处理First(β)中的符号
                            for(int k = 1; k <= symbolCount; k++) {
                                if(tempSymbols[k] != "E") {
                                    bool exists = false;
                                    for(const auto& symbol : Follow[j].rightSymbols) {
                                        if(symbol == tempSymbols[k]) {
                                            exists = true;
                                            break;
                                        }
                                    }
                                    if(!exists) {
                                        Follow[j].rightSymbols.push_back(tempSymbols[k]);
                                    }
                                }
                            }
                            
                            // 如果β可以推导出空串，需要继续处理
                            if(emptyFlag == 1) {
                                // 将A的Follow集加入B的Follow集
                                for(int k = 1; k <= followSetCount; k++) {
                                    if(*Follow[k].leftSymbols.begin() == gram[i].leftSide) {
                                        for(const auto& symbol : Follow[k].rightSymbols) {
                                            bool exists = false;
                                            for(const auto& existing : Follow[j].rightSymbols) {
                                                if(existing == symbol) {
                                                    exists = true;
                                                    break;
                                                }
                                            }
                                            if(!exists) {
                                                Follow[j].rightSymbols.push_back(symbol);
                                            }
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    // 情况2：A -> αB
                    else {
                        // 将A的Follow集加入B的Follow集
                        for(int k = 1; k <= followSetCount; k++) {
                            if(*Follow[k].leftSymbols.begin() == gram[i].leftSide &&
                               *Follow[k].leftSymbols.begin() != current) {
                                for(const auto& symbol : Follow[k].rightSymbols) {
                                    bool exists = false;
                                    for(const auto& existing : Follow[j].rightSymbols) {
                                        if(existing == symbol) {
                                            exists = true;
                                            break;
                                        }
                                    }
                                    if(!exists) {
                                        Follow[j].rightSymbols.push_back(symbol);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void deal_T() {
    // 使用set来避免重复
    std::set<std::string> terminal_symbols;
    
    for(int i = 0; i < productionCount; i++) {
        for(const auto& symbol : gram[i].rightSide) {
            if(isend(symbol)) {
                terminal_symbols.insert(symbol);
            }
        }
    }
    
    // 将set中的符号转移到T数组
    for(const auto& symbol : terminal_symbols) {
        T[++numT] = symbol;
    }
}

// 初始化I0状态
void init_I0() {
    // 添加增广文法的起始产生式
    I[0][0].leftSide = "program'";  // 广文法的新起始符号
    I[0][0].rightSide.push_back("program");  // 指向原文法的起始符号
    I[0][0].dotPosition = 1;  // 点号位置初始化为1
    
    // 复制所有原始产生式到I0
    for(int i = 0; i < productionCount; i++) {
        I[0][i+1].leftSide = gram[i].leftSide;  // 复制左部
        I[0][i+1].rightSide = gram[i].rightSide;  // 复制右部
        I[0][i+1].dotPosition = 1;  // 点号位置初始化为1
    }
    
    // 设置I0中产生式的数量
    Inum[0] = productionCount + 1;
    
    // 调试输出
    /*
    std::cout << "Initial state I0:" << std::endl;
    for(int i = 0; i < Inum[0]; i++) {
        std::cout << I[0][i].leftSide << " -> ";
        int j = 1;
        for(const auto& symbol : I[0][i].rightSide) {
            if(j == I[0][i].dotPosition) std::cout << ". ";
            std::cout << symbol << " ";
            j++;
        }
        if(j == I[0][i].dotPosition) std::cout << ".";
        std::cout << std::endl;
    }
    */
}

// 从当前状态添加闭包项目
void from_or_state(int u) {
    // 遍历当前状态的所有项目
    for(int j = 0; j < Inum[u]; j++) {
        // 获取点号后的符号
        auto it = I[u][j].rightSide.begin();
        for(int k = 1; k < I[u][j].dotPosition; k++) {
            if(it != I[u][j].rightSide.end()) {
                ++it;
            }
        }
        
        // 如果点号已到末尾或点号后是终结符，跳过
        if(it == I[u][j].rightSide.end() || isend(*it)) {
            continue;
        }
        
        // 查找以点号后符号为左部的产生式
        for(int i = 0; i < Inum[0]; i++) {
            if(I[0][i].leftSide == *it && !get_state[i]) {
                // 添加新的项目到当前状态
                I[statenum][Inum[statenum]].leftSide = I[0][i].leftSide;
                I[statenum][Inum[statenum]].rightSide = I[0][i].rightSide;
                I[statenum][Inum[statenum]].dotPosition = I[0][i].dotPosition;
                Inum[statenum]++;
                get_state[i] = true;  // 标记该产生式已被使用
            }
        }
    }
}

// 检查两个项目是否相同
bool gram_is_equal(const Gram& x, const Gram& y) {
    // 检查左部、点号位置和右部是否都相同
    return x.leftSide == y.leftSide && 
           x.dotPosition == y.dotPosition && 
           x.rightSide == y.rightSide;
}

// 构建自动机的状态转换
void trans_automaton(int u) {
    // 清空状态使用标记
    std::memset(used_state, 0, sizeof(used_state));
    
    // 遍历当前状态的所有项目
    for(int i = 0; i < Inum[u]; i++) {
        if(used_state[i]) continue;
        used_state[i] = true;
        
        // 获取点号后的符号
        auto it = I[u][i].rightSide.begin();
        for(int j = 1; j < I[u][i].dotPosition; j++) {
            if(it != I[u][i].rightSide.end()) {
                ++it;
            }
        }
        
        // 如果点号已到末尾或是空产生式，跳过
        if(it == I[u][i].rightSide.end() || *it == "E") {
            continue;
        }
        
        // 创建新状态
        statenum++;
        std::string current_symbol = *it;
        
        // 设置状态转换
        trans_state_table[u][statenum] = current_symbol;
        is_trans_state_table[u][statenum] = true;
        
        // 记录新状态
        update_state[++updatenum] = statenum;
        Inum[statenum] = 0;
        
        // 移动点号创建新项目
        I[statenum][Inum[statenum]] = I[u][i];
        I[statenum][Inum[statenum]].dotPosition++;
        Inum[statenum] = 1;
        
        // 查找其他可以移动的项目
        for(int j = 0; j < Inum[u]; j++) {
            if(used_state[j]) continue;
            
            auto jit = I[u][j].rightSide.begin();
            for(int k = 1; k < I[u][j].dotPosition; k++) {
                if(jit != I[u][j].rightSide.end()) {
                    ++jit;
                }
            }
            
            if(jit != I[u][j].rightSide.end() && *jit == current_symbol) {
                I[statenum][Inum[statenum]] = I[u][j];
                I[statenum][Inum[statenum]].dotPosition++;
                Inum[statenum]++;
                used_state[j] = true;
            }
        }
        
        // 添加闭包项目
        std::memset(get_state, 0, sizeof(get_state));
        for(int j = 1; j <= 8; j++) {
            from_or_state(statenum);
        }
        
        // 检查是否可以合并到已有状态
        for(int j = 0; j < statenum; j++) {
            if(Inum[statenum] == Inum[j]) {
                bool states_equal = true;
                for(int k = 0; k < Inum[j]; k++) {
                    if(!gram_is_equal(I[statenum][k], I[j][k])) {
                        states_equal = false;
                        break;
                    }
                }
                
                if(states_equal) {
                    // 合并状态
                    is_trans_state_table[u][statenum] = false;
                    trans_state_table[u][j] = current_symbol;
                    is_trans_state_table[u][j] = true;
                    
                    // 回退状态计数
                    Inum[statenum] = 0;
                    statenum--;
                    updatenum--;
                    break;
                }
            }
        }
    }
    
    // 递归处理新状态
    if(topnum <= updatenum) {
        topnum++;
        trans_automaton(update_state[topnum-1]);
    }
}

void make_slr_table() {
    // 处理终结符和添加$符号
    deal_T();
    T[++numT] = "$";
    
    // 处理移进动作
    for(int i = 0; i <= statenum; i++) {
        for(int j = 0; j <= statenum; j++) {
            if(!is_trans_state_table[i][j]) continue;
            
            int x = i, y = 0;
            // 查找终结符
            for(int k = 1; k <= numT; k++) {
                if(trans_state_table[i][j] == T[k]) {
                    y = k;
                    break;
                }
            }
            
            // 查找非终结符
            if(y == 0) {
                for(int k = 1; k <= followSetCount; k++) {
                    if(trans_state_table[i][j] == NT[k]) {
                        y = numT + k;
                        break;
                    }
                }
            }
            
            slt_table[x][y] = j;
            MorS[x][y] = 1;  // 1表示移进
        }
    }
    
    // 处理规约动作
    for(int i = 2; i <= statenum; i++) {
        for(int j = 0; j < Inum[i]; j++) {
            auto it = I[i][j].rightSide.begin();
            std::advance(it, I[i][j].dotPosition - 1);
            
            if(it == I[i][j].rightSide.end() || *I[i][j].rightSide.begin() == "E") {
                // 查找产生式编号
                int target = 0;
                for(int k = 0; k < Inum[0]; k++) {
                    if(I[i][j].leftSide == I[0][k].leftSide && I[i][j].rightSide == I[0][k].rightSide) {
                        target = k;
                        break;
                    }
                }
                
                // 根据Follow集合添加规约动作
                for(int k = 1; k <= followSetCount; k++) {
                    if(*Follow[k].leftSymbols.begin() == I[0][target].leftSide) {
                        for(const auto& symbol : Follow[k].rightSymbols) {
                            for(int y = 1; y <= numT; y++) {
                                if(symbol == T[y]) {
                                    if(MorS[i][y] != 0) {
                                        std::cout << "error:不是SLR文法,发生移进规约冲突" << i << std::endl;
                                    }
                                    slt_table[i][y] = target;
                                    MorS[i][y] = 2;  // 2表示规约
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    // 处理接受动作
    for(int y = 1; y <= numT; y++) {
        if(T[y] == "$") {
            slt_table[1][y] = -1;
            MorS[1][y] = 3;  // 3表示acc
        }
    }
}

void slr_analysis() {
    // 初始化析栈和输入
    parseStack[++stackTop].state = 0;
    input[++inputTop] = "$";
    for(int i = cnt; i >= 1; i--) {
        input[++inputTop] = token[i];
    }
    
    bool error_found = false;
    while(!error_found) {
        int x = parseStack[stackTop].state;
        int y = 0;
        
        // 查找当前输入符号对应的列号
        for(int i = 1; i <= numT; i++) {
            if(T[i] == input[inputTop]) {
                y = i;
                break;
            }
        }
        
        if(MorS[x][y] == 1) {  // 移进
            parseStack[++stackTop].state = slt_table[x][y];
            parseStack[stackTop].symbol = input[inputTop];
            inputTop--;
        }
        else if(MorS[x][y] == 2) {  // 规约
            if(*I[0][slt_table[x][y]].rightSide.begin() != "E") {
                stackTop -= I[0][slt_table[x][y]].rightSide.size();
            }
            output[++outputTop] = slt_table[x][y];
            parseStack[++stackTop].symbol = I[0][slt_table[x][y]].leftSide;
            
            // 查找goto表项
            for(int j = 1; j <= followSetCount; j++) {
                if(parseStack[stackTop].symbol == NT[j]) {
                    int xx = parseStack[stackTop-1].state;
                    int yy = j + numT;
                    parseStack[stackTop].state = slt_table[xx][yy];
                    break;
                }
            }
        }
        else if(MorS[x][y] == 3) {  // 接受
            return;
        }
        else {  // 错误
            if(inputTop > 1) {
                input[++inputTop] = ";";
                std::cout << "语法错误，第4行，缺少\"" << input[inputTop] << "\"" << std::endl;
                error_found = true;
            }
            else {
                return;
            }
        }
    }
    
    // 如果发现错误，继续分析直到结束
    if(error_found) {
        while(stackTop > 0 && inputTop > 0) {
            int x = parseStack[stackTop].state;
            int y = 0;
            
            for(int i = 1; i <= numT; i++) {
                if(T[i] == input[inputTop]) {
                    y = i;
                    break;
                }
            }
            
            if(MorS[x][y] == 1) {
                parseStack[++stackTop].state = slt_table[x][y];
                parseStack[stackTop].symbol = input[inputTop];
                inputTop--;
            }
            else if(MorS[x][y] == 2) {
                if(*I[0][slt_table[x][y]].rightSide.begin() != "E") {
                    stackTop -= I[0][slt_table[x][y]].rightSide.size();
                }
                output[++outputTop] = slt_table[x][y];
                parseStack[++stackTop].symbol = I[0][slt_table[x][y]].leftSide;
                
                for(int j = 1; j <= followSetCount; j++) {
                    if(parseStack[stackTop].symbol == NT[j]) {
                        int xx = parseStack[stackTop-1].state;
                        int yy = j + numT;
                        parseStack[stackTop].state = slt_table[xx][yy];
                        break;
                    }
                }
            }
            else {
                return;
            }
        }
    }
}

void print_pris(int stepNumber) {
    for(int symbolIndex = printTop; symbolIndex >= 1; symbolIndex--) {
        std::cout << printSymbols[symbolIndex] << " ";
    }
    if(stepNumber != 1) std::cout << "=> " << std::endl;
}

void print_max_right() {
    printSymbols[++printTop] = "program";
    print_pris(0);
    
    for(int outputIndex = outputTop; outputIndex >= 1; outputIndex--) {
        for(int symbolIndex = 1; symbolIndex <= printTop; symbolIndex++) {
            if(printSymbols[symbolIndex] == I[0][output[outputIndex]].leftSide) {
                // 移动后续符号
                for(int moveIndex = printTop; moveIndex >= symbolIndex + 1; moveIndex--) {
                    printSymbols[moveIndex + I[0][output[outputIndex]].rightSide.size() - 1] = printSymbols[moveIndex];
                }
                
                if(*I[0][output[outputIndex]].rightSide.begin() == "E") {  // 处理空产生式
                    for(int shiftIndex = symbolIndex; shiftIndex <= printTop; shiftIndex++) {
                        printSymbols[shiftIndex] = printSymbols[shiftIndex + 1];
                    }
                    printTop--;
                }
                else {  // 替换为右侧符号
                    printTop = printTop + I[0][output[outputIndex]].rightSide.size() - 1;
                    auto rightSideIterator = I[0][output[outputIndex]].rightSide.begin();
                    for(int replaceIndex = symbolIndex + I[0][output[outputIndex]].rightSide.size() - 1; replaceIndex >= symbolIndex; replaceIndex--) {
                        printSymbols[replaceIndex] = *rightSideIterator++;
                    }
                }
                break;
            }
        }
        print_pris(outputIndex);
    }
}

// 添加辅助函数

void deal_follow() {
    // 收集所有非终结符
    for(int i = 0; i < productionCount; i++) {
        if(i == 0 || gram[i].leftSide != gram[i-1].leftSide) {
            Follow[++followSetCount].leftSymbols.push_back(gram[i].leftSide);
            NT[followSetCount] = gram[i].leftSide;
        }
    }
    
    // 重复3次以确保所有Follow集都被计算
    for(int i = 1; i <= 3; i++) {
        get_follow();
    }
}

void deal_automaton() {
    init_I0();  // 初始化增广文法
    trans_automaton(0);  // 构建自动机
    make_slr_table();  // 构建SLR分析表
}

void Grammatical_Analysis() {
    // 定义文法规则
    std::string input[50] = {
        "program->compoundstmt",
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
        "simpleexpr->ID|NUM|( arithexpr )"
    };
    
    // 处理文法
    gram_divide(input, 14);
    
    // 构建First和Follow集
    deal_first();
    deal_follow();
    
    // 构建自动机和分析表
    deal_automaton();
}

void Analysis() {
    read_prog(prog);
    prog += '\n';
    
    // 词法分析
    token_divide();
    
    // 语法分析
    Grammatical_Analysis();
    
    // SLR分析
    slr_analysis();
    
    // 输出最右推导
    print_max_right();
}

// 添加调试输出函数
void print_I(int state) {
    std::cout << "State " << state << " contains:" << std::endl;
    for(int i = 0; i < Inum[state]; i++) {
        // 输出左部
        std::cout << I[state][i].leftSide << " -> ";
        
        // 输出右部，包括点号位置
        int j = 1;
        for(const auto& symbol : I[state][i].rightSide) {
            if(j == I[state][i].dotPosition) {
                std::cout << ". ";
            }
            std::cout << symbol << " ";
            j++;
        }
        // 如果点号在最后
        if(j == I[state][i].dotPosition) {
            std::cout << ".";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

// 添加SLR分析表打印函数
void print_slr_table() {
    std::cout << "\nSLR Analysis Table:" << std::endl;
    
    // 打印表头
    std::cout << "State\t";
    // 打印终结符列
    for(int i = 1; i <= numT; i++) {
        std::cout << T[i] << "\t";
    }
    // 打印非终结符列
    for(int i = 1; i <= followSetCount; i++) {
        std::cout << NT[i] << "\t";
    }
    std::cout << std::endl;
    
    // 打印每个状态的动作
    for(int i = 0; i <= statenum; i++) {
        std::cout << i << "\t";
        for(int j = 1; j <= numT + followSetCount; j++) {
            if(MorS[i][j] == 1) {
                std::cout << "s" << slt_table[i][j] << "\t";
            }
            else if(MorS[i][j] == 2) {
                std::cout << "r" << slt_table[i][j] << "\t";
            }
            else if(MorS[i][j] == 3) {
                std::cout << "acc\t";
            }
            else {
                std::cout << "\t";
            }
        }
        std::cout << std::endl;
    }
}

// 添加First集合打印函数
void print_first_sets() {
    std::cout << "\nFirst Sets:" << std::endl;
    for(int i = 1; i <= firstSetCount; i++) {
        // 打印左部
        for(const auto& symbol : First[i].leftSymbols) {
            std::cout << symbol << " ";
        }
        std::cout << "-> { ";
        
        // 打印右部
        for(const auto& symbol : First[i].rightSymbols) {
            std::cout << symbol << " ";
        }
        
        // 如果可以推导出空串
        if(First[i].canBeEmpty) {
            std::cout << "ε ";
        }
        
        std::cout << "}" << std::endl;
    }
}

// 添加Follow集合打印函数
void print_follow_sets() {
    std::cout << "\nFollow Sets:" << std::endl;
    for(int i = 1; i <= followSetCount; i++) {
        // 打印非终结符
        std::cout << *Follow[i].leftSymbols.begin() << " -> { ";
        
        // 打印Follow集合
        for(const auto& symbol : Follow[i].rightSymbols) {
            std::cout << symbol << " ";
        }
        std::cout << "}" << std::endl;
    }
}

// 添加状态转换表打印函数
void print_transition_table() {
    std::cout << "\nState Transition Table:" << std::endl;
    std::cout << "From\tTo\tSymbol" << std::endl;
    
    for(int i = 0; i <= statenum; i++) {
        for(int j = 0; j <= statenum; j++) {
            if(is_trans_state_table[i][j]) {
                std::cout << i << "\t" << j << "\t" 
                         << trans_state_table[i][j] << std::endl;
            }
        }
    }
}

// 添加分析栈内容打印函数
void print_stack_content() {
    std::cout << "\nStack content:" << std::endl;
    for(int i = 1; i <= stackTop; i++) {
        std::cout << "(" << parseStack[i].state << ", " << parseStack[i].symbol << ") ";
    }
    std::cout << std::endl;
    
    std::cout << "Input: ";
    for(int i = inputTop; i >= 1; i--) {
        std::cout << input[i] << " ";
    }
    std::cout << std::endl;
}

// 添加错误处理和恢复函
void handle_error(const std::string& expected_token) {
    std::cout << "语法错误，第4行，缺少\"" << expected_token << "\"" << std::endl;
    
    // 可以在这里添加更详细的错误信息
    std::cout << "当前分析栈状态：" << std::endl;
    print_stack_content();
    
    // 错误恢复策略
    input[++inputTop] = expected_token;
}
