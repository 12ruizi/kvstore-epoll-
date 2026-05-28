目标：写一个epoll reactor http kv存储项目；
模块：已经完成：epoll tcp connect_pool (protocol_prase（已有http解析器，差kv存储的解析器） prorocol_factory（创建解析器）) 
未完成：线程池，日志（事件轮或者最小堆实现定时器)，内存池，

5月19日：今日完成connect_pool  http_prase 
        问题：1，头文件重复包含 ，前置声明或者。cppp文件包含
        2，独联体union（ void*  data*) 可以用作空闲链表（内存池的分配）还有 char a[] 柔性数组，不定长块分配；
        3 new 定位用法（用来处理话malloc已经分配过的空间）  new (target)(构造函数） ，这样可以避免隐式转换使对象乱码。
        4 模板 头文件和实现文件不可以分离，虚函数和继承可以；
        5 单例模式，创建唯一类单例；

5月28日：完成工作流
        问题：1，宏定义要和宏分开，否则会重复
              2，find（string，ptr)不行 ，因为ptr会一直到\0 ,(之前用的string——view比较）

-----------跑通了整个工作流，明日任务1：打包任务成lambda 让线程池完成
                                    2：kv任务的处理机制实现 
                                    3：log 和定时任务的实现（底层涉及到定时器，跳表，hash，红黑树，堆结构）
                                    4：mysql的连接


              
