// 在文件开头添加这些头文件
#pragma once
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <stack>
#include <list>
#include <queue>
#include <regex>
#include <climits>  // 为 INT_MAX 和 INT_MIN
#include <cmath>    // 为 fabs
#include <iomanip>  // 为 setprecision 和 fixed
#include <chrono>   // 为性能分析功能添加
#include <stdexcept> // 为 runtime_error 异常类添加


using namespace std;
/* 不要修改这个标准输入函数 */
void read_prog(string& prog)
{
	char c;
	while (scanf("%c", &c) != EOF) {
		prog += c;
	}
}
/* 你可以添加其他函数 */
queue<string> errors;
class Var {
private:
	bool isReal_;  // 类型标志
	union {
		int i;     // 整数值
		double d;  // 浮点值
	} value;

	// 内部辅助函数:检查数值溢出
	bool checkOverflow(double val) const {
		return (val > INT_MAX || val < INT_MIN);
	}

public:
	// 类型查询
	inline bool isReal() const {
		return isReal_;
	}

	// 构造函数
	Var() : isReal_(false) {
		value.i = 0;
	}

	explicit Var(int value_) : isReal_(false) {
		value.i = value_;
	}

	explicit Var(double value_) : isReal_(true) {
		value.d = value_;
	}

	// 拷贝赋值运算符
	Var& operator=(const Var& other) {
		if (this != &other) {
			isReal_ = other.isReal_;
			value = other.value;
		}
		return *this;
	}

	// bool类型转换运算符
	explicit operator bool() const {
		if (!isReal_) {
			return value.i != 0;
		}
		return value.d != 0.0;
	}

	// 类型转换函数
	bool to_int() {
		if (isReal_) {
			if (checkOverflow(value.d)) {
				errors.push("error message: double value overflow when converting to int");
				value.i = 0;
			} else {
				value.i = static_cast<int>(value.d);
			}
			isReal_ = false;
			return true;
		}
		return false;
	}

	void to_real() {
		if (!isReal_) {
			value.d = static_cast<double>(value.i);
			isReal_ = true;
		}
	}

	// 获取值函数
	int getInt() const {
		if (isReal_) {
			if (checkOverflow(value.d)) {
				errors.push("error message: double value overflow when getting int");
				return 0;
			}
			return static_cast<int>(value.d);
		}
		return value.i;
	}

	double getReal() const {
		return isReal_ ? value.d : static_cast<double>(value.i);
	}

	// 算术运算符
	Var operator+(const Var& other) const {
		if (!isReal_ && !other.isReal_) {
			// 检查整数加法溢出
			if ((other.value.i > 0 && value.i > INT_MAX - other.value.i) ||
				(other.value.i < 0 && value.i < INT_MIN - other.value.i)) {
				errors.push("error message: integer overflow in addition");
				return Var(0);
			}
			return Var(value.i + other.value.i);
		}
		return Var(getReal() + other.getReal());
	}

	Var operator-(const Var& other) const {
		if (!isReal_ && !other.isReal_) {
			// 检查整数减法溢出
			if ((other.value.i < 0 && value.i > INT_MAX + other.value.i) ||
				(other.value.i > 0 && value.i < INT_MIN + other.value.i)) {
				errors.push("error message: integer overflow in subtraction");
				return Var(0);
			}
			return Var(value.i - other.value.i);
		}
		return Var(getReal() - other.getReal());
	}

	Var operator*(const Var& other) const {
		if (!isReal_ && !other.isReal_) {
			// 检查整数乘法溢出
			if (value.i > 0 ? 
				(other.value.i > 0 ? value.i > INT_MAX/other.value.i : other.value.i < INT_MIN/value.i) :
				(other.value.i > 0 ? value.i < INT_MIN/other.value.i : value.i != 0 && other.value.i < INT_MAX/value.i)) {
				errors.push("error message: integer overflow in multiplication");
				return Var(0);
			}
			return Var(value.i * other.value.i);
		}
		return Var(getReal() * other.getReal());
	}

	Var operator/(const Var& other) const {
		// 除零检查
		if ((!other.isReal_ && other.value.i == 0) || 
			(other.isReal_ && fabs(other.value.d) < 1e-10)) {
			errors.push("error message:division by zero");
			return Var(0);
		}

		if (!isReal_ && !other.isReal_) {
			// 检查整数除法的特殊情况
			if (value.i == INT_MIN && other.value.i == -1) {
				errors.push("error message: integer overflow in division");
				return Var(0);
			}
			return Var(value.i / other.value.i);
		}
		return Var(getReal() / other.getReal());
	}

	// 比较运算符声明为友元函数
	friend bool operator<(const Var& first, const Var& second);
	friend bool operator>(const Var& first, const Var& second);
	friend bool operator<=(const Var& first, const Var& second);
	friend bool operator>=(const Var& first, const Var& second);
	friend bool operator==(const Var& first, const Var& second);
	
	// 输出运算符
	friend ostream& operator<<(ostream& os, const Var& var) {
		if (var.isReal_) {
			return os << fixed << setprecision(6) << var.value.d;
		}
		return os << var.value.i;
	}
};

// 比较运算符的全局实现

