# AM Applications

在单独的分支开发每个app，成熟后合并到主分支。

大项目可以考虑使用submodule。

## App目录组织

在`apps/`里建立app名称的子目录，例如`hello`。

包含两个目录：

* `apps/hello/include`：头文件，将会被加入到`-I`选项中。
* `apps/hello/src`：源代码文件。所有源代码文件会被归档到`build/hello-arch.a`。

## 编译App

在最外层`make [play] ARCH=体系结构 APP=hello`可以编译hello。`play`参数会在编译完成后运行app。

如果上一次编译的体系结构不同，需要先执行`make clean`。