/*
* modified by kim doohee on 2019.12.13
*
* 본 프로그램은 "만들면서 배우는 인터프린터"에 있는
* 전자 계산기 프로그램을 참고하였습니다.
*
* 프로그램 사용법은 'README.md'를 참조하시면 됩니다.
*
*/

#include "stdafx.h"
#include <iostream>
#include <cstdlib>			// for exit()
#include <cctype>			// for is...()
using namespace std;

enum TknKind				// 토큰의 종류 
{
	Others,
	Program, Var, Begin, Int, Float, End, TaskName,
	VarName, Print, Comment,
	Assign, Lparen, Rparen, Plus, Minus, Multi, Divi, Power,
	IntNum, FloatNum, Comma, FinStc, EofTkn
};

struct Token				// 토큰 구조체
{
	TknKind kind;			// 토큰의 종류
	int intVal;				// 상수값이나 변수 번호
	float floatVal;
	Token() { kind = Others; intVal = 0; }
	Token(TknKind k) { kind = k; intVal = 0; }
	Token(TknKind k, int d) { kind = k; intVal = d; }
	Token(TknKind k, float f) { kind = k; floatVal = f; }
};

struct Symbol				// 변수 구조체
{
	TknKind kind;
	int intNum;
	float floatNum;
	bool isAvailable;		// 변수 사용 가능 여부
	Symbol() { kind = Others; isAvailable = false; }
	Symbol(TknKind k) { kind = k; isAvailable = true; }
};

#define STK_SIZ 20			// 스택 크기
void input();
bool programExit();
void statement();
void createVar(TknKind k);
void expression();
void term();
void factor();
void power();
Token nextTkn();
TknKind getWordTkn(char* str);
int nextCh();
void operate(TknKind op);
void powerOperate(float d1, int d2);
void push(float n);
float pop();
bool chkTkn(TknKind kd);
bool exceptComment();
void errorMessage(int err);


float stack[STK_SIZ + 1];	// 스택
int stkct;					// 스택 관리
Token token;				// 토큰 저장
char buf[80];				// 입력용
int idx;					// 문자 위치
int ch;						// 가져온 문자를 저장
Symbol symVar[26];			// 변수 a-z
int errF;					// 오류 발생
char word[10];				// 키워드 또는 프로그램 이름
bool isProgram;				// Program 선언 유무
bool isVar;					// Var 선언 유무
bool isBegin;				// Begin 선언 유무


/*
 * 프로그램의 메인 함수
 */
int main()
{
	isProgram = false;
	isVar = false;
	isBegin = false;

	while (1) {
		input();							// 입력
		token = nextTkn();					// 최초 토큰
		if (token.kind == EofTkn) {			// 종료 판별
			if (programExit()) exit(1);
			else continue;
		}
		statement();
		if (errF) errorMessage(errF);		// 에러 메세지
	}
	return 0;
}

/*
 * 문자열을 입력받는 함수
 */
void input()
{
	errF = 0; stkct = 0;					// 초기 설정
	cin.getline(buf, 80);					// 80문자 이내의 입력
	idx = 0;								// 시작 문자 위치
	ch = nextCh();							// 초기 문자 가져오기
}

/*
 * 프로그램 종료 여부를 판별하는 함수
 */
bool programExit()
{
	char s[10];

	cout << " -- Exit the program?(y/n) --" << endl;
	cin.getline(s, 10);

	switch (s[0])
	{
	case 'y': return true;
	case 'n': return false;
	default:
		cout << " -- Incorrect input --" << endl;
		return false;
	}
	return false;
}

/*
 * 토큰에 따른 각종 문법을 처리하는 함수 (선언문, 대입문, 출력문)
 */
