目标：写一个epoll reactor http kv存储项目；
模块：已经完成：epoll tcp connect_pool (protocol_prase（已有http解析器，差kv存储的解析器） prorocol_factory（创建解析器）) 
未完成：线程池，日志（事件轮或者最小堆实现定时器)，内存池，

5月19日：今日完成connect_pool  http_prase 
        问题：1，头文件重复包含 ，前置声明或者。cppp文件包含
        2，独联体union（ void*  data*) 可以用作空闲链表（内存池的分配）还有 char a[] 柔性数组，不定长块分配；
        3 new 定位用法（用来处理话malloc已经分配过的空间）  new (target)(构造函数） ，这样可以避免隐式转换使对象乱码。
        4 模板 头文件和实现文件不可以分离，虚函数和继承可以；
        5 单例模式，创建唯一类单例；
