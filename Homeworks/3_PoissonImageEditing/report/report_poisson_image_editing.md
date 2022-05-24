### 1. 类视图

![](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220524130039284.png)

### 2. 软件界面

![](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220524131832575.png)

打开图像后，先选中要复制的源图像，再点击RectChoose/PolygonChoose，在源图像中选择相应的区域后选中目标图像，点击Clone/MixingClone实现无缝克隆.

### 3. 无缝克隆[^Poisson Image Editing]

#### 3.1 引导插值

引导插值是指根据给定的向量场 $\mathbf{v}$，求解目标函数$f^*$在区域${\Omega}$上的插值函数$f$。又由于对于每个颜色通道，求解过程是独立的，所以只需要考虑$f$是标量的情况。现在通过求解如下变分问题
$$
\mathop{min}\limits_f\iint_{\Omega}{|{\nabla}f-\mathbf{v}|}^2, f|_{\partial{\Omega}}=f^*|_{\partial{\Omega}} \quad\quad (1)
$$


得到$f$。上述公式中的符号参照下图

![](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220524150621615.png)



$S$是图像的定义域，它是$\R^2$中的闭集；$\Omega$是$S$的闭子集，边界记作${\partial{\Omega}}$；$f^*:$定义在$S$\\${\Omega}$的已知标量函数；$f:$定义在${\Omega}$的待求标量函数；$\mathbf{v}$是定义在${\Omega}$上的的向量场，它可以取源函数$g$的梯度场。

公式$(1)$等价于在${\Omega}$上成立泊松方程
$$
\Delta f= div \mathbf{v}, 且满足Dilichlet边界条件f|_{\partial{\Omega}}=f^*|_{\partial{\Omega}}
$$

其中 $div \mathbf{v}=\frac{\partial u}{\partial x}+\frac{\partial v}{\partial x}$是$\mathbf{v}=(u,v)$的散度。

#### 3.2 离散泊松求解器 

不失一般性，这部分的讨论将沿用连续情况的记号：$S$，${\Omega}$是定义在无限离散网格上的有限点集，对${\forall}p{\in}S$，记$N_p$是它的上，下，左，右与$S$的交组成的集合，当$q{\in}N_p$时，记$<p,q>$，${\Omega}$的边界$\partial \Omega$ = { ${p{\in}S}$\\$\Omega$:$N_p{\cap}\Omega\neq\emptyset$ }，$f_p$表示$f$在$p$点的值。目标是求解$f|_\Omega$ = { $f_p,p\in\Omega$ }。

由于Dirichlet边界条件中的边界是任意形状，最好是直接离散化变分问题$(1)$，$(1)$ 的有限差分离散化产生以下离散的二次优化问题：
$$
\mathop{min}\limits_{f|_\Omega}\sum _ {<p,q>{\cap}\Omega\neq\emptyset}(f_p -f_q-v_{pq})^2,且满足f_p=f_p^*,\forall p{\in}{\partial\Omega}
$$

等价于
$$
|N_p|f_p-\sum _ {q\in N_p{\cap}\Omega}f_q = \sum _ {q\in N_p{\cap}\partial\Omega}f_q^*+\sum _{q\in N_p}v_{pq},\forall p{\in}{\Omega}\quad\quad (2)
$$
通常选源图像的梯度场作为$\mathbf v，即$$\mathbf{v}=\nabla g$，于是
$$
v_{pq} = g_p-g_q,对所有的<p,q>\quad\quad (3)
$$
将$(3)$带入$(2)$得到一个典型的稀疏（带状）、对称正定系统。对红，绿，蓝三个颜色通道分别求解该稀疏矩阵得到克隆图像。注意求解出来的数值可能不在0~255之间，需要修正。

![clone](F:\CG\USTC_CG\Homeworks\3_PoissonImageEditing\project\bin\clone.bmp)

#### 3.3 混合梯度 

上述的克隆方法使得目标图像在区域$\Omega$中的信息全部丢失了。如下图的场景，![image-20220524180152944](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220524180152944.png)

当源图像中有孔，或者我们希望保持目标图像的纹理时，混合梯度是比较好的选择。混合梯度是指在$\Omega$中的每一点处选择$f^*$和$g$中梯度变化较强的那个作为$\mathbf v$，即
$$
\mathbf v(\mathbf x)= \begin{cases} 
\nabla f^*(\mathbf x),  & \text{if }|\nabla f^*(\mathbf x)|>|\nabla g(\mathbf x)| \\
\nabla g(\mathbf x), & otherwise
\end{cases}
$$
混合梯度后的图像

![image-20220524182243113](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220524182243113.png)

#### 3.4 扫描线算法 

这里的扫描线算法，先用Bresenham划直线的算法找到多边形每条边上的点，再根据y坐标从低到高找出多边形内点，并将内点的坐标作为字典的key值（将x坐标放在高16位，y坐标放在低16位），在对应稀疏矩阵中未知量的列值作为value实现的。

![image-20220524183207479](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220524183207479.png)

### 3.3 白缝

#### 3.4 预处理

从公式$(2)$得知,稀疏矩阵由源图像中决定，所以可以先预处理该矩阵（比较费时间），当在目标图像中粘贴时，随着鼠标移动动态生成方程的右值，再求解出发$f$，绘制出来，达到实时刷新的效果。

#### 3.5 内存泄漏误报

添加Opencv的库文件，`CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF)`会误报内存泄漏

### 参考文献

[^Poisson Image Editin]: Pérez P, Gangnet M, Blake A. [**Poisson image editing**](https://www.cs.jhu.edu/~misha/Fall07/Papers/Perez03.pdf), ACM Transactions on Graphics (Proc. SIGGRAPH), 22(3): 313-318, 2003.

