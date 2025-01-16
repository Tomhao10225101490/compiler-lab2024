// 自定义的C语言词法分析器 
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

// 定义词法分析器的错误码
enum LexerError {
    NO_ERROR = 0,
    INVALID_CHARACTER = 1,
    NUMBER_TOO_LONG = 2,
    IDENTIFIER_TOO_LONG = 3,
    UNCLOSED_COMMENT = 4,
    ENCODING_ERROR = 5
};

// 定义Token类型
enum TokenType {
    KEYWORD = 1,
    IDENTIFIER = 81,
    NUMBER = 80,
    COMMENT = 79,
    OPERATOR = 70
};

// 词法分析器的配置参数
struct LexerConfig {
    static const int MAX_IDENTIFIER_LENGTH = 255;
    static const int MAX_NUMBER_LENGTH = 20;
    static const int MAX_COMMENT_LENGTH = 1000;
    static const bool CASE_SENSITIVE = true;
    static const bool SUPPORT_UNICODE = false;
};

// 错误处理类
class LexerErrorHandler {
private:
    static vector<pair<LexerError, int>> errorLog;
    static int errorCount;

public:
    static void logError(LexerError error, int position) {
        errorLog.push_back({error, position});
        errorCount++;
        
        if (errorCount > 10) {
            cout << "\n警告：检测到大量错误，请检查输入代码" << endl;
        }
    }

    static void printErrorSummary() {
        if (!errorLog.empty()) {
            cout << "\n错误统计：" << endl;
            for (const auto& error : errorLog) {
                cout << "位置 " << error.second << ": ";
                switch (error.first) {
                    case INVALID_CHARACTER:
                        cout << "无效字符"; break;
                    case NUMBER_TOO_LONG:
                        cout << "数字过长"; break;
                    case IDENTIFIER_TOO_LONG:
                        cout << "标识符过长"; break;
                    case UNCLOSED_COMMENT:
                        cout << "注释未闭合"; break;
                    case ENCODING_ERROR:
                        cout << "编码错误"; break;
                }
                cout << endl;
            }
        }
    }
};

vector<pair<LexerError, int>> LexerErrorHandler::errorLog;
int LexerErrorHandler::errorCount = 0;

/* 获取输入并填充程序字符串 */
void fetch_code(string& codeStr) {
    char tempChar;
    stringstream buffer;
    int lineCount = 1;
    bool inString = false;
    
    while (scanf("%c", &tempChar) != EOF) {
        // 处理字符串常量
        if (tempChar == '\"') {
            inString = !inString;
        }
        
        // 统计行数
        if (tempChar == '\n') {
            lineCount++;
        }
        
        // 检查无效字符
        if (!inString && !isprint(tempChar) && !isspace(tempChar)) {
            LexerErrorHandler::logError(INVALID_CHARACTER, codeStr.length());
            continue;
        }
        
        buffer << tempChar;
    }
    
    codeStr = buffer.str();
    cout << "读取了 " << lineCount << " 行代码" << endl;
}

/* 判断字符是否为数字 */
inline bool check_digit(char charToken) {
    return (charToken >= '0' && charToken <= '9');
}

/* 判断字符是否为字母 */
inline bool check_alpha(char charToken) {
    if (LexerConfig::CASE_SENSITIVE) {
        return (charToken >= 'a' && charToken <= 'z') || 
               (charToken >= 'A' && charToken <= 'Z') || 
               charToken == '_';
    }
    return ((unsigned)(charToken | 32) - 'a' < 26) || charToken == '_';
}

/* 判断是否为关键字 */
bool validate_keyword(const string& lexeme) {
    static map<string, int> keywordMap;
    
    // 初始化关键字映射表（仅在第一次调用时执行）
    if (keywordMap.empty()) {
        for (int i = 1; i <= 32; i++) {
            keywordMap[keywordBank[i]] = i;
        }
    }
    
    // 使用map查找关键字
    auto it = keywordMap.find(lexeme);
    if (it != keywordMap.end()) {
        if (tokenID != 1) cout << endl;
        cout << tokenID << ": " << "<" << lexeme << "," << it->second << ">";
        tokenID++;
        return true;
    }
    return false;
}

