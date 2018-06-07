минимальный android пример работы с java оберткой классического интерфейса.
Для работы необходимо положить в `jniLibs` актуальные версии `libcppbase_fr_drv_ng.so` и `libclassic_fr_drv_ng.so` так же необходима библиотека `libgnustl_shared.so` из состава NDK  

NDK android переходит с gnu на llvm тулчейн. В будущем драйвер будет собираться с llvm и работать под libc++ STL.

**ВНИМАНИЕ** в данный момент(06.18) остается нерешенным вопрос с динамической линковкой под gnustl_shared в актуальном android NDK, поэтому `libclassic_fr_drv_ng.so` линкуется с `libcppbase_fr_drv_ng` статически. Соответственно в `jniLibs` достаточно добавить только `libclassic_fr_drv_ng.so` и `libgnustl_shared.so`


```java
//Необходимо вручную отсоединяться от classic 
ci.Disconnect();
//или форсить GC.
System.gc();
System.runFinalization();
```

Иначе до вызова GC будет висеть соединение, а ФР/КЯ работает только с 1 соединением

