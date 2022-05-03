### 1. 类视图

![image-20220501183150247](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220501183150247.png)

### 2. 软件界面

![image-20220501185836899](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220501185836899.png)

打开图像后，点击SetAnchor开始设置锚点，设置过程中点击右键可取消上一个锚点，Restore恢复图像，同时清除所有锚点.

### 3. 变形方法

#### 3.1 IDW方法 [^IDW]

局部插值函数 $\mathbf{f} _ i(\mathbf{x}):\mathbb{R}^2\to\mathbb{R}^2$ 满足 $f _ i(\mathbf{p} _ i)=\mathbf{q} _ i$，具体为

$$
\mathbf{f} _ i(\mathbf{x})=\mathbf{q} _ i+\mathbf{D} _ i(\mathbf{x}-\mathbf{p} _ i)
$$
其中 $\mathbf{D} _ i:\mathbb{R}^2\to\mathbb{R}^2$，满足 $\mathbf{D} _ i(\mathbf{0})=\mathbf{0}$ ，可以取 $\mathbf{D} _ i$ 为线性变换，插值函数为

$$
\mathbf{f}(\mathbf{x})=\sum _ {i=1}^n w _ i(\mathbf{x})\mathbf{f} _ i(\mathbf{x})
$$

其中 $w _ i:\mathbb{R}^2\to\mathbb{R}$，为

$$
w _ i(\mathbf{x})=\frac{\sigma _ i(\mathbf{x})}{\sum _ {j=1}^n \sigma _ j(\mathbf{x})}
$$

$$
\sigma _ i(\mathbf{x})=\frac{1}{\Vert\mathbf{x}-\mathbf{x} _ i\Vert^\mu}
$$

其中 $\mu>1$ ，通常取$\mu=1$ 。显然 $0\le w _ i(\pmb{x})\le 1$，且 $\sum _ {i=1}^n w _ i(\mathbf{x})=1$ ，

简单地，可直接取 $\mathbf{D} _ i=\mathbf{I}$，此时 $\mathbf{f}(\mathbf{x})=\sum _ {i=1}^n w _ i(\mathbf{x})(\mathbf{q} _ i+\mathbf{x}-\mathbf{p}_ i)$ ，这时已经能够达到很好的效果了。

![image-20220503095941798](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220503095941798.png)

​	也可以定义能量

$$
\begin{aligned}
E _ i(\mathbf{D} _ i)
=&\sum _ {j=1,j\neq i}^n \sigma _ i(\mathbf{p}_ j)\left\Vert\mathbf{q} _ i+\left(\begin{array}{c}d _ {i,11} & d _ {i,12}\newline d _ {i,21} & d _ {i,22}\end{array}\right)(\mathbf{p} _ j-\mathbf{p} _ i)-\mathbf{q} _ j\right\Vert^2\newline
=&\sum _ {j=1,j\neq i}^n \sigma _ i(\mathbf{p}_ j)(
(d _ {i,11}(p _ {j,1}-p _ {i,1})+d _ {i,12}(p _ {j,2}-p _ {i,2})+q _ {i,1}-q _ {j,1})^2+\newline
&(d _ {i,21}(p _ {j,1}-p _ {i,1})+d _ {i,22}(p _ {j,2}-p _ {i,2})+q _ {i,2}-q _ {j,2})^2)
\end{aligned}
$$

最小化该能量可求得 $\mathbf{D} _ i=\left(\begin{array}{c}d _ {i,11} & d _ {i,12}\newline d _ {i,21} & d _ {i,22}\end{array}\right)$ ，把${E} _i$看成是$d _ {i,11},d _ {i,12},d _ {i,21},d _ {i,22}$的多元函数，分别对各分量求导后，令偏导数等于0，求得$\mathbf{D} _ i$.

![image-20220503095830237](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220503095830237.png)

与直接取$\mathbf{D} _ i=\mathbf{I}$的对比，最小化能量的方法会尽可能保持变形的对称性，这一点在设置非对称的锚点时很明显。

### 3.2 RBF方法 [^RBF]

给定 $n$ 对控制点 $(\mathbf{p} _ i,\mathbf{q _ i})$，$\mathbf{p} _ i,\mathbf{q} _ i\in\mathbb{R}^2$，$i=1,\dots,n$ 

插值函数

$$
\pmb{f}(\pmb{p})=\sum _ {i=1}^n \boldsymbol{\alpha} _ i R(\Vert\mathbf{p}-\mathbf{p} _ i\Vert)+A\mathbf{p}+\mathbf{b}
$$

