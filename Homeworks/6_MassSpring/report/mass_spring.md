### 1. 欧拉半隐式

#### 1.1伪代码

```c++
/*Algorithm [Spring-Mass System Timestep Loop]*/
//参数初始化
k_s = 8; 				//设置劲度系数，布料约8
k_d = 0.5;   			//设置阻尼系数，布料约0.5
gravity = [0,-9.8,0];	//空间中的力，形式为三维向量，重力加速度g=9.8m/s²

//计算每个质点所受的外力（这里只有重力）
for each Particle p: particles do
	p.frc = 0;					//frc为质点所受力 
    p.frc += p.mass * gravity; 	//p.mass设置为1
end for

//计算每个质点所受的内力（弹力和阻尼）从而求得质点所受合力
for all each Spring s(i,j): springs do
	d = j.pos - i.pos;			//计算弹簧方向 
    len = d.norm;				//计算弹簧长度
	v = j.vel - i.vel;
	f = (k_s*(len/len0 - 1) + k_d*(v/len0).dot(d/len))*d/len	//计算质点所受内力，其中len0为弹簧原长
    i.frc += f;
	k.frc -= f; 
end for

//欧拉半隐式求解
for each Particle p: particles do 
    p.vel += dt * (p.frc/p.mass); 
    p.pos += dt * p.vel;
end for 
```

#### 1.2 仿真结果

步长取0.002，劲度系数取100时，固定上边界，网格在重力作用下向下运动，产生很大形变，

![stiff=100EulorSemiImplicit(12fps)](F:\CG\USTC_CG\Homeworks\6_MassSpring\report\mass_spring.assets\stiff=100EulorSemiImplicit(12fps).gif)

而且从网格形变可以看出，左边形变大于右侧形变，这是由于网格剖分非对称导致的。

尝试增大劲度系数到10000，使其运动起来像布料

![stiff=10000_EulorSemiImplicit(12fps)](F:\CG\USTC_CG\Homeworks\6_MassSpring\report\mass_spring.assets\stiff=10000_EulorSemiImplicit(12fps).gif)

当步长取0.033时，仿真总是发散，欧拉显示方法简单，能带来仿真的实时性，但稳定性差。

### 2.欧拉隐式

#### 2.1原理

问题：由前 $i$ 帧信息，求得第 $i+1$ 帧信息（位移 $\boldsymbol x$，速度 $\boldsymbol v$）（设时间步长为 $h$）？
$$
\boldsymbol x_{i+1}=\boldsymbol x_i+h\boldsymbol v_{i+1},\\
 \boldsymbol v_{i+1}=\boldsymbol v_i+h\boldsymbol M^{-1}(\boldsymbol f_{int}(t_{i+1}) +\boldsymbol f_{ext}),
$$
记
$$
\boldsymbol y =\boldsymbol x_i + h\boldsymbol v_i + h^2\boldsymbol M^{-1}\boldsymbol f_{ext}, \tag{*}
$$
显然$\boldsymbol y$由第$i$帧数据决定，是个已知量。原问题转化为求解关于$\boldsymbol x$的方程：
$$
\boldsymbol g(\boldsymbol x) = \boldsymbol M(\boldsymbol x-\boldsymbol y) -h^2\boldsymbol f_{int}(\boldsymbol x) = 0,
$$
利用牛顿法求解该方程，主要迭代步骤：
$$
\boldsymbol x^{(k+1)}=\boldsymbol x^{(k)}-(\nabla \boldsymbol g(\boldsymbol x^{(k)}))^{-1}\boldsymbol g(\boldsymbol x^{(k)}).
$$

迭代初值可选为$\boldsymbol x^{(0)}=\boldsymbol y$ ,对有n个质点网格,将坐标列为列向量, $\boldsymbol x\in R^{3n}$.

迭代得到位移$x$后更新速度$v_{n+1}=(x_{n+1}-x_{n})/h$

