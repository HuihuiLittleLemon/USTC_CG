### 1. 高尔顿钉板

在数学课堂上，讲到正态分布，很多老师都会以这样一个著名的试验导入

<img src="F:\CG\USTC_CG\Homeworks\7_SimulationTaichi\project\report\simulation_taichi.assets\galton_knocked_boards.jpg" alt="galton_knocked_boards" style="zoom: 50%;" />

这就是高尔顿钉板。当下方的格子数分割得越来越细，这些小球的分布就近似于正态分布。从直觉上说，由于小球每下落一层向左右两边掉的概率都是 $\frac{1}{2}$ ，这些小球的分布应该更接近一个二项分布$B(n,\frac{1}{2})$ ,根据著名的棣莫弗-拉普拉斯定理，随着组距不断减小，二项分布的极限分布趋于正态分布，所以理论上我们会得到如下结果

![正态分布](F:\CG\USTC_CG\Homeworks\7_SimulationTaichi\project\report\simulation_taichi.assets\正态分布.jpg)

### 2.仿真高尔顿钉板实验

考虑taichi有完善的碰撞检测系统和图形绘制功能，只需要添加钉子及隔板的绘制，以及小球与钉子、隔板的碰撞检测代码。另外，由于，小球是由1000个质点组成，当系统中质点添加到一定数量时，运行帧率会越来越低，所以设计成小球落入隔板后，隔板“水位”上升一个单位。在合理设置小球体积，和重力加速度后，最终效果图如下

<img src="F:\CG\USTC_CG\Homeworks\7_SimulationTaichi\project\report\simulation_taichi.assets\galton_knocked_boards.gif" alt="galton_knocked_boards" style="zoom:50%;" />



### 3.遗留问题

系统在运行一段时间后gui不再刷新，可以确定程序任然在正常运行，一直找不到bug在哪。
