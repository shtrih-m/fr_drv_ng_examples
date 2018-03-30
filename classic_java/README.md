минимальный пример работы с java оберткой классического интерфейса.
для работы необходимо добавить путь до dll/so в java.library.path и classic_java_fr_drv_ng.jar в classpath

```java
//Необходимо вручную отсоединяться от classic 
ci.Disconnect();
//или форсить GC.
System.gc();
System.runFinalization();
```

Иначе до вызова GC будет висеть соединение, а ФР/КЯ работает только с 1 соединением