/* 处理数字 token */
int process_number(int position) {
    int newPos = position;
    bool hasDecimalPoint = false;
    bool hasExponent = false;
    
    // 处理整数部分
    while (check_digit(programCode[newPos])) newPos++;
    
    // 处理小数点和小数部分
    if (programCode[newPos] == '.') {
        hasDecimalPoint = true;
        newPos++;
        while (check_digit(programCode[newPos])) newPos++;
    }
    
    // 处理科学计数法
    if (programCode[newPos] == 'e' || programCode[newPos] == 'E') {
        hasExponent = true;
        newPos++;
        if (programCode[newPos] == '+' || programCode[newPos] == '-') newPos++;
        while (check_digit(programCode[newPos])) newPos++;
    }
    
    // 检查数字长度
    if (newPos - position > LexerConfig::MAX_NUMBER_LENGTH) {
        LexerErrorHandler::logError(NUMBER_TOO_LONG, position);
    }
    
    string digitToken = programCode.substr(position, newPos - position);
    if (tokenID != 1) cout << '\n';
    
    // 输出带有类型信息的token
    cout << tokenID << ": " << "<" << digitToken << "," << NUMBER;
    if (hasDecimalPoint || hasExponent) cout << ",float";
    else cout << ",int";
    cout << ">";
    
    tokenID++;
    return newPos;
}

/* 处理标识符或关键字 token */
int process_identifier(int position) {
    int newPos = position;
    bool containsUnicode = false;
    
    // 处理标识符字符
    while (check_alpha(programCode[newPos]) || check_digit(programCode[newPos])) {
        if ((unsigned char)programCode[newPos] > 127) {
            containsUnicode = true;
        }
        newPos++;
    }
    
    // 提取标识符
    string identifierToken = programCode.substr(position, newPos - position);
    
    // 长度检查
    if (identifierToken.length() > LexerConfig::MAX_IDENTIFIER_LENGTH) {
        LexerErrorHandler::logError(IDENTIFIER_TOO_LONG, position);
        identifierToken = identifierToken.substr(0, LexerConfig::MAX_IDENTIFIER_LENGTH);
    }
    
    // Unicode支持检查
    if (containsUnicode && !LexerConfig::SUPPORT_UNICODE) {
        LexerErrorHandler::logError(ENCODING_ERROR, position);
    }
    
    // 检查是否为关键字
    if (validate_keyword(identifierToken)) return newPos;
    
    if (tokenID != 1) cout << endl;
    cout << tokenID << ": " << "<" << identifierToken << "," << IDENTIFIER;
    if (containsUnicode) cout << ",unicode";
    cout << ">";
    
    tokenID++;
    return newPos;
}

/* 处理单行注释 */
int process_single_line_comment(int position) {
    int newPos = position;
    int commentLength = 0;
    
    while (programCode[newPos] != '\n' && programCode[newPos] != '\0') {
        newPos++;
        commentLength++;
        
        if (commentLength > LexerConfig::MAX_COMMENT_LENGTH) {
            cout << "\n警告：注释过长" << endl;
            break;
        }
    }
    
    string commentToken = programCode.substr(position, newPos - position);
    if (tokenID != 1) cout << endl;
    cout << tokenID << ": " << "<" << commentToken << "," << COMMENT << ",单行>";
    tokenID++;
    
    return newPos;
}

/* 处理多行注释 */
int process_multi_line_comment(int position) {
    int newPos = position;
    int commentLength = 0;
    int nestedLevel = 1;
    bool foundEnd = false;
    
    while (nestedLevel > 0 && programCode[newPos] != '\0') {
        newPos++;
        commentLength++;
        
        if (programCode[newPos] == '*' && programCode[newPos + 1] == '/') {
            nestedLevel--;
            newPos += 2;
            foundEnd = true;
        } else if (programCode[newPos] == '/' && programCode[newPos + 1] == '*') {
            nestedLevel++;
            newPos++;
        }
        
        if (commentLength > LexerConfig::MAX_COMMENT_LENGTH) {
            cout << "\n警告：注释过长" << endl;
            break;
        }
    }
    
    if (!foundEnd) {
        LexerErrorHandler::logError(UNCLOSED_COMMENT, position);
    }
    
    string commentToken = programCode.substr(position, newPos - position);
    if (tokenID != 1) cout << endl;
    cout << tokenID << ": " << "<" << commentToken << "," << COMMENT << ",多行>";
    tokenID++;
    
    return newPos;
}

