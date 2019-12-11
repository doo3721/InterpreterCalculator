# Interpreter Calculator


**이 프로젝트는 오토마타와 컴파일러 과제를 위한 것입니다**

**코드는 "만들면서 배우는 인터프린터"에 있는 전자 계산기 프로그램을 참고하였습니다**

---


**사용법**

본 프로그램은 다음과 같은 문법을 따릅니다

```
program task        // Declare program
var                 // Declare variables
    int a, b, c;    // Int variables
    int d, e;
begin               // Declare begin
    a = 10;
    b = 20;
    c = a+b;
    print c;        // Execute print
 -- 30 --           // Print c value
    print c/3;
 -- 10 --
end                 // Program end
```



 * 모든 영문자는 소문자를 사용합니다

 * program, var, begin, end 순으로 선언해야 합니다

 * program을 선언할 때, 프로그램 이름도 같이 입력합니다

 * var는 변수 선언, begin은 대입과 출력, end는 종료를 할 수 있습니다

 * 변수는 영어 소문자 1글자만 가능합니다 (총 26개)

 * 변수 선언, 대입, 출력은 항상 세미콜론(;)으로 끝내야 합니다

 * begin에서는 선언한 변수에 값을 대입할 수 있습니다
   * var에서 선언하지 않은 변수는 값을 대입할 수 없습니다

 * 가능한 연산은 다음과 같습니다
   * 사칙연산 (+,-,*,/)
   * 거듭제곱 (^)
   * Grouping ('(',')')

 * 출력문은 변수뿐만 아니라 식을 입력하는 것이 가능합니다

 * 주석을 입력할 수 있습니다

 * 띄어쓰기는 자유롭게 사용할 수 있습니다

---


**현재 진행상황**
 * 프로젝트 생성 및 토대 구축

---


**TODO LIST**
 * Float 타입 추가
 * 음수 연산

---


**변경 사항**
 * 계산기에서 약식 인터프리터 프로그램으로 변경
 * 변수는 선언된 변수만 사용가능 하도록 변경