其中权重系数 $\boldsymbol{\alpha} _ i\in\mathbb{R}^2$，$A\in\mathbb{R}^{2\times 2}$，$\mathbf{b}\in\mathbb{R}^2$，径向基函数 $R(d)=(d^2+r^2)^{\mu/2}$ ,此处$r$,$\mu$为常数，取$r=50$，$\mu=-1$.

要求满足插值条件

$$
\mathbf{f}(\mathbf{p} _ j)=\sum _ {i=1}^n\boldsymbol{\alpha} _ i R(\Vert\mathbf{p} _ j-\mathbf{p} _ i\Vert)+A\mathbf{p} _ j+\mathbf{b}=\mathbf{q} _ j,\quad j=1,\dots,n
$$

自由度每维有 $n+3$ 个

可选的补充约束为

$$
\left[\begin{array}{c}
\mathbf{p} _ 1 & \dots & \mathbf{p} _ n\newline
1            & \dots & 1
\end{array}\right] _ {3\times n}
\left[\begin{array}{c}
\boldsymbol{\alpha} _ 1 \newline
\vdots \newline
\boldsymbol{\alpha} _ n
\end{array}\right] _ {n\times2}
=\mathbf{0} _ {3\times 2}
$$
记$R_{ij}=R(\Vert\mathbf{p} _ i-\mathbf{p} _ j\Vert)$,则有
$$
\left[\begin{array}{c}
{R}_{11} &{R}_{12} &\cdots &{R}_{1n} &{p}_{11} &{p} _{12} &1\newline
{R}_{21} &{R}_{22} &\cdots &{R}_{2n} &{p}_{21} &{p} _{22} &1\newline
\vdots &\vdots &\cdots &\vdots &\vdots &\vdots &\vdots\newline
{R}_{n1} &{R}_{n2} &\cdots &{R}_{nn} &{p}_{n1} &{p} _{n2} &1\newline
{p}_{11} &{p}_{21} &\cdots &{p}_{n1} &0 &0 &0\newline
{p}_{12} &{p}_{22} &\cdots &{p}_{n2} &0 &0 &0\newline
1 &1 &\cdots &1 &0 &0 &0\newline
\end{array}\right] _ {(n+3)\times (n+3)}
\left[\begin{array}{c}
{\alpha} _ {11} &{\alpha} _ {12}\newline
{\alpha} _ {21} &{\alpha} _ {22}\newline
\vdots &\vdots\newline
{\alpha} _ {n1} &{\alpha} _ {n2}\newline
A_{11} &A_{21}\newline
A_{12} &A_{22}\newline
b_1 &b_2\newline
\end{array}\right] _ {(n+3)\times2}
=\left[\begin{array}{c}
q_{11} &q_{12}\newline
q_{21} &q_{22}\newline
\vdots &\vdots\newline
q_{n1} &q_{n2}\newline
0 &0\newline
0 &0\newline
0 &0\newline
\end{array}\right] _ {(n+3)\times2}
$$
求解出中间矩阵(由$\boldsymbol{\alpha}$,$\mathbf{A}$,$\mathbf{b}$构成)，对由图片上每个像素点位置，计算相应的$\left[\begin{array}{c}
{R}_{x1} &{R}_{x2} &\cdots &{R}_{xn} &{p}_{x1} &{p} _{x2} &1\newline
\end{array}\right]$即可,

其中$R_{xi}=R(\Vert \mathbf{x}-\mathbf{p} _ i\Vert)$.

![image-20220503114959111](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220503114959111.png)

可以看出RBF方法和最小化能量的IDW方法效果几乎相同。

### 3.3 白缝

无论IDW还是RBF变形方法都可能产生白缝，这里利用白缝周边已填充点的像素值平均来进行填充，只会产生轻微的模糊，效果很好。

### 参考文献

[^IDW]: Ruprecht D, Muller H. [**Image warping with scattered data interpolation**](https://pdfs.semanticscholar.org/5a9e/2604064d08f2a8b7dcef4cd4e9a2ce2a88c2.pdf)[J]. IEEE Computer Graphics and Applications, 1995, 15(2): 37-43.

[^RBF]: Arad N, Reisfeld D. [**Image warping using few anchor points and radial functions**](https://pdfs.semanticscholar.org/5a9e/2604064d08f2a8b7dcef4cd4e9a2ce2a88c2.pdf )[C]//Computer graphics forum. Edinburgh, UK: Blackwell Science Ltd, 1995, 14(1): 35-46.