/* 处理运算符 token */
int process_operator(int position) {
    // 边界检查
    if (position >= programCode.length()) {
        return position;
    }

    // 特殊字符处理映射表
    const map<char, string> specialCharMap = {
        {'\'', "字符常量"},
        {'\"', "字符串常量"},
        {'\\', "转义字符"}
    };

    // 检查是否是特殊字符
    if (specialCharMap.find(programCode[position]) != specialCharMap.end()) {
        if (tokenID != 1) cout << endl;
        cout << tokenID << ": " << "<特殊字符: " << specialCharMap.at(programCode[position]) << "," << 90 << ">";
        tokenID++;
        return position + 1;
    }

    // 尝试匹配三字符运算符
    if (position + 2 < programCode.length()) {
        string tripleOp = programCode.substr(position, 3);
        // 检查常见的三字符运算符
        vector<string> tripleOps = {"<<=", ">>=", "..."};
        for (const auto& op : tripleOps) {
            if (tripleOp == op) {
                if (tokenID != 1) cout << endl;
                cout << tokenID << ": " << "<" << op << "," << 70 << ">";
                tokenID++;
                return position + 3;
            }
        }
    }

    // 尝试匹配双字符运算符
    if (position + 1 < programCode.length()) {
        string doubleOp = programCode.substr(position, 2);
        // 创建双字符运算符查找表
        map<string, int> doubleOps = {
            {"+=", 35}, {"-=", 36}, {"*=", 37}, {"/=", 38},
            {"++", 39}, {"--", 40}, {"==", 41}, {"!=", 42},
            {"&&", 43}, {"||", 44}, {"<<", 45}, {">>", 46},
            {"<=", 47}, {">=", 48}, {"->", 49}, {"::", 50}
        };

        if (doubleOps.find(doubleOp) != doubleOps.end()) {
            if (tokenID != 1) cout << endl;
            cout << tokenID << ": " << "<" << doubleOp << "," << doubleOps[doubleOp] << ">";
            tokenID++;
            return position + 2;
        }
    }

    // 常规运算符处理（从运算符表中查找）
    for (int length = 3; length >= 1; length--) {
        if (position + length > programCode.length()) {
            continue;
        }

        string currentOp = programCode.substr(position, length);
        for (int i = 1; i <= 46; i++) {
            if (currentOp == symbolArray[i]) {
                // 运算符优先级检查
                int priority = 0;
                switch (symbolArray[i][0]) {
                    case '*': case '/': case '%':
                        priority = 1;
                        break;
                    case '+': case '-':
                        priority = 2;
                        break;
                    case '<': case '>': case '=': case '!':
                        priority = 3;
                        break;
                    case '&': case '|': case '^':
                        priority = 4;
                        break;
                }

                if (tokenID != 1) cout << endl;
                cout << tokenID << ": " << "<" << currentOp << "," << i + 32 
                     << ",优先级:" << priority << ">";
                tokenID++;
                return position + length;
            }
        }
    }

    // 错误处理：未识别的字符
    if (tokenID != 1) cout << endl;
    cout << tokenID << ": " << "<错误:未识别的字符 '" << programCode[position] 
         << "' at position " << position << "," << 99 << ">";
    tokenID++;
    
    // 记录错误日志
    static vector<pair<char, int>> errorLog;
    errorLog.push_back({programCode[position], position});

    // 如果积累了太多错误，输出警告
    if (errorLog.size() > 10) {
        cout << endl << "警告：检测到大量未识别字符，请检查输入代码的编码格式" << endl;
    }

    return position + 1;
}

/* 词法分析器主函数 */
void Analysis() {
    fetch_code(programCode);  // 获取输入程序
    int pos = 0;  // 当前处理的位置
    int programLength = programCode.length();  // 程序的总长度

    // 开始处理整个程序
    while (pos < programLength) {
        // 跳过空格和换行符
        while (programCode[pos] == ' ' || programCode[pos] == '\n') pos++;
        if (pos >= programLength) break;  // 如果到达程序结尾，结束

        // 判断当前字符的类型并处理
        if (check_digit(programCode[pos]))  // 处理数字
            pos = process_number(pos);
        else if (check_alpha(programCode[pos]))  // 处理标识符或关键字
            pos = process_identifier(pos);
        else {
            // 处理注释
            if (programCode[pos] == '/' && programCode[pos + 1] == '/')  // 单行注释
                pos = process_single_line_comment(pos);
            else if (programCode[pos] == '/' && programCode[pos + 1] == '*')  // 多行注释
                pos = process_multi_line_comment(pos);
                // 处理特殊符号 '%' 和字母组合
            else if (programCode[pos] == '%' && check_alpha(programCode[pos + 1])) {
                if (tokenID != 1) cout << endl;
                cout << tokenID << ": " << "<" << programCode.substr(pos, 2) << "," << 81 << ">";
                tokenID++;
                pos += 2;  // 跳过两个字符
            } else
                pos = process_operator(pos);  // 处理运算符
        }
    }
}