/**
 * 小于运算符
 * 如果两个操作数都是整数，直接比较整数值
 * 如果任一操作数是浮点数，则将两个操作数都转换为浮点数后比较
 */
bool operator<(const Var& first, const Var& second) {
    // 两个操作数都是整数时的快速路径
    if (!first.isReal_ && !second.isReal_) {
        return first.value.i < second.value.i;
    }
    else {
        // 浮点数比较时考虑精度误差
        static const double EPSILON = 1e-10;
        double diff = first.getReal() - second.getReal();
        if (fabs(diff) < EPSILON) {
            return false; // 认为相等
        }
        return diff < 0;
    }
}

/**
 * 大于运算符
 * 通过复用小于运算符来实现，保证比较的一致性
 */
bool operator>(const Var& first, const Var& second) {
    // 两个操作数都是整数时的快速路径
    if (!first.isReal_ && !second.isReal_) {
        return first.value.i > second.value.i;
    }
    else {
        // 浮点数比较时考虑精度误差
        static const double EPSILON = 1e-10;
        double diff = first.getReal() - second.getReal();
        if (fabs(diff) < EPSILON) {
            return false; // 认为相等
        }
        return diff > 0;
    }
}

/**
 * 小于等于运算符
 * 对于浮点数，考虑了精度误差
 */
bool operator<=(const Var& first, const Var& second) {
    // 两个操作数都是整数时的快速路径
    if (!first.isReal_ && !second.isReal_) {
        return first.value.i <= second.value.i;
    }
    else {
        // 浮点数比较时考虑精度误差
        static const double EPSILON = 1e-10;
        double diff = first.getReal() - second.getReal();
        return diff < EPSILON; // diff < EPSILON 意味着 first <= second
    }
}

/**
 * 大于等于运算符
 * 对于浮点数，考虑了精度误差
 */
bool operator>=(const Var& first, const Var& second) {
    // 两个操作数都是整数时的快速路径
    if (!first.isReal_ && !second.isReal_) {
        return first.value.i >= second.value.i;
    }
    else {
        // 浮点数比较时考虑精度误差
        static const double EPSILON = 1e-10;
        double diff = first.getReal() - second.getReal();
        return diff > -EPSILON; // diff > -EPSILON 意味着 first >= second
    }
}

/**
 * 相等运算符
 * 对于整数进行精确比较
 * 对于浮点数，考虑了精度误差，使用epsilon值进行近似相等判断
 */
bool operator==(const Var& first, const Var& second) {
    // 两个操作数都是整数时的快速路径
    if (!first.isReal_ && !second.isReal_) {
        return first.value.i == second.value.i;
    }
    else {
        // 浮点数比较时考虑精度误差
        static const double EPSILON = 1e-10;
        // 使用fabs获取差值的绝对值，避免方向性误差
        return fabs(first.getReal() - second.getReal()) < EPSILON;
    }
}
/*词法单元，包括终结符与非终结符*/
class Token {
private:
	static const char id2stringMap[41][16];
	static const map<const string, int> string2idMap;
public:
	int id;
	Token() :id(40) {}
	Token(int tokenId_) :id(tokenId_) {}
	Token(const char* tokenNameC) {
		int spacePos = -1;
		for (int i = 0; tokenNameC[i] != '\0'; i++) {
			if (isspace(tokenNameC[i])) {
				spacePos = i;
				break;
			}
		}
		string tokenName = tokenNameC;
		if (spacePos > 0) {
			tokenName = tokenName.substr(0, spacePos);
		}
		auto it = string2idMap.find(tokenName);
		if (it != string2idMap.end()) {
			id = it->second;
			return;
		}
		else {
			//不匹配的可能是ID, INTNUM或REALNUM
			regex int_regex("\\d*");
			regex real_regex("(((\\d+(\\.\\d*)?)|(\\d*\\.\\d+))((E|e)\\d+)?)");
			if (regex_match(tokenName, int_regex)) {
				id = 23;//int类型的id
			}
			else if (regex_match(tokenName, real_regex)) {
				id = 24;//real类型的id
			}
			else {//其他的一律认为是ID
				id = 22;
			}
		}

	}

	    /**
     * 从字符串构造Token
     * @param tokenName_ 输入的token名称
     */
    Token(const string& tokenName_) : Token(tokenName_.c_str()) {
        // 委托构造函数，复用c-string版本的构造逻辑
    }

    /**
     * 转换为string类型的运算符
     * 用于将Token转换为其字符串表示
     * @return 返回Token对应的字符串
     */
    explicit operator const string() const {
        if (id < 0 || id >= sizeof(id2stringMap) / sizeof(id2stringMap[0])) {
            return "InvalidToken";  // 安全检查：防止数组越界
        }
        return id2stringMap[id];
    }

    /**
     * 转换为C风格字符串的运算符
     * @return 返回Token对应的C风格字符串
     */
    explicit operator const char* () const {
        if (id < 0 || id >= sizeof(id2stringMap) / sizeof(id2stringMap[0])) {
            return "InvalidToken";  // 安全检查：防止数组越界
        }
        return id2stringMap[id];
    }

