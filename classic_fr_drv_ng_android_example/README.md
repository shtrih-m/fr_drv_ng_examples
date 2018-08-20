Минимальный android пример работы с java оберткой классического интерфейса.
NDK android переходит с gnu на llvm тулчейн. В будущем драйвер будет собираться с llvm и работать под libc++ STL.
В данный момент сборки формируются для обоих тулчейнов старый(GNU/gcc) и текущий (LLVM/clang).

Для работы примера необходимо положить в `jniLibs` файлы библиотек `libcppbase_fr_drv_ng.so` и `libclassic_fr_drv_ng.so` из архива сборки и stl из состава NDK в зависимости тулчейна `libgnustl_shared.so` для GNU и `libc++_shared.so` для LLVM.  

```java
//Необходимо вручную отсоединяться от classic 
ci.Disconnect();
//или форсить GC.
System.gc();
System.runFinalization();
```

Иначе до вызова GC будет висеть соединение, а ФР/КЯ работает только с 1 соединением

