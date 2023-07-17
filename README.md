# Test libvips on Windows

## 先决条件

安装：“[qt-opensource-windows-x86-mingw492-5.6.3.exe](https://download.qt.io/new_archive/qt/5.6/5.6.3/qt-opensource-windows-x86-mingw492-5.6.3.exe)”。

## 编译运行

1. 开始菜单中运行“Qt 5.6.3 for Desktop (MinGW 4.9.2 32 bit)”。
2. 切换到`t_libvips`目录中运行`build.bat`即可。

## 测试

### 如何通过`generate`方式获取图片整个像素数组并保存为文件

见“[t_generate.c](t_generate.c)”：

1. `travel_pixels_generate()`的参数`b`为申请的外部数组用于存放整个像素数组。
2. 这两种复制像素的方法都可以使用：

``` c++
for (x = 0; x < line_size; ++x) {
  q[x] = 255 - p[x];
  ((unsigned char *)b)[line_size * (r->top + y) + r->left + x] = p[x];
  // memcpy((unsigned char *)b + line_size * (r->top + y), p, line_size);
}
```

3. 注意一定要让`out`写入到文件，否则`out2`将生成黑色图片，即那一堆`printf()`输出没有执行，说明`out2`的`generate`操作也没有执行：

``` c++
if (vips_image_write_to_file(out, argv[2], NULL))
  vips_error_exit("unable to write");
```

4. 注意获取整个图片像素数组也可以通过如下方法获得，但是效率低：

``` c++
const void *data = vips_image_write_to_memory(pImage, &vips_size);
if (!data) {
  printf("vips_image_write_to_memory failed: %s\n", vips_error_buffer());
  return NULL;
}
```

试了下，`vips_image_write_to_memory`的效率不慢呀，感觉比`generate`快多了，怎么回事儿？

### 测试`travel_pixels`和`vips_image_write_to_memory`的效率

1. 以防`printf()`带来效率损失，去掉了`travel_pixels`循环中的`printf()`和`out`图片的输出。
2. 当`travel_pixels`运行结束后，`in`需要重新读图，否则`vips_image_write_to_memory`失败。

输出如下：

``` shellsession
> main.exe bg1a.jpg 1.jpg 2.jpg
time(travel_pixels): 26ms
time(vips_image_write_to_memory): 21ms

> main.exe bg1a.jpg 1.jpg 2.jpg
time(travel_pixels): 27ms
time(vips_image_write_to_memory): 21ms
```