    /**
     * 相等比较运算符
     * 比较两个Token是否表示相同的词法单元
     * @param second 要比较的另一个Token
     * @return 如果两个Token相等返回true，否则返回false
     */
    bool operator ==(const Token& second) const {
        // 直接比较id值，id相同意味着表示相同的词法单元
        return this->id == second.id;
    }

    /**
     * 不等比较运算符
     * 比较两个Token是否表示不同的词法单元
     * @param second 要比较的另一个Token
     * @return 如果两个Token不相等返回true，否则返回false
     */
    bool operator !=(const Token& second) const {
        return this->id != second.id;
    }

    /**
     * 小于比较运算符
     * 用于在集合中建立Token的全序关系
     * @param second 要比较的另一个Token
     * @return 如果当前Token小于参数Token返回true，否则返回false
     */
    bool operator <(const Token& second) const {
        return this->id < second.id;
    }

    /**
     * 判断是否为终结符
     * 终结符的id范围为[0,24]
     * @return 如果是终结符返回true，否则返回false
     */
    bool isTerminal() const {
        static const int TERMINAL_MIN = 0;
        static const int TERMINAL_MAX = 24;
        return id >= TERMINAL_MIN && id <= TERMINAL_MAX;
    }

    /**
     * 判断是否为非终结符
     * 非终结符的id范围为[25,39]
     * @return 如果是非终结符返回true，否则返回false
     */
    bool is_non_terminal() const {
        static const int NONTERMINAL_MIN = 25;
        static const int NONTERMINAL_MAX = 39;
        return id >= NONTERMINAL_MIN && id <= NONTERMINAL_MAX;
    }

    /**
     * 声明输出流运算符为友元
     * 允许直接打印Token对象
     */
    friend ostream& operator<<(ostream& os, const Token& token);
};

/**
 * 输出流运算符的实现
 * 将Token转换为字符串并输出到流
 * @param os 输出流对象
 * @param token 要输出的Token对象
 * @return 返回输出流对象的引用
 */
ostream& operator<<(ostream& os, const Token& token) {
    try {
        const string tokenStr = static_cast<const string>(token);
        return os << tokenStr;
    } catch (...) {
        // 处理潜在的转换异常
        return os << "ErrorToken";
    }
}

/**
 * 词法单元类型定义
 * 用于区分终结符和非终结符
 */
typedef Token Terminal;    // 终结符类型（包括空终结符）
typedef Token NonTerminal; // 非终结符类型

/**
 * Token类的静态成员定义
 * 包含所有支持的词法单元及其映射关系
 */

// 词法单元到字符串的映射表
const char Token::id2stringMap[41][16] = {
    // 特殊符号
    "$",        // 0: 结束符
    "E",        // 1: 空串
    "{",        // 2: 左花括号
    "}",        // 3: 右花括号
    "(",        // 4: 左圆括号
    ")",        // 5: 右圆括号
    
    // 关键字
    "if",       // 6: if关键字
    "then",     // 7: then关键字
    "else",     // 8: else关键字
    
    // 运算符
    "=",        // 9:  赋值运算符
    "<",        // 10: 小于运算符
    ">",        // 11: 大于运算符
    "<=",       // 12: 小于等于运算符
    ">=",       // 13: 大于等于运算符
    "==",       // 14: 相等运算符
    "+",        // 15: 加法运算符
    "-",        // 16: 减法运算符
    "*",        // 17: 乘法运算符
    "/",        // 18: 除法运算符
    ";",        // 19: 分号
    
    // 数据类型
    "int",      // 20: 整型关键字
    "real",     // 21: 实型关键字
    
    // 标识符和常量
    "ID",       // 22: 标识符
    "INTNUM",   // 23: 整数常量
    "REALNUM",  // 24: 实数常量
    
    // 非终结符
    "program",        // 25: 程序
    "decls",          // 26: 声明序列
    "decl",           // 27: 单个声明
    "stmt",           // 28: 语句
    "compoundstmt",   // 29: 复合语句
    "stmts",          // 30: 语句序列
    "ifstmt",         // 31: if语句
    "assgstmt",       // 32: 赋值语句
    "boolexpr",       // 33: 布尔表达式
    "boolop",         // 34: 布尔运算符
    "arithexpr",      // 35: 算术表达式
    "arithexprprime", // 36: 算术表达式递归部分
    "multexpr",       // 37: 乘法表达式
    "multexprprime",  // 38: 乘法表达式递归部分
    "simpleexpr",     // 39: 简单表达式
    "error",          // 40: 错误标记
};

// 字符串到词法单元ID的映射表
const map<const string, int> Token::string2idMap{
    // 特殊符号映射
    {"$", 0},  {"E", 1},   {"{", 2},   {"}", 3},   {"(", 4},   {")", 5},
    
    // 关键字映射
    {"if", 6},   {"then", 7},   {"else", 8},
    
    // 运算符映射
    {"=", 9},    {"<", 10},     {">", 11},
    {"<=", 12},  {">=", 13},    {"==", 14},
    {"+", 15},   {"-", 16},     {"*", 17},
    {"/", 18},   {";", 19},
    
    // 数据类型映射
    {"int", 20},  {"real", 21},
    
    // 标识符和常量映射
    {"ID", 22},      {"INTNUM", 23},    {"REALNUM", 24},
    
    // 非终结符映射
    {"program", 25},        {"decls", 26},          {"decl", 27},
    {"stmt", 28},          {"compoundstmt", 29},    {"stmts", 30},
    {"ifstmt", 31},        {"assgstmt", 32},        {"boolexpr", 33},
    {"boolop", 34},        {"arithexpr", 35},       {"arithexprprime", 36},
    {"multexpr", 37},      {"multexprprime", 38},   {"simpleexpr", 39},
    {"error", 40},
};