上式中涉及关于弹力的求导，对于单个弹簧（端点为$\boldsymbol  x_1$，$\boldsymbol  x_2$），劲度系数为$k$，原长为$l$，有：
$$
\boldsymbol x_1所受弹力：     \boldsymbol f_1(\boldsymbol x_1,\boldsymbol x_2)=k(||\boldsymbol x_1-\boldsymbol x_2||-l)\frac{\boldsymbol x_2-\boldsymbol x_1}{||\boldsymbol x_1-\boldsymbol x_2||},\\
\boldsymbol x_2所受弹力：     \boldsymbol f_2(\boldsymbol x_1,\boldsymbol x_2)=-\boldsymbol f_1(\boldsymbol x_1,\boldsymbol x_2),
$$
对
$$
\boldsymbol h(\boldsymbol x)=k(||\boldsymbol x||-l)\frac{-\boldsymbol x}{||\boldsymbol x||},
$$
求导有
$$
\frac{ d  \boldsymbol h}{d \boldsymbol x} = k(\frac{l}{||\boldsymbol x||}-1)\boldsymbol I-kl||\boldsymbol x||^{-3}\boldsymbol x \boldsymbol x^T.
$$
带入弹力公式得：
$$
\frac{\partial  \boldsymbol f_1}{\partial \boldsymbol x_1} =\frac{\partial  \boldsymbol h(\boldsymbol x_1-\boldsymbol x_2)}{\partial \boldsymbol x_1}=k(\frac{l}{||\boldsymbol r||}-1)\boldsymbol I-kl||\boldsymbol r||^{-3}\boldsymbol r \boldsymbol r^T,其中\boldsymbol r=\boldsymbol x_1-\boldsymbol x_2, \boldsymbol I为单位阵\\
$$


$$
\frac{\partial  \boldsymbol f_1}{\partial \boldsymbol x_2}=-\frac{\partial  \boldsymbol f_1}{\partial \boldsymbol x_1},
\frac{\partial  \boldsymbol f_2}{\partial \boldsymbol x_1}=-\frac{\partial  \boldsymbol f_1}{\partial \boldsymbol x_1},
\frac{\partial  \boldsymbol f_2}{\partial \boldsymbol x_2}=\frac{\partial  \boldsymbol f_1}{\partial \boldsymbol x_1},
$$

对所有弹簧求导并组装即可求得力的导数（组装为稀疏矩阵，矩阵为对称阵）。

#### 2.2 仿真结果

步长取0.033，劲度系数取100时，固定上边界，仿真收敛，且得到跟欧拉半隐式方法相同的结果，弊端也是显而易见的，出现了明显的卡顿，注意到$\nabla \boldsymbol g(\boldsymbol x^{(k)})$ 是系数矩阵，它求逆很快，可以断定延时主要是由于每次迭代都要重新生成$\boldsymbol g(\boldsymbol x^{(k)})和$$\nabla \boldsymbol g(\boldsymbol x^{(k)})$ ，这无法改善。另外，牛顿法迭代法在初始值选择不合理的情况下，偶尔也会发散。

![stiff=100_EulorImplicit(12fps)](F:\CG\USTC_CG\Homeworks\6_MassSpring\report\mass_spring.assets\stiff=100_EulorImplicit(12fps).gif)

另外，当网格规模较小时，仿真速度可以接受

![stiff=100_EulorImplicit_sparse(12fps)](F:\CG\USTC_CG\Homeworks\6_MassSpring\report\mass_spring.assets\stiff=100_EulorImplicit_sparse(12fps).gif)

### 3.加速方法（projective dynamic)[^Liu13]

#### 3.1原理

在上述欧拉方法中，对于内力（为保守力）有：
$$
\boldsymbol f_{int}(x)=-\nabla E(\boldsymbol x)
$$
故对方程$(*)$的求解可以转为为一个最小化问题：
$$
\boldsymbol x_{n+1}=arg \min\limits_{x}\frac{1}{2}(\boldsymbol x-\boldsymbol y)^T\boldsymbol M(\boldsymbol x-\boldsymbol y)+h^2E(\boldsymbol x)
$$
同时对于弹簧的弹性势能可以描述为一个最小化问题：
$$
\frac{1}{2}k(||\boldsymbol p_1-\boldsymbol p_2||-r)^2=\frac{1}{2}k \min\limits_{||\boldsymbol d||=r}||\boldsymbol p_1-\boldsymbol p_2-\boldsymbol d||^2,
$$
从而原问题转化为：
$$
\boldsymbol x_{n+1}=arg \min\limits_{\boldsymbol x,\boldsymbol d\in\boldsymbol U}\frac{1}{2}\boldsymbol x^T(\boldsymbol M+h^2\boldsymbol L)\boldsymbol x-h^2\boldsymbol x^T\boldsymbol J \boldsymbol d-\boldsymbol x^T \boldsymbol M \boldsymbol y
$$
其中
$$
\boldsymbol U= \{ \boldsymbol d=(\boldsymbol d_1,\boldsymbol d_2,...,\boldsymbol d_s),\boldsymbol d_s\in R^3,||\boldsymbol d_i||=l_i \} (l_i为第i个弹簧原长),
$$
![remark](F:\CG\USTC_CG\Homeworks\6_MassSpring\report\mass_spring.assets\remark.png)

