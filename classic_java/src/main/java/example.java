import ru.shtrih_m.fr_drv_ng.classic_interface.Classic;
import ru.shtrih_m.fr_drv_ng.classic_interface.ClassicImpl;


public class example {
    public static void checkResult(int ret) {
        if (ret != 0) {
            throw new RuntimeException("error, bad return code: "+ ret);
        }
    }

    public static void main(String[] args) {
        System.out.println(System.getProperty("java.library.path"));
        //в java.library.path должна быть директория с dll драйвера.
        Classic ci = new ClassicImpl();
        try {
            ci.Set_Password(30);
            if (args.length > 1) {
                //можно передать URI в качестве аргумента
                ci.Set_ConnectionURI(args[1]);
            } else {
                ci.Set_ConnectionURI("tcp://192.168.137.111:7778?timeout=1000&protocol=v1");
            }
            checkResult(ci.Connect()); //соединяемся
            checkResult(ci.GetECRStatus()); //получаем статус
            switch (ci.Get_ECRMode()) {
                case 3:
                    checkResult(ci.PrintReportWithCleaning()); //снимаем Z отчет если смена больше 24 часов
                    break;
                case 4:
                    checkResult(ci.OpenSession()); //если смена закрыта - открываем
                    break;
                case 8:
                    checkResult(ci.CancelCheck()); // отменяем документ если открыт
                    break;
            }
            checkResult(ci.OpenCheck()); //открываем чек
            ci.Set_Password(30);
            ci.Set_Quantity(1.0);
            ci.Set_Department(1);
            ci.Set_Price(10000);
            ci.Set_Tax1(0);
            ci.Set_Tax2(0);
            ci.Set_Tax3(0);
            ci.Set_Tax4(0);
            ci.Set_StringForPrinting("Молоко");
            checkResult(ci.Sale());
            ci.Set_Summ1(10000);
            ci.Set_StringForPrinting("строчка");
            checkResult(ci.CloseCheck());
            System.exit(0);
        } catch (Exception e) {
            System.err.println("error:" + e.getLocalizedMessage());
        }
        System.exit(1);
    }
}