/**
 * 符号表：存储变量及其值
 * key: 变量名
 * value: 变量值（可以是整数或实数）
 */
map<string, Var> vars;


/*每个节点是一个词法单元，若该词法单元是非终结符，可以连接若干子节点，代表一个产生式，若是终结符，则必须为叶子节点*/
class Node {
public:
	int line = 1;
	Var inhValue;
	Token head;
	Node() = default;
	explicit Node(const char* token_) :head(Token(token_)) {
	}
	explicit Node(const string& token_) :head(Token(token_)) {
	}
	explicit Node(Token token_) :head(token_) {
	}
	virtual ~Node() = default;
	virtual Var run() {
		cout << "调用了未实现的函数Node.run()" << endl;
		return Var(0);
	}
	virtual string getName() {
		cout << "调用了未实现的函数Node.getName()" << endl;
		return "NAN";
	}
	typedef bool (*Fp)(const Var&, const Var&);
	virtual Fp getOp() {
		switch (head.id) {
		case 10:
			return operator<;
		case 11:
			return operator>;
		case 12:
			return operator<=;
		case 13:
			return operator>=;
		case 14:
			return operator==;
		default:
			cout << "调用的终结符/非终结符\"" << head << "\"不具有getOp函数" << endl;
			return nullptr;
		};
	}
	virtual void parse() {
		return;
	}
	virtual void show(int depth) const {
		for (int i = 0; i < depth; i++) {
			cout << '\t';
		}
		cout << head << endl;
		return;
	}
};
class IDNode :public Node {
private:
	string name;
public:
	IDNode(const char* symbolName) :Node(Token(22)), name(symbolName) {
	}
	IDNode(const string& symbolName) :Node(Token(22)), name(symbolName) {
	}
	string getName() {
		return name;
	}
	Var run() {
		return vars[name];
	}
};
class VarNode :public Node {
private:
	Var value;
public:
	VarNode(int intValue) :Node(Token(23)), value(intValue) {
	}
	VarNode(double realValue) :Node(Token(24)), value(realValue) {
	}
	Var run() {
		return value;
	}
};

class Production :public Node {
public:
	int productionId;
	vector<Node*> body;//改成指针实现，使用基类指针设计
	Production() = default;
	//在得到一个非终结符时仅初始化一个头部
	explicit Production(NonTerminal head_) :Node(head_) {
	}
	Production(NonTerminal head_, int productionId_) :
		Node(head_), productionId(productionId_) {
	}
	
	Production(Token token_, const Token* body_, int prodLen, int productionId_)
		:Node(token_), productionId(productionId_) {
		for (int i = 0; i != prodLen; i++) {
			//构造产生式期间区分子节点是叶子节点还是中间节点即可
			if (body_[i].isTerminal()) {//字节点是叶子节点
				body.emplace_back(new Node(body_[i]));
			}
			else {//字节点是中间节点
				body.emplace_back(new Production(body_[i]));
			}
		}
	}
	~Production() {
		for (auto it = body.begin(); it != body.end(); it++) {
			delete* it;
		}
	}
	//由于使用了指针，赋值构造函数与赋值函数要改为深拷贝
	Production(const Production& other) :
		Node(other.head), productionId(other.productionId) {
		body.reserve(other.body.size());
		for (auto it = other.body.begin(); it != other.body.end(); it++) {
			body.emplace_back(new Node((*it)->head));
		}
	}
	Production(Production&& other) noexcept :
		Node(other.head), productionId(other.productionId) {
		body.reserve(other.body.size());
		for (auto it = other.body.begin(); it != other.body.end(); it++) {
			body.emplace_back(*it);
			*it = nullptr;
		}
	}
	Production& operator=(const Production& other) {
		productionId = other.productionId;
		if (this != &other) {
			head = other.head;
			for (auto it = other.body.begin(); it != other.body.end(); it++) {
				body.emplace_back(new Node((*it)->head));
			}
		}
		return *this;
	}
	//LLparser不断沿栈赋值
	void parse();
	void show(int depth) const {
		for (int i = 0; i < depth; i++) {
			cout << '\t';
		}
		cout << head << endl;

		if (body.empty()) {
			for (int i = 0; i <= depth; i++) {
				cout << '\t';
			}
			cout << Token(1) << endl;
			return;
		}
		else {
			depth++;
			for (auto it = body.begin(); it != body.end(); it++) {
				(*it)->show(depth);
			}
			depth--;
		}

	}
	Fp getOp() {
		if (head.id == Token(34).id) {
			return body[0]->getOp();
		}
		else {
			cout << "调用的产生式未实现getOp函数" << endl;
			return nullptr;
		}
	}

