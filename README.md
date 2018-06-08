---

title: Clips C++ interface
date: 2018-06-04 16:24:24
tags: [ Clips, C++ ]
categories: [ Local ]

---

下载依赖
========

官网下载: 
    [630](https://sourceforge.net/projects/clipsrules/files/CLIPS/6.30/)
    [640](https://netix.dl.sourceforge.net/project/clipsrules/CLIPS/6.40_Beta_2/clips_core_source_640.zip)

百度云盘: 
    [630](https://pan.baidu.com/s/1hzLVwX_clG50FjaJ0t7hdg)
    [640](https://pan.baidu.com/s/1calD7UUv94IVyq4mcuXPLA)


解压到当前路径下`clips_core_source_630, clips_core_source_640`

编译630
=======

build lib:
    1. `cd clips_core_source_630/core`
    2. `ln -s ../makefiles/makefile.lib makefile`
    3. `make`

build clips:
    1. `cd clips_core_source_630/core`
    2. `ln -s ../makefiles/makefile.gcc makefile`
    3. `make`

编译640
=======

1. `cd clips_core_source_640/core`
2. `make`


调试编译
========

1. 修改`Makefile`指定`CLIPS630_DIR`路径

2. make; make test; make clean

TODO
====

640和630版本变化太大, 本代码使用630编译