void statement()
{
	int vNbr;

	switch (token.kind)
	{
	case Program:	// 프로그램 선언
		if (isProgram) { errF = 2; return; }
		token = nextTkn();
		if (!chkTkn(TaskName)) { errF = 1; return; }
		token = nextTkn();
		if (!chkTkn(EofTkn)) { errF = 1; return; }
		isProgram = true;
		return;

	case Var:		// 변수 선언
		if (!isProgram) { errF = 3; return; }
		if (isVar) { errF = 2; return; }
		token = nextTkn();
		if (!chkTkn(EofTkn)) { errF = 1; return; }
		isVar = true;
		return;

	case Begin:		// 대입 & 출력
		if (!isVar) { errF = 4; return; }
		if (isBegin) { errF = 2; return; }
		token = nextTkn();
		if (!chkTkn(EofTkn)) { errF = 1; return; }
		isBegin = true;
		return;

	case End:		// 프로그램 종료
		if(!isBegin) { errF = 5; return; }
		token = nextTkn();
		if (!chkTkn(EofTkn)) { errF = 1; return; }
		isProgram = false;							// 선언 초기화
		isVar = false;
		isBegin = false;
		memset(stack, 0, STK_SIZ + 1);				// 스택 초기화
		memset(symVar, 0, sizeof(symVar));			// 변수 초기화
		return;

	case Int:		// Int 변수 선언
		createVar(Int);
		return;

	case Float:		// Float 변수 선언
		createVar(Float);
		return;

	case VarName:		// 대입문
		if(!isBegin) { errF = 5; return; }
		vNbr = token.intVal;													// 대입할 곳 보존 
		if(!symVar[vNbr].isAvailable) { errF = 10; return; }					// 변수 사용 가능 여부 체크
		token = nextTkn();
		if (!chkTkn(Assign)) { errF = 1; return; }								// '=' 일 것
		token = nextTkn();
		expression();															// 우변 계산
		if (!chkTkn(FinStc)) { errF = 1; return; }
		token = nextTkn();
		if (!chkTkn(EofTkn)) { errF = 1; return; }
		if (symVar[vNbr].kind == Int) symVar[vNbr].intNum = pop();				// int 대입 실행
		else if (symVar[vNbr].kind == Float) symVar[vNbr].floatNum = pop();		// float 대입 실행
		else errF = 1;
		return;

	case Print:			// 출력문
		if (!isBegin) { errF = 5; return; }
		token = nextTkn();
		expression();
		if (!chkTkn(FinStc)) { errF = 1; return; }
		token = nextTkn();
		if (!chkTkn(EofTkn)) { errF = 1; return; }
		if (!errF) cout << " -- " << pop() << " --"<< endl;	// 에러가 없을 경우만 출력
		return;

	case Comment:		// 주석
		return;

	default:
		errF = 1;
		return;
	}
}

/*
 * 변수를 선언을 도와주는 함수
 */
void createVar(TknKind k)
{
	int vNbr;
	Symbol tempVar[26];

	if (!isVar) { errF = 4; return; }
	if (isBegin) { errF = 6; return; }
	memcpy(tempVar, symVar, sizeof(symVar));

	token = nextTkn();
	if (!chkTkn(VarName)) { errF = 1; return; }

	vNbr = token.intVal;
	if (!symVar[vNbr].isAvailable) symVar[vNbr] = Symbol(k);	// 변수 중복 체크
	else {
		memset(symVar, 0, sizeof(symVar));
		memcpy(symVar, tempVar, sizeof(tempVar));
		errF = 11; return;
	}

	token = nextTkn();
	while (token.kind == Comma) {				// 여러개의 변수를 선언할 경우
		token = nextTkn();
		if (!chkTkn(VarName)) {
			memset(symVar, 0, sizeof(symVar));
			memcpy(symVar, tempVar, sizeof(tempVar));
			errF = 1; return;
		}
		vNbr = token.intVal;
		if (!symVar[vNbr].isAvailable) symVar[vNbr] = Symbol(k);
		else {
			memset(symVar, 0, sizeof(symVar));
			memcpy(symVar, tempVar, sizeof(tempVar));
			errF = 11; return;
		}
		token = nextTkn();
	}

	if (!chkTkn(FinStc)) {
		memset(symVar, 0, sizeof(symVar));
		memcpy(symVar, tempVar, sizeof(tempVar));
		errF = 1; return;
	}

	token = nextTkn();
	if (!chkTkn(EofTkn)) {
		memset(symVar, 0, sizeof(symVar));
		memcpy(symVar, tempVar, sizeof(tempVar));
		errF = 1; return;
	}
}