	Var run() {
		// 日志记录当前执行的产生式
		#ifdef DEBUG_MODE
			cout << "Executing production " << productionId << " at line " << line << endl;
		#endif

		// 记录执行开始时间（用于性能分析）
		auto start_time = chrono::high_resolution_clock::now();
		
		try {
			switch (productionId) {
				case 1: { // program -> decls compoundstmt
					// 程序入口点，执行声明和复合语句
					if (!body[0] || !body[1]) {
						throw runtime_error("Invalid program structure");
					}
					
					// 执行声明部分
					auto decls_result = body[0]->run();
					// 执行复合语句部分
					auto compound_result = body[1]->run();
					
					#ifdef DEBUG_MODE
						cout << "Program execution completed successfully" << endl;
					#endif
					
					return Var();
				}
				
				case 2: { // decls -> decl ; decls
					// 连续处理多个声明
					if (body.size() < 3) {
						throw runtime_error("Invalid declarations structure");
					}
					
					// 执行当前声明
					auto current_decl = body[0]->run();
					// 递归处理剩余声明
					auto remaining_decls = body[2]->run();
					
					return Var();
				}
				
				case 3: // decls -> ε (空产生式)
					return Var();
				
				case 4: { // decl -> int ID = INTNUM
					// 整数变量声明和初始化
					if (body.size() < 4) {
						throw runtime_error("Invalid integer declaration");
					}
					
					string var_name = body[1]->getName();
					auto init_value = body[3]->run();
					
					// 检查变量重定义
					if (vars.find(var_name) != vars.end()) {
						errors.push("error message:line " + to_string(line) + 
								", variable '" + var_name + "' already declared");
						return Var();
					}
					
					// 类型检查
					if (init_value.isReal()) {
						errors.push("error message:line " + to_string(line) + 
								", cannot initialize int with real value");
						init_value.to_int();
					}
					
					vars[var_name] = init_value;
					return Var();
				}
				
				case 5: { // decl -> real ID = REALNUM
					// 浮点数变量声明和初始化
					if (body.size() < 4) {
						throw runtime_error("Invalid real declaration");
					}
					
					string var_name = body[1]->getName();
					auto init_value = body[3]->run();
					
					// 检查变量重定义
					if (vars.find(var_name) != vars.end()) {
						errors.push("error message:line " + to_string(line) + 
								", variable '" + var_name + "' already declared");
						return Var();
					}
					
					// 确保值为浮点数类型
					init_value.to_real();
					vars[var_name] = init_value;
					return Var();
				}
				
				case 6: // stmt -> ifstmt
				case 7: // stmt -> assgstmt
				case 8: // stmt -> compoundstmt
					if (!body[0]) {
						throw runtime_error("Invalid statement structure");
					}
					return body[0]->run();
				
				case 9: { // compoundstmt -> { stmts }
					// 复合语句执行
					if (body.size() < 3) {
						throw runtime_error("Invalid compound statement structure");
					}
					return body[1]->run();
				}
				
				case 10: { // stmts -> stmt stmts
					// 连续执行多个语句
					if (body.size() < 2) {
						throw runtime_error("Invalid statements structure");
					}
					
					// 执行当前语句
					auto current_stmt = body[0]->run();
					// 递归执行剩余语句
					auto remaining_stmts = body[1]->run();
					
					return Var();
				}
				
				case 11: // stmts -> ε
					return Var();
				
				case 12: { // ifstmt -> if ( boolexpr ) then stmt else stmt
					if (body.size() < 8) {
						throw runtime_error("Invalid if statement structure");
					}
					
					// 计算条件表达式
					auto condition = body[2]->run();
					
					// 根据条件选择执行分支
					if (static_cast<bool>(condition)) {
						#ifdef DEBUG_MODE
							cout << "Executing 'then' branch at line " << line << endl;
						#endif
						return body[5]->run();
					} else {
						#ifdef DEBUG_MODE
							cout << "Executing 'else' branch at line " << line << endl;
						#endif
						return body[7]->run();
					}
				}
				
				case 13: { // assgstmt -> ID = arithexpr ;
					if (body.size() < 4) {
						throw runtime_error("Invalid assignment statement");
					}
					
					string var_name = body[0]->getName();
					
					// 检查变量是否已声明
					if (vars.find(var_name) == vars.end()) {
						errors.push("error message:line " + to_string(line) + 
								", undefined variable '" + var_name + "'");
						return Var();
					}
					
					// 计算右侧表达式
					auto rhs_value = body[2]->run();
					
					// 类型兼容性检查
					if (!vars[var_name].isReal() && rhs_value.isReal()) {
						errors.push("error message:line " + to_string(line) + 
								", cannot assign real to int");
						rhs_value.to_int();
					}
					
					vars[var_name] = rhs_value;
					return Var();
				}
				
				case 14: { // boolexpr -> arithexpr boolop arithexpr
					if (body.size() < 3) {
						throw runtime_error("Invalid boolean expression");
					}
					
					auto left = body[0]->run();
					auto right = body[2]->run();
					auto op = body[1]->getOp();
					
					if (!op) {
						throw runtime_error("Invalid boolean operator");
					}
					
					return Var(op(left, right) ? 1 : 0);
				}
				
				// 算术表达式处理
				case 20: { // arithexpr -> multexpr arithexprprime
					if (body.size() < 2) {
						throw runtime_error("Invalid arithmetic expression");
					}
					
					body[1]->inhValue = body[0]->run();
					return body[1]->run();
				}
				
				case 21: { // arithexprprime -> + multexpr arithexprprime
					if (body.size() < 3) {
						throw runtime_error("Invalid addition expression");
					}
					
					auto term = body[1]->run();
					body[2]->inhValue = inhValue + term;
					return body[2]->run();
				}
				
				case 22: { // arithexprprime -> - multexpr arithexprprime
					if (body.size() < 3) {
						throw runtime_error("Invalid subtraction expression");
					}
					
					auto term = body[1]->run();
					body[2]->inhValue = inhValue - term;
					return body[2]->run();
				}
				
				case 23: // arithexprprime -> ε
					return inhValue;
				
				case 24: { // multexpr -> simpleexpr multexprprime
					if (body.size() < 2) {
						throw runtime_error("Invalid multiplication expression");
					}
					
					body[1]->inhValue = body[0]->run();
					return body[1]->run();
				}
				
				case 25: { // multexprprime -> * simpleexpr multexprprime
					if (body.size() < 3) {
						throw runtime_error("Invalid multiplication chain");
					}
					
					auto factor = body[1]->run();
					body[2]->inhValue = inhValue * factor;
					return body[2]->run();
				}
				
				case 26: { // multexprprime -> / simpleexpr multexprprime
					if (body.size() < 3) {
						throw runtime_error("Invalid division chain");
					}
					
					auto divisor = body[1]->run();
					
					// 除零检查
					if ((!divisor.isReal() && divisor.getInt() == 0) ||
						(divisor.isReal() && fabs(divisor.getReal()) < 1e-10)) {
						errors.push("error message:line " + to_string(line) + 
								",division by zero");
						divisor = Var(1);
					}
					
					body[2]->inhValue = inhValue / divisor;
					return body[2]->run();
				}
				
				case 27: // multexprprime -> ε
					return inhValue;
				
				case 28: { // simpleexpr -> ID
					string var_name = body[0]->getName();
					
					// 检查变量是否已声明
					if (vars.find(var_name) == vars.end()) {
						errors.push("error message:line " + to_string(line) + 
								", undefined variable '" + var_name + "'");
						return Var();
					}
					
					return vars[var_name];
				}
				
				case 29: // simpleexpr -> INTNUM
				case 30: // simpleexpr -> REALNUM
					return body[0]->run();
				
				case 31: { // simpleexpr -> ( arithexpr )
					if (body.size() < 3) {
						throw runtime_error("Invalid parenthesized expression");
					}
					return body[1]->run();
				}
				
				default:
					throw runtime_error("Unknown production ID: " + to_string(productionId));
			}
		} catch (const exception& e) {
			errors.push("error message:line " + to_string(line) + 
					", runtime error: " + string(e.what()));
			return Var();
		}
		
		// 性能分析（仅在调试模式下）
		#ifdef DEBUG_MODE
			auto end_time = chrono::high_resolution_clock::now();
			auto duration = chrono::duration_cast<chrono::microseconds>(end_time - start_time);
			cout << "Production " << productionId << " executed in " 
				<< duration.count() << " microseconds" << endl;
		#endif
	}
};

