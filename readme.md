## poorjson 是一个用C++实现的json共享库


为什么叫poorjson，上高中英语课的时候，同学和我说“You are so poor in English.”，我现在也“so poor in C++”，这个json库真的很poor。

### json库的功能如下：

1. 把 JSON 文本解析为一个树状数据结构（parse）。
2. 提供接口访问该数据结构（access）。
3. 把数据结构转换成 JSON 文本（stringify）。

### 使用步骤：

#### 共享库制作：

1. cd poorjson
2. cmake .
3. make
4. sudo cp libpoorjson.so /usr/lib    ##库文件拷贝

#### 共享库调用：

1. cd poorjson_test
2. make .
3. make


调用共享库的时候，需要注意  using namespace poorjson;    一定要在 #include 下面。



### 涉及的知识如下：

1. 强类型枚举的使用。

   ~~~
       enum class json_type
       {
           NULL1,
           FALSE,
           TRUE,
           NUMBER,
           STRING,
           ARRAY,
           OBJECT
       }; //枚举类替换枚举
   ~~~
   枚举类型不是类型安全的，枚举类型被视为整数，这使得两种不同的枚举类型之间可以进行比较，枚举的名称全数暴露于一般范围中，因此C++03两个不同的枚举，不可以有相同的枚举名。

2. 
   不可能将强类型的枚举隐式转换为整数，必须使用static_cast（强迫隐式转换 implicit conversions ）明确显示转换，《effective C++》条款27：尽量少做转型动作中，“宁可使用C++ -style（新式）转型，不要使用旧式转型。”
   ~~~
   /*把枚举类的OK转为int*/
   static_cast<int>(return_json::OK)
   ~~~
   新式转型老长了，一眼就看得见，容易找到问题，同时可以有针对性的转型可供使用。

3. 
   部分宏用内联函数代替，《effective C++》条款2：尽可能以const，enum，inline替换#define，宏中会出现一些“无聊的事情”，比如实参的小括号之类，template inline函数可以解决这个问题。
   ~~~
       template<typename T>
       inline bool ISDIGIT(const T& ch)        
       {
           return ((ch) >= '0' && (ch) <= '9');
       }
   ~~~
   程序中还留着的宏， 比如`__LINE__` 这个编译器提供的宏，代表编译时该行的行号。如果用内联函数，每次的行号便都会相同，不方便调试。对于宏中会出现一些“无聊的事情”，参考 “leptjson”中宏的编写技巧 来避免。

4. 
   右值引用，临时变量提高效率。《effective C++》条款20，宁以 pass-by-reference-to-const 替换 pass-by-value，by-value是C++继承C的方式，缺省情况下by-value，效率较低。const是必要的，不这样做可能会被改变，VScode下不加const会报错：非常量引用必须是左值。

5. 
   断言的使用，当程序以 release 配置编译时，`assert()` 不会做检测；而当在 debug 配置时，则会在运行时检测 `assert(cond)` 中的条件是否为真（非 0），断言失败会直接令程序崩溃，利于程序调试。

6. 命名空间namespace的使用。命名空间namespace和枚举类的使用可以在定义变量时更加“嚣张”，毕竟要使用枚举类的元素时还要加上enum class的名称::。
   
6. 在生成json时，由于已经被声明为const，再赋值给它会出错，用const_cast可以实现。
   
   ```
   /*const_cast 将对象的常量性移除，唯一有能力的C++-style转型操作符*/
   c.json =const_cast<char*>(c.json); 
   ```
   
   string是可变长的，用于临时储存json生成的结果，C++11以后，标准库提供了std::to_string辅助函数转化各类型为一个字符串，
   
   ```
   case (json_type::NUMBER): s=to_string(v->n); break; 
   ```
   
   同样，要把*string 转为char*  * 赋值给json，可自定义函数 char*  strTochar(string s) 实现。但是malloc 的内存没有free，会导致内存泄露。
   
   ```
   char* strTochar(string s){
       char *data;
       int len = s.size();
       data = (char *)malloc(len+1);
       s.copy(data,len,0);
       return data;
   }
   ```
   
   事实上可用c_str() 将*string 转为char*  * 。
   
   ```
   const char* c_str() const noexcept;
   ```
   
   用  valgrind --leak-check=full ./test 检测无内存泄露
   
   ==8760== All heap blocks were freed -- no leaks are possible
   
8. cmake共享库的编写和使用，简单的CMakeLists.txt编写使用。

   ```
   #共享库的CMakeLists.txt
   cmake_minimum_required (VERSION 3.10)
   project (poorjson)
   
   #C++11 编译
   set(CMAKE_CXX_STANDARD 11)
   
   add_library(poorjson SHARED poorjson.cpp poorjson.h )
   ```

   

   ```
   #调用共享库的test的CMakeLists.txt
   cmake_minimum_required(VERSION 3.10)
   
   #C++11 编译
   set(CMAKE_CXX_STANDARD 11)
   
   set(TEST_SRC test.cpp)
   ADD_EXECUTABLE(test ${TEST_SRC})
   
   #链接 MySharedLib 库
   target_link_libraries(test poorjson)
   ```

   

### json库的实现参考son-tutorial，链接如下：

https://github.com/miloyip/json-tutorial

