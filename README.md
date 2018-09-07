# Примеры использования кросс-платформенного драйвера

## Общие
Из [релизов github репозитория](https://github.com/shtrih-m/fr_drv_ng/releases) или [внутреннего gitlab](https://git.shtrih-m.ru/fr_drv_ng/fr_drv_ng/pipelines) необходимо взять последнюю версию библиотек/java/заголовочных файлов для вашей архитектуры/ОС.
### classic_interface
Это пример использования "классического" интерфейса драйвера "как в Драйвер ФР под Windows"  
Для сборки необходимо скопировать заголовочный файл **classic_interface.h** и библиотеки: **libcppbase_fr_drv_ng.so**, **libclassic_fr_drv_ng.so** (или dll для Windows)  
Сборка:
```bash
cmake .
cmake --build .
```
### classic_java
Это java обертка для классического интерфейса.
Сборка:
```bash
gradlew build
```
Для запуска необходимы java библиотека **classic_java_fr_drv_ng-$VERSION.jar** и нативные библиотеки: **libcppbase_fr_drv_ng.so**, **libclassic_fr_drv_ng.so** (или dll для Windows)  
Запуск:  
В java.library.path необходимо добавить нативные библиотеки, в classpath java.  
Пример:  

```bash
java -Djava.library.path="." -cp "classic_java_fr_drv_ng-1.1.jar:com.example-1.0.jar" example
```
### classic_fr_drv_ng_android_example
Пример использования **classic_java** под android.

### console_test
Содержит примеры скриптов, использующих консольный тест драйвера.


### javapos
Это java обретка для интерфейса unifiedpos (opos/javapos).
Для запуска необходимы java библиотеки javapos, **javapos_fr_drv_ng-1.0.jar** и нативные библиотеки: **libcppbase_fr_drv_ng.so**, **libunifiedpos_fr_drv_ng.so**  
Сборка:  
```bash
gradlew build
```
В java.library.path необходимо добавить нативные библиотеки, в classpath java.  
Пример:  

```bash
LD_LIBRARY_PATH="." java  -Djava.library.path='.' -cp "javapos_fr_drv_ng_java_example-1.0.jar:jpos114.jar:jpos114-controls.jar:javapos_fr_drv_ng-1.0.jar" example
```