const Token prodBody[32][8] = { {},
{Token(26), Token(29), },
{Token(27), Token(19), Token(26), },
{},
{Token(20), Token(22), Token(9), Token(23), },
{Token(21), Token(22), Token(9), Token(24), },
{Token(31), },
{Token(32), },
{Token(29), },
{Token(2), Token(30), Token(3), },
{Token(28), Token(30), },
{},
{Token(6), Token(4), Token(33), Token(5), Token(7), Token(28), Token(8), Token(28), },
{Token(22), Token(9), Token(35), Token(19), },
{Token(35), Token(34), Token(35), },
{Token(10), },
{Token(11), },
{Token(12), },
{Token(13), },
{Token(14), },
{Token(37), Token(36), },
{"+", Token(37), Token(36), },
{Token(16), Token(37), Token(36), },
{},
{Token(39), Token(38), },
{Token(17), Token(39), Token(38), },
{Token(18), Token(39), Token(38), },
{},
{Token(22), },
{Token(23), },
{Token(24), },
{Token(4), Token(35), Token(5), },
};
const Production productions[32] = { {},
{Token(25), prodBody[1], 2, 1},
{Token(26), prodBody[2], 3, 2},
{Token(26), prodBody[3], 0, 3},
{Token(27), prodBody[4], 4, 4},
{Token(27), prodBody[5], 4, 5},
{Token(28), prodBody[6], 1, 6},
{Token(28), prodBody[7], 1, 7},
{Token(28), prodBody[8], 1, 8},
{Token(29), prodBody[9], 3, 9},
{Token(30), prodBody[10], 2, 10},
{Token(30), prodBody[11], 0, 11},
{Token(31), prodBody[12], 8, 12},
{Token(32), prodBody[13], 4, 13},
{Token(33), prodBody[14], 3, 14},
{Token(34), prodBody[15], 1, 15},
{Token(34), prodBody[16], 1, 16},
{Token(34), prodBody[17], 1, 17},
{Token(34), prodBody[18], 1, 18},
{Token(34), prodBody[19], 1, 19},
{Token(35), prodBody[20], 2, 20},
{Token(36), prodBody[21], 3, 21},
{Token(36), prodBody[22], 3, 22},
{Token(36), prodBody[23], 0, 23},
{Token(37), prodBody[24], 2, 24},
{Token(38), prodBody[25], 3, 25},
{Token(38), prodBody[26], 3, 26},
{Token(38), prodBody[27], 0, 27},
{Token(39), prodBody[28], 1, 28},
{Token(39), prodBody[29], 1, 29},
{Token(39), prodBody[30], 1, 30},
{Token(39), prodBody[31], 3, 31},
};


