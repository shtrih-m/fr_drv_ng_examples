Минимальный android пример работы с java оберткой классического интерфейса.

Для работы примера необходимо положить в `jniLibs` файлы библиотек `libcppbase_fr_drv_ng.so`, `libclassic_fr_drv_ng.so` и `libc++_shared.so`.  

```java
//Необходимо вручную отсоединяться от classic 
ci.Disconnect();
//или форсить GC.
System.gc();
System.runFinalization();
```

Иначе до вызова GC будет висеть соединение, а ФР/КЯ работает только с 1 соединением

