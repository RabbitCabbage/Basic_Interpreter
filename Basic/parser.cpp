/*
 * File: parser.cpp
 * ----------------
 * Implements the parser.h interface.
 */

#include <iostream>
#include <string>
#include <sstream>
#include "exp.h"
#include "parser.h"
#include "../StanfordCPPLib/error.h"
#include "../StanfordCPPLib/strlib.h"
#include "../StanfordCPPLib/tokenscanner.h"
#include "program.h"

using namespace std;

unordered_set<string> command_issue{"REM", "INPUT", "LET", "PRINT", "END", "IF", "RUN", "CLEAR", "QUIT", "HELP", "LIST",
                                    "GOTO"};
bool end_mark = false;
/*
 * Implementation notes: parseExp
 * ------------------------------
 * This code just reads an expression and then checks for extra tokens.
 */
//将具有计算功能的部分特化，我们仍要输入一个string，仅含有运算式,相当于之前的效果一点没变
Expression *parseExp_calculation(TokenScanner &scanner) {
    Expression *exp = readE(scanner);
    if (scanner.hasMoreTokens()) {
        error("parseExp: Found extra token: " + scanner.nextToken());
    }
    return exp;
}

//parser的部分是有行号的部分
int parseExp(TokenScanner &scanner, Program &program, EvalState &state) {
    while (scanner.hasMoreTokens()) {
        string token = scanner.nextToken();
        //处理行数
        int ascii = token[0] - '0';
        if (ascii < 0 || ascii > 9) {
            if (command_issue.count(token)) {
                if (token == "IF") {
                    //variable for condition
                    string condition_lhs, condition_rhs, condition_op;
                    int condition_lhs_value, condition_rhs_value;
                    bool condition_judge;
                    TokenScanner lhs_scanner, rhs_scanner;
                    Expression *expl;
                    Expression *expr;
                    //找到比较运算符，并且比较两边的值
                    while (scanner.hasMoreTokens()) {
                        token = scanner.nextToken();
                        if (token == "=" || token == "<" || token == ">") {
                            condition_op = token;
                            break;
                        } else {
                            condition_lhs += token;
                        }
                    }
                    while (scanner.hasMoreTokens()) {
                        token = scanner.nextToken();
                        if (token == "THEN")break;
                        else condition_rhs += token;
                    }
                    lhs_scanner.ignoreWhitespace();
                    lhs_scanner.scanNumbers();
                    lhs_scanner.setInput(condition_lhs);
                    expl = parseExp_calculation(lhs_scanner);
                    condition_lhs_value = expl->eval(state);
                    rhs_scanner.ignoreWhitespace();
                    rhs_scanner.scanNumbers();
                    rhs_scanner.setInput(condition_rhs);
                    expr = parseExp_calculation(rhs_scanner);
                    condition_rhs_value = expr->eval(state);
                    delete expl;
                    delete expr;
                    if (condition_op == "=") {
                        if (condition_lhs_value == condition_rhs_value)
                            condition_judge = true;
                        else condition_judge = false;
                    } else if (condition_op == "<") {
                        if (condition_lhs_value < condition_rhs_value)
                            condition_judge = true;
                        else condition_judge = false;
                    } else {
                        if (condition_lhs_value > condition_rhs_value)
                            condition_judge = true;
                        else condition_judge = false;
                    }
                    if (condition_judge) {
                        string line_number = scanner.nextToken();
                        stringstream ss;
                        ss.clear();
                        ss << line_number;
                        int line_num;
                        ss >> line_num;
                        return line_num;
                    }

                } else if (token == "LET") {
                    string let_expression;
                    while (scanner.hasMoreTokens()) {
                        token = scanner.nextToken();
                        let_expression += token;
                    }
                    TokenScanner let_scanner;
                    let_scanner.ignoreWhitespace();
                    let_scanner.scanNumbers();
                    let_scanner.setInput(let_expression);
                    Expression *let_exp = parseExp_calculation(let_scanner);
                    int value = let_exp->eval(state);

                } else if (token == "INPUT") {
                    token = scanner.nextToken();
                    string input_string = token;
                    int input_num;
                    cout << " ? ";
                    cin >> input_num;
                    if (!cin.fail()) {
                        if (getchar() != '\n') {
                            cout << "INVALID NUMBER" << endl;
                            cin.clear();
                            cout << " ? ";
                            cin.ignore(10000, '\n');
                            cin >> input_num;
                            getchar();
                        }
                    }
                    while (cin.fail()) {
                        cout << "INVALID NUMBER" << endl;
                        cin.clear();
                        cout << " ? ";
                        cin.ignore(10000, '\n');
                        cin >> input_num;
                        if (!cin.fail()) {
                            if (getchar() != '\n') {
                                cout << "INVALID NUMBER" << endl;
                                cin.clear();
                                cout << " ? ";
                                cin.ignore(10000, '\n');
                                cin >> input_num;
                                getchar();
                            }
                        }

                    }

                    stringstream ss;
                    ss.clear();
                    ss << input_num;
                    string input_trans;
                    ss >> input_trans;
                    input_string += "=";
                    if (input_num < 0)input_string += "0";
                    input_string += input_trans;
                    TokenScanner input_scanner;
                    input_scanner.ignoreWhitespace();
                    input_scanner.scanNumbers();
                    input_scanner.setInput(input_string);
                    Expression *input_exp = parseExp_calculation(input_scanner);
                    int value = input_exp->eval(state);
                } else if (token == "PRINT") {
                    string print_expression;
                    while (scanner.hasMoreTokens()) {
                        token = scanner.nextToken();
                        print_expression += token;
                    }
                    TokenScanner print_scanner;
                    print_scanner.ignoreWhitespace();
                    print_scanner.scanNumbers();
                    print_scanner.setInput(print_expression);
                    Expression *print_exp = parseExp_calculation(print_scanner);
                    int value = print_exp->eval(state);
                    if (value != -19260817)
                        cout << value << endl;

                } else if (token == "END") {
                    end_mark = true;
                    break;
                } else if (token == "REM") {
                    break;
                } else if (token == "GOTO") {
                    token = scanner.nextToken();
                    stringstream ss;
                    ss.clear();
                    ss << token;
                    int line_goto;
                    ss >> line_goto;
                    return line_goto;//这里返回了goto的句子编号
                } else if (token == "RUN") {
                    end_mark=false;
                    int line_number = program.getFirstLineNumber();
                    while (line_number != -1) {
                        string line = program.getSourceLine(line_number);
                        TokenScanner run_scanner;
                        run_scanner.ignoreWhitespace();
                        run_scanner.scanNumbers();
                        run_scanner.setInput(line);
                        int return_number = parseExp(run_scanner, program, state);//如果有goto语句就返回了下一句的行号
                        if (return_number == -1) line_number = program.getNextLineNumber(line_number);
                        else {
                            line_number = return_number;
                            if (!program.require(line_number)) {
                                cout << "LINE NUMBER ERROR" << endl;
                                break;
                            } else continue;
                        }
                        if (end_mark)break;
                    }
                } else if (token == "LIST") {
                    int line_number = program.getFirstLineNumber();
                    while (line_number != -1) {
                        string line = program.getSourceLine(line_number);
                        cout << line << endl;
                        line_number = program.getNextLineNumber(line_number);
                    }
                } else if (token == "CLEAR") {
                    program.clear();
                    state.clear();
                } else if (token == "QUIT") {
                    exit(0);
                }

            }

        }
    }
    return -1;
};