/*
 * 식을 처리하는 함수 (덧셈과 뺄셈)
 */
void expression()
{
	TknKind op;

	term();
	while (token.kind == Plus || token.kind == Minus) {
		op = token.kind;
		token = nextTkn();
		term();
		operate(op);
	}
}

/*
 * 식을 처리하는 함수 (곱셈과 나눗셈)
 */
void term()	
{
	TknKind op;

	power();
	while (token.kind == Multi || token.kind == Divi) {
		op = token.kind;
		token = nextTkn();
		power();
		operate(op);
	}
}

/*
 * 식을 처리하는 함수 (거듭제곱)
 */
void power()
{
	TknKind op;

	factor();
	while (token.kind == Power) {
		op = token.kind;
		token = nextTkn();
		factor();
		operate(op);
	}
}

/*
 * 식의 인자를 처리하는 함수 (변수, 상수, 괄호)
 */
void factor()
{
	switch (token.kind)
	{
	case VarName:		// 변수
		if (!symVar[token.intVal].isAvailable) { errF = 10; break; }	// 변수 사용 가능 여부 체크
		if (symVar[token.intVal].kind == Int) push(symVar[token.intVal].intNum);
		else if (symVar[token.intVal].kind == Float) push(symVar[token.intVal].floatNum);
		else errF = 1;
		break;
	case IntNum:		// 정수
		push(token.intVal);
		break;
	case FloatNum:		// 실수
		push(token.floatVal);
		break;
	case Minus:			// 음수
		token = nextTkn();
		if (chkTkn(IntNum)) push(-1* token.intVal);		// 상수 일 것
		else if (chkTkn(FloatNum)) push(-1.0 * token.floatVal);
		else errF = 1;
		break;
	case Lparen:		// (식)
		token = nextTkn();
		expression();
		if (!chkTkn(Rparen)) errF = 1;		// ')'일 것
		break;
	default:
		errF = 1;
	}
	token = nextTkn();
}

/*
 * 토큰을 반환하는 함수
 */
Token nextTkn()
{
	TknKind kd = Others;
	int num;
	float fnum;

	if (exceptComment()) {						// 주석을 처음부터 쓸 경우, Comment 토큰을 반환
		return Token(Comment, 0);
	}

	while (isspace(ch))							// 공백 건너뛰기
		ch = nextCh();

	if (isdigit(ch)) {							// 정수와 실수를 처리하기 위한 과정
		for (num = 0; isdigit(ch); ch = nextCh())
			num = num * 10 + (ch - '0');

		if (ch == '.') {
			ch = nextCh();

			if (isdigit(ch)) {
				fnum = num;
				for (float temp = 1.0; isdigit(ch); ch = nextCh()) {
					temp *= 0.1;
					fnum = fnum + (ch - '0')*temp;
				}
				return Token(FloatNum, fnum);
			}
			else {
				errF = 1;
			}
		}
		else {
			return Token(IntNum, num);
		}
	}

	else if (islower(ch)) {						// 변수 or 키워드
		if (islower(buf[idx])) {				// 키워드
			for (int i = 0; islower(ch); ch = nextCh()) {
				word[i] = ch;
				i++;
			}

			kd = getWordTkn(word);				
			return Token(kd);
		}
		else {									// 변수
			num = ch - 'a';						// 변수 번호 0-25
			ch = nextCh();
			return Token(VarName, num);
		}
	}

	else {
		switch (ch)
		{
		case '(': kd = Lparen; break;
		case ')': kd = Rparen; break;
		case '+': kd = Plus; break;
		case '-': kd = Minus; break;
		case '*': kd = Multi; break;
		case '/': kd = Divi; break;
		case '=': kd = Assign; break;
		case '\0': kd = EofTkn; break;
		case '^': kd = Power; break;
		case ';': kd = FinStc; break;
		case ',': kd = Comma; break;
		}
		ch = nextCh();
		return Token(kd);
	}
}