显然利用矩阵分块乘法有$\boldsymbol L = ( \boldsymbol A * \boldsymbol D * \boldsymbol A^T) \bigotimes \boldsymbol  I_3$ ，其中$\boldsymbol A = \{ \boldsymbol A_1,\boldsymbol A_2,...,\boldsymbol A_s \}$，$\boldsymbol D =diag \{ k_1,k_2,...,k_s \}$,考虑到按照公式计算比较费时，可以直接构造出稀疏矩阵$\boldsymbol L$，同理$\boldsymbol J = (\boldsymbol A * \boldsymbol D * \boldsymbol S^T)\bigotimes \boldsymbol I_3$，$\boldsymbol S = \{ \boldsymbol S_1,\boldsymbol S_2,...,\boldsymbol S_s \} = I$,也可以直接构造得到。

从而可以对 $\boldsymbol x$，$\boldsymbol d$ 迭代优化求得该优化问题的解：
$$
\boldsymbol x 优化：求解方程(\boldsymbol M+h^2\boldsymbol L)\boldsymbol x=h^2\boldsymbol J \boldsymbol d+ \boldsymbol M \boldsymbol y（这里可以预分解矩阵），\\
$$

$$
\boldsymbol d 优化：\boldsymbol d_i=l_i\frac{\boldsymbol p_{i_1}-\boldsymbol p_{i_2}}{||\boldsymbol p_{i_1}-\boldsymbol p_{i_2}||}（这里l_i为第i个弹簧原长，\boldsymbol p_{i_1}，\boldsymbol p_{i_2}为其两端点），
$$

重复迭代过程直到收敛。

#### 3.2仿真结果

步长取0.033，劲度系数取100时，固定上边界，可以看到加速方法非常流畅

![stiff=100_Acc(12fps)](F:\CG\USTC_CG\Homeworks\6_MassSpring\report\mass_spring.assets\stiff=100_Acc(12fps).gif)

再将劲度系数设为10000时，网格表现的像块布料

![stiff=10000_Acc(12fps)](F:\CG\USTC_CG\Homeworks\6_MassSpring\report\mass_spring.assets\stiff=10000_Acc(12fps).gif)



## 4.边界条件和约束

通常模拟过程中物体会有各种约束或额外条件，例如物体被固定了几个点，对某些点施加外力（如重力、浮力、风力等）。

### 外力条件

- 物体受到的外力可以直接加在模拟的外力项中，其导数为 0
- 对于重力，可以将其加在外力中，另一方面，重力为保守力，也可以将重力势能加在能量项中与弹性势能进行合并

### 位移约束

这里主要考虑固定部分质点的情形，有两种方法处理：

- 第一种方法是在每一帧中求出该点的内力，再施加与该内力大小相同，方向相反的外力，但与上一种情形不同的是，若该内力对位移导数不为 0，则该外力对位移导数也不为 0，需要将其导数考虑进去；

- 第二种方法为仅考虑真正的自由坐标，降低问题的维数，具体如下：

将所有n个质点的坐标列为列向量 $x\in R^{3n}$，将所有 m 个自由质点坐标（无约束坐标）列为列向量 $x_f\in R^{3m}$,则两者关系：
$$
\boldsymbol x_f=\boldsymbol K\boldsymbol x,\\  \boldsymbol x=\boldsymbol K^T\boldsymbol x_f+\boldsymbol b,
$$
其中 $K\in R^{3m\times 3n}$ 为单位阵删去约束坐标序号对应行所得的稀疏矩阵，$b$ 为与约束位移有关的向量，计算为 $b=x-K^TKx$, 若约束为固定质点则 $b$ 为常量。由此我们将原本的关于 $x$ 的优化问题转化为对 $x_f$ 的优化问题：欧拉隐式方法中求解方程为：
$$
\boldsymbol g_1(\boldsymbol x_f) = K(\boldsymbol M(\boldsymbol x-\boldsymbol y) -h^2\boldsymbol f_{int}(\boldsymbol x)) = 0,\\
梯度：\nabla_{x_f} \boldsymbol g_1(\boldsymbol x_f) = K\nabla_{x} \boldsymbol g(\boldsymbol x)K^T,\\
$$
加速方法中优化问题中 $x$ 迭代步骤转化为求解关于 $x_f$ 的方程：
$$
K(\boldsymbol M+h^2\boldsymbol L)K^T\boldsymbol x_f=K(h^2\boldsymbol J \boldsymbol d+ \boldsymbol M \boldsymbol y-(\boldsymbol M+h^2\boldsymbol L)\boldsymbol b)
$$

### 参考文献

[^Liu13]: Tiantian Liu, et al. "Fast simulation of mass-spring systems." *Acm Transactions on Graphics (Pro. Siggraph Asia)* 32.6(2013):1-7.

