import ru.shtrih_m.fr_drv_ng.classic_interface.classic_interface;

import java.util.Date;

public class example {
    public static void checkResult(int ret) {
        if (ret != 0) {
            throw new RuntimeException("error, bad return code: " + ret);
        }
    }
// т.к. интерфейс единый, а java - это только обёртка вокруг c++ класса
// стоит так же смотреть пример c++. В основном модифицироваться будет он.
// https://git.shtrih-m.ru/fr_drv_ng/examples/blob/master/classic_interface/main.cpp
    public static void main(String[] args) {
        System.out.println(System.getProperty("java.library.path"));
        //в java.library.path должна быть директория с dll драйвера.
        classic_interface ci = new classic_interface();
        ci.Set_SCPassword(0); //Пароль ЦТО, нужен для установки нового пароля ЦТО + можно позже
        //воспользоваться для записи служебных таблиц(им необходим пароль ЦТО)
        ci.Set_SysAdminPassword(30); //Пароль сист. администратора
        ci.Set_Password(1); //Пароль кассира(может совпадать с паролем администратора)
        //        ci.Set_AutoEoD(true); //Включаем обмен с ОФД средствами драйвера
        try {
            ci.Set_AutoEoD(true);
            ci.Set_ConnectionURI("tcp://192.168.137.111:7778?timeout=1000&protocol=v1");
            checkResult(ci.Connect());
            checkResult(ci.GetECRStatus());
            ci.Set_TagValueDateTime(new Date());
            ci.Set_ReconnectPort(true);
            ci.Set_CheckType(1); // Операция - приход
            ci.Set_Price(4440); // Цена за единицу товара с учетом скидок
            ci.Set_Quantity(4); // Количество
            ci.Set_Summ1Enabled(true); // Указываем, что сами рассчитываем цену
            ci.Set_Summ1(17760); // Сумма позиции с учетом скидок
            ci.Set_TaxValueEnabled(false); // Налог мы не рассчитываем
            ci.Set_Tax1(1); // НДС 18%
            ci.Set_Department(1); // Номер отдела
            ci.Set_PaymentTypeSign(
                    4); // Признак способа расчета (Полный расчет) Необходим для ФФД 1.05
            ci.Set_PaymentItemSign(1); // Признак предмета расчета (Товар) Необходим для ФФД 1.05
            ci.Set_StringForPrinting("Товар"); // Наименование товара

            ci.FNOperation();
            // ci.Sale();

            ci.Set_Summ1(17760); // Наличные
            ci.Set_Summ2(0); // Остальные типы оплаты нулевые, но их необходимо заполнить
            ci.Set_Summ3(0);
            ci.Set_Summ4(0);
            ci.Set_Summ5(0);
            ci.Set_Summ6(0);
            ci.Set_Summ7(0);
            ci.Set_Summ8(0);
            ci.Set_Summ9(0);
            ci.Set_Summ10(0);
            ci.Set_Summ11(0);
            ci.Set_Summ12(0);
            ci.Set_Summ13(0);
            ci.Set_Summ14(0);
            ci.Set_Summ15(0);
            ci.Set_Summ16(0);
            ci.Set_RoundingSumm(0); // Сумма округления
            ci.Set_TaxValue1(0); // Налоги мы не считаем
            ci.Set_TaxValue2(0);
            ci.Set_TaxValue3(0);
            ci.Set_TaxValue4(0);
            ci.Set_TaxValue5(0);
            ci.Set_TaxValue6(0);
            ci.Set_TaxType(1); // Основная система налогообложения
            ci.Set_StringForPrinting("");
            ci.FNCloseCheckEx();
            ci.WaitForPrinting();
            ci.Disconnect();
        } catch (Exception e) {
            System.err.println("error:" + e.getLocalizedMessage());
        }
        System.exit(1);
    }
}
