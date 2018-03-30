минимальный android пример работы с java оберткой классического интерфейса.
Для работы необходимо положить в jniLibs актуальные версии libcppbase_fr_drv_ng.so и libclassic_fr_drv_ng.so так же необходима библиотека libgnustl_shared.so из состава NDK

```java
//Необходимо вручную отсоединяться от classic 
ci.Disconnect();
//или форсить GC.
System.gc();
System.runFinalization();
```

Иначе до вызова GC будет висеть соединение, а ФР/КЯ работает только с 1 соединением