class LLParserTable {
private:
	typedef const map<NonTerminal, Production> Line;
	static const Line table[15];
	//const map不能使用operator[]，因为该函数的返回值为引用类型，会破坏代码的只读属性
	//为了代码可读性这里的map就不用const了
public:
	//根据栈顶的非终结符与输入的终结符选择使用的产生式
	static const Production& parse(Terminal input, NonTerminal stackTop) {
		auto& line = table[stackTop.id - 25];
		auto it = line.find(input);
		if (it != line.end())
			return it->second;
		else {
			return productions[0];
		}
	}
};
const LLParserTable::Line LLParserTable::table[] = {
{//非终结符编号:25, 数组索引:0
{Token(2),productions[1]},
{Token(20),productions[1]},
{Token(21),productions[1]},
},
{//非终结符编号:26, 数组索引:1
{Token(2),productions[3]},
{Token(20),productions[2]},
{Token(21),productions[2]},
},
{//非终结符编号:27, 数组索引:2
{Token(20),productions[4]},
{Token(21),productions[5]},
},
{//非终结符编号:28, 数组索引:3
{Token(2),productions[8]},
{Token(6),productions[6]},
{Token(22),productions[7]},
},
{//非终结符编号:29, 数组索引:4
{Token(2),productions[9]},
},
{//非终结符编号:30, 数组索引:5
{Token(2),productions[10]},
{Token(3),productions[11]},
{Token(6),productions[10]},
{Token(22),productions[10]},
},
{//非终结符编号:31, 数组索引:6
{Token(6),productions[12]},
},
{//非终结符编号:32, 数组索引:7
{Token(22),productions[13]},
},
{//非终结符编号:33, 数组索引:8
{Token(4),productions[14]},
{Token(22),productions[14]},
{Token(23),productions[14]},
{Token(24),productions[14]},
},
{//非终结符编号:34, 数组索引:9
{Token(10),productions[15]},
{Token(11),productions[16]},
{Token(12),productions[17]},
{Token(13),productions[18]},
{Token(14),productions[19]},
},
{//非终结符编号:35, 数组索引:10
{Token(4),productions[20]},
{Token(22),productions[20]},
{Token(23),productions[20]},
{Token(24),productions[20]},
},
{//非终结符编号:36, 数组索引:11
{Token(4),productions[23]},
{Token(5),productions[23]},
{Token(10),productions[23]},
{Token(11),productions[23]},
{Token(12),productions[23]},
{Token(13),productions[23]},
{Token(14),productions[23]},
{"+",productions[21]},
{Token(16),productions[22]},
{Token(19),productions[23]},
{Token(22),productions[23]},
{Token(23),productions[23]},
{Token(24),productions[23]},
},
{//非终结符编号:37, 数组索引:12
{Token(4),productions[24]},
{Token(22),productions[24]},
{Token(23),productions[24]},
{Token(24),productions[24]},
},
{//非终结符编号:38, 数组索引:13
{Token(4),productions[27]},
{Token(5),productions[27]},
{Token(10),productions[27]},
{Token(11),productions[27]},
{Token(12),productions[27]},
{Token(13),productions[27]},
{Token(14),productions[27]},
{"+",productions[27]},
{Token(16),productions[27]},
{Token(17),productions[25]},
{Token(18),productions[26]},
{Token(19),productions[27]},
{Token(22),productions[27]},
{Token(23),productions[27]},
{Token(24),productions[27]},
},
{//非终结符编号:39, 数组索引:14
{Token(4),productions[31]},
{Token(22),productions[28]},
{Token(23),productions[29]},
{Token(24),productions[30]},
},
};


class TokenStream {
private:
	string prog;//程序的字节流
	int begin = 0;
	int end = 0;
	//bool为0时说明没有下一个字符
public:
	Token nextTerminal;	//下一个词法单元
	int line = 0; //词法单元所在的行数
	//返回词法单元的名称
	string getTokenName() {
		return prog.substr(begin, end - begin);
	}
	TokenStream() {
	}
	TokenStream(const string& prog_) {
		prog = prog_;
		begin = 0;
		end = 0;
		line = 1;
		get_symbol();
	}
	//设置prog
	void set(const string& prog_) {
		prog = prog_;
		begin = 0;
		end = 0;
		line = 1;
		get_symbol();
	}
	//更新nextTerminal
	bool get_symbol() {
		begin = end;
		while (isspace(prog[begin])) {//如果是空格，则忽略
			if (prog[begin] == '\n') {
				line++;
			}
			begin++;
		}

		int progLen = prog.length();
		if (begin >= progLen) {
			nextTerminal = Token(0);
			//cout << "next input token:" << nextTerminal << endl;
			return false;
		}
		end = begin + 1;
		while (end < progLen && !isspace(prog[end])) {//如果非空格，则读入
			end++;
		}
		nextTerminal = prog.substr(begin, end - begin);

		//cout << "next input token:" << nextTerminal << endl;
		return true;
	}
};