/*
 * Implementation notes: readE
 * Usage: exp = readE(scanner, prec);
 * ----------------------------------
 * This version of readE uses precedence to resolve the ambiguity in
 * the grammar.  At each recursive level, the parser reads operators and
 * subexpressions until it finds an operator whose precedence is greater
 * than the prevailing one.  When a higher-precedence operator is found,
 * readE calls itself recursively to read in that subexpression as a unit.
 */

Expression *readE(TokenScanner &scanner, int prec) {
    Expression *exp = readT(scanner);
    string token;
    while (true) {
        token = scanner.nextToken();
        int newPrec = precedence(token);
        if (newPrec <= prec) break;
        Expression *rhs = readE(scanner, newPrec);
        exp = new CompoundExp(token, exp, rhs);
    }
    scanner.saveToken(token);
    return exp;
}

/*
 * Implementation notes: readT
 * ---------------------------
 * This function scans a term, which is either an integer, an identifier,
 * or a parenthesized subexpression.
 */

Expression *readT(TokenScanner &scanner) {
    string token = scanner.nextToken();
    TokenType type = scanner.getTokenType(token);
    if (type == WORD) return new IdentifierExp(token);
    if (type == NUMBER) return new ConstantExp(stringToInteger(token));
    if (token != "(") error("Illegal term in expression");
    Expression *exp = readE(scanner);
    if (scanner.nextToken() != ")") {
        error("Unbalanced parentheses in expression");
    }
    return exp;
}

/*
 * Implementation notes: precedence
 * --------------------------------
 * This function checks the token against each of the defined operators
 * and returns the appropriate precedence value.
 */

int precedence(string token) {
    if (token == "=") return 1;
    if (token == "+" || token == "-") return 2;
    if (token == "*" || token == "/") return 3;
    return 0;
}
