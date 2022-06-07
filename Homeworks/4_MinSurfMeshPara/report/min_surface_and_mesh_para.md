### 1. 极小曲面

如何求解以空间曲线为边界的极小曲面？

极小曲面的平均曲率处处为 0，即

$$
H(v_i)=0,\forall i
$$

又

![image-20220606202631357](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220606202631357.png)
$$
\lim_{len(y)\to 0}\frac{1}{len(\gamma)}\int_{v\in \gamma}(v_i-v)\mathrm{d}s=H(v_i)\pmb{n}_i
$$
则微分坐标

$$
\delta_i=v_i-\sum_{j\in N(i)}w_jv_j=0
$$
边界点固定（保持不变），所有顶点联立，得到整个网格的 Laplacian 方程 $AV=b$ ，对x，y，z坐标分量分别求解方程，得到极小曲面的顶点。

权重选取方法

- Uniform weight (geometry oblivious，无视几何): $w_j = 1$ ， $\overline{w}_j=w_j/\sum_k w_k$ 

- Cotangent weight (geometry aware，几何感知): $w_j =  (\cot \alpha + \cot\beta)$ ， $\overline{w}_j=w_j/\sum_k w_k$ 

  

### 2. 参数化[^ParametricFitting]

#### 2.1曲线参数化

求数据点所对应的参数：一个降维的问题

![](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220606200540152.png)

然后极小化误差度量:$E = \sum _ {i=1}^n(||\mathbf{p}(t_i) -\mathbf{p}_i||)^2$.

这个t该如何选取呢？有如下方法（称为点列的参数化）

##### 点列的参数化

+ Equidistant (uniform) parameterization

   $t_{i+1}-t_i = const$，如$t_i=i$，缺点是未考虑数据的几何特征

+ Chordal parameterizatio

   $t_{i+1}-t_i = ||\mathbf{k}_{i+1}-\mathbf{k}_i||$，即参数间隔与相邻控制点距离成比例，其中$\mathbf{k}_i=(x_i,y_i)^T$

+ Centripetal parameterizatio

   $t_{i+1}-t_i = \sqrt{||\mathbf{k}_{i+1}-\mathbf{k}_i||}$

+  Foley parameterization

   $t_{i+1}-t_i = ||\mathbf{k}_{i+1}-\mathbf{k}_i||*(1+\frac{3}{2}\frac{\hat{\alpha}_i||\mathbf{k}_i-\mathbf{k}_{i-1}||}{||||\mathbf{k}_i-\mathbf{k}_{i-1}||+\mathbf{k}_{i+1}-\mathbf{k}_i||}+\frac{3}{2}\frac{\hat{\alpha}_{i+1}||\mathbf{k}_{i+1}-\mathbf{k}_i||}{||||\mathbf{k}_{i+1}-\mathbf{k}_i||+\mathbf{k}_{i+2}-\mathbf{k}_{i+1}||})$，其中$\hat{\alpha}_i=min(\pi-\mathbf{angle(\mathbf{k}_{i-1},\mathbf{k}_i,\mathbf{k}_{i+1})},\frac{\pi}{2})$

#### 2.2曲面参数化

三维的点找二维的参数：也是一个降维的问题

![image-20220606201102196](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220606201102196.png)

考虑将 3D 网格曲面（非封闭、带边界的）展开到平面，即将 3D网格的边界固定到平面凸多边形（比如单位圆或单位正方形）上，再求解极小曲面，得到一张平面图形，就得到了该曲面的参数化。

### 2. 软件界面

![image-20220606204547510](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220606204547510.png)

### 3. 示例(以下图片均左:Uniform weight，右:Cotangent weight)

#### 3.1 极小化曲面

![image-20220606204829510](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220606204829510.png)

容易看出Cotangent weight能够保持3D网格曲面眼睛，鼻子，嘴处更多细节，产生的形变较小。

![image-20220606205332640](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220606205332640.png)

同上，可以看出Cotangent weight在顶点密集处产生的形变较小。

#### 3.2 参数化

将3D网格映射到单位圆上

![image-20220607133426485](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220607133426485.png)

![image-20220607133055884](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220607133055884.png)

贴上纹理图片

![image-20220607134356018](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220607134356018.png)

![image-20220607134719997](C:\Users\little lemon\AppData\Roaming\Typora\typora-user-images\image-20220607134719997.png)

这里再次说明Cotangent weight产生的扭曲较小。

### 参考文献

[^ParametricFitting]:http://staff.ustc.edu.cn/~lgliu/Courses/GAMES102_2020/PPT/GAMES102-3_ParametricFitting.pdf

