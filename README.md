# 编译原理课程实践报告：StupidSysY2RV

## 一、编译器概述

### 1.1 基本功能

本编译器基本具备如下功能：
1. 将[SysY](https://pku-minic.github.io/online-doc/#/misc-app-ref/sysy-spec)语言编译为[Koopa IR](https://pku-minic.github.io/online-doc/#/misc-app-ref/koopa)形式；
1. 将[SysY](https://pku-minic.github.io/online-doc/#/misc-app-ref/sysy-spec)语言编译为RISC-V代码；
1. 大概率通过自身崩溃检查代码错误的功能；

### 1.2 主要特点

包含了基本要求的功能：表达式编译、常量变量、语句块作用域、if, while语句、函数调用、全局变量、数组。

还有如下等特点：速度慢、容易崩溃……/(ㄒoㄒ)/~~

## 二、编译器设计

整体设计：

1. 使用词法、语法分析器分析SysY代码文件，构建抽象语法树AST；
2. 调用根节点AST的函数，直接通过AST生成Koopa IR结构的内存形式；
3. 将Koopa IR结构输出为代码，重新读取后，再遍历Koopa IR结构的内存形式，生成RISC-V代码。

### 2.1 主要模块组成

编译器由4个主要模块组成：

1. 分析模块：```sysy.l```以及```sysy.y```分别进行词法分析和语法分析，构建抽象语法树(AST)；
2. AST模块：```ast```文件夹中的代码。实现AST结构，并使用该结构生成Koopa结构。
3. Builder模块：```builder```文件夹中的代码。读取Koopa结构，生成RISC-V代码。
4. 工具模块：其他文件。包含基本块维护器，循环结构维护器，生成Koopa结构的工具函数等。

### 2.2 主要数据结构

#### 抽象语法树

抽象语法树节点均继承自```BaseAST```类，该类规定了一些可以实现的功能，方便使用多态调用。比如```build_koopa_values```用于将该节点的语义转化为Koopa结构并返回；```CalcValue```用于计算常量表达式的值。

```BaseAST``` 中有3个static成员，作为全局信息：用于维护生成IR时需要的符号表```symbol_list```；当前基本块的语句缓存区```block_maintainer```；维护嵌套循环结构的```loop_maintainer```

```c++
class BaseAST
{
public:
    static SymbolList symbol_list;
    static BlockMaintainer block_maintainer;
    static LoopMaintainer loop_maintainer;

    virtual ~BaseAST() = default;
    virtual std::string to_string() const;
    virtual void *build_koopa_values() const;
    virtual int CalcValue() const;
    virtual void *koopa_leftvalue() const;
};
```

AST节点示例：

赋值语句的AST需要包含左值节点和右值表达式节点。其重写函数```build_koopa_values```中，调用了子节点的函数，以获取他们的Koopa结构，并由此构建自己的Koopa结构（Store语句），再通过全局结构```block_maintainer```将该语句添加进当前基本块。

```c++
class AssignmentAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> lval;
    std::unique_ptr<BaseAST> exp;
    AssignmentAST(std::unique_ptr<BaseAST> &_lval, std::unique_ptr<BaseAST> &_exp);
    void *build_koopa_values() const override
    {
        koopa_raw_value_data *res = new koopa_raw_value_data();
        // .. some code to build koopa_raw_value_data
        block_maintainer.AddInst(res);
        return res;
    }
};
```

#### 基本块维护器

在```BlockMaintainer```中，需要存储当前正在构建的函数、基本块、指令缓冲区等Koopa结构信息。由于其为静态成员，任意AST节点都可以调用他来添加基本块、语句等。

当一个基本块完成时，该类的```FinishCurrentBlock```函数负责清楚基本块末尾无法到达的语句，以及添加Jump、Return之类的语句防止基本块出错。

```c++
class BlockMaintainer
{
    koopa_raw_function_t current_func;
    std::vector<const void *> current_insts_buf;
    std::vector<const void *> *basic_block_buf;
public:
    void SetCurrentFunction(koopa_raw_function_t _cur_func);
    void SetBasicBlockBuf(std::vector<const void *> *_basic_block_buf);
    void FinishCurrentBlock();
    void AddNewBasicBlock(koopa_raw_basic_block_data_t *basic_block);
    void AddInst(const void *inst);
};
```

#### 循环维护器

嵌套的循环结构会造成大量基本块，出现复杂的跳转结构，为此实现了一个类来维护。

该类使用一个vector来构建一个栈，用于表示嵌套的循环结构。栈中元素存储对应循环的各个基本块信息，方便AST节点构建Koopa时使用。

```c++
struct KoopaWhile
{
    koopa_raw_basic_block_data_t *while_entry;
    koopa_raw_basic_block_data_t *while_body;
    koopa_raw_basic_block_data_t *end_block;
};
class LoopMaintainer
{
    std::vector<KoopaWhile> loop_stk;
public:
    void AddLoop(...)
    {
        KoopaWhile kw;
        // ... 构建kw
        loop_stk.push_back(kw);
    }
    KoopaWhile GetLoop() { return loop_stk[loop_stk.size() - 1]; }
    void PopLoop() { loop_stk.pop_back(); }
};
```

#### RISC-V构建器 (builder)

RISC-V构建器中包含一个输出流```output```，通过```build```函数递归遍历Koopa结构，生成的RISC-V代码直接输出到output中。

构建器中包含一个```Env```结构，用于存储Koopa指令到int的映射，代表该指令的计算结果存储在栈上的位置，RISC-V找到该结构并读取。

```c++
class RISCVBuilder
{
    class Env
    {
        int total_size;    
        map<koopa_raw_value_t, int> addr;
    public:
        int cur;
        bool has_call;
        void NewEnv(int size, bool _has_call);
        int GetTotalSize();
        int GetAddr(koopa_raw_value_t kval);
    } env;
    ostream &output;
    
    static int calc_func_size(koopa_raw_function_t kfunc, bool &has_call);
    // ... some functions to calculate memory
    static int calc_type_size(koopa_raw_type_t ty);
    
    void traversal_raw_slice(const koopa_raw_slice_t *rs);
    // ... some functions for every instructions
    void gen_riscv_value_return(const koopa_raw_return_t *kret);
public:
    RISCVBuilder(ostream &_out) : output(_out) {}
    void build(const koopa_raw_program_t *raw);
};
```

### 2.3 主要设计考虑及算法选择

#### 2.3.1 符号表的设计考虑

符号表类里面定义了一个vector实现的栈，表示作用域的嵌套结构，新建作用域时在栈顶添加元素。

栈中每个元素为一个map，将字符串映射到Koopa结构，表示该作用域的符号表。

当需要查找符号时，从栈顶开始在map中查找，若没有找到，则进入栈中前一个元素继续查找，实现往父作用域的查找。

```c++
struct LValSymbol
{
    enum SymbolType type;
    void *number;
};
class SymbolList
{
    std::vector<std::map<std::string, LValSymbol>> sym_stk;
public:
    void NewEnv();
    void AddSymbol(const std::string &name, LValSymbol koopa_item);
    LValSymbol GetSymbol(const std::string &name)
    {
        LValSymbol res;
        for (size_t i = sym_stk.size() - 1; i >= 0; i--)
            if (sym_stk[i].count(name) != 0)
            {
                res = sym_stk[i][name];
                break;
            }
        return res;
    }
    void DeleteEnv();
};
```

#### 2.3.2 寄存器分配策略
寄存器分配采用全部放在栈上的策略，仅在一些运算中途会使用寄存器。

首先Builder进入函数时，计算所有语句以及变量等占用的空间大小，在栈上分配足够的空间。

每个语句确保自己执行的内部寄存器不会冲突，执行完毕将语句的结构存在栈上指定位置，寄存器信息不再保存，可被其他语句复用。

每个语句仅使用t0~t6寄存器，函数参数即返回值使用a0~a7。

#### 2.3.3 其它补充设计考虑

##### 优化语法分析中的列表

在语法分析器中，针对列表状的语句。=，如```int x0,x1,x2....```任意多个变量。

在yacc中设计为如下形式：

```c
VarDecl : BType VarDefList ';';
VarDefList : VarDef | VarDefList ',' VarDef;
VarDef : ..... {add_inst(...);}
```

并在全局开一个vector

对于VarDecl，VarDefList不再定义语法分析树节点，而是仅将VarDef生成的语法分析树节点添加到全局的vector中。然后上一级AST节点读取全局vector，将子节点添加进自己的结构。

这样设计有效简化了复杂的AST链状结构。

## 三、编译器实现

### 3.1 各阶段编码细节

[lab文档](https://pku-minic.github.io/online-doc/#/)

#### Lv1. main函数和Lv2. 初试目标代码生成

该阶段仅仅按照文档完成简易功能。构建了AST的类结构，选择了直接生成Koopa结构内存形式的方式。

使用简单函数生成RISC-V，没有构建其他复杂的数据结构。

#### Lv3. 表达式
按照文档的语法规范，对每种表达式设计AST类。

由于Koopa没有逻辑与/或，只有按位与/或，所以这两个运算之前，添加不等于运算来将结果转化为1和0，再进行按位运算。

对于RISC-V代码，此时设计了栈的大小计算，将koopa语句与栈上地址映射，设计对应的类。

Koopa格式已经是简单的三地址语句，只需要读取栈上数据，使用固定语句计算，再存回栈上即可。

#### Lv4. 常量和变量
对于常量，在表达式的AST中添加了CalcValue功能用于求值。

因为变量的出现，完成了SymbolList类，并将其设为静态，作为全局信息管理。

由于之前把所有AST都视为BaseAST，使用多态调用build_koopa_value。在这里LValAST有时需要右值形态的Koopa，有时需要左值形态的Koopa，无法使用一个函数的多态来完成，出现了一定困难。于是在BaseAST中添加了新的虚函数用于处理左值。

对于RISC-V代码，仅在栈空间计算以及koopa与地址映射中添加了对变量的处理。

#### Lv5. 语句块和作用域
和Lv4一起完成的，在设计SymbolList时已经实现为栈结构，能够处理嵌套的作用域。

在语法分析阶段，建立了全局的语句缓存区栈

栈中每个元素为一个vector，存储该语句块中的语句

到了需要存储语句的AST时，从栈顶开始读取语句块内容，将其存入自己的结构中。

#### Lv6. if语句
出现二义性时，我将```IF exp ```合并为```IfExp : IF exp```，再将if语句定义为

```
Stmt : IfExp Stmt | IfExp Stmt ELSE {...} Stmt {...};
```

解决了问题。

对于If中的几个语句块，他们都存在了全局语句栈中。读取栈顶的两项即可获得If的语句块和Else的语句块。

由于基本块越来越多，在这个阶段设计了BlockMaintainer来专门维护各个基本块的语句缓存。

对于短路求值，直接将&&，||改为跳转形式，而不再使用按位与等运算。

#### Lv7. while语句
因为嵌套的循环语句不方便找到需要跳转的块，设计了LoopMaintainer类来维护嵌套的循环，使用栈结构来存储每个循环的基本块信息。

这样break，continue能找到当前所在的循环，从而找到跳转位置。

这些跳转语句，直接对应RISC-V中的bnez和j命令，比较简单。

由于Koopa库能够自动处理名字重复问题，通过生成Koopa代码再重新读入回来，已经不存在标签重名的情况。

#### Lv8. 函数和全局变量
对于形参和实参都设计了专门的AST类实现，同时在语法分析阶段开了全局vector用来存储这些参数列表。

在函数定义FuncDefAST中读取列表构建AST，函数调用LVal中读取列表生成参数。

对于RISC-V阶段，增加了计算函数返回地址，传参在栈上存储的位置。以及调用函数将参数放入指定寄存器或栈上位置的功能。

#### Lv9. 数组
设计InitValAST专门用于处理数组的初始化列表，并且分开处理了全局变量和局部变量的形式。

InitValAST首先调用预处理preprocess，根据指定的数组size将数据预处理成数组格式的数据。

然后根据是全局还是局部变量，生成aggerate的koopa内容或者读取某一位的值，在函数内部通过赋值初始化。

对于RISC-V，由于之前将Koopa对象映射到栈上位置，用的一直是Koopa语句对象本身的指针，他的ty表示的一定是一个整数指针（koopa语句返回值都是指针）。而目前因为数组参数的出现，出现了真正的指针，导致映射器无法区分是要读取指针本身还是指针指向的对象，于是只能对get_element_ptr和get_ptr进行特判处理。

get_element_ptr等语句，处理时会读取koopa对象中的ty以计算指针需要移动的大小，使用mul，add等语句计算得到指定位置的指针。

### 3.2 工具软件介绍
1. `flex/bison`：编写```sysy.l```和```sysy.y```，完成词法分析和语法分析，通过表达式识别代码生成了语法分析树。
2. [libkoopa](https://github.com/pku-minic/koopa)：使用该库的Koopa各种结构体，直接在AST中构建了Koopa结构的内存形式。使用该库生成Koopa代码。该库生成的代码自动完成了各种名称的去重，于是通过该库的生成再读取得到一个名字没有重复的Koopa程序，方便RISC-V的生成。

### 3.3 测试情况说明

使用了往年的测试数据，测试某些编译器崩溃的问题，有以下重点问题：

1. 对于int类型函数，如果执行结束没有return，需要自己添加return 0命令；
2. 对于极长的代码，会超出```bnez```的跳转范围，必须修改为跳转到近处某个位置，使用```j```命令跳转到远处。
3. 发现把```new char[len]```写成了```new char(len)```，到处内存非法访问，到处出错。。。

## 四、实习总结

### 4.1 收获和体会

实现了一个编译器，非常有成就感。

加深理解了这门课程各个部分的内容。

通过实践学到了大软件开发的一些难点，最初的设计会如何影响到后续的扩展问题。

### 4.2 学习过程中的难点，以及对实习过程和内容的建议

1. Koopa库的文档不足，无法准确理解koopa中每个结构体的意思，需要自己去猜或试出来，建议助教完善一些；
2. 有些常见错误比较难考虑到，最好在文档里添加：比如int类型函数没写返回需要自己添加return 0

### 4.3 对老师讲解内容与方式的建议

希望老师上课能讲得更有趣一些
