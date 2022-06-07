### 1. 类视图

![](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220607142418941.png)

### 2. 主界面及各类图形的绘制

![](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220607161506323.png)



![image-20220607161601405](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220607161601405.png)

### 3. Bresenham直线绘制算法[^Bresenham]

本文的直线绘制没有直接调用drawline，而是实现了Bresenham直线绘制算法。基于该算法，又实现了多边形的填充。下面给出Bresenham的伪代码。这里仅处理了 $x_0<x_1, 0 < slope < 1$的情况，其他象限可以转换到该象限处理。

![image-20220607163630277](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220607163630277.png)



### 参考文献

[^Bresenham]: https://www.cs.montana.edu/courses/spring2009/425/dslectures/Bresenham.pdf