TokenStream tokenStream;
void Production::parse() {
	auto& expectedBody = productions[productionId].body;
	body.reserve(expectedBody.size());
	for (auto it = expectedBody.begin(); it != expectedBody.end(); it++) {
		const Token& expected = (*it)->head;
		auto nextTerminal = tokenStream.nextTerminal;
		if (expected.isTerminal()) {
			if (expected == tokenStream.nextTerminal) {
				//show(Stacktop, depth);
				if (expected == Token(22)) {
					body.emplace_back(new IDNode(tokenStream.getTokenName()));
				}
				else if (expected == Token(23)) {
					body.emplace_back(new VarNode(atoi(tokenStream.getTokenName().c_str())));
				}
				else if (expected == Token(24)) {
					body.emplace_back(new VarNode(atof(tokenStream.getTokenName().c_str())));
				}
				else {
					body.emplace_back(new Node(expected));
				}
				tokenStream.get_symbol();
			}
			else if (expected == Token(1)) {
				cout << "出现意外的词法单元E" << endl;
				//show(Stacktop, depth);
			}
			else {
				if (expected == Token(23) && tokenStream.nextTerminal == Token(24)) {
					errors.emplace("error message:line " + to_string(tokenStream.line)
						+ ",realnum can not be translated into int type");
					body.emplace_back(new VarNode(atoi(tokenStream.getTokenName().c_str())));
					tokenStream.get_symbol();
				}
				else {
					cout << "无法识别的错误" << endl;
					cout << "预期的词法单元\"" << expected <<
						"\"与输入的词法单元\"" << tokenStream.nextTerminal << "\"不一致" << endl;
				}//LLparser.errors.emplace(make_pair(LLparser.tokenStream.line, expected));

				//cout << Token(40) << endl;
			}
		}
		else if (expected.is_non_terminal()) {//非终结符
			//show(it->token, depth);
			//当输入文法符号串存在语法错误时无法正常解析

			auto& nextNode = LLParserTable::parse(tokenStream.nextTerminal, expected);
			if (nextNode.productionId == 0) {
				cout << "发现错误，无法解析" << endl;
				body.emplace_back(new Production(nextNode.head, nextNode.productionId));
			}
			body.emplace_back(new Production(nextNode.head, nextNode.productionId));
			auto last = *(body.rbegin());
			last->line = tokenStream.line;
			last->parse();
		}
		else {
			cout << "出现无法解析的符号" << endl;
			(*it)->head = Token(1);
		}
	}
	return;
	
}

void Analysis() {
    // 程序输入和初始化
    string prog;
    read_prog(prog);
    
    /* 骚年们 请开始你们的表演 */
    /********* Begin *********/
    
    try {
        // 输入验证
        if (prog.empty()) {
            errors.push("error message: empty input program");
            goto error_handling;
        }

        // 初始化解析树根节点
        Production root(productions[1].head, 1);
        if (!productions[1].head.is_non_terminal()) {
            errors.push("error message: invalid root production");
            goto error_handling;
        }

        // 设置词法分析器
        tokenStream.set(prog);
        
        // 语法分析阶段
        try {
            root.parse();
        } catch (const exception& e) {
            errors.push("error message: parsing failed - " + string(e.what()));
            goto error_handling;
        }

        // 语义分析和执行阶段
        try {
            root.run();
        } catch (const exception& e) {
            errors.push("error message: execution failed - " + string(e.what()));
            goto error_handling;
        }

        // 结果输出处理
        if (errors.empty()) {
            // 格式化输出变量表
            for (auto it = vars.begin(); it != vars.end(); ++it) {
                const string& varName = it->first;
                const Var& value = it->second;

                // 变量名输出
                printf("%s: ", varName.c_str());

                // 根据类型格式化输出值
                if (value.isReal()) {
                    // 浮点数输出，使用%g以获得最佳表现形式
                    printf("%g\n", value.getReal());
                } else {
                    // 整数输出
                    printf("%d\n", value.getInt());
                }
            }
        } else {
            goto error_handling;
        }

        // 可选的调试输出
        #ifdef DEBUG_MODE
            cout << "\nDebug: Parse tree structure:" << endl;
            root.show(0);
            
            cout << "\nDebug: Variable count: " << vars.size() << endl;
            cout << "Debug: Error count: " << errors.size() << endl;
        #endif

        return;

    } catch (const exception& e) {
        // 捕获所有未处理的异常
        errors.push("error message: unexpected error - " + string(e.what()));
    }

error_handling:
    // 统一的错误处理
    if (!errors.empty()) {
        // 按顺序输出所有错误信息
        while (!errors.empty()) {
            const string& error = errors.front();
            printf("%s\n", error.c_str());
            errors.pop();
        }
    }

    // 清理资源
    vars.clear();
    while (!errors.empty()) {
        errors.pop();
    }
    
    /********* End *********/
}