/*
 * word의 토큰을 반환하는 함수
 */
TknKind getWordTkn(char* str)
{
	if (!strncmp(word, "program", 7)) {
		return Program;
	}
	else if (!strncmp(word, "var", 3)) {
		return Var;
	}
	else if (!strncmp(word, "begin", 5)) {
		return Begin;
	}
	else if (!strncmp(word, "end", 3)) {
		return End;
	}
	else if (!strncmp(word, "int", 3)) {
		return Int;
	}
	else if (!strncmp(word, "float", 5)) {
		return Float;
	}
	else if (!strncmp(word, "print", 5)) {
		return Print;
	}
	else {
		return TaskName;
	}
}

/*
 * 문자열의 다음 문자를 반환하는 함수
 */
int nextCh()
{
	if (buf[idx] == '\0') return '\0';
	else return buf[idx++];
}

/*
 * 연산을 실행하는 함수
 */
void operate(TknKind op)
{
	float d2 = pop(), d1 = pop();

	if (op == Divi && d2 == 0) errF = 12;
	if (errF) return;
	switch (op)
	{
	case Plus: push(d1 + d2); break;
	case Minus: push(d1 - d2); break;
	case Multi: push(d1 * d2); break;
	case Divi: push(d1 / d2); break;
	case Power: powerOperate(d1, d2); break;
	}
}

/*
 * 거듭제곱을 계산하는 함수
 */
void powerOperate(float d1, int d2)
{
	float n = 1;
	for (int i = 0; i < d2; i++) { n *= d1; }
	push(n);
}

/*
 * 스택에 저장하는 함수
 */
void push(float n)
{
	if (errF) return;
	if (stkct + 1 > STK_SIZ) { cout << "stack overflow\n"; exit(1); }
	stack[++stkct] = n;
}

/*
 * 스택에서 추출하는 함수
 */
float pop()
{
	if (errF) return 1;			// 오류 시는 단순히 1을 반환한다
	if (stkct < 1) { cout << "stack underflow\n"; exit(1); }
	return stack[stkct--];
}

/*
 * 토큰의 종류를 확인하는 함수
 */
bool chkTkn(TknKind kd)	
{
	if (token.kind == kd) return true;
	else return false;
}

/*
 * 주석을 처리하는 함수
 */
bool exceptComment()
{
	bool result = false;

	while (ch != '\0' && isspace(ch)) ch = nextCh();
	if (ch == '\0') return result;

	if (ch == '/')
	{
		if (buf[idx] == '/')
		{
			if (idx == 1) result = true;		// 주석을 처음부터 쓸 경우
			while (ch != '\0') ch = nextCh();
		}
	}

	return result;
}

/*
 * 에러 메세지를 출력하는 함수
 */
void errorMessage(int err)
{
	switch (err)
	{
	case 1:
		cout << " -- Incorrect grammar --\n";				// 문법에 맞지 않습니다
		break;
	case 2:
		cout << " -- Already declared --\n";				// 이미 선언되어 있습니다
		break;
	case 3:
		cout << " -- Declare 'program' first --\n";			// program을 먼저 선언하십시오
		break;
	case 4:
		cout << " -- Declare 'var' first --\n";				// var를 먼저 선언하십시오
		break;
	case 5:
		cout << " -- Declare 'begin' first --\n";			// begin을 먼저 선언하십시오
		break;
	case 6:
		cout << " -- Can't declare a variable --\n";		// 지금은 변수를 선언할 수 없습니다
		break;
	case 10:
		cout << " -- Variable not declared --\n";			// 변수가 선언되지 않았습니다
		break;
	case 11:
		cout << " -- The variable already exists --\n";		// 선언된 변수가 존재합니다
		break;
	case 12:
		cout << " -- division by 0 --\n";					// 0으로 나누기를 시도 했습니다
		break;
	}
}